#include "gamepad_companion.h"

#include "esphome/core/log.h"
#include <cinttypes>
#include <cstring>

namespace esphome {
namespace gamepad_companion {

static const char *const TAG = "gamepad_companion";

uint8_t GamepadCompanion::crc8_update(uint8_t crc, uint8_t byte) {
  crc ^= byte;
  for (int i = 0; i < 8; i++) {
    crc = (crc & 0x80) ? ((crc << 1) ^ 0x07) : (crc << 1);
  }
  return crc;
}

void GamepadCompanion::setup() {
  this->supported_inputs_ = 0;
  this->supported_outputs_ = 0;
  this->m_last_sent_valid_ = false;
  this->m_master_ready_ = false;
  this->m_lightbar_r_ = 0;
  this->m_lightbar_g_ = 0;
  this->m_lightbar_b_ = 0;
  this->m_mute_led_ = 0;
  this->configure_supported_fields_();
}

void GamepadCompanion::loop() {
  while (this->available() && this->m_rx_length_ < cproto::MAX_FRAME_SIZE) {
    uint8_t b;
    if (!this->read_byte(&b)) {
      break;
    }
    this->m_rx_buffer_[this->m_rx_length_++] = b;
  }

  while (this->try_parse_frame()) {
  }

  uint32_t prev_binary = this->m_last_binary_state_;
  float prev_analog[cproto::ANALOG_COUNT];
  std::memcpy(prev_analog, this->m_last_analog_state_, sizeof(prev_analog));
  uint8_t prev_battery = this->m_last_battery_level_;

  this->update_input_state_();

  bool changed = this->m_binary_state_ != prev_binary || this->m_battery_level_ != prev_battery;
  if (!changed) {
    for (uint8_t i = 0; i < cproto::ANALOG_COUNT; i++) {
      if (this->m_analog_state_[i] != prev_analog[i]) {
        changed = true;
        break;
      }
    }
  }

  if (this->m_master_ready_ && changed) {
    this->send_input_update_(false);
  }
}

void GamepadCompanion::dump_config() {}

void GamepadCompanion::set_left_trigger_zone_sensor(uint8_t index, sensor::Sensor *sensor) {
  if (index < 10) {
    this->m_left_trigger_zone_sensors_[index] = sensor;
  }
}

void GamepadCompanion::set_right_trigger_zone_sensor(uint8_t index, sensor::Sensor *sensor) {
  if (index < 10) {
    this->m_right_trigger_zone_sensors_[index] = sensor;
  }
}

bool GamepadCompanion::has_input(uint8_t field) const {
  if (field >= 64) {
    return false;
  }
  return (this->supported_inputs_ >> field) & 1ULL;
}

bool GamepadCompanion::has_output(uint8_t field) const {
  if (field >= cproto::OUTPUT_FIELD_COUNT) {
    return false;
  }
  return (this->supported_outputs_ >> field) & 1ULL;
}

void GamepadCompanion::configure_supported_fields_() {
  this->supported_inputs_ = 0;
  if (this->m_a_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_A_BUTTON;
  }
  if (this->m_b_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_B_BUTTON;
  }
  if (this->m_x_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_X_BUTTON;
  }
  if (this->m_y_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_Y_BUTTON;
  }
  if (this->m_lb_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_LB_BUTTON;
  }
  if (this->m_rb_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_RB_BUTTON;
  }
  if (this->m_ls_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_LS_BUTTON;
  }
  if (this->m_rs_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_RS_BUTTON;
  }
  if (this->m_l3_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_L3_BUTTON;
  }
  if (this->m_r3_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_R3_BUTTON;
  }
  if (this->m_share_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_SHARE_BUTTON;
  }
  if (this->m_mute_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_MUTE_BUTTON;
  }
  if (this->m_l4_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_L4_BUTTON;
  }
  if (this->m_r4_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_R4_BUTTON;
  }
  if (this->m_l5_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_L5_BUTTON;
  }
  if (this->m_r5_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_R5_BUTTON;
  }
  if (this->m_mode_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_MODE_BUTTON;
  }
  if (this->m_start_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_START_BUTTON;
  }
  if (this->m_select_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_SELECT_BUTTON;
  }
  if (this->m_dpad_up_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_DPAD_UP;
  }
  if (this->m_dpad_right_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_DPAD_RIGHT;
  }
  if (this->m_dpad_down_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_DPAD_DOWN;
  }
  if (this->m_dpad_left_button) {
    this->supported_inputs_ |= 1ULL << cproto::IN_DPAD_LEFT;
  }
  if (this->m_left_touch_sensor
#ifdef GAMEPAD_USE_TOUCHSCREEN
      || this->m_left_touchscreen_
#endif
  ) {
    this->supported_inputs_ |= 1ULL << cproto::IN_LEFT_TOUCH;
  }
  if (this->m_right_touch_sensor
#ifdef GAMEPAD_USE_TOUCHSCREEN
      || this->m_right_touchscreen_
#endif
  ) {
    this->supported_inputs_ |= 1ULL << cproto::IN_RIGHT_TOUCH;
  }
  if (this->m_charging_status_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_CHARGING;
  }
  if (this->m_headphones_plugged_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_HEADPHONES;
  }
  if (this->m_headphone_mic_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_HEADPHONE_MIC;
  }
  if (this->m_usb_plugged_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_USB_PLUGGED;
  }
  if (this->m_mute_active_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_MUTE_ACTIVE;
  }

  if (this->m_battery_level_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_BATTERY;
  }
  if (this->m_left_thumb_x_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_LEFT_THUMB_X;
  }
  if (this->m_left_thumb_y_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_LEFT_THUMB_Y;
  }
  if (this->m_right_thumb_x_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_RIGHT_THUMB_X;
  }
  if (this->m_right_thumb_y_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_RIGHT_THUMB_Y;
  }
  if (this->m_left_trigger_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_LEFT_TRIGGER;
  }
  if (this->m_right_trigger_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_RIGHT_TRIGGER;
  }
  if (this->m_yaw_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_YAW;
  }
  if (this->m_pitch_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_PITCH;
  }
  if (this->m_roll_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_ROLL;
  }
  if (this->m_ax_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_AX;
  }
  if (this->m_ay_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_AY;
  }
  if (this->m_az_sensor) {
    this->supported_inputs_ |= 1ULL << cproto::IN_AZ;
  }

  this->supported_outputs_ = 0;
  if (this->m_lightbar_light || this->m_lightbar_red_sensor || this->m_lightbar_green_sensor ||
      this->m_lightbar_blue_sensor) {
    this->supported_outputs_ |= 1ULL << cproto::OUT_LIGHTBAR_R;
    this->supported_outputs_ |= 1ULL << cproto::OUT_LIGHTBAR_G;
    this->supported_outputs_ |= 1ULL << cproto::OUT_LIGHTBAR_B;
  }
  if (this->m_mute_light || this->m_mute_led_sensor) {
    this->supported_outputs_ |= 1ULL << cproto::OUT_MUTE_LED;
  }
  if (this->m_player_number_sensor) {
    this->supported_outputs_ |= 1ULL << cproto::OUT_PLAYER_LEDS;
  }
  if (this->m_left_trigger_effect_sensor || this->m_left_trigger_effect_active_sensor) {
    this->supported_outputs_ |= 1ULL << cproto::OUT_LEFT_TRIGGER_EFFECT;
  }
  if (this->m_right_trigger_effect_sensor || this->m_right_trigger_effect_active_sensor) {
    this->supported_outputs_ |= 1ULL << cproto::OUT_RIGHT_TRIGGER_EFFECT;
  }
  if (this->m_headphone_volume_sensor) {
    this->supported_outputs_ |= 1ULL << cproto::OUT_HEADPHONE_VOL;
  }
  if (this->m_speaker_volume_sensor) {
    this->supported_outputs_ |= 1ULL << cproto::OUT_SPEAKER_VOL;
  }
  if (this->m_mic_volume_sensor) {
    this->supported_outputs_ |= 1ULL << cproto::OUT_MIC_VOL;
  }
  if (this->m_led_brightness_sensor) {
    this->supported_outputs_ |= 1ULL << cproto::OUT_LED_BRIGHTNESS;
  }
  if (this->m_rumble_weak_level_sensor || this->m_rumble_weak_sensor) {
    this->supported_outputs_ |= 1ULL << cproto::OUT_RUMBLE_WEAK_LEVEL;
  }
  if (this->m_rumble_strong_level_sensor || this->m_rumble_strong_sensor) {
    this->supported_outputs_ |= 1ULL << cproto::OUT_RUMBLE_STRONG_LEVEL;
  }
  for (uint8_t i = 0; i < 10; i++) {
    if (this->m_left_trigger_zone_sensors_[i]) {
      this->supported_outputs_ |= 1ULL << (cproto::OUT_LEFT_ZONE_BASE + i);
    }
    if (this->m_right_trigger_zone_sensors_[i]) {
      this->supported_outputs_ |= 1ULL << (cproto::OUT_RIGHT_ZONE_BASE + i);
    }
  }
}

void GamepadCompanion::update_input_state_() {
  this->m_binary_state_ = 0;
  if (this->m_a_button && this->m_a_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_A_BUTTON;
  }
  if (this->m_b_button && this->m_b_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_B_BUTTON;
  }
  if (this->m_x_button && this->m_x_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_X_BUTTON;
  }
  if (this->m_y_button && this->m_y_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_Y_BUTTON;
  }
  if (this->m_lb_button && this->m_lb_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_LB_BUTTON;
  }
  if (this->m_rb_button && this->m_rb_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_RB_BUTTON;
  }
  if (this->m_ls_button && this->m_ls_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_LS_BUTTON;
  }
  if (this->m_rs_button && this->m_rs_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_RS_BUTTON;
  }
  if (this->m_l3_button && this->m_l3_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_L3_BUTTON;
  }
  if (this->m_r3_button && this->m_r3_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_R3_BUTTON;
  }
  if (this->m_share_button && this->m_share_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_SHARE_BUTTON;
  }
  if (this->m_mute_button && this->m_mute_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_MUTE_BUTTON;
  }
  if (this->m_l4_button && this->m_l4_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_L4_BUTTON;
  }
  if (this->m_r4_button && this->m_r4_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_R4_BUTTON;
  }
  if (this->m_l5_button && this->m_l5_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_L5_BUTTON;
  }
  if (this->m_r5_button && this->m_r5_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_R5_BUTTON;
  }
  if (this->m_mode_button && this->m_mode_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_MODE_BUTTON;
  }
  if (this->m_start_button && this->m_start_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_START_BUTTON;
  }
  if (this->m_select_button && this->m_select_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_SELECT_BUTTON;
  }
  if (this->m_dpad_up_button && this->m_dpad_up_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_DPAD_UP;
  }
  if (this->m_dpad_right_button && this->m_dpad_right_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_DPAD_RIGHT;
  }
  if (this->m_dpad_down_button && this->m_dpad_down_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_DPAD_DOWN;
  }
  if (this->m_dpad_left_button && this->m_dpad_left_button->state) {
    this->m_binary_state_ |= 1U << cproto::IN_DPAD_LEFT;
  }
  if ((this->m_left_touch_sensor && this->m_left_touch_sensor->state)
#ifdef GAMEPAD_USE_TOUCHSCREEN
      || (this->m_left_touchscreen_ && this->m_left_touchscreen_->get_touch().has_value())
#endif
  ) {
    this->m_binary_state_ |= 1U << cproto::IN_LEFT_TOUCH;
  }
  if ((this->m_right_touch_sensor && this->m_right_touch_sensor->state)
#ifdef GAMEPAD_USE_TOUCHSCREEN
      || (this->m_right_touchscreen_ && this->m_right_touchscreen_->get_touch().has_value())
#endif
  ) {
    this->m_binary_state_ |= 1U << cproto::IN_RIGHT_TOUCH;
  }
  if (this->m_charging_status_sensor && this->m_charging_status_sensor->state) {
    this->m_binary_state_ |= 1U << cproto::IN_CHARGING;
  }
  if (this->m_headphones_plugged_sensor && this->m_headphones_plugged_sensor->state) {
    this->m_binary_state_ |= 1U << cproto::IN_HEADPHONES;
  }
  if (this->m_headphone_mic_sensor && this->m_headphone_mic_sensor->state) {
    this->m_binary_state_ |= 1U << cproto::IN_HEADPHONE_MIC;
  }
  if (this->m_usb_plugged_sensor && this->m_usb_plugged_sensor->state) {
    this->m_binary_state_ |= 1U << cproto::IN_USB_PLUGGED;
  }
  if (this->m_mute_active_sensor && this->m_mute_active_sensor->state) {
    this->m_binary_state_ |= 1U << cproto::IN_MUTE_ACTIVE;
  }

  this->m_battery_level_ = this->get_battery_level_();
  if (this->m_left_thumb_x_sensor) {
    this->m_analog_state_[cproto::IN_LEFT_THUMB_X - cproto::ANALOG_OFFSET] = this->m_left_thumb_x_sensor->state;
  }
  if (this->m_left_thumb_y_sensor) {
    this->m_analog_state_[cproto::IN_LEFT_THUMB_Y - cproto::ANALOG_OFFSET] = this->m_left_thumb_y_sensor->state;
  }
  if (this->m_right_thumb_x_sensor) {
    this->m_analog_state_[cproto::IN_RIGHT_THUMB_X - cproto::ANALOG_OFFSET] = this->m_right_thumb_x_sensor->state;
  }
  if (this->m_right_thumb_y_sensor) {
    this->m_analog_state_[cproto::IN_RIGHT_THUMB_Y - cproto::ANALOG_OFFSET] = this->m_right_thumb_y_sensor->state;
  }
  if (this->m_left_trigger_sensor) {
    this->m_analog_state_[cproto::IN_LEFT_TRIGGER - cproto::ANALOG_OFFSET] = this->m_left_trigger_sensor->state;
  }
  if (this->m_right_trigger_sensor) {
    this->m_analog_state_[cproto::IN_RIGHT_TRIGGER - cproto::ANALOG_OFFSET] = this->m_right_trigger_sensor->state;
  }
  if (this->m_yaw_sensor) {
    this->m_analog_state_[cproto::IN_YAW - cproto::ANALOG_OFFSET] = this->m_yaw_sensor->state;
  }
  if (this->m_pitch_sensor) {
    this->m_analog_state_[cproto::IN_PITCH - cproto::ANALOG_OFFSET] = this->m_pitch_sensor->state;
  }
  if (this->m_roll_sensor) {
    this->m_analog_state_[cproto::IN_ROLL - cproto::ANALOG_OFFSET] = this->m_roll_sensor->state;
  }
  if (this->m_ax_sensor) {
    this->m_analog_state_[cproto::IN_AX - cproto::ANALOG_OFFSET] = this->m_ax_sensor->state;
  }
  if (this->m_ay_sensor) {
    this->m_analog_state_[cproto::IN_AY - cproto::ANALOG_OFFSET] = this->m_ay_sensor->state;
  }
  if (this->m_az_sensor) {
    this->m_analog_state_[cproto::IN_AZ - cproto::ANALOG_OFFSET] = this->m_az_sensor->state;
  }
#ifdef GAMEPAD_USE_TOUCHSCREEN
  if (this->m_left_touchscreen_) {
    auto tp = this->m_left_touchscreen_->get_touch();
    if (tp.has_value()) {
      this->m_analog_state_[cproto::IN_LEFT_TOUCH_X - cproto::ANALOG_OFFSET] = static_cast<float>(tp->x);
      this->m_analog_state_[cproto::IN_LEFT_TOUCH_Y - cproto::ANALOG_OFFSET] = static_cast<float>(tp->y);
    }
  } else
#endif
  {
    if (this->m_left_touch_x_sensor) {
      this->m_analog_state_[cproto::IN_LEFT_TOUCH_X - cproto::ANALOG_OFFSET] = this->m_left_touch_x_sensor->state;
    }
    if (this->m_left_touch_y_sensor) {
      this->m_analog_state_[cproto::IN_LEFT_TOUCH_Y - cproto::ANALOG_OFFSET] = this->m_left_touch_y_sensor->state;
    }
  }
#ifdef GAMEPAD_USE_TOUCHSCREEN
  if (this->m_right_touchscreen_) {
    auto tp = this->m_right_touchscreen_->get_touch();
    if (tp.has_value()) {
      this->m_analog_state_[cproto::IN_RIGHT_TOUCH_X - cproto::ANALOG_OFFSET] = static_cast<float>(tp->x);
      this->m_analog_state_[cproto::IN_RIGHT_TOUCH_Y - cproto::ANALOG_OFFSET] = static_cast<float>(tp->y);
    }
  } else
#endif
  {
    if (this->m_right_touch_x_sensor) {
      this->m_analog_state_[cproto::IN_RIGHT_TOUCH_X - cproto::ANALOG_OFFSET] = this->m_right_touch_x_sensor->state;
    }
    if (this->m_right_touch_y_sensor) {
      this->m_analog_state_[cproto::IN_RIGHT_TOUCH_Y - cproto::ANALOG_OFFSET] = this->m_right_touch_y_sensor->state;
    }
  }
}

uint8_t GamepadCompanion::get_battery_level_() const {
  if (!this->m_battery_level_sensor) {
    return 0;
  }
  float value = this->m_battery_level_sensor->state;
  if (value < 0.0f) {
    return 0;
  }
  if (value > 100.0f) {
    return 100;
  }
  return static_cast<uint8_t>(value + 0.5f);
}

void GamepadCompanion::send_cap_response_() {
  uint8_t payload[8 + cproto::OUTPUT_MASK_BYTES]{};
  for (int i = 0; i < 8; i++) {
    payload[i] = static_cast<uint8_t>((this->supported_inputs_ >> (i * 8)) & 0xFF);
  }
  for (int i = 0; i < cproto::OUTPUT_MASK_BYTES; i++) {
    payload[8 + i] = static_cast<uint8_t>((this->supported_outputs_ >> (i * 8)) & 0xFF);
  }
  this->send_frame(cproto::MSG_CAP_RESPONSE, payload, sizeof(payload));
}

void GamepadCompanion::send_input_update_(bool all) {
  uint32_t binary_mask = 0;
  uint32_t binary_values = 0;
  uint32_t analog_mask = 0;
  uint8_t values[cproto::ANALOG_COUNT * 4 + 1];
  uint8_t value_count = 0;

  if (this->has_input(cproto::IN_A_BUTTON)) {
    binary_mask |= 1U << cproto::IN_A_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_A_BUTTON)) {
      binary_values |= 1U << cproto::IN_A_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_B_BUTTON)) {
    binary_mask |= 1U << cproto::IN_B_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_B_BUTTON)) {
      binary_values |= 1U << cproto::IN_B_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_X_BUTTON)) {
    binary_mask |= 1U << cproto::IN_X_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_X_BUTTON)) {
      binary_values |= 1U << cproto::IN_X_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_Y_BUTTON)) {
    binary_mask |= 1U << cproto::IN_Y_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_Y_BUTTON)) {
      binary_values |= 1U << cproto::IN_Y_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_LB_BUTTON)) {
    binary_mask |= 1U << cproto::IN_LB_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_LB_BUTTON)) {
      binary_values |= 1U << cproto::IN_LB_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_RB_BUTTON)) {
    binary_mask |= 1U << cproto::IN_RB_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_RB_BUTTON)) {
      binary_values |= 1U << cproto::IN_RB_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_LS_BUTTON)) {
    binary_mask |= 1U << cproto::IN_LS_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_LS_BUTTON)) {
      binary_values |= 1U << cproto::IN_LS_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_RS_BUTTON)) {
    binary_mask |= 1U << cproto::IN_RS_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_RS_BUTTON)) {
      binary_values |= 1U << cproto::IN_RS_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_L3_BUTTON)) {
    binary_mask |= 1U << cproto::IN_L3_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_L3_BUTTON)) {
      binary_values |= 1U << cproto::IN_L3_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_R3_BUTTON)) {
    binary_mask |= 1U << cproto::IN_R3_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_R3_BUTTON)) {
      binary_values |= 1U << cproto::IN_R3_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_SHARE_BUTTON)) {
    binary_mask |= 1U << cproto::IN_SHARE_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_SHARE_BUTTON)) {
      binary_values |= 1U << cproto::IN_SHARE_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_MUTE_BUTTON)) {
    binary_mask |= 1U << cproto::IN_MUTE_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_MUTE_BUTTON)) {
      binary_values |= 1U << cproto::IN_MUTE_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_L4_BUTTON)) {
    binary_mask |= 1U << cproto::IN_L4_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_L4_BUTTON)) {
      binary_values |= 1U << cproto::IN_L4_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_R4_BUTTON)) {
    binary_mask |= 1U << cproto::IN_R4_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_R4_BUTTON)) {
      binary_values |= 1U << cproto::IN_R4_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_L5_BUTTON)) {
    binary_mask |= 1U << cproto::IN_L5_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_L5_BUTTON)) {
      binary_values |= 1U << cproto::IN_L5_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_R5_BUTTON)) {
    binary_mask |= 1U << cproto::IN_R5_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_R5_BUTTON)) {
      binary_values |= 1U << cproto::IN_R5_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_MODE_BUTTON)) {
    binary_mask |= 1U << cproto::IN_MODE_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_MODE_BUTTON)) {
      binary_values |= 1U << cproto::IN_MODE_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_START_BUTTON)) {
    binary_mask |= 1U << cproto::IN_START_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_START_BUTTON)) {
      binary_values |= 1U << cproto::IN_START_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_SELECT_BUTTON)) {
    binary_mask |= 1U << cproto::IN_SELECT_BUTTON;
    if (this->m_binary_state_ & (1U << cproto::IN_SELECT_BUTTON)) {
      binary_values |= 1U << cproto::IN_SELECT_BUTTON;
    }
  }
  if (this->has_input(cproto::IN_DPAD_UP)) {
    binary_mask |= 1U << cproto::IN_DPAD_UP;
    if (this->m_binary_state_ & (1U << cproto::IN_DPAD_UP)) {
      binary_values |= 1U << cproto::IN_DPAD_UP;
    }
  }
  if (this->has_input(cproto::IN_DPAD_RIGHT)) {
    binary_mask |= 1U << cproto::IN_DPAD_RIGHT;
    if (this->m_binary_state_ & (1U << cproto::IN_DPAD_RIGHT)) {
      binary_values |= 1U << cproto::IN_DPAD_RIGHT;
    }
  }
  if (this->has_input(cproto::IN_DPAD_DOWN)) {
    binary_mask |= 1U << cproto::IN_DPAD_DOWN;
    if (this->m_binary_state_ & (1U << cproto::IN_DPAD_DOWN)) {
      binary_values |= 1U << cproto::IN_DPAD_DOWN;
    }
  }
  if (this->has_input(cproto::IN_DPAD_LEFT)) {
    binary_mask |= 1U << cproto::IN_DPAD_LEFT;
    if (this->m_binary_state_ & (1U << cproto::IN_DPAD_LEFT)) {
      binary_values |= 1U << cproto::IN_DPAD_LEFT;
    }
  }
  if (this->has_input(cproto::IN_LEFT_TOUCH)) {
    binary_mask |= 1U << cproto::IN_LEFT_TOUCH;
    if (this->m_binary_state_ & (1U << cproto::IN_LEFT_TOUCH)) {
      binary_values |= 1U << cproto::IN_LEFT_TOUCH;
    }
  }
  if (this->has_input(cproto::IN_RIGHT_TOUCH)) {
    binary_mask |= 1U << cproto::IN_RIGHT_TOUCH;
    if (this->m_binary_state_ & (1U << cproto::IN_RIGHT_TOUCH)) {
      binary_values |= 1U << cproto::IN_RIGHT_TOUCH;
    }
  }
  if (this->has_input(cproto::IN_CHARGING)) {
    binary_mask |= 1U << cproto::IN_CHARGING;
    if (this->m_binary_state_ & (1U << cproto::IN_CHARGING)) {
      binary_values |= 1U << cproto::IN_CHARGING;
    }
  }
  if (this->has_input(cproto::IN_HEADPHONES)) {
    binary_mask |= 1U << cproto::IN_HEADPHONES;
    if (this->m_binary_state_ & (1U << cproto::IN_HEADPHONES)) {
      binary_values |= 1U << cproto::IN_HEADPHONES;
    }
  }
  if (this->has_input(cproto::IN_HEADPHONE_MIC)) {
    binary_mask |= 1U << cproto::IN_HEADPHONE_MIC;
    if (this->m_binary_state_ & (1U << cproto::IN_HEADPHONE_MIC)) {
      binary_values |= 1U << cproto::IN_HEADPHONE_MIC;
    }
  }
  if (this->has_input(cproto::IN_USB_PLUGGED)) {
    binary_mask |= 1U << cproto::IN_USB_PLUGGED;
    if (this->m_binary_state_ & (1U << cproto::IN_USB_PLUGGED)) {
      binary_values |= 1U << cproto::IN_USB_PLUGGED;
    }
  }
  if (this->has_input(cproto::IN_MUTE_ACTIVE)) {
    binary_mask |= 1U << cproto::IN_MUTE_ACTIVE;
    if (this->m_binary_state_ & (1U << cproto::IN_MUTE_ACTIVE)) {
      binary_values |= 1U << cproto::IN_MUTE_ACTIVE;
    }
  }

  if (this->has_input(cproto::IN_BATTERY)) {
    analog_mask |= 1U << (cproto::IN_BATTERY - cproto::ANALOG_OFFSET);
    values[value_count++] = this->m_battery_level_;
  }

  const auto append_analog = [&](uint8_t field) {
    const uint8_t bit = field - cproto::ANALOG_OFFSET;
    if (!this->has_input(field)) {
      return;
    }
    analog_mask |= 1U << bit;
    if (field == cproto::IN_BATTERY) {
      values[value_count++] = this->m_battery_level_;
      return;
    }
    float val = this->m_analog_state_[bit];
    uint8_t bytes[4];
    std::memcpy(bytes, &val, sizeof(val));
    for (uint8_t i = 0; i < 4; i++) {
      values[value_count++] = bytes[i];
    }
  };

  for (uint8_t field = cproto::IN_LEFT_THUMB_X; field <= cproto::IN_RIGHT_TOUCH_Y; field++) {
    if (field == cproto::IN_BATTERY) {
      continue;
    }
    append_analog(field);
  }

  if (binary_mask == 0 && analog_mask == 0) {
    return;
  }

  if (!this->m_last_sent_valid_ || all || this->m_binary_state_ != this->m_last_binary_state_ ||
      this->m_battery_level_ != this->m_last_battery_level_) {
    uint8_t payload[12 + cproto::ANALOG_COUNT * 4 + 1];
    payload[0] = static_cast<uint8_t>(binary_mask & 0xFF);
    payload[1] = static_cast<uint8_t>((binary_mask >> 8) & 0xFF);
    payload[2] = static_cast<uint8_t>((binary_mask >> 16) & 0xFF);
    payload[3] = static_cast<uint8_t>((binary_mask >> 24) & 0xFF);
    payload[4] = static_cast<uint8_t>(binary_values & 0xFF);
    payload[5] = static_cast<uint8_t>((binary_values >> 8) & 0xFF);
    payload[6] = static_cast<uint8_t>((binary_values >> 16) & 0xFF);
    payload[7] = static_cast<uint8_t>((binary_values >> 24) & 0xFF);
    payload[8] = static_cast<uint8_t>(analog_mask & 0xFF);
    payload[9] = static_cast<uint8_t>((analog_mask >> 8) & 0xFF);
    payload[10] = static_cast<uint8_t>((analog_mask >> 16) & 0xFF);
    payload[11] = static_cast<uint8_t>((analog_mask >> 24) & 0xFF);
    std::memcpy(payload + 12, values, value_count);
    ESP_LOGD(TAG, "TX input update: binary=0x%08" PRIX32 " analog_mask=0x%08" PRIX32, binary_values, analog_mask);
    this->send_frame(cproto::MSG_INPUT_UPDATE, payload, 12 + value_count);

    this->m_last_binary_state_ = this->m_binary_state_;
    this->m_last_battery_level_ = this->m_battery_level_;
    std::memcpy(this->m_last_analog_state_, this->m_analog_state_, sizeof(this->m_analog_state_));
    this->m_last_sent_valid_ = true;
  }
}

void GamepadCompanion::send_frame(uint8_t msg_type, const uint8_t *payload, uint16_t len) {
  const uint8_t len_l = len & 0xFF;
  const uint8_t len_h = (len >> 8) & 0xFF;
  uint8_t crc = 0;
  crc = this->crc8_update(crc, msg_type);
  crc = this->crc8_update(crc, len_l);
  crc = this->crc8_update(crc, len_h);
  for (uint16_t i = 0; i < len; i++) {
    crc = this->crc8_update(crc, payload[i]);
  }

  this->write_byte(cproto::SYNC_1);
  this->write_byte(cproto::SYNC_2);
  this->write_byte(msg_type);
  this->write_byte(len_l);
  this->write_byte(len_h);
  if (len > 0) {
    this->write_array(payload, len);
  }
  this->write_byte(crc);
}

bool GamepadCompanion::try_parse_frame() {
  uint16_t start = 0;
  while (start + 1 < this->m_rx_length_) {
    if (this->m_rx_buffer_[start] == cproto::SYNC_1 && this->m_rx_buffer_[start + 1] == cproto::SYNC_2) {
      break;
    }
    start++;
  }
  if (start > 0) {
    if (start >= this->m_rx_length_) {
      this->m_rx_length_ = 0;
      return false;
    }
    std::memmove(this->m_rx_buffer_, this->m_rx_buffer_ + start, this->m_rx_length_ - start);
    this->m_rx_length_ -= start;
  }
  if (this->m_rx_length_ < 5) {
    return false;
  }
  const uint8_t msg_type = this->m_rx_buffer_[2];
  const uint16_t payload_len =
      static_cast<uint16_t>(this->m_rx_buffer_[3]) | (static_cast<uint16_t>(this->m_rx_buffer_[4]) << 8);
  const uint16_t total = 5 + payload_len + 1;
  if (total > cproto::MAX_FRAME_SIZE) {
    std::memmove(this->m_rx_buffer_, this->m_rx_buffer_ + 2, this->m_rx_length_ - 2);
    this->m_rx_length_ -= 2;
    return true;
  }
  if (this->m_rx_length_ < total) {
    return false;
  }
  uint8_t crc = 0;
  for (uint16_t i = 2; i < 5 + payload_len; i++) {
    crc = this->crc8_update(crc, this->m_rx_buffer_[i]);
  }
  if (crc != this->m_rx_buffer_[5 + payload_len]) {
    ESP_LOGW(TAG, "CRC mismatch — resyncing");
    std::memmove(this->m_rx_buffer_, this->m_rx_buffer_ + 2, this->m_rx_length_ - 2);
    this->m_rx_length_ -= 2;
    return true;
  }

  this->dispatch_frame(msg_type, this->m_rx_buffer_ + 5, payload_len);
  const uint16_t remaining = this->m_rx_length_ - total;
  if (remaining > 0) {
    std::memmove(this->m_rx_buffer_, this->m_rx_buffer_ + total, remaining);
  }
  this->m_rx_length_ = remaining;
  return true;
}

void GamepadCompanion::dispatch_frame(uint8_t msg_type, const uint8_t *payload, uint16_t len) {
  switch (msg_type) {
    case cproto::MSG_CAP_REQUEST:
      ESP_LOGD(TAG, "Received CAP_REQUEST — sending capabilities");
      this->send_cap_response_();
      break;
    case cproto::MSG_STATE_REQUEST:
      ESP_LOGI(TAG, "Link established — master connected");
      this->update_input_state_();
      this->send_input_update_(true);
      this->m_master_ready_ = true;
      break;
    case cproto::MSG_OUTPUT_UPDATE:
      this->handle_output_update_(payload, len);
      break;
    default:
      ESP_LOGW(TAG, "Unknown msg 0x%02X", msg_type);
      break;
  }
}

void GamepadCompanion::handle_output_update_(const uint8_t *payload, uint16_t len) {
  if (len < cproto::OUTPUT_MASK_BYTES) {
    ESP_LOGW(TAG, "OUTPUT_UPDATE too short (%u)", len);
    return;
  }
  uint64_t output_mask = 0;
  for (uint8_t i = 0; i < cproto::OUTPUT_MASK_BYTES; i++) {
    output_mask |= static_cast<uint64_t>(payload[i]) << (i * 8);
  }
  ESP_LOGD(TAG, "RX output update: mask=0x%010llX", (unsigned long long) output_mask);
  uint16_t pos = cproto::OUTPUT_MASK_BYTES;
  for (uint8_t field = 0; field < cproto::OUTPUT_FIELD_COUNT; field++) {
    if (!(output_mask & (1ULL << field))) {
      continue;
    }
    if (pos >= len) {
      ESP_LOGW(TAG, "OUTPUT_UPDATE missing value for field %u", field);
      return;
    }
    uint8_t value = payload[pos++];
    if (this->has_output(field)) {
      this->publish_output_(field, value);
    }
  }
}

void GamepadCompanion::publish_output_(uint8_t field, uint8_t value) {
  switch (field) {
    case cproto::OUT_RUMBLE_WEAK_LEVEL:
      if (this->m_rumble_weak_level_sensor) {
        this->m_rumble_weak_level_sensor->publish_state(value);
      }
      if (this->m_rumble_weak_sensor) {
        this->m_rumble_weak_sensor->publish_state(value > 0);
      }
      break;
    case cproto::OUT_RUMBLE_STRONG_LEVEL:
      if (this->m_rumble_strong_level_sensor) {
        this->m_rumble_strong_level_sensor->publish_state(value);
      }
      if (this->m_rumble_strong_sensor) {
        this->m_rumble_strong_sensor->publish_state(value > 0);
      }
      break;
    case cproto::OUT_LIGHTBAR_R:
      this->m_lightbar_r_ = value;
      if (this->m_lightbar_red_sensor) {
        this->m_lightbar_red_sensor->publish_state(value);
      }
      this->publish_lightbar_();
      break;
    case cproto::OUT_LIGHTBAR_G:
      this->m_lightbar_g_ = value;
      if (this->m_lightbar_green_sensor) {
        this->m_lightbar_green_sensor->publish_state(value);
      }
      this->publish_lightbar_();
      break;
    case cproto::OUT_LIGHTBAR_B:
      this->m_lightbar_b_ = value;
      if (this->m_lightbar_blue_sensor) {
        this->m_lightbar_blue_sensor->publish_state(value);
      }
      this->publish_lightbar_();
      break;
    case cproto::OUT_MUTE_LED:
      this->m_mute_led_ = value;
      this->publish_mute_();
      break;
    case cproto::OUT_PLAYER_LEDS:
      if (this->m_player_number_sensor) {
        this->m_player_number_sensor->publish_state(value);
      }
      break;
    case cproto::OUT_LEFT_TRIGGER_EFFECT:
      if (this->m_left_trigger_effect_sensor) {
        this->m_left_trigger_effect_sensor->publish_state(value);
      }
      if (this->m_left_trigger_effect_active_sensor) {
        this->m_left_trigger_effect_active_sensor->publish_state(value > 0);
      }
      break;
    case cproto::OUT_RIGHT_TRIGGER_EFFECT:
      if (this->m_right_trigger_effect_sensor) {
        this->m_right_trigger_effect_sensor->publish_state(value);
      }
      if (this->m_right_trigger_effect_active_sensor) {
        this->m_right_trigger_effect_active_sensor->publish_state(value > 0);
      }
      break;
    case cproto::OUT_HEADPHONE_VOL:
      if (this->m_headphone_volume_sensor) {
        this->m_headphone_volume_sensor->publish_state(value);
      }
      break;
    case cproto::OUT_SPEAKER_VOL:
      if (this->m_speaker_volume_sensor) {
        this->m_speaker_volume_sensor->publish_state(value);
      }
      break;
    case cproto::OUT_MIC_VOL:
      if (this->m_mic_volume_sensor) {
        this->m_mic_volume_sensor->publish_state(value);
      }
      break;
    case cproto::OUT_LED_BRIGHTNESS:
      if (this->m_led_brightness_sensor) {
        this->m_led_brightness_sensor->publish_state(value);
      }
      break;
    default:
      if (field >= cproto::OUT_LEFT_ZONE_BASE && field < cproto::OUT_LEFT_ZONE_BASE + 10) {
        const uint8_t index = field - cproto::OUT_LEFT_ZONE_BASE;
        if (this->m_left_trigger_zone_sensors_[index]) {
          this->m_left_trigger_zone_sensors_[index]->publish_state(value);
        }
      } else if (field >= cproto::OUT_RIGHT_ZONE_BASE && field < cproto::OUT_RIGHT_ZONE_BASE + 10) {
        const uint8_t index = field - cproto::OUT_RIGHT_ZONE_BASE;
        if (this->m_right_trigger_zone_sensors_[index]) {
          this->m_right_trigger_zone_sensors_[index]->publish_state(value);
        }
      }
      break;
  }
}

void GamepadCompanion::publish_lightbar_() {
  if (!this->m_lightbar_light) {
    return;
  }
  auto call = this->m_lightbar_light->make_call();
  const bool on = this->m_lightbar_r_ || this->m_lightbar_g_ || this->m_lightbar_b_;
  call.set_state(on);
  if (on) {
    call.set_rgb(this->m_lightbar_r_ / 255.0f, this->m_lightbar_g_ / 255.0f, this->m_lightbar_b_ / 255.0f);
  }
  call.perform();
}

void GamepadCompanion::publish_mute_() {
  if (this->m_mute_led_sensor) {
    this->m_mute_led_sensor->publish_state(this->m_mute_led_);
  }
  if (!this->m_mute_light) {
    return;
  }
  auto call = this->m_mute_light->make_call();
  call.set_state(this->m_mute_led_ != 0);
  if (this->m_mute_led_ == 2 && this->m_mute_pulse_effect) {
    call.set_effect(this->m_mute_pulse_effect);
  }
  call.perform();
}

}  // namespace gamepad_companion
}  // namespace esphome
