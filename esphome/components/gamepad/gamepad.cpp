#include "gamepad.h"
#include "esphome/core/log.h"
#include <BleConnectionStatus.h>
#include <BleCompositeHID.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace esphome {
namespace gamepad {

#define CONFIG_BT_NIMBLE_EXT_ADV 1  // NOLINT

static const char *const TAG = "gamepad";

static Gamepad *g_feedback_gamepad{nullptr};

static void decode_trigger_zones(const DualsenseGamepadOutputReportData::ParsedTriggerEffect &effect,
                                 uint8_t out_zones[10]) {
  for (int i = 0; i < 10; i++) {
    out_zones[i] = 0;
  }
  switch (effect.subtype()) {
    case DS_TRIGGER_SUBTYPE_FEEDBACK:
    case DS_TRIGGER_SUBTYPE_SLOPE_FEEDBACK: {
      const auto fb = effect.asFeedback();
      for (int i = 0; i < 10; i++)
        out_zones[i] = fb.per_position_strength[i];
      break;
    }
    case DS_TRIGGER_SUBTYPE_MULTIPLE_POSITION_FEEDBACK: {
      const auto mp = effect.asMultiPosition();
      for (int i = 0; i < 10; i++)
        out_zones[i] = mp.per_position_strength[i];
      break;
    }
    case DS_TRIGGER_SUBTYPE_WEAPON: {
      const auto w = effect.asWeapon();
      for (int i = w.start_position; i <= w.end_position && i < 10; i++)
        out_zones[i] = w.strength;
      break;
    }
    case DS_TRIGGER_SUBTYPE_VIBRATION: {
      const auto v = effect.asVibration();
      for (int i = 0; i < 10; i++)
        out_zones[i] = v.per_position_amplitude[i];
      break;
    }
    case DS_TRIGGER_SUBTYPE_MULTIPLE_POSITION_VIBRATION: {
      const auto mv = effect.asMultiVibration();
      for (int i = 0; i < 10; i++)
        out_zones[i] = mv.per_position_amplitude[i];
      break;
    }
    default:
      break;
  }
}

void feedback_callback(DualsenseGamepadOutputReportData data) {
  if (g_feedback_gamepad != nullptr) {
    g_feedback_gamepad->handle_feedback(data);
  }
}

Gamepad::~Gamepad() = default;

void Gamepad::handle_feedback(DualsenseGamepadOutputReportData data) {
  this->m_feedback_data_ = data;
  this->m_feedback_pending_ = true;
}

void Gamepad::process_pending_feedback() {
  if (!this->m_feedback_pending_) {
    return;
  }
  this->m_feedback_pending_ = false;
  const DualsenseGamepadOutputReportData &data = this->m_feedback_data_;

  const bool weak_active = data.motor_right > 0;
  const bool strong_active = data.motor_left > 0;

  if (this->m_rumble_weak_sensor != nullptr) {
    this->m_rumble_weak_sensor->publish_state(weak_active);
  }
  if (this->m_rumble_strong_sensor != nullptr) {
    this->m_rumble_strong_sensor->publish_state(strong_active);
  }
  if (this->m_rumble_weak_level_sensor != nullptr) {
    this->m_rumble_weak_level_sensor->publish_state(data.motor_right);
  }
  if (this->m_rumble_strong_level_sensor != nullptr) {
    this->m_rumble_strong_level_sensor->publish_state(data.motor_left);
  }

  if (this->m_lightbar_light != nullptr) {
    this->m_lightbar_red = data.lightbar_red;
    this->m_lightbar_green = data.lightbar_green;
    this->m_lightbar_blue = data.lightbar_blue;
    this->m_lightbar_state = data.lightbar_red > 0 || data.lightbar_green > 0 || data.lightbar_blue > 0;
    this->m_lightbar_state_pending = true;
  }

  if (this->m_mute_output != nullptr) {
    this->m_mute_output->publish_state(data.mute_button_led > 0);
  }
  if (this->m_mute_light_ != nullptr) {
    this->m_mute_led_mode_ = data.mute_button_led;
    this->m_mute_light_pending_ = true;
  }

  if (this->m_player_number_sensor != nullptr) {
    this->m_player_number_sensor->publish_state(static_cast<float>(data.player_leds));
  }

  if (data.hasLeftTriggerEffect()) {
    const auto lt = data.leftTrigger();
    const bool active = static_cast<DsTriggerMode>(lt.mode) != DsTriggerMode::Off;
    if (this->m_left_trigger_effect_active_sensor != nullptr) {
      this->m_left_trigger_effect_active_sensor->publish_state(active);
    }
    if (this->m_left_trigger_effect_sensor != nullptr) {
      uint8_t mode_num;
      switch (static_cast<DsTriggerMode>(lt.mode)) {
        case DsTriggerMode::Feedback:
          mode_num = 1;
          break;
        case DsTriggerMode::Weapon:
          mode_num = 2;
          break;
        case DsTriggerMode::Vibration:
          mode_num = 3;
          break;
        default:
          mode_num = 0;
          break;
      }
      this->m_left_trigger_effect_sensor->publish_state(mode_num);
    }
    uint8_t left_zones[10];
    decode_trigger_zones(lt, left_zones);
    for (int i = 0; i < 10; i++) {
      if (this->m_left_trigger_zone_sensors_[i] != nullptr && left_zones[i] != this->m_left_trigger_zones_last_[i]) {
        this->m_left_trigger_zones_last_[i] = left_zones[i];
        this->m_left_trigger_zone_sensors_[i]->publish_state(left_zones[i]);
      }
    }
  } else {
    for (int i = 0; i < 10; i++) {
      if (this->m_left_trigger_zone_sensors_[i] != nullptr && this->m_left_trigger_zones_last_[i] != 0) {
        this->m_left_trigger_zones_last_[i] = 0;
        this->m_left_trigger_zone_sensors_[i]->publish_state(0);
      }
    }
  }

  if (data.hasRightTriggerEffect()) {
    const auto rt = data.rightTrigger();
    const bool active = static_cast<DsTriggerMode>(rt.mode) != DsTriggerMode::Off;
    if (this->m_right_trigger_effect_active_sensor != nullptr) {
      this->m_right_trigger_effect_active_sensor->publish_state(active);
    }
    if (this->m_right_trigger_effect_sensor != nullptr) {
      uint8_t mode_num;
      switch (static_cast<DsTriggerMode>(rt.mode)) {
        case DsTriggerMode::Feedback:
          mode_num = 1;
          break;
        case DsTriggerMode::Weapon:
          mode_num = 2;
          break;
        case DsTriggerMode::Vibration:
          mode_num = 3;
          break;
        default:
          mode_num = 0;
          break;
      }
      this->m_right_trigger_effect_sensor->publish_state(mode_num);
    }
    uint8_t right_zones[10];
    decode_trigger_zones(rt, right_zones);
    for (int i = 0; i < 10; i++) {
      if (this->m_right_trigger_zone_sensors_[i] != nullptr && right_zones[i] != this->m_right_trigger_zones_last_[i]) {
        this->m_right_trigger_zones_last_[i] = right_zones[i];
        this->m_right_trigger_zone_sensors_[i]->publish_state(right_zones[i]);
      }
    }
  } else {
    for (int i = 0; i < 10; i++) {
      if (this->m_right_trigger_zone_sensors_[i] != nullptr && this->m_right_trigger_zones_last_[i] != 0) {
        this->m_right_trigger_zones_last_[i] = 0;
        this->m_right_trigger_zone_sensors_[i]->publish_state(0);
      }
    }
  }

  if (this->m_headphone_volume_sensor != nullptr) {
    this->m_headphone_volume_sensor->publish_state(data.headphone_volume);
  }
  if (this->m_speaker_volume_sensor != nullptr) {
    this->m_speaker_volume_sensor->publish_state(data.speaker_volume);
  }
  if (this->m_mic_volume_sensor != nullptr) {
    this->m_mic_volume_sensor->publish_state(data.mic_volume);
  }
  if (this->m_led_brightness_sensor != nullptr) {
    this->m_led_brightness_sensor->publish_state(data.led_brightness);
  }

  if (this->m_companion_ != nullptr && this->m_companion_->is_ready()) {
    send_companion_output(data);
  }
}

void Gamepad::send_companion_output(const DualsenseGamepadOutputReportData &data) {
  uint8_t lt_effect = 0, rt_effect = 0;
  if (data.hasLeftTriggerEffect()) {
    switch (static_cast<DsTriggerMode>(data.leftTrigger().mode)) {
      case DsTriggerMode::Feedback:
        lt_effect = 1;
        break;
      case DsTriggerMode::Weapon:
        lt_effect = 2;
        break;
      case DsTriggerMode::Vibration:
        lt_effect = 3;
        break;
      default:
        break;
    }
  }
  if (data.hasRightTriggerEffect()) {
    switch (static_cast<DsTriggerMode>(data.rightTrigger().mode)) {
      case DsTriggerMode::Feedback:
        rt_effect = 1;
        break;
      case DsTriggerMode::Weapon:
        rt_effect = 2;
        break;
      case DsTriggerMode::Vibration:
        rt_effect = 3;
        break;
      default:
        break;
    }
  }

  uint8_t left_zones[10]{}, right_zones[10]{};
  if (data.hasLeftTriggerEffect())
    decode_trigger_zones(data.leftTrigger(), left_zones);
  if (data.hasRightTriggerEffect())
    decode_trigger_zones(data.rightTrigger(), right_zones);

  this->m_companion_->send_output(data.motor_right, data.motor_left, data.lightbar_red, data.lightbar_green,
                                  data.lightbar_blue, data.mute_button_led, data.player_leds, lt_effect, rt_effect,
                                  data.headphone_volume, data.speaker_volume, data.mic_volume, data.led_brightness,
                                  left_zones, right_zones);
}

bool Gamepad::companion_binary(uint8_t field) const {
  return m_companion_ != nullptr && m_companion_->is_ready() && m_companion_->has_input(field) &&
         m_companion_->get_binary(field);
}

float Gamepad::companion_float(uint8_t field) const {
  if (m_companion_ == nullptr || !m_companion_->is_ready() || !m_companion_->has_input(field))
    return std::numeric_limits<float>::quiet_NaN();
  return m_companion_->get_float(field);
}

void Gamepad::setup() {
  if (m_companion_ != nullptr)
    m_companion_->setup();
  update_battery_level();
  if (this->m_timing_info_switch != nullptr) {
    this->m_timing_info = this->m_timing_info_switch->state;
    this->m_timing_info_switch->add_on_state_callback([this](bool state) { this->set_timing_info(state); });
  }
  m_composite_hid = new BleCompositeHID(m_name, m_manufacturer_id, m_battery_level);
  m_dualsense_config = new DualsenseEdgeControllerDeviceConfiguration();

  m_dualsense_config->setAutoReport(false);
  m_dualsense_config->setAutoDefer(false);

  m_host_config = m_dualsense_config->getIdealHostConfiguration();

  m_dualsense = new DualsenseGamepadDevice(m_dualsense_config);

  g_feedback_gamepad = this;

  FunctionSlot<DualsenseGamepadOutputReportData> feedback_slot(feedback_callback);

  m_dualsense->onReceivedOutputReport.attach(feedback_slot);

  m_composite_hid->addDevice(m_dualsense);

  m_composite_hid->begin(m_host_config);
}

void Gamepad::update() {
  if (m_timing_info) {
    m_now = millis();
    ESP_LOGI(TAG, "Update loop period: %lu ms", static_cast<unsigned long>(m_now - m_last_report_time));
    m_last_report_time = m_now;
  }

  if (m_companion_ != nullptr)
    m_companion_->process();

  if (!is_connected()) {
    m_init_sent = false;
    m_left_touch_was_active = false;
    m_right_touch_was_active = false;
    return;
  }

  if (!m_init_sent) {
    delay(150);  // NOLINT
    m_dualsense->sendPairingInfoReport();

    m_dualsense->sendFirmInfoReport();

    m_dualsense->sendCalibrationReport();

    delay(1000);  // NOLINT
    m_dualsense->resetInputs();
    m_init_sent = true;
  }

  process_pending_feedback();
  update_battery_level();
  update_charging_status();
  update_peripheral_status();
  handle_all_buttons();
  update_dpad();
  update_thumbsticks();
  update_all_triggers();
  update_touchpad();
  update_motion_inputs();
  update_mute_light();
  update_lightbar_light();

  m_dualsense->sendGamepadReport();
  if (m_timing_info) {
    ESP_LOGI(TAG, "Finished gamepad update loop in %lu ms", static_cast<unsigned long>(millis() - m_now));
  }
}

void Gamepad::update_all_triggers() {
  update_left_trigger();
  update_right_trigger();
}

uint16_t Gamepad::scale_touchpad_raw(float value, float input_min, float input_max, uint16_t output_min,
                                     uint16_t output_max) {
  const float range = input_max - input_min;
  if (range == 0.0f) {
    return output_min;
  }
  const float scaled =
      (value - input_min) / range * static_cast<float>(output_max - output_min) + static_cast<float>(output_min);
  return static_cast<uint16_t>(
      std::clamp(static_cast<int>(std::lround(scaled)), static_cast<int>(output_min), static_cast<int>(output_max)));
}

uint16_t Gamepad::scale_touchpad_axis(sensor::Sensor *sensor, float input_min, float input_max, uint16_t output_min,
                                      uint16_t output_max) {
  if (sensor == nullptr) {
    return output_min;
  }
  const float value = sensor->state;
  if (std::isnan(value)) {
    return output_min;
  }
  return scale_touchpad_raw(value, input_min, input_max, output_min, output_max);
}

void Gamepad::update_touchpad_contact(binary_sensor::BinarySensor *touch_sensor, sensor::Sensor *x_sensor,
                                      sensor::Sensor *y_sensor, float x_min, float x_max, float y_min, float y_max,
                                      bool &was_active, int8_t &touch_id, uint16_t x_out_min, uint16_t x_out_max,
                                      uint8_t touch_field, uint8_t x_field, uint8_t y_field) {
  static constexpr uint16_t TOUCHPAD_Y_MAX = 1079;

  const bool active = (touch_sensor != nullptr) ? touch_sensor->state : companion_binary(touch_field);
  if (touch_sensor == nullptr && !m_companion_->has_input(touch_field))
    return;

  if (!active) {
    if (was_active) {
      m_dualsense->touchpadStopTouch(touch_id);
      touch_id = -1;
      was_active = false;
    }
    return;
  }

  // Resolve X/Y from sensor or companion float
  auto get_axis = [&](sensor::Sensor *s, uint8_t cf, float in_min, float in_max, uint16_t out_min,
                      uint16_t out_max) -> uint16_t {
    float val = NAN;
    if (s != nullptr)
      val = s->state;
    else if (m_companion_ != nullptr && m_companion_->is_ready() && m_companion_->has_input(cf))
      val = m_companion_->get_float(cf);
    if (std::isnan(val))
      return out_min;
    return scale_touchpad_raw(val, in_min, in_max, out_min, out_max);
  };

  const uint16_t x = get_axis(x_sensor, x_field, x_min, x_max, x_out_min, x_out_max);
  const uint16_t y = get_axis(y_sensor, y_field, y_min, y_max, 0, TOUCHPAD_Y_MAX);

  if (!was_active) {
    touch_id = m_dualsense->touchpadStartTouch(x, y);
    was_active = true;
  } else {
    m_dualsense->touchpadUpdatePosition(x, y, touch_id);
  }
}

void Gamepad::update_touchpad() {
  // When both touchpads are configured the DualSense surface is split at its midpoint:
  //   left  → touch point 1, X mapped to [0,   959]
  //   right → touch point 2, X mapped to [960, 1919]
  // When only one touchpad is configured it occupies the full surface (X 0–1919).
  static constexpr uint16_t TOUCHPAD_X_MAX = 1919;
  static constexpr uint16_t TOUCHPAD_X_LEFT_MAX = 959;
  static constexpr uint16_t TOUCHPAD_X_RIGHT_MIN = 960;

  handle_button(m_touchpad_button, DUALSENSE_BUTTON_TOUCHPAD, cproto::IN_TOUCHPAD_BTN);

  const bool has_left = m_left_touch_sensor != nullptr || (m_companion_ != nullptr && m_companion_->is_ready() &&
                                                           m_companion_->has_input(cproto::IN_LEFT_TOUCH));
  const bool has_right = m_right_touch_sensor != nullptr || (m_companion_ != nullptr && m_companion_->is_ready() &&
                                                             m_companion_->has_input(cproto::IN_RIGHT_TOUCH));

  if (has_left && has_right) {
    update_touchpad_contact(m_left_touch_sensor, m_left_touch_x_sensor, m_left_touch_y_sensor, m_left_touch_x_min,
                            m_left_touch_x_max, m_left_touch_y_min, m_left_touch_y_max, m_left_touch_was_active,
                            m_left_touch_id, 0, TOUCHPAD_X_LEFT_MAX, cproto::IN_LEFT_TOUCH, cproto::IN_LEFT_TOUCH_X,
                            cproto::IN_LEFT_TOUCH_Y);
    update_touchpad_contact(m_right_touch_sensor, m_right_touch_x_sensor, m_right_touch_y_sensor, m_right_touch_x_min,
                            m_right_touch_x_max, m_right_touch_y_min, m_right_touch_y_max, m_right_touch_was_active,
                            m_right_touch_id, TOUCHPAD_X_RIGHT_MIN, TOUCHPAD_X_MAX, cproto::IN_RIGHT_TOUCH,
                            cproto::IN_RIGHT_TOUCH_X, cproto::IN_RIGHT_TOUCH_Y);
  } else if (has_left) {
    update_touchpad_contact(m_left_touch_sensor, m_left_touch_x_sensor, m_left_touch_y_sensor, m_left_touch_x_min,
                            m_left_touch_x_max, m_left_touch_y_min, m_left_touch_y_max, m_left_touch_was_active,
                            m_left_touch_id, 0, TOUCHPAD_X_MAX, cproto::IN_LEFT_TOUCH, cproto::IN_LEFT_TOUCH_X,
                            cproto::IN_LEFT_TOUCH_Y);
  } else if (has_right) {
    update_touchpad_contact(m_right_touch_sensor, m_right_touch_x_sensor, m_right_touch_y_sensor, m_right_touch_x_min,
                            m_right_touch_x_max, m_right_touch_y_min, m_right_touch_y_max, m_right_touch_was_active,
                            m_right_touch_id, 0, TOUCHPAD_X_MAX, cproto::IN_RIGHT_TOUCH, cproto::IN_RIGHT_TOUCH_X,
                            cproto::IN_RIGHT_TOUCH_Y);
  }
}

void Gamepad::handle_all_buttons() {
  handle_button(m_a_button, DUALSENSE_BUTTON_A, cproto::IN_A_BUTTON);
  handle_button(m_b_button, DUALSENSE_BUTTON_B, cproto::IN_B_BUTTON);
  handle_button(m_x_button, DUALSENSE_BUTTON_X, cproto::IN_X_BUTTON);
  handle_button(m_y_button, DUALSENSE_BUTTON_Y, cproto::IN_Y_BUTTON);
  handle_button(m_lb_button, DUALSENSE_BUTTON_LB, cproto::IN_LB_BUTTON);
  handle_button(m_rb_button, DUALSENSE_BUTTON_RB, cproto::IN_RB_BUTTON);
  handle_button(m_ls_button, DUALSENSE_BUTTON_LT, cproto::IN_LS_BUTTON);
  handle_button(m_rs_button, DUALSENSE_BUTTON_RT, cproto::IN_RS_BUTTON);
  handle_button(m_l3_button, DUALSENSE_BUTTON_LS, cproto::IN_L3_BUTTON);
  handle_button(m_r3_button, DUALSENSE_BUTTON_RS, cproto::IN_R3_BUTTON);
  handle_button(m_share_button, DUALSENSE_BUTTON_SHARE, cproto::IN_SHARE_BUTTON);
  handle_button(m_mute_button, DUALSENSE_BUTTON_MUTE, cproto::IN_MUTE_BUTTON);
  handle_button(m_l4_button, DUALSENSE_BUTTON_L4, cproto::IN_L4_BUTTON);
  handle_button(m_r4_button, DUALSENSE_BUTTON_R4, cproto::IN_R4_BUTTON);
  handle_button(m_l5_button, DUALSENSE_BUTTON_L5, cproto::IN_L5_BUTTON);
  handle_button(m_r5_button, DUALSENSE_BUTTON_R5, cproto::IN_R5_BUTTON);
  handle_button(m_mode_button, DUALSENSE_BUTTON_MODE, cproto::IN_MODE_BUTTON);
  handle_button(m_start_button, DUALSENSE_BUTTON_START, cproto::IN_START_BUTTON);
  handle_button(m_select_button, DUALSENSE_BUTTON_SELECT, cproto::IN_SELECT_BUTTON);
}

void Gamepad::handle_button(binary_sensor::BinarySensor *button, uint32_t input_button, uint8_t companion_field) {
  bool state = false;
  if (button != nullptr) {
    state = button->state;
  } else if (companion_field != 0xFF && m_companion_ != nullptr && m_companion_->is_ready() &&
             m_companion_->has_input(companion_field)) {
    state = m_companion_->get_binary(companion_field);
  } else {
    return;
  }
  if (state) {
    m_dualsense->press(input_button);
  } else {
    m_dualsense->release(input_button);
  }
}

bool Gamepad::is_connected() {
  if (m_composite_hid == nullptr) {
    return false;
  }
  return m_composite_hid->isConnected();
}

void Gamepad::update_battery_level() {
  uint8_t battery_level = m_battery_level;
  if (m_battery_level_sensor != nullptr) {
    const float v = m_battery_level_sensor->state;
    if (!std::isnan(v))
      battery_level = static_cast<uint8_t>(std::clamp(static_cast<int>(std::lround(v)), 0, 100));
  } else if (m_companion_ != nullptr && m_companion_->is_ready() && m_companion_->has_input(cproto::IN_BATTERY)) {
    battery_level = m_companion_->get_battery();
  }
  if (m_battery_level != battery_level) {
    m_battery_level = battery_level;
    m_composite_hid->setBatteryLevel(m_battery_level);
    m_dualsense->setBatteryLevel(m_battery_level);
  }
}

void Gamepad::update_charging_status() {
  bool charging = false;
  if (m_charging_status_sensor != nullptr) {
    charging = m_charging_status_sensor->state;
  } else if (m_companion_ != nullptr && m_companion_->is_ready() && m_companion_->has_input(cproto::IN_CHARGING)) {
    charging = m_companion_->get_binary(cproto::IN_CHARGING);
  } else {
    return;
  }
  if (charging == m_charging_status_last)
    return;
  m_charging_status_last = charging;
  m_dualsense->setChargingStatus(charging);
}

static bool get_bool_input(binary_sensor::BinarySensor *sensor, uint8_t companion_field,
                           GamepadCompanionLink *companion) {
  if (sensor != nullptr)
    return sensor->state;
  if (companion != nullptr && companion->is_ready() && companion->has_input(companion_field))
    return companion->get_binary(companion_field);
  return false;
}

void Gamepad::update_peripheral_status() {
  if (m_headphones_plugged_sensor != nullptr ||
      (m_companion_ != nullptr && m_companion_->has_input(cproto::IN_HEADPHONES))) {
    m_dualsense->setHeadphonesPlugged(get_bool_input(m_headphones_plugged_sensor, cproto::IN_HEADPHONES, m_companion_));
  }
  if (m_headphone_mic_sensor != nullptr ||
      (m_companion_ != nullptr && m_companion_->has_input(cproto::IN_HEADPHONE_MIC))) {
    m_dualsense->setHeadphoneMic(get_bool_input(m_headphone_mic_sensor, cproto::IN_HEADPHONE_MIC, m_companion_));
  }
  if (m_usb_plugged_sensor != nullptr || (m_companion_ != nullptr && m_companion_->has_input(cproto::IN_USB_PLUGGED))) {
    m_dualsense->setUsbPlugged(get_bool_input(m_usb_plugged_sensor, cproto::IN_USB_PLUGGED, m_companion_));
  }
  if (m_mute_active_sensor != nullptr || (m_companion_ != nullptr && m_companion_->has_input(cproto::IN_MUTE_ACTIVE))) {
    m_dualsense->setMuteActive(get_bool_input(m_mute_active_sensor, cproto::IN_MUTE_ACTIVE, m_companion_));
  }
}

int8_t Gamepad::update_axis(sensor::Sensor *sensor, uint8_t companion_field) {
  float sensor_state = NAN;
  if (sensor != nullptr) {
    sensor_state = sensor->state;
  } else if (m_companion_ != nullptr && m_companion_->is_ready() && m_companion_->has_input(companion_field)) {
    sensor_state = m_companion_->get_float(companion_field);
  }
  if (std::isnan(sensor_state))
    return 0;

  const float in_min = static_cast<float>(m_stick_axis_min);
  const float in_max = static_cast<float>(m_stick_axis_max);
  const float range = in_max - in_min;
  if (range == 0.0f)
    return 0;
  const float normalized = (sensor_state - in_min) / range;
  const float result = normalized * static_cast<float>(DUALSENSE_STICK_MAX - DUALSENSE_STICK_MIN) +
                       static_cast<float>(DUALSENSE_STICK_MIN);
  return static_cast<int8_t>(std::clamp(static_cast<int>(std::lround(result)), static_cast<int>(DUALSENSE_STICK_MIN),
                                        static_cast<int>(DUALSENSE_STICK_MAX)));
}

void Gamepad::update_thumbsticks() {
  m_dualsense->setLeftThumb(update_axis(m_left_thumb_x_sensor, cproto::IN_LEFT_THUMB_X),
                            update_axis(m_left_thumb_y_sensor, cproto::IN_LEFT_THUMB_Y));
  m_dualsense->setRightThumb(update_axis(m_right_thumb_x_sensor, cproto::IN_RIGHT_THUMB_X),
                             update_axis(m_right_thumb_y_sensor, cproto::IN_RIGHT_THUMB_Y));
}

uint8_t Gamepad::update_trigger(sensor::Sensor *sensor, uint8_t companion_field) {
  float sensor_state = NAN;
  if (sensor != nullptr) {
    sensor_state = sensor->state;
  } else if (m_companion_ != nullptr && m_companion_->is_ready() && m_companion_->has_input(companion_field)) {
    sensor_state = m_companion_->get_float(companion_field);
  }
  if (std::isnan(sensor_state))
    return 0;

  const float in_min = static_cast<float>(m_trigger_min);
  const float in_max = static_cast<float>(m_trigger_max);
  const float range = in_max - in_min;
  if (range == 0.0f)
    return 0;
  const float normalized = (sensor_state - in_min) / range;
  const float result = normalized * static_cast<float>(DUALSENSE_TRIGGER_MAX - DUALSENSE_TRIGGER_MIN) +
                       static_cast<float>(DUALSENSE_TRIGGER_MIN);
  return static_cast<uint8_t>(std::clamp(static_cast<int>(std::lround(result)), static_cast<int>(DUALSENSE_TRIGGER_MIN),
                                         static_cast<int>(DUALSENSE_TRIGGER_MAX)));
}

void Gamepad::update_left_trigger() {
  const bool has_companion_lt =
      m_companion_ != nullptr && m_companion_->is_ready() && m_companion_->has_input(cproto::IN_LEFT_TRIGGER);
  if (m_ls_button != nullptr) {
    m_dualsense->setLeftTrigger(m_ls_button->state ? DUALSENSE_TRIGGER_MAX : DUALSENSE_TRIGGER_MIN);
  } else if (m_left_trigger_sensor != nullptr || has_companion_lt) {
    m_dualsense->setLeftTrigger(update_trigger(m_left_trigger_sensor, cproto::IN_LEFT_TRIGGER));
  }
}

void Gamepad::update_right_trigger() {
  const bool has_companion_rt =
      m_companion_ != nullptr && m_companion_->is_ready() && m_companion_->has_input(cproto::IN_RIGHT_TRIGGER);
  if (m_rs_button != nullptr) {
    m_dualsense->setRightTrigger(m_rs_button->state ? DUALSENSE_TRIGGER_MAX : DUALSENSE_TRIGGER_MIN);
  } else if (m_right_trigger_sensor != nullptr || has_companion_rt) {
    m_dualsense->setRightTrigger(update_trigger(m_right_trigger_sensor, cproto::IN_RIGHT_TRIGGER));
  }
}

bool Gamepad::update_motion_value(sensor::Sensor *sensor, int16_t *value, float scale, uint8_t companion_field) {
  if (value == nullptr)
    return false;
  float v = NAN;
  if (sensor != nullptr) {
    v = sensor->state;
  } else if (m_companion_ != nullptr && m_companion_->is_ready() && m_companion_->has_input(companion_field)) {
    v = m_companion_->get_float(companion_field);
  }
  if (std::isnan(v))
    return false;
  *value = static_cast<int16_t>(std::clamp(static_cast<int>(std::lround(v * scale)),
                                           static_cast<int>(std::numeric_limits<int16_t>::min()),
                                           static_cast<int>(std::numeric_limits<int16_t>::max())));
  return true;
}

void Gamepad::update_motion_inputs() {
  static constexpr float GYRO_SCALE = static_cast<float>(DUALSENSE_GYRO_RES_PER_DEG_S);
  static constexpr float ACCEL_SCALE = static_cast<float>(DUALSENSE_ACC_RES_PER_G) / 9.80665f;

  const bool has_gyro = m_yaw_sensor != nullptr || m_pitch_sensor != nullptr || m_roll_sensor != nullptr ||
                        (m_companion_ != nullptr && m_companion_->is_ready() &&
                         (m_companion_->has_input(cproto::IN_YAW) || m_companion_->has_input(cproto::IN_PITCH) ||
                          m_companion_->has_input(cproto::IN_ROLL)));
  const bool has_accel = m_ax_sensor != nullptr || m_ay_sensor != nullptr || m_az_sensor != nullptr ||
                         (m_companion_ != nullptr && m_companion_->is_ready() &&
                          (m_companion_->has_input(cproto::IN_AX) || m_companion_->has_input(cproto::IN_AY) ||
                           m_companion_->has_input(cproto::IN_AZ)));

  update_motion_value(m_yaw_sensor, &m_yaw, GYRO_SCALE, cproto::IN_YAW);
  update_motion_value(m_pitch_sensor, &m_pitch, GYRO_SCALE, cproto::IN_PITCH);
  update_motion_value(m_roll_sensor, &m_roll, GYRO_SCALE, cproto::IN_ROLL);
  update_motion_value(m_ax_sensor, &m_ax, ACCEL_SCALE, cproto::IN_AX);
  update_motion_value(m_ay_sensor, &m_ay, ACCEL_SCALE, cproto::IN_AY);
  update_motion_value(m_az_sensor, &m_az, ACCEL_SCALE, cproto::IN_AZ);

  if (has_gyro)
    m_dualsense->setGyro(m_pitch, m_yaw, m_roll);
  if (has_accel)
    m_dualsense->setAccel(m_ax, m_ay, m_az);
}

bool Gamepad::update_dpad() {
  const bool has_hat_input = m_dpad_up_button != nullptr || m_dpad_right_button != nullptr ||
                             m_dpad_down_button != nullptr || m_dpad_left_button != nullptr;
  if (!has_hat_input) {
    return false;
  }

  const bool up = (m_dpad_up_button != nullptr) ? m_dpad_up_button->state : companion_binary(cproto::IN_DPAD_UP);
  const bool right =
      (m_dpad_right_button != nullptr) ? m_dpad_right_button->state : companion_binary(cproto::IN_DPAD_RIGHT);
  const bool down =
      (m_dpad_down_button != nullptr) ? m_dpad_down_button->state : companion_binary(cproto::IN_DPAD_DOWN);
  const bool left =
      (m_dpad_left_button != nullptr) ? m_dpad_left_button->state : companion_binary(cproto::IN_DPAD_LEFT);

  uint8_t hat = DUALSENSE_BUTTON_DPAD_NONE;
  if (up && !down) {
    if (right && !left) {
      hat = DUALSENSE_BUTTON_DPAD_NORTHEAST;
    } else if (left && !right) {
      hat = DUALSENSE_BUTTON_DPAD_NORTHWEST;
    } else {
      hat = DUALSENSE_BUTTON_DPAD_NORTH;
    }
  } else if (down && !up) {
    if (right && !left) {
      hat = DUALSENSE_BUTTON_DPAD_SOUTHEAST;
    } else if (left && !right) {
      hat = DUALSENSE_BUTTON_DPAD_SOUTHWEST;
    } else {
      hat = DUALSENSE_BUTTON_DPAD_SOUTH;
    }
  } else if (right && !left) {
    hat = DUALSENSE_BUTTON_DPAD_EAST;
  } else if (left && !right) {
    hat = DUALSENSE_BUTTON_DPAD_WEST;
  }

  m_dualsense->pressDPadDirection(hat);
  return true;
}

void Gamepad::update_mute_light() {
  if (this->m_mute_light_ == nullptr) {
    return;
  }
  if (!this->m_mute_light_pending_) {
    return;
  }
  this->m_mute_light_pending_ = false;
  const uint8_t mode = this->m_mute_led_mode_;
  if (mode == this->m_mute_led_mode_last_) {
    return;
  }
  this->m_mute_led_mode_last_ = mode;

  auto call = this->m_mute_light_->make_call();
  if (mode == 0) {
    // Don't set effect here — setting any effect while turning off triggers a warning.
    // Effects stop automatically when the light turns off.
    call.set_state(false);
  } else if (mode == 2 && this->m_mute_pulse_effect_name_ != nullptr) {
    call.set_state(true);
    call.set_effect(this->m_mute_pulse_effect_name_);
  } else {
    if (this->m_mute_pulse_effect_name_ != nullptr) {
      call.set_effect("None");
    }
    call.set_state(true);
  }
  call.perform();
}

void Gamepad::update_lightbar_light() {
  if (m_lightbar_light == nullptr || !m_lightbar_state_pending) {
    return;
  }
  m_lightbar_state_pending = false;

  // When off, color values don't matter — only check state change.
  // When on, also check color to avoid repeated calls with identical color.
  const bool state_changed = m_lightbar_state != m_lightbar_state_last;
  const bool color_changed =
      m_lightbar_state && (m_lightbar_red != m_lightbar_red_last || m_lightbar_green != m_lightbar_green_last ||
                           m_lightbar_blue != m_lightbar_blue_last);
  if (!state_changed && !color_changed) {
    return;
  }

  auto call = m_lightbar_light->make_call();
  call.set_state(m_lightbar_state);
  if (m_lightbar_state) {
    call.set_rgb(m_lightbar_red / 255.0f, m_lightbar_green / 255.0f, m_lightbar_blue / 255.0f);
  }
  call.perform();

  m_lightbar_state_last = m_lightbar_state;
  if (m_lightbar_state) {
    m_lightbar_red_last = m_lightbar_red;
    m_lightbar_green_last = m_lightbar_green;
    m_lightbar_blue_last = m_lightbar_blue;
  }
}

}  // namespace gamepad
}  // namespace esphome
