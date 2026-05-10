#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/sensor/sensor.h"
#ifdef GAMEPAD_USE_TOUCHSCREEN
#include "esphome/components/touchscreen/touchscreen.h"
#endif
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"

#include "companion_protocol.h"

namespace esphome {
namespace gamepad_companion {

class GamepadCompanion : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_a_button(binary_sensor::BinarySensor *button) { this->m_a_button = button; }
  void set_b_button(binary_sensor::BinarySensor *button) { this->m_b_button = button; }
  void set_x_button(binary_sensor::BinarySensor *button) { this->m_x_button = button; }
  void set_y_button(binary_sensor::BinarySensor *button) { this->m_y_button = button; }
  void set_lb_button(binary_sensor::BinarySensor *button) { this->m_lb_button = button; }
  void set_rb_button(binary_sensor::BinarySensor *button) { this->m_rb_button = button; }
  void set_ls_button(binary_sensor::BinarySensor *button) { this->m_ls_button = button; }
  void set_rs_button(binary_sensor::BinarySensor *button) { this->m_rs_button = button; }
  void set_l3_button(binary_sensor::BinarySensor *button) { this->m_l3_button = button; }
  void set_r3_button(binary_sensor::BinarySensor *button) { this->m_r3_button = button; }
  void set_share_button(binary_sensor::BinarySensor *button) { this->m_share_button = button; }
  void set_mute_button(binary_sensor::BinarySensor *button) { this->m_mute_button = button; }
  void set_l4_button(binary_sensor::BinarySensor *button) { this->m_l4_button = button; }
  void set_r4_button(binary_sensor::BinarySensor *button) { this->m_r4_button = button; }
  void set_l5_button(binary_sensor::BinarySensor *button) { this->m_l5_button = button; }
  void set_r5_button(binary_sensor::BinarySensor *button) { this->m_r5_button = button; }
  void set_mode_button(binary_sensor::BinarySensor *button) { this->m_mode_button = button; }
  void set_start_button(binary_sensor::BinarySensor *button) { this->m_start_button = button; }
  void set_select_button(binary_sensor::BinarySensor *button) { this->m_select_button = button; }
  void set_dpad_up_button(binary_sensor::BinarySensor *button) { this->m_dpad_up_button = button; }
  void set_dpad_right_button(binary_sensor::BinarySensor *button) { this->m_dpad_right_button = button; }
  void set_dpad_down_button(binary_sensor::BinarySensor *button) { this->m_dpad_down_button = button; }
  void set_dpad_left_button(binary_sensor::BinarySensor *button) { this->m_dpad_left_button = button; }
  void set_left_touch_sensor(binary_sensor::BinarySensor *sensor) { this->m_left_touch_sensor = sensor; }
#ifdef GAMEPAD_USE_TOUCHSCREEN
  void set_left_touchscreen(touchscreen::Touchscreen *ts) { this->m_left_touchscreen_ = ts; }
#endif
  void set_right_touch_sensor(binary_sensor::BinarySensor *sensor) { this->m_right_touch_sensor = sensor; }
#ifdef GAMEPAD_USE_TOUCHSCREEN
  void set_right_touchscreen(touchscreen::Touchscreen *ts) { this->m_right_touchscreen_ = ts; }
#endif

  void set_battery_level_sensor(sensor::Sensor *sensor) { this->m_battery_level_sensor = sensor; }
  void set_left_thumb_x_sensor(sensor::Sensor *sensor) { this->m_left_thumb_x_sensor = sensor; }
  void set_left_thumb_y_sensor(sensor::Sensor *sensor) { this->m_left_thumb_y_sensor = sensor; }
  void set_right_thumb_x_sensor(sensor::Sensor *sensor) { this->m_right_thumb_x_sensor = sensor; }
  void set_right_thumb_y_sensor(sensor::Sensor *sensor) { this->m_right_thumb_y_sensor = sensor; }
  void set_left_trigger_sensor(sensor::Sensor *sensor) { this->m_left_trigger_sensor = sensor; }
  void set_right_trigger_sensor(sensor::Sensor *sensor) { this->m_right_trigger_sensor = sensor; }
  void set_yaw_sensor(sensor::Sensor *sensor) { this->m_yaw_sensor = sensor; }
  void set_pitch_sensor(sensor::Sensor *sensor) { this->m_pitch_sensor = sensor; }
  void set_roll_sensor(sensor::Sensor *sensor) { this->m_roll_sensor = sensor; }
  void set_ax_sensor(sensor::Sensor *sensor) { this->m_ax_sensor = sensor; }
  void set_ay_sensor(sensor::Sensor *sensor) { this->m_ay_sensor = sensor; }
  void set_az_sensor(sensor::Sensor *sensor) { this->m_az_sensor = sensor; }
  void set_charging_status_sensor(binary_sensor::BinarySensor *sensor) { this->m_charging_status_sensor = sensor; }
  void set_headphones_plugged_sensor(binary_sensor::BinarySensor *sensor) {
    this->m_headphones_plugged_sensor = sensor;
  }
  void set_headphone_mic_sensor(binary_sensor::BinarySensor *sensor) { this->m_headphone_mic_sensor = sensor; }
  void set_usb_plugged_sensor(binary_sensor::BinarySensor *sensor) { this->m_usb_plugged_sensor = sensor; }
  void set_mute_active_sensor(binary_sensor::BinarySensor *sensor) { this->m_mute_active_sensor = sensor; }

  void set_left_touch_x_sensor(sensor::Sensor *sensor) { this->m_left_touch_x_sensor = sensor; }
  void set_left_touch_y_sensor(sensor::Sensor *sensor) { this->m_left_touch_y_sensor = sensor; }
  void set_left_touch_x_min(float value) { this->m_left_touch_x_min = value; }
  void set_left_touch_x_max(float value) { this->m_left_touch_x_max = value; }
  void set_left_touch_y_min(float value) { this->m_left_touch_y_min = value; }
  void set_left_touch_y_max(float value) { this->m_left_touch_y_max = value; }
  void set_right_touch_x_sensor(sensor::Sensor *sensor) { this->m_right_touch_x_sensor = sensor; }
  void set_right_touch_y_sensor(sensor::Sensor *sensor) { this->m_right_touch_y_sensor = sensor; }
  void set_right_touch_x_min(float value) { this->m_right_touch_x_min = value; }
  void set_right_touch_x_max(float value) { this->m_right_touch_x_max = value; }
  void set_right_touch_y_min(float value) { this->m_right_touch_y_min = value; }
  void set_right_touch_y_max(float value) { this->m_right_touch_y_max = value; }

  void set_lightbar_light(light::LightState *light) { this->m_lightbar_light = light; }
  void set_mute_light(light::LightState *light) { this->m_mute_light = light; }
  void set_mute_pulse_effect(const char *effect) { this->m_mute_pulse_effect = effect; }

  void set_rumble_weak_level_sensor(sensor::Sensor *sensor) { this->m_rumble_weak_level_sensor = sensor; }
  void set_rumble_strong_level_sensor(sensor::Sensor *sensor) { this->m_rumble_strong_level_sensor = sensor; }
  void set_rumble_weak_sensor(binary_sensor::BinarySensor *sensor) { this->m_rumble_weak_sensor = sensor; }
  void set_rumble_strong_sensor(binary_sensor::BinarySensor *sensor) { this->m_rumble_strong_sensor = sensor; }
  void set_player_number_sensor(sensor::Sensor *sensor) { this->m_player_number_sensor = sensor; }
  void set_left_trigger_effect_sensor(sensor::Sensor *sensor) { this->m_left_trigger_effect_sensor = sensor; }
  void set_right_trigger_effect_sensor(sensor::Sensor *sensor) { this->m_right_trigger_effect_sensor = sensor; }
  void set_left_trigger_effect_active_sensor(binary_sensor::BinarySensor *sensor) {
    this->m_left_trigger_effect_active_sensor = sensor;
  }
  void set_right_trigger_effect_active_sensor(binary_sensor::BinarySensor *sensor) {
    this->m_right_trigger_effect_active_sensor = sensor;
  }
  void set_headphone_volume_sensor(sensor::Sensor *sensor) { this->m_headphone_volume_sensor = sensor; }
  void set_speaker_volume_sensor(sensor::Sensor *sensor) { this->m_speaker_volume_sensor = sensor; }
  void set_mic_volume_sensor(sensor::Sensor *sensor) { this->m_mic_volume_sensor = sensor; }
  void set_led_brightness_sensor(sensor::Sensor *sensor) { this->m_led_brightness_sensor = sensor; }
  void set_mute_led_sensor(sensor::Sensor *sensor) { this->m_mute_led_sensor = sensor; }
  void set_lightbar_red_sensor(sensor::Sensor *sensor) { this->m_lightbar_red_sensor = sensor; }
  void set_lightbar_green_sensor(sensor::Sensor *sensor) { this->m_lightbar_green_sensor = sensor; }
  void set_lightbar_blue_sensor(sensor::Sensor *sensor) { this->m_lightbar_blue_sensor = sensor; }

  void set_left_trigger_zone_sensor(uint8_t index, sensor::Sensor *sensor);
  void set_right_trigger_zone_sensor(uint8_t index, sensor::Sensor *sensor);

 protected:
  bool has_input(uint8_t field) const;
  bool has_output(uint8_t field) const;
  void configure_supported_fields_();
  void update_input_state_();
  uint8_t get_battery_level_() const;
  uint8_t crc8_update(uint8_t crc, uint8_t byte);
  void send_cap_response_();
  void send_input_update_(bool all);
  void send_frame(uint8_t msg_type, const uint8_t *payload, uint16_t len);
  bool try_parse_frame();
  void dispatch_frame(uint8_t msg_type, const uint8_t *payload, uint16_t len);
  void handle_output_update_(const uint8_t *payload, uint16_t len);
  void publish_output_(uint8_t field, uint8_t value);
  void publish_lightbar_();
  void publish_mute_();

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
  binary_sensor::BinarySensor *m_left_touch_sensor{nullptr};
#ifdef GAMEPAD_USE_TOUCHSCREEN
  touchscreen::Touchscreen *m_left_touchscreen_{nullptr};
#endif
  binary_sensor::BinarySensor *m_right_touch_sensor{nullptr};
#ifdef GAMEPAD_USE_TOUCHSCREEN
  touchscreen::Touchscreen *m_right_touchscreen_{nullptr};
#endif

  sensor::Sensor *m_battery_level_sensor{nullptr};
  sensor::Sensor *m_left_thumb_x_sensor{nullptr};
  sensor::Sensor *m_left_thumb_y_sensor{nullptr};
  sensor::Sensor *m_right_thumb_x_sensor{nullptr};
  sensor::Sensor *m_right_thumb_y_sensor{nullptr};
  sensor::Sensor *m_left_trigger_sensor{nullptr};
  sensor::Sensor *m_right_trigger_sensor{nullptr};
  sensor::Sensor *m_yaw_sensor{nullptr};
  sensor::Sensor *m_pitch_sensor{nullptr};
  sensor::Sensor *m_roll_sensor{nullptr};
  sensor::Sensor *m_ax_sensor{nullptr};
  sensor::Sensor *m_ay_sensor{nullptr};
  sensor::Sensor *m_az_sensor{nullptr};
  binary_sensor::BinarySensor *m_charging_status_sensor{nullptr};
  binary_sensor::BinarySensor *m_headphones_plugged_sensor{nullptr};
  binary_sensor::BinarySensor *m_headphone_mic_sensor{nullptr};
  binary_sensor::BinarySensor *m_usb_plugged_sensor{nullptr};
  binary_sensor::BinarySensor *m_mute_active_sensor{nullptr};

  sensor::Sensor *m_left_touch_x_sensor{nullptr};
  sensor::Sensor *m_left_touch_y_sensor{nullptr};
  float m_left_touch_x_min{0.0f};
  float m_left_touch_x_max{0.0f};
  float m_left_touch_y_min{0.0f};
  float m_left_touch_y_max{0.0f};
  sensor::Sensor *m_right_touch_x_sensor{nullptr};
  sensor::Sensor *m_right_touch_y_sensor{nullptr};
  float m_right_touch_x_min{0.0f};
  float m_right_touch_x_max{0.0f};
  float m_right_touch_y_min{0.0f};
  float m_right_touch_y_max{0.0f};

  light::LightState *m_lightbar_light{nullptr};
  light::LightState *m_mute_light{nullptr};
  const char *m_mute_pulse_effect{nullptr};
  sensor::Sensor *m_rumble_weak_level_sensor{nullptr};
  sensor::Sensor *m_rumble_strong_level_sensor{nullptr};
  binary_sensor::BinarySensor *m_rumble_weak_sensor{nullptr};
  binary_sensor::BinarySensor *m_rumble_strong_sensor{nullptr};
  sensor::Sensor *m_player_number_sensor{nullptr};
  sensor::Sensor *m_left_trigger_effect_sensor{nullptr};
  sensor::Sensor *m_right_trigger_effect_sensor{nullptr};
  binary_sensor::BinarySensor *m_left_trigger_effect_active_sensor{nullptr};
  binary_sensor::BinarySensor *m_right_trigger_effect_active_sensor{nullptr};
  sensor::Sensor *m_headphone_volume_sensor{nullptr};
  sensor::Sensor *m_speaker_volume_sensor{nullptr};
  sensor::Sensor *m_mic_volume_sensor{nullptr};
  sensor::Sensor *m_led_brightness_sensor{nullptr};
  sensor::Sensor *m_mute_led_sensor{nullptr};
  sensor::Sensor *m_lightbar_red_sensor{nullptr};
  sensor::Sensor *m_lightbar_green_sensor{nullptr};
  sensor::Sensor *m_lightbar_blue_sensor{nullptr};
  sensor::Sensor *m_left_trigger_zone_sensors_[10]{};
  sensor::Sensor *m_right_trigger_zone_sensors_[10]{};

  uint64_t supported_inputs_{0};
  uint64_t supported_outputs_{0};
  uint32_t m_binary_state_{0};
  uint32_t m_last_binary_state_{0};
  float m_analog_state_[cproto::ANALOG_COUNT]{};
  float m_last_analog_state_[cproto::ANALOG_COUNT]{};
  uint8_t m_battery_level_{0};
  uint8_t m_last_battery_level_{0};
  bool m_last_sent_valid_{false};
  bool m_master_ready_{false};

  uint8_t m_lightbar_r_{0};
  uint8_t m_lightbar_g_{0};
  uint8_t m_lightbar_b_{0};
  uint8_t m_mute_led_{0};

  uint8_t m_rx_buffer_[cproto::MAX_FRAME_SIZE]{};
  size_t m_rx_length_{0};
};

}  // namespace gamepad_companion
}  // namespace esphome
