#include "chsc6x_touchscreen.h"

namespace esphome {
namespace chsc6x {

static const char *const TAG = "chsc6x.touchscreen";

bool CHSC6XTouchscreen::initialize_() {
  if (this->write(nullptr, 0) != i2c::ERROR_OK) {
    return false;
  }

  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->setup();
    this->attach_interrupt_(this->interrupt_pin_, gpio::INTERRUPT_FALLING_EDGE);
  }
  this->write_byte(CHSC6X_REG_INT_MODE, CHSC6X_INT_MODE_ENABLE);

  if (this->x_raw_max_ == this->x_raw_min_) {
    this->x_raw_max_ = this->display_->get_native_width();
  }
  if (this->y_raw_max_ == this->y_raw_min_) {
    this->y_raw_max_ = this->display_->get_native_height();
  }

  this->initialized_ = true;
  return true;
}

void CHSC6XTouchscreen::setup() {
  if (!this->initialize_()) {
    ESP_LOGW(TAG, "CHSC6X not found on I2C bus (address 0x%02X), will retry", this->address_);
  }
}

void CHSC6XTouchscreen::update_touches() {
  if (!this->initialized_) {
    if (!this->initialize_()) {
      return;
    }
    ESP_LOGI(TAG, "CHSC6X initialized");
  }

  uint8_t data[CHSC6X_DATA_LEN];
  if (!this->read_bytes(CHSC6X_REG_POINT_DATA, data, sizeof(data))) {
    ESP_LOGW(TAG, "Failed to read touch data");
    return;
  }

  uint8_t num_of_touches = data[0];
  if (num_of_touches == 0 || num_of_touches > CHSC6X_MAX_POINTS) {
    return;
  }

  uint16_t x0 = ((data[1] << 8) | data[2]) & 0x0FFF;
  uint16_t y0 = ((data[3] << 8) | data[4]) & 0x0FFF;
  this->add_raw_touch_position_(0, x0, y0);

  if (num_of_touches == 2) {
    uint16_t x1 = ((data[7] << 8) | data[8]) & 0x0FFF;
    uint16_t y1 = ((data[9] << 8) | data[10]) & 0x0FFF;
    this->add_raw_touch_position_(1, x1, y1);
  }
}

void CHSC6XTouchscreen::dump_config() {
  ESP_LOGCONFIG(TAG, "CHSC6X Touchscreen:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
}

}  // namespace chsc6x
}  // namespace esphome
