#pragma once

#include "esphome/components/uart/uart.h"
#include <cstdint>

namespace esphome {
namespace gamepad {

// Protocol constants (mirrored in gamepad_companion/companion_protocol.h)
namespace cproto {

static constexpr uint8_t SYNC_1 = 0xAA;
static constexpr uint8_t SYNC_2 = 0x55;

// Gamepad (master) → companion
static constexpr uint8_t MSG_CAP_REQUEST = 0x01;
static constexpr uint8_t MSG_STATE_REQUEST = 0x02;
static constexpr uint8_t MSG_OUTPUT_UPDATE = 0x03;

// Companion → gamepad (master)
static constexpr uint8_t MSG_CAP_RESPONSE = 0x81;
static constexpr uint8_t MSG_INPUT_UPDATE = 0x82;

// Binary input field IDs (bits 0-30 of binary_mask)
static constexpr uint8_t IN_A_BUTTON = 0;
static constexpr uint8_t IN_B_BUTTON = 1;
static constexpr uint8_t IN_X_BUTTON = 2;
static constexpr uint8_t IN_Y_BUTTON = 3;
static constexpr uint8_t IN_LB_BUTTON = 4;
static constexpr uint8_t IN_RB_BUTTON = 5;
static constexpr uint8_t IN_LS_BUTTON = 6;
static constexpr uint8_t IN_RS_BUTTON = 7;
static constexpr uint8_t IN_L3_BUTTON = 8;
static constexpr uint8_t IN_R3_BUTTON = 9;
static constexpr uint8_t IN_SHARE_BUTTON = 10;
static constexpr uint8_t IN_MUTE_BUTTON = 11;
static constexpr uint8_t IN_L4_BUTTON = 12;
static constexpr uint8_t IN_R4_BUTTON = 13;
static constexpr uint8_t IN_L5_BUTTON = 14;
static constexpr uint8_t IN_R5_BUTTON = 15;
static constexpr uint8_t IN_MODE_BUTTON = 16;
static constexpr uint8_t IN_START_BUTTON = 17;
static constexpr uint8_t IN_SELECT_BUTTON = 18;
static constexpr uint8_t IN_DPAD_UP = 19;
static constexpr uint8_t IN_DPAD_RIGHT = 20;
static constexpr uint8_t IN_DPAD_DOWN = 21;
static constexpr uint8_t IN_DPAD_LEFT = 22;
static constexpr uint8_t IN_TOUCHPAD_BTN = 23;
static constexpr uint8_t IN_CHARGING = 24;
static constexpr uint8_t IN_HEADPHONES = 25;
static constexpr uint8_t IN_HEADPHONE_MIC = 26;
static constexpr uint8_t IN_USB_PLUGGED = 27;
static constexpr uint8_t IN_MUTE_ACTIVE = 28;
static constexpr uint8_t IN_LEFT_TOUCH = 29;
static constexpr uint8_t IN_RIGHT_TOUCH = 30;

// Analog input field IDs (analog_mask bit N = field ID N+32)
static constexpr uint8_t IN_BATTERY = 32;       // uint8 (0-100)
static constexpr uint8_t IN_LEFT_THUMB_X = 33;  // float (sensor units)
static constexpr uint8_t IN_LEFT_THUMB_Y = 34;
static constexpr uint8_t IN_RIGHT_THUMB_X = 35;
static constexpr uint8_t IN_RIGHT_THUMB_Y = 36;
static constexpr uint8_t IN_LEFT_TRIGGER = 37;
static constexpr uint8_t IN_RIGHT_TRIGGER = 38;
static constexpr uint8_t IN_YAW = 39;  // float (deg/s)
static constexpr uint8_t IN_PITCH = 40;
static constexpr uint8_t IN_ROLL = 41;
static constexpr uint8_t IN_AX = 42;  // float (m/s²)
static constexpr uint8_t IN_AY = 43;
static constexpr uint8_t IN_AZ = 44;
static constexpr uint8_t IN_LEFT_TOUCH_X = 45;
static constexpr uint8_t IN_LEFT_TOUCH_Y = 46;
static constexpr uint8_t IN_RIGHT_TOUCH_X = 47;
static constexpr uint8_t IN_RIGHT_TOUCH_Y = 48;

static constexpr uint8_t ANALOG_OFFSET = 32;
static constexpr uint8_t ANALOG_COUNT = 17;  // fields 32-48

// Output field IDs (bits in 5-byte / 40-bit output mask).
// All output fields are uint8; the gamepad sends only changed fields.
static constexpr uint8_t OUT_RUMBLE_WEAK_LEVEL = 0;
static constexpr uint8_t OUT_RUMBLE_STRONG_LEVEL = 1;
static constexpr uint8_t OUT_LIGHTBAR_R = 2;
static constexpr uint8_t OUT_LIGHTBAR_G = 3;
static constexpr uint8_t OUT_LIGHTBAR_B = 4;
static constexpr uint8_t OUT_MUTE_LED = 5;  // 0=off 1=solid 2=pulse
static constexpr uint8_t OUT_PLAYER_LEDS = 6;
static constexpr uint8_t OUT_LEFT_TRIGGER_EFFECT = 7;  // 0=off 1=feedback 2=weapon 3=vibration
static constexpr uint8_t OUT_RIGHT_TRIGGER_EFFECT = 8;
static constexpr uint8_t OUT_HEADPHONE_VOL = 9;
static constexpr uint8_t OUT_SPEAKER_VOL = 10;
static constexpr uint8_t OUT_MIC_VOL = 11;
static constexpr uint8_t OUT_LED_BRIGHTNESS = 12;
static constexpr uint8_t OUT_LEFT_ZONE_BASE = 13;   // +0-9 → IDs 13-22
static constexpr uint8_t OUT_RIGHT_ZONE_BASE = 23;  // +0-9 → IDs 23-32
static constexpr uint8_t OUTPUT_FIELD_COUNT = 33;
static constexpr uint8_t OUTPUT_MASK_BYTES = 5;  // ceil(33/8) = 5 bytes

}  // namespace cproto

// Manages UART communication with a gamepad_companion device.
// Call setup() once, then process() every loop iteration.
class GamepadCompanionLink {
 public:
  explicit GamepadCompanionLink(uart::UARTComponent *uart) : uart_(uart) {}

  void setup();
  void process();

  bool is_ready() const { return state_ == STATE_READY; }

  // Returns true if the companion advertised support for this input field.
  bool has_input(uint8_t field_id) const {
    if (field_id >= 64)
      return false;
    return (supported_inputs_ >> field_id) & 1ULL;
  }

  // Binary input value (field_id 0-30).
  bool get_binary(uint8_t field_id) const {
    if (field_id >= 32)
      return false;
    return (binary_state_ >> field_id) & 1U;
  }

  // Analog float input value (field_id 33-48). Field 32 (battery) uses get_battery().
  float get_float(uint8_t field_id) const {
    if (field_id < cproto::ANALOG_OFFSET + 1 || field_id >= cproto::ANALOG_OFFSET + cproto::ANALOG_COUNT)
      return 0.0f;
    return analog_state_[field_id - cproto::ANALOG_OFFSET];
  }

  uint8_t get_battery() const { return battery_level_; }

  // Send output report data to the companion (called from gamepad feedback handler).
  void send_output(uint8_t rumble_weak, uint8_t rumble_strong, uint8_t lightbar_r, uint8_t lightbar_g,
                   uint8_t lightbar_b, uint8_t mute_led, uint8_t player_leds, uint8_t lt_effect, uint8_t rt_effect,
                   uint8_t hp_vol, uint8_t sp_vol, uint8_t mic_vol, uint8_t led_brightness, const uint8_t *left_zones,
                   const uint8_t *right_zones);

 protected:
  uart::UARTComponent *uart_;

  enum State { STATE_INIT, STATE_WAIT_CAP, STATE_WAIT_STATE, STATE_READY } state_{STATE_INIT};

  uint64_t supported_inputs_{0};
  uint64_t supported_outputs_{0};

  uint32_t binary_state_{0};
  float analog_state_[cproto::ANALOG_COUNT]{};
  uint8_t battery_level_{100};

  // Last-sent output values (for change detection)
  uint8_t last_out_[cproto::OUTPUT_FIELD_COUNT]{};
  bool last_out_valid_{false};

  static constexpr uint16_t RX_BUF_SIZE = 256;
  uint8_t rx_buf_[RX_BUF_SIZE]{};
  uint16_t rx_pos_{0};
  uint32_t last_request_ms_{0};

  void send_frame(uint8_t msg_type, const uint8_t *payload, uint16_t len);
  void send_cap_request();
  void send_state_request();
  bool try_parse_frame();
  void dispatch_frame(uint8_t msg_type, const uint8_t *payload, uint16_t len);
  void handle_cap_response(const uint8_t *payload, uint16_t len);
  void handle_input_update(const uint8_t *payload, uint16_t len);

  static uint8_t crc8_update(uint8_t crc, uint8_t byte);
};

}  // namespace gamepad
}  // namespace esphome
