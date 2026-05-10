#include "gamepad.h"
#include "esphome/core/log.h"
#include <BleConnectionStatus.h>
#include <BleCompositeHID.h>
#include <algorithm>
#include <cmath>
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
}

void Gamepad::setup() {
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

  if (!is_connected()) {
    m_init_sent = false;
    m_touch_was_active = false;
    m_touch2_was_active = false;
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

uint16_t Gamepad::scale_touchpad_raw(float value, float input_min, float input_max, uint16_t output_max) {
  const float range = input_max - input_min;
  if (range == 0.0f) {
    return 0;
  }
  const float scaled = (value - input_min) / range * static_cast<float>(output_max);
  return static_cast<uint16_t>(std::clamp(static_cast<int>(std::lround(scaled)), 0, static_cast<int>(output_max)));
}

uint16_t Gamepad::scale_touchpad_axis(sensor::Sensor *sensor, float input_min, float input_max, uint16_t output_max) {
  if (sensor == nullptr) {
    return 0;
  }
  const float value = sensor->state;
  if (std::isnan(value)) {
    return 0;
  }
  return scale_touchpad_raw(value, input_min, input_max, output_max);
}

void Gamepad::update_touchpad_contact(binary_sensor::BinarySensor *touch_sensor, sensor::Sensor *x_sensor,
                                      sensor::Sensor *y_sensor, float x_min, float x_max, float y_min, float y_max,
                                      bool &was_active, int8_t &touch_id) {
  static constexpr uint16_t TOUCHPAD_X_MAX = 1919;
  static constexpr uint16_t TOUCHPAD_Y_MAX = 1079;

  if (touch_sensor == nullptr) {
    return;
  }

  if (!touch_sensor->state) {
    if (was_active) {
      m_dualsense->touchpadStopTouch(touch_id);
      touch_id = -1;
      was_active = false;
    }
    return;
  }

  const uint16_t x = scale_touchpad_axis(x_sensor, x_min, x_max, TOUCHPAD_X_MAX);
  const uint16_t y = scale_touchpad_axis(y_sensor, y_min, y_max, TOUCHPAD_Y_MAX);

  if (!was_active) {
    touch_id = m_dualsense->touchpadStartTouch(x, y);
    was_active = true;
  } else {
    m_dualsense->touchpadUpdatePosition(x, y, touch_id);
  }
}

void Gamepad::update_touchpad_split() {
  static constexpr uint16_t TOUCHPAD_X_MAX = 1919;
  static constexpr uint16_t TOUCHPAD_Y_MAX = 1079;

  if (m_touch_sensor == nullptr) {
    return;
  }

  if (!m_touch_sensor->state) {
    if (m_touch_was_active) {
      m_dualsense->touchpadStopTouch(m_touch_id);
      m_touch_id = -1;
      m_touch_was_active = false;
    }
    if (m_touch2_was_active) {
      m_dualsense->touchpadStopTouch(m_touch2_id);
      m_touch2_id = -1;
      m_touch2_was_active = false;
    }
    return;
  }

  float x_val = m_touch_x_min;
  if (m_touch_x_sensor != nullptr && !std::isnan(m_touch_x_sensor->state)) {
    x_val = m_touch_x_sensor->state;
  }
  const uint16_t y = scale_touchpad_axis(m_touch_y_sensor, m_touch_y_min, m_touch_y_max, TOUCHPAD_Y_MAX);
  const float x_mid = (m_touch_x_min + m_touch_x_max) * 0.5f;

  if (x_val >= x_mid) {
    if (m_touch_was_active) {
      m_dualsense->touchpadStopTouch(m_touch_id);
      m_touch_id = -1;
      m_touch_was_active = false;
    }
    const uint16_t x = scale_touchpad_raw(x_val, x_mid, m_touch_x_max, TOUCHPAD_X_MAX);
    if (!m_touch2_was_active) {
      m_touch2_id = m_dualsense->touchpadStartTouch(x, y);
      m_touch2_was_active = true;
    } else {
      m_dualsense->touchpadUpdatePosition(x, y, m_touch2_id);
    }
  } else {
    if (m_touch2_was_active) {
      m_dualsense->touchpadStopTouch(m_touch2_id);
      m_touch2_id = -1;
      m_touch2_was_active = false;
    }
    const uint16_t x = scale_touchpad_raw(x_val, m_touch_x_min, x_mid, TOUCHPAD_X_MAX);
    if (!m_touch_was_active) {
      m_touch_id = m_dualsense->touchpadStartTouch(x, y);
      m_touch_was_active = true;
    } else {
      m_dualsense->touchpadUpdatePosition(x, y, m_touch_id);
    }
  }
}

void Gamepad::update_touchpad() {
  handle_button(m_touchpad_button, DUALSENSE_BUTTON_TOUCHPAD);

  if (m_touchpad_split) {
    update_touchpad_split();
    return;
  }

  update_touchpad_contact(m_touch_sensor, m_touch_x_sensor, m_touch_y_sensor, m_touch_x_min, m_touch_x_max,
                          m_touch_y_min, m_touch_y_max, m_touch_was_active, m_touch_id);
  update_touchpad_contact(m_touch2_sensor, m_touch2_x_sensor, m_touch2_y_sensor, m_touch2_x_min, m_touch2_x_max,
                          m_touch2_y_min, m_touch2_y_max, m_touch2_was_active, m_touch2_id);
}

void Gamepad::handle_all_buttons() {
  handle_button(m_a_button, DUALSENSE_BUTTON_A);
  handle_button(m_b_button, DUALSENSE_BUTTON_B);
  handle_button(m_x_button, DUALSENSE_BUTTON_X);
  handle_button(m_y_button, DUALSENSE_BUTTON_Y);
  handle_button(m_lb_button, DUALSENSE_BUTTON_LB);
  handle_button(m_rb_button, DUALSENSE_BUTTON_RB);
  handle_button(m_ls_button, DUALSENSE_BUTTON_LT);
  handle_button(m_rs_button, DUALSENSE_BUTTON_RT);
  handle_button(m_l3_button, DUALSENSE_BUTTON_LS);
  handle_button(m_r3_button, DUALSENSE_BUTTON_RS);
  handle_button(m_share_button, DUALSENSE_BUTTON_SHARE);
  handle_button(m_mute_button, DUALSENSE_BUTTON_MUTE);
  handle_button(m_l4_button, DUALSENSE_BUTTON_L4);
  handle_button(m_r4_button, DUALSENSE_BUTTON_R4);
  handle_button(m_l5_button, DUALSENSE_BUTTON_L5);
  handle_button(m_r5_button, DUALSENSE_BUTTON_R5);
  handle_button(m_mode_button, DUALSENSE_BUTTON_MODE);
  handle_button(m_start_button, DUALSENSE_BUTTON_START);
  handle_button(m_select_button, DUALSENSE_BUTTON_SELECT);
}

void Gamepad::handle_button(binary_sensor::BinarySensor *button, uint32_t input_button) {
  if (button == nullptr) {
    return;
  }

  if (button->state) {
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
  if (m_battery_level_sensor == nullptr) {
    return;
  }

  const float battery_state = m_battery_level_sensor->state;
  if (std::isnan(battery_state)) {
    return;
  }

  const auto battery_level = static_cast<uint8_t>(std::clamp(static_cast<int>(std::lround(battery_state)), 0, 100));
  if (m_battery_level != battery_level) {
    m_battery_level = battery_level;
    m_composite_hid->setBatteryLevel(m_battery_level);
    m_dualsense->setBatteryLevel(m_battery_level);
  }
}

void Gamepad::update_charging_status() {
  if (m_charging_status_sensor == nullptr) {
    return;
  }

  const bool charging_status = m_charging_status_sensor->state;
  if (charging_status == m_charging_status_last) {
    return;
  }

  m_charging_status_last = charging_status;
  m_dualsense->setChargingStatus(charging_status);
}

void Gamepad::update_peripheral_status() {
  if (m_headphones_plugged_sensor != nullptr) {
    m_dualsense->setHeadphonesPlugged(m_headphones_plugged_sensor->state);
  }
  if (m_headphone_mic_sensor != nullptr) {
    m_dualsense->setHeadphoneMic(m_headphone_mic_sensor->state);
  }
  if (m_usb_plugged_sensor != nullptr) {
    m_dualsense->setUsbPlugged(m_usb_plugged_sensor->state);
  }
  if (m_mute_active_sensor != nullptr) {
    m_dualsense->setMuteActive(m_mute_active_sensor->state);
  }
}

int8_t Gamepad::update_axis(sensor::Sensor *sensor) {
  if (sensor == nullptr) {
    return 0;
  }

  const float sensor_state = sensor->state;
  if (std::isnan(sensor_state)) {
    return 0;
  }

  const float in_min = static_cast<float>(m_stick_axis_min);
  const float in_max = static_cast<float>(m_stick_axis_max);
  const float range = in_max - in_min;
  if (range == 0.0f) {
    return 0;
  }
  const float normalized = (sensor_state - in_min) / range;
  const float result = normalized * static_cast<float>(DUALSENSE_STICK_MAX - DUALSENSE_STICK_MIN) +
                       static_cast<float>(DUALSENSE_STICK_MIN);
  return static_cast<int8_t>(std::clamp(static_cast<int>(std::lround(result)), static_cast<int>(DUALSENSE_STICK_MIN),
                                        static_cast<int>(DUALSENSE_STICK_MAX)));
}

void Gamepad::update_thumbsticks() {
  m_dualsense->setLeftThumb(update_axis(m_left_thumb_x_sensor), update_axis(m_left_thumb_y_sensor));
  m_dualsense->setRightThumb(update_axis(m_right_thumb_x_sensor), update_axis(m_right_thumb_y_sensor));
}

uint8_t Gamepad::update_trigger(sensor::Sensor *sensor) {
  if (sensor == nullptr) {
    return 0;
  }

  const float sensor_state = sensor->state;
  if (std::isnan(sensor_state)) {
    return 0;
  }

  const float in_min = static_cast<float>(m_trigger_min);
  const float in_max = static_cast<float>(m_trigger_max);
  const float range = in_max - in_min;
  if (range == 0.0f) {
    return 0;
  }
  const float normalized = (sensor_state - in_min) / range;
  const float result = normalized * static_cast<float>(DUALSENSE_TRIGGER_MAX - DUALSENSE_TRIGGER_MIN) +
                       static_cast<float>(DUALSENSE_TRIGGER_MIN);
  return static_cast<uint8_t>(std::clamp(static_cast<int>(std::lround(result)), static_cast<int>(DUALSENSE_TRIGGER_MIN),
                                         static_cast<int>(DUALSENSE_TRIGGER_MAX)));
}

void Gamepad::update_left_trigger() {
  if (m_ls_button != nullptr) {
    if (m_ls_button->state) {
      m_dualsense->setLeftTrigger(DUALSENSE_TRIGGER_MAX);
    } else {
      m_dualsense->setLeftTrigger(DUALSENSE_TRIGGER_MIN);
    }
  }
  if (m_left_trigger_sensor != nullptr) {
    m_dualsense->setLeftTrigger(update_trigger(m_left_trigger_sensor));
  }
}

void Gamepad::update_right_trigger() {
  if (m_rs_button != nullptr) {
    if (m_rs_button->state) {
      m_dualsense->setRightTrigger(DUALSENSE_TRIGGER_MAX);
    } else {
      m_dualsense->setRightTrigger(DUALSENSE_TRIGGER_MIN);
    }
  }
  if (m_right_trigger_sensor != nullptr) {
    m_dualsense->setRightTrigger(update_trigger(m_right_trigger_sensor));
  }
}

bool Gamepad::update_motion_value(sensor::Sensor *sensor, int16_t *value, float scale) {
  if (sensor == nullptr || value == nullptr) {
    return false;
  }

  const float sensor_state = sensor->state;
  if (std::isnan(sensor_state)) {
    return false;
  }

  *value = static_cast<int16_t>(std::clamp(static_cast<int>(std::lround(sensor_state * scale)),
                                           static_cast<int>(std::numeric_limits<int16_t>::min()),
                                           static_cast<int>(std::numeric_limits<int16_t>::max())));
  return true;
}

void Gamepad::update_motion_inputs() {
  // Gyro sensors output deg/s; DualSense expects 1024 LSB per deg/s
  static constexpr float GYRO_SCALE = static_cast<float>(DUALSENSE_GYRO_RES_PER_DEG_S);
  // Accel sensors output m/s²; DualSense expects 8192 LSB per g
  static constexpr float ACCEL_SCALE = static_cast<float>(DUALSENSE_ACC_RES_PER_G) / 9.80665f;

  const bool has_gyro = m_yaw_sensor != nullptr || m_pitch_sensor != nullptr || m_roll_sensor != nullptr;
  const bool has_accel = m_ax_sensor != nullptr || m_ay_sensor != nullptr || m_az_sensor != nullptr;

  update_motion_value(m_yaw_sensor, &m_yaw, GYRO_SCALE);
  update_motion_value(m_pitch_sensor, &m_pitch, GYRO_SCALE);
  update_motion_value(m_roll_sensor, &m_roll, GYRO_SCALE);
  update_motion_value(m_ax_sensor, &m_ax, ACCEL_SCALE);
  update_motion_value(m_ay_sensor, &m_ay, ACCEL_SCALE);
  update_motion_value(m_az_sensor, &m_az, ACCEL_SCALE);

  if (has_gyro) {
    m_dualsense->setGyro(m_pitch, m_yaw, m_roll);
  }
  if (has_accel) {
    m_dualsense->setAccel(m_ax, m_ay, m_az);
  }
}

bool Gamepad::update_dpad() {
  const bool has_hat_input = m_dpad_up_button != nullptr || m_dpad_right_button != nullptr ||
                             m_dpad_down_button != nullptr || m_dpad_left_button != nullptr;
  if (!has_hat_input) {
    return false;
  }

  const bool up = (m_dpad_up_button != nullptr) && m_dpad_up_button->state;
  const bool right = (m_dpad_right_button != nullptr) && m_dpad_right_button->state;
  const bool down = (m_dpad_down_button != nullptr) && m_dpad_down_button->state;
  const bool left = (m_dpad_left_button != nullptr) && m_dpad_left_button->state;

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
