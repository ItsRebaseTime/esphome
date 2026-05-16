#include "companion_link.h"

#ifdef GAMEPAD_USE_COMPANION_UART

#include "esphome/core/log.h"
#include <cinttypes>
#include <cstring>

namespace esphome {
namespace gamepad {

static const char *const TAG = "gamepad.companion";

static constexpr uint32_t CAP_RETRY_MS = 2000;
static constexpr uint32_t STATE_RETRY_MS = 1000;

uint8_t GamepadCompanionLink::crc8_update(uint8_t crc, uint8_t byte) {
  crc ^= byte;
  for (int i = 0; i < 8; i++) {
    crc = (crc & 0x80) ? ((crc << 1) ^ 0x07) : (crc << 1);
  }
  return crc;
}

void GamepadCompanionLink::setup() {
  state_ = STATE_INIT;
  last_request_ms_ = 0;
}

void GamepadCompanionLink::process() {
  while (uart_->available() && rx_pos_ < RX_BUF_SIZE) {
    uint8_t b;
    if (!uart_->read_byte(&b))
      break;
    rx_buf_[rx_pos_++] = b;
  }

  while (try_parse_frame()) {
  }

  const uint32_t now = millis();
  if (state_ == STATE_INIT) {
    if (last_request_ms_ == 0 || now - last_request_ms_ >= CAP_RETRY_MS) {
      ESP_LOGD(TAG, "Sending CAP_REQUEST");
      send_cap_request();
      last_request_ms_ = now;
      state_ = STATE_WAIT_CAP;
    }
  } else if (state_ == STATE_WAIT_CAP) {
    if (now - last_request_ms_ >= CAP_RETRY_MS) {
      ESP_LOGW(TAG, "No CAP_RESPONSE, retrying");
      state_ = STATE_INIT;
      last_request_ms_ = 0;
    }
  } else if (state_ == STATE_WAIT_STATE) {
    if (now - last_request_ms_ >= STATE_RETRY_MS) {
      ESP_LOGW(TAG, "No STATE response, retrying");
      send_state_request();
      last_request_ms_ = now;
    }
  }
}

void GamepadCompanionLink::send_frame(uint8_t msg_type, const uint8_t *payload, uint16_t len) {
  const uint8_t len_l = len & 0xFF;
  const uint8_t len_h = (len >> 8) & 0xFF;

  uint8_t crc = 0;
  crc = crc8_update(crc, msg_type);
  crc = crc8_update(crc, len_l);
  crc = crc8_update(crc, len_h);
  for (uint16_t i = 0; i < len; i++) {
    crc = crc8_update(crc, payload[i]);
  }

  uart_->write_byte(cproto::SYNC_1);
  uart_->write_byte(cproto::SYNC_2);
  uart_->write_byte(msg_type);
  uart_->write_byte(len_l);
  uart_->write_byte(len_h);
  if (len > 0)
    uart_->write_array(payload, len);
  uart_->write_byte(crc);
}

void GamepadCompanionLink::send_cap_request() { send_frame(cproto::MSG_CAP_REQUEST, nullptr, 0); }

void GamepadCompanionLink::send_state_request() { send_frame(cproto::MSG_STATE_REQUEST, nullptr, 0); }

bool GamepadCompanionLink::try_parse_frame() {
  // Scan for sync pattern
  uint16_t start = 0;
  while (start + 1 < rx_pos_) {
    if (rx_buf_[start] == cproto::SYNC_1 && rx_buf_[start + 1] == cproto::SYNC_2)
      break;
    start++;
  }
  if (start > 0) {
    if (start >= rx_pos_) {
      rx_pos_ = 0;
      return false;
    }
    std::memmove(rx_buf_, rx_buf_ + start, rx_pos_ - start);
    rx_pos_ -= start;
  }

  // Need SYNC_1 SYNC_2 MSG_TYPE LEN_L LEN_H = 5 bytes minimum
  if (rx_pos_ < 5)
    return false;

  const uint8_t msg_type = rx_buf_[2];
  const uint16_t payload_len = static_cast<uint16_t>(rx_buf_[3]) | (static_cast<uint16_t>(rx_buf_[4]) << 8);

  const uint16_t total = 5 + payload_len + 1;  // header + payload + crc
  if (total > RX_BUF_SIZE) {
    // Frame too large — discard sync and resync
    std::memmove(rx_buf_, rx_buf_ + 2, rx_pos_ - 2);
    rx_pos_ -= 2;
    return true;
  }
  if (rx_pos_ < total)
    return false;

  // Verify CRC
  uint8_t crc = 0;
  for (uint16_t i = 2; i < 5 + payload_len; i++) {
    crc = crc8_update(crc, rx_buf_[i]);
  }
  if (crc != rx_buf_[5 + payload_len]) {
    ESP_LOGW(TAG, "CRC mismatch — resyncing");
    std::memmove(rx_buf_, rx_buf_ + 2, rx_pos_ - 2);
    rx_pos_ -= 2;
    return true;
  }

  dispatch_frame(msg_type, rx_buf_ + 5, payload_len);

  const uint16_t remaining = rx_pos_ - total;
  if (remaining > 0)
    std::memmove(rx_buf_, rx_buf_ + total, remaining);
  rx_pos_ = remaining;
  return true;
}

void GamepadCompanionLink::dispatch_frame(uint8_t msg_type, const uint8_t *payload, uint16_t len) {
  switch (msg_type) {
    case cproto::MSG_CAP_RESPONSE:
      handle_cap_response(payload, len);
      break;
    case cproto::MSG_INPUT_UPDATE:
      handle_input_update(payload, len);
      break;
    default:
      ESP_LOGW(TAG, "Unknown msg 0x%02X from companion", msg_type);
      break;
  }
}

void GamepadCompanionLink::handle_cap_response(const uint8_t *payload, uint16_t len) {
  if (len < 13) {
    ESP_LOGW(TAG, "CAP_RESPONSE too short (%u)", len);
    return;
  }
  supported_inputs_ = 0;
  for (int i = 0; i < 8; i++) {
    supported_inputs_ |= static_cast<uint64_t>(payload[i]) << (i * 8);
  }
  supported_outputs_ = 0;
  for (int i = 0; i < 5; i++) {
    supported_outputs_ |= static_cast<uint64_t>(payload[8 + i]) << (i * 8);
  }
  ESP_LOGD(TAG, "Companion inputs=0x%016llX outputs=0x%010llX", (unsigned long long) supported_inputs_,
           (unsigned long long) supported_outputs_);
  state_ = STATE_WAIT_STATE;
  last_request_ms_ = millis();
  send_state_request();
}

void GamepadCompanionLink::handle_input_update(const uint8_t *payload, uint16_t len) {
  // Payload: [binary_mask:4][binary_values:4][analog_mask:4][values...]
  if (len < 12) {
    ESP_LOGW(TAG, "INPUT_UPDATE too short (%u)", len);
    return;
  }

  const uint32_t binary_mask = static_cast<uint32_t>(payload[0]) | (static_cast<uint32_t>(payload[1]) << 8) |
                               (static_cast<uint32_t>(payload[2]) << 16) | (static_cast<uint32_t>(payload[3]) << 24);
  const uint32_t binary_vals = static_cast<uint32_t>(payload[4]) | (static_cast<uint32_t>(payload[5]) << 8) |
                               (static_cast<uint32_t>(payload[6]) << 16) | (static_cast<uint32_t>(payload[7]) << 24);
  const uint32_t analog_mask = static_cast<uint32_t>(payload[8]) | (static_cast<uint32_t>(payload[9]) << 8) |
                               (static_cast<uint32_t>(payload[10]) << 16) | (static_cast<uint32_t>(payload[11]) << 24);

  ESP_LOGD(TAG, "RX input update: binary=0x%08" PRIX32 " analog_mask=0x%08" PRIX32, binary_vals, analog_mask);
  binary_state_ = (binary_state_ & ~binary_mask) | (binary_vals & binary_mask);

  uint16_t pos = 12;
  for (uint8_t bit = 0; bit < cproto::ANALOG_COUNT; bit++) {
    if (!(analog_mask & (1U << bit)))
      continue;
    const uint8_t field_id = bit + cproto::ANALOG_OFFSET;
    if (field_id == cproto::IN_BATTERY) {
      if (pos < len)
        battery_level_ = payload[pos++];
    } else {
      if (pos + 4 <= len) {
        float val;
        std::memcpy(&val, payload + pos, 4);
        analog_state_[bit] = val;
        pos += 4;
      }
    }
  }

  if (state_ == STATE_WAIT_STATE) {
    state_ = STATE_READY;
    ESP_LOGI(TAG, "Companion ready");
  }
}

void GamepadCompanionLink::send_output(uint8_t rumble_weak, uint8_t rumble_strong, uint8_t lightbar_r,
                                       uint8_t lightbar_g, uint8_t lightbar_b, uint8_t mute_led, uint8_t player_leds,
                                       uint8_t lt_effect, uint8_t rt_effect, uint8_t hp_vol, uint8_t sp_vol,
                                       uint8_t mic_vol, uint8_t led_brightness, const uint8_t *left_zones,
                                       const uint8_t *right_zones) {
  // Build current values array indexed by output field ID
  uint8_t cur[cproto::OUTPUT_FIELD_COUNT] = {};
  cur[cproto::OUT_RUMBLE_WEAK_LEVEL] = rumble_weak;
  cur[cproto::OUT_RUMBLE_STRONG_LEVEL] = rumble_strong;
  cur[cproto::OUT_LIGHTBAR_R] = lightbar_r;
  cur[cproto::OUT_LIGHTBAR_G] = lightbar_g;
  cur[cproto::OUT_LIGHTBAR_B] = lightbar_b;
  cur[cproto::OUT_MUTE_LED] = mute_led;
  cur[cproto::OUT_PLAYER_LEDS] = player_leds;
  cur[cproto::OUT_LEFT_TRIGGER_EFFECT] = lt_effect;
  cur[cproto::OUT_RIGHT_TRIGGER_EFFECT] = rt_effect;
  cur[cproto::OUT_HEADPHONE_VOL] = hp_vol;
  cur[cproto::OUT_SPEAKER_VOL] = sp_vol;
  cur[cproto::OUT_MIC_VOL] = mic_vol;
  cur[cproto::OUT_LED_BRIGHTNESS] = led_brightness;
  for (int i = 0; i < 10; i++) {
    cur[cproto::OUT_LEFT_ZONE_BASE + i] = left_zones[i];
    cur[cproto::OUT_RIGHT_ZONE_BASE + i] = right_zones[i];
  }

  // Build bitmask of changed (or initial) fields that companion wants
  uint8_t fields_mask[cproto::OUTPUT_MASK_BYTES] = {};
  uint8_t values[cproto::OUTPUT_FIELD_COUNT];
  uint8_t value_count = 0;

  for (uint8_t i = 0; i < cproto::OUTPUT_FIELD_COUNT; i++) {
    // Skip fields the companion didn't request
    const bool companion_wants = (supported_outputs_ >> i) & 1ULL;
    if (!companion_wants)
      continue;

    const bool changed = !last_out_valid_ || cur[i] != last_out_[i];
    if (!changed)
      continue;

    fields_mask[i / 8] |= 1U << (i % 8);
    values[value_count++] = cur[i];
  }

  if (value_count == 0)
    return;

  // Payload: fields_mask[5] + values[N]
  uint8_t payload[cproto::OUTPUT_MASK_BYTES + cproto::OUTPUT_FIELD_COUNT];
  std::memcpy(payload, fields_mask, cproto::OUTPUT_MASK_BYTES);
  std::memcpy(payload + cproto::OUTPUT_MASK_BYTES, values, value_count);

  ESP_LOGD(TAG, "TX output update: %u field(s) changed", value_count);
  send_frame(cproto::MSG_OUTPUT_UPDATE, payload, cproto::OUTPUT_MASK_BYTES + value_count);

  std::memcpy(last_out_, cur, sizeof(cur));
  last_out_valid_ = true;
}

}  // namespace gamepad
}  // namespace esphome

#endif  // GAMEPAD_USE_COMPANION_UART
