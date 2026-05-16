#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#ifdef GAMEPAD_USE_TOUCHSCREEN
#include "esphome/components/touchscreen/touchscreen.h"
#endif
#include "companion_link.h"
#include <BleCompositeHID.h>
#include <memory>
#include <string>
#include "DualsenseGamepadDevice.h"

namespace esphome {
namespace gamepad {

class TimingInfoSwitch;

class Gamepad : public PollingComponent {
 public:
  Gamepad(std::string name, std::string manufacturer_id)
      : m_name(name), m_manufacturer_id(manufacturer_id), PollingComponent(1) {}
  ~Gamepad();

  void setup() override;
  void update() override;

  float get_setup_priority() const override { return setup_priority::AFTER_BLUETOOTH; }

  // Button setters
  void set_a_button(binary_sensor::BinarySensor *button) { m_a_button = button; }
  void set_b_button(binary_sensor::BinarySensor *button) { m_b_button = button; }
  void set_x_button(binary_sensor::BinarySensor *button) { m_x_button = button; }
  void set_y_button(binary_sensor::BinarySensor *button) { m_y_button = button; }
  void set_lb_button(binary_sensor::BinarySensor *button) { m_lb_button = button; }
  void set_rb_button(binary_sensor::BinarySensor *button) { m_rb_button = button; }
  void set_ls_button(binary_sensor::BinarySensor *button) { m_ls_button = button; }
  void set_rs_button(binary_sensor::BinarySensor *button) { m_rs_button = button; }
  void set_l3_button(binary_sensor::BinarySensor *button) { m_l3_button = button; }
  void set_r3_button(binary_sensor::BinarySensor *button) { m_r3_button = button; }
  void set_share_button(binary_sensor::BinarySensor *button) { m_share_button = button; }
  void set_mute_button(binary_sensor::BinarySensor *button) { m_mute_button = button; }
  void set_l4_button(binary_sensor::BinarySensor *button) { m_l4_button = button; }
  void set_r4_button(binary_sensor::BinarySensor *button) { m_r4_button = button; }
  void set_l5_button(binary_sensor::BinarySensor *button) { m_l5_button = button; }
  void set_r5_button(binary_sensor::BinarySensor *button) { m_r5_button = button; }
  void set_mode_button(binary_sensor::BinarySensor *button) { m_mode_button = button; }
  void set_start_button(binary_sensor::BinarySensor *button) { m_start_button = button; }
  void set_select_button(binary_sensor::BinarySensor *button) { m_select_button = button; }
  void set_dpad_up_button(binary_sensor::BinarySensor *button) { m_dpad_up_button = button; }
  void set_dpad_right_button(binary_sensor::BinarySensor *button) { m_dpad_right_button = button; }
  void set_dpad_down_button(binary_sensor::BinarySensor *button) { m_dpad_down_button = button; }
  void set_dpad_left_button(binary_sensor::BinarySensor *button) { m_dpad_left_button = button; }
  void set_battery_level_sensor(sensor::Sensor *sensor) { m_battery_level_sensor = sensor; }
  void set_charging_status_sensor(binary_sensor::BinarySensor *sensor) { m_charging_status_sensor = sensor; }
  void set_rumble_weak_sensor(binary_sensor::BinarySensor *sensor) { m_rumble_weak_sensor = sensor; }
  void set_rumble_strong_sensor(binary_sensor::BinarySensor *sensor) { m_rumble_strong_sensor = sensor; }
  void set_rumble_weak_level_sensor(sensor::Sensor *sensor) { m_rumble_weak_level_sensor = sensor; }
  void set_rumble_strong_level_sensor(sensor::Sensor *sensor) { m_rumble_strong_level_sensor = sensor; }
  void set_lightbar_light(light::LightState *light) { m_lightbar_light = light; }
  void set_mute_light(light::LightState *light) { this->m_mute_light_ = light; }
  void set_mute_pulse_effect(const char *name) { this->m_mute_pulse_effect_name_ = name; }
  void set_mute_output(binary_sensor::BinarySensor *sensor) { m_mute_output = sensor; }
  void set_player_number_sensor(sensor::Sensor *sensor) { m_player_number_sensor = sensor; }

  // Peripheral status setters (binary sensor inputs → set status2 bits in input report)
  void set_headphones_plugged_sensor(binary_sensor::BinarySensor *sensor) { m_headphones_plugged_sensor = sensor; }
  void set_headphone_mic_sensor(binary_sensor::BinarySensor *sensor) { m_headphone_mic_sensor = sensor; }
  void set_usb_plugged_sensor(binary_sensor::BinarySensor *sensor) { m_usb_plugged_sensor = sensor; }
  void set_mute_active_sensor(binary_sensor::BinarySensor *sensor) { m_mute_active_sensor = sensor; }

  // Host output feedback sensor setters (published when host sends output report)
  void set_left_trigger_effect_sensor(sensor::Sensor *sensor) { m_left_trigger_effect_sensor = sensor; }
  void set_right_trigger_effect_sensor(sensor::Sensor *sensor) { m_right_trigger_effect_sensor = sensor; }
  void set_left_trigger_zone_sensor_0(sensor::Sensor *s) { this->m_left_trigger_zone_sensors_[0] = s; }
  void set_left_trigger_zone_sensor_1(sensor::Sensor *s) { this->m_left_trigger_zone_sensors_[1] = s; }
  void set_left_trigger_zone_sensor_2(sensor::Sensor *s) { this->m_left_trigger_zone_sensors_[2] = s; }
  void set_left_trigger_zone_sensor_3(sensor::Sensor *s) { this->m_left_trigger_zone_sensors_[3] = s; }
  void set_left_trigger_zone_sensor_4(sensor::Sensor *s) { this->m_left_trigger_zone_sensors_[4] = s; }
  void set_left_trigger_zone_sensor_5(sensor::Sensor *s) { this->m_left_trigger_zone_sensors_[5] = s; }
  void set_left_trigger_zone_sensor_6(sensor::Sensor *s) { this->m_left_trigger_zone_sensors_[6] = s; }
  void set_left_trigger_zone_sensor_7(sensor::Sensor *s) { this->m_left_trigger_zone_sensors_[7] = s; }
  void set_left_trigger_zone_sensor_8(sensor::Sensor *s) { this->m_left_trigger_zone_sensors_[8] = s; }
  void set_left_trigger_zone_sensor_9(sensor::Sensor *s) { this->m_left_trigger_zone_sensors_[9] = s; }
  void set_right_trigger_zone_sensor_0(sensor::Sensor *s) { this->m_right_trigger_zone_sensors_[0] = s; }
  void set_right_trigger_zone_sensor_1(sensor::Sensor *s) { this->m_right_trigger_zone_sensors_[1] = s; }
  void set_right_trigger_zone_sensor_2(sensor::Sensor *s) { this->m_right_trigger_zone_sensors_[2] = s; }
  void set_right_trigger_zone_sensor_3(sensor::Sensor *s) { this->m_right_trigger_zone_sensors_[3] = s; }
  void set_right_trigger_zone_sensor_4(sensor::Sensor *s) { this->m_right_trigger_zone_sensors_[4] = s; }
  void set_right_trigger_zone_sensor_5(sensor::Sensor *s) { this->m_right_trigger_zone_sensors_[5] = s; }
  void set_right_trigger_zone_sensor_6(sensor::Sensor *s) { this->m_right_trigger_zone_sensors_[6] = s; }
  void set_right_trigger_zone_sensor_7(sensor::Sensor *s) { this->m_right_trigger_zone_sensors_[7] = s; }
  void set_right_trigger_zone_sensor_8(sensor::Sensor *s) { this->m_right_trigger_zone_sensors_[8] = s; }
  void set_right_trigger_zone_sensor_9(sensor::Sensor *s) { this->m_right_trigger_zone_sensors_[9] = s; }
  void set_left_trigger_effect_active_sensor(binary_sensor::BinarySensor *sensor) {
    m_left_trigger_effect_active_sensor = sensor;
  }
  void set_right_trigger_effect_active_sensor(binary_sensor::BinarySensor *sensor) {
    m_right_trigger_effect_active_sensor = sensor;
  }
  void set_headphone_volume_sensor(sensor::Sensor *sensor) { m_headphone_volume_sensor = sensor; }
  void set_speaker_volume_sensor(sensor::Sensor *sensor) { m_speaker_volume_sensor = sensor; }
  void set_mic_volume_sensor(sensor::Sensor *sensor) { m_mic_volume_sensor = sensor; }
  void set_led_brightness_sensor(sensor::Sensor *sensor) { m_led_brightness_sensor = sensor; }
  void set_left_thumb_x_sensor(sensor::Sensor *sensor) { m_left_thumb_x_sensor = sensor; }
  void set_left_thumb_y_sensor(sensor::Sensor *sensor) { m_left_thumb_y_sensor = sensor; }
  void set_right_thumb_x_sensor(sensor::Sensor *sensor) { m_right_thumb_x_sensor = sensor; }
  void set_right_thumb_y_sensor(sensor::Sensor *sensor) { m_right_thumb_y_sensor = sensor; }
  void set_left_trigger_sensor(sensor::Sensor *sensor) { m_left_trigger_sensor = sensor; }
  void set_right_trigger_sensor(sensor::Sensor *sensor) { m_right_trigger_sensor = sensor; }
  void set_yaw_sensor(sensor::Sensor *sensor) { m_yaw_sensor = sensor; }
  void set_pitch_sensor(sensor::Sensor *sensor) { m_pitch_sensor = sensor; }
  void set_roll_sensor(sensor::Sensor *sensor) { m_roll_sensor = sensor; }
  void set_ax_sensor(sensor::Sensor *sensor) { m_ax_sensor = sensor; }
  void set_ay_sensor(sensor::Sensor *sensor) { m_ay_sensor = sensor; }
  void set_az_sensor(sensor::Sensor *sensor) { m_az_sensor = sensor; }

  // Touchpad setters
  void set_touchpad_button(binary_sensor::BinarySensor *button) { m_touchpad_button = button; }
  void set_left_touch_sensor(binary_sensor::BinarySensor *s) { m_left_touch_sensor = s; }
#ifdef GAMEPAD_USE_TOUCHSCREEN
  void set_left_touchscreen(touchscreen::Touchscreen *ts) { m_left_touchscreen_ = ts; }
#endif
  void set_left_touch_x_sensor(sensor::Sensor *s) { m_left_touch_x_sensor = s; }
  void set_left_touch_y_sensor(sensor::Sensor *s) { m_left_touch_y_sensor = s; }
  void set_left_touch_x_min(float v) { m_left_touch_x_min = v; }
  void set_left_touch_x_max(float v) { m_left_touch_x_max = v; }
  void set_left_touch_y_min(float v) { m_left_touch_y_min = v; }
  void set_left_touch_y_max(float v) { m_left_touch_y_max = v; }
  void set_right_touch_sensor(binary_sensor::BinarySensor *s) { m_right_touch_sensor = s; }
#ifdef GAMEPAD_USE_TOUCHSCREEN
  void set_right_touchscreen(touchscreen::Touchscreen *ts) { m_right_touchscreen_ = ts; }
#endif
  void set_right_touch_x_sensor(sensor::Sensor *s) { m_right_touch_x_sensor = s; }
  void set_right_touch_y_sensor(sensor::Sensor *s) { m_right_touch_y_sensor = s; }
  void set_right_touch_x_min(float v) { m_right_touch_x_min = v; }
  void set_right_touch_x_max(float v) { m_right_touch_x_max = v; }
  void set_right_touch_y_min(float v) { m_right_touch_y_min = v; }
  void set_right_touch_y_max(float v) { m_right_touch_y_max = v; }

  // Range setters
  void set_stick_axis_min(int16_t min) { m_stick_axis_min = min; }
  void set_stick_axis_max(int16_t max) { m_stick_axis_max = max; }
  void set_trigger_min(int16_t min) { m_trigger_min = min; }
  void set_trigger_max(int16_t max) { m_trigger_max = max; }
  void set_timing_info(bool enable) { m_timing_info = enable; }
  void set_timing_info_switch(switch_::Switch *sw) { this->m_timing_info_switch = sw; }
#ifdef GAMEPAD_USE_COMPANION_UART
  void set_companion_uart(uart::UARTComponent *uart) { this->m_companion_ = new GamepadCompanionLink(uart); }
#endif
  void handle_feedback(DualsenseGamepadOutputReportData data);
  void start();
  void stop();

 protected:
  // Button pointers
  binary_sensor::BinarySensor *m_a_button{nullptr};
  binary_sensor::BinarySensor *m_b_button{nullptr};
  binary_sensor::BinarySensor *m_x_button{nullptr};
  binary_sensor::BinarySensor *m_y_button{nullptr};
  binary_sensor::BinarySensor *m_lb_button{nullptr};
  binary_sensor::BinarySensor *m_rb_button{nullptr};
  binary_sensor::BinarySensor *m_ls_button{nullptr};
  binary_sensor::BinarySensor *m_rs_button{nullptr};
  binary_sensor::BinarySensor *m_l3_button{nullptr};
  binary_sensor::BinarySensor *m_r3_button{nullptr};
  binary_sensor::BinarySensor *m_share_button{nullptr};
  binary_sensor::BinarySensor *m_mute_button{nullptr};
  binary_sensor::BinarySensor *m_l4_button{nullptr};
  binary_sensor::BinarySensor *m_r4_button{nullptr};
  binary_sensor::BinarySensor *m_l5_button{nullptr};
  binary_sensor::BinarySensor *m_r5_button{nullptr};
  binary_sensor::BinarySensor *m_mode_button{nullptr};
  binary_sensor::BinarySensor *m_start_button{nullptr};
  binary_sensor::BinarySensor *m_select_button{nullptr};
  binary_sensor::BinarySensor *m_dpad_up_button{nullptr};
  binary_sensor::BinarySensor *m_dpad_right_button{nullptr};
  binary_sensor::BinarySensor *m_dpad_down_button{nullptr};
  binary_sensor::BinarySensor *m_dpad_left_button{nullptr};

  uint8_t m_hat_state{DUALSENSE_BUTTON_DPAD_NONE};

  sensor::Sensor *m_battery_level_sensor{nullptr};
  uint8_t m_battery_level{100};
  binary_sensor::BinarySensor *m_charging_status_sensor{nullptr};
  bool m_charging_status_last{false};

  binary_sensor::BinarySensor *m_rumble_weak_sensor{nullptr};
  binary_sensor::BinarySensor *m_rumble_strong_sensor{nullptr};
  sensor::Sensor *m_rumble_weak_level_sensor{nullptr};
  sensor::Sensor *m_rumble_strong_level_sensor{nullptr};
  light::LightState *m_lightbar_light{nullptr};
  binary_sensor::BinarySensor *m_mute_output{nullptr};
  sensor::Sensor *m_player_number_sensor{nullptr};

  // Peripheral status inputs (binary sensors driving status2 bits in input report)
  binary_sensor::BinarySensor *m_headphones_plugged_sensor{nullptr};
  binary_sensor::BinarySensor *m_headphone_mic_sensor{nullptr};
  binary_sensor::BinarySensor *m_usb_plugged_sensor{nullptr};
  binary_sensor::BinarySensor *m_mute_active_sensor{nullptr};

  // Host output feedback sensors
  sensor::Sensor *m_left_trigger_effect_sensor{nullptr};
  sensor::Sensor *m_right_trigger_effect_sensor{nullptr};
  binary_sensor::BinarySensor *m_left_trigger_effect_active_sensor{nullptr};
  binary_sensor::BinarySensor *m_right_trigger_effect_active_sensor{nullptr};
  sensor::Sensor *m_left_trigger_zone_sensors_[10]{};
  sensor::Sensor *m_right_trigger_zone_sensors_[10]{};
  uint8_t m_left_trigger_zones_last_[10]{};
  uint8_t m_right_trigger_zones_last_[10]{};
  sensor::Sensor *m_headphone_volume_sensor{nullptr};
  sensor::Sensor *m_speaker_volume_sensor{nullptr};
  sensor::Sensor *m_mic_volume_sensor{nullptr};
  sensor::Sensor *m_led_brightness_sensor{nullptr};

  sensor::Sensor *m_left_thumb_x_sensor{nullptr};
  sensor::Sensor *m_left_thumb_y_sensor{nullptr};
  sensor::Sensor *m_right_thumb_x_sensor{nullptr};
  sensor::Sensor *m_right_thumb_y_sensor{nullptr};
  sensor::Sensor *m_left_trigger_sensor{nullptr};
  sensor::Sensor *m_right_trigger_sensor{nullptr};

  // Configurable axis ranges
  int16_t m_stick_axis_min{-4095};
  int16_t m_stick_axis_max{4095};
  int16_t m_trigger_min{0};
  int16_t m_trigger_max{4095};

  // Touchpad members — left and right physical touchpads combined into 2 DualSense touch points
  binary_sensor::BinarySensor *m_touchpad_button{nullptr};
  binary_sensor::BinarySensor *m_left_touch_sensor{nullptr};
#ifdef GAMEPAD_USE_TOUCHSCREEN
  touchscreen::Touchscreen *m_left_touchscreen_{nullptr};
#endif
  sensor::Sensor *m_left_touch_x_sensor{nullptr};
  sensor::Sensor *m_left_touch_y_sensor{nullptr};
  float m_left_touch_x_min{0.0f};
  float m_left_touch_x_max{1919.0f};
  float m_left_touch_y_min{0.0f};
  float m_left_touch_y_max{1079.0f};
  bool m_left_touch_was_active{false};
  int8_t m_left_touch_id{-1};
  binary_sensor::BinarySensor *m_right_touch_sensor{nullptr};
#ifdef GAMEPAD_USE_TOUCHSCREEN
  touchscreen::Touchscreen *m_right_touchscreen_{nullptr};
#endif
  sensor::Sensor *m_right_touch_x_sensor{nullptr};
  sensor::Sensor *m_right_touch_y_sensor{nullptr};
  float m_right_touch_x_min{0.0f};
  float m_right_touch_x_max{1919.0f};
  float m_right_touch_y_min{0.0f};
  float m_right_touch_y_max{1079.0f};
  bool m_right_touch_was_active{false};
  int8_t m_right_touch_id{-1};

  sensor::Sensor *m_yaw_sensor{nullptr};
  sensor::Sensor *m_pitch_sensor{nullptr};
  sensor::Sensor *m_roll_sensor{nullptr};
  sensor::Sensor *m_ax_sensor{nullptr};
  sensor::Sensor *m_ay_sensor{nullptr};
  sensor::Sensor *m_az_sensor{nullptr};
  switch_::Switch *m_timing_info_switch{nullptr};

  int16_t m_yaw{0};
  int16_t m_pitch{0};
  int16_t m_roll{0};
  int16_t m_ax{0};
  int16_t m_ay{0};
  int16_t m_az{0};
  bool m_timing_info{false};
  GamepadCompanionLink *m_companion_{nullptr};

 private:
  void handle_all_buttons();
  bool is_connected();
  void update_battery_level();
  void update_charging_status();
  void update_peripheral_status();
  int8_t update_axis(sensor::Sensor *sensor, uint8_t companion_field);
  void update_thumbsticks();
  bool update_dpad();
  void update_all_triggers();
  uint8_t update_trigger(sensor::Sensor *sensor, uint8_t companion_field);
  void update_left_trigger();
  void update_right_trigger();
  bool update_motion_value(sensor::Sensor *sensor, int16_t *value, float scale, uint8_t companion_field);
  void update_motion_inputs();
  void update_touchpad();
  void update_touchpad_contact(binary_sensor::BinarySensor *touch_sensor, sensor::Sensor *x_sensor,
                               sensor::Sensor *y_sensor, float x_min, float x_max, float y_min, float y_max,
                               bool &was_active, int8_t &touch_id, uint16_t x_out_min, uint16_t x_out_max,
                               uint8_t touch_field, uint8_t x_field, uint8_t y_field);
#ifdef GAMEPAD_USE_TOUCHSCREEN
  void update_touchpad_contact_from_touchscreen(touchscreen::Touchscreen *ts, float x_min, float x_max, float y_min,
                                                float y_max, bool &was_active, int8_t &touch_id, uint16_t x_out_min,
                                                uint16_t x_out_max);
#endif
  uint16_t scale_touchpad_axis(sensor::Sensor *sensor, float input_min, float input_max, uint16_t output_min,
                               uint16_t output_max);
  uint16_t scale_touchpad_raw(float value, float input_min, float input_max, uint16_t output_min, uint16_t output_max);
  void handle_button(binary_sensor::BinarySensor *button, uint32_t input_button, uint8_t companion_field = 0xFF);
  bool companion_binary(uint8_t field) const;
  float companion_float(uint8_t field) const;
  void send_companion_output(const DualsenseGamepadOutputReportData &data);

  std::string m_name;
  std::string m_manufacturer_id;
  BleCompositeHID *m_composite_hid;
  GamepadDevice *m_gamepad;
  DualsenseGamepadDevice *m_dualsense;
  BLEHostConfiguration m_host_config;
  DualsenseEdgeControllerDeviceConfiguration *m_dualsense_config;
  bool m_temp_state = false;
  bool m_started = false;
  bool m_init_sent = false;
  uint32_t m_last_report_time = 0;
  uint32_t m_now = 0;
  float counter = 0.0f;

  // Feedback report deferred from BLE callback
  bool m_feedback_pending_{false};
  DualsenseGamepadOutputReportData m_feedback_data_{};

  // Mute LED light (component controls on/off/pulse; user controls colour)
  light::LightState *m_mute_light_{nullptr};
  const char *m_mute_pulse_effect_name_{nullptr};
  uint8_t m_mute_led_mode_{0};  // 0=off, 1=solid, 2=pulsing (raw mute_button_led byte)
  uint8_t m_mute_led_mode_last_{0};
  bool m_mute_light_pending_{false};

  // Lightbar state deferred from BLE callback
  bool m_lightbar_state_pending{false};
  bool m_lightbar_state{false};
  uint8_t m_lightbar_red{0};
  uint8_t m_lightbar_green{0};
  uint8_t m_lightbar_blue{0};
  bool m_lightbar_state_last{false};
  uint8_t m_lightbar_red_last{255};
  uint8_t m_lightbar_green_last{255};
  uint8_t m_lightbar_blue_last{255};
  void process_pending_feedback();
  void update_mute_light();
  void update_lightbar_light();
};
}  // namespace gamepad
}  // namespace esphome
