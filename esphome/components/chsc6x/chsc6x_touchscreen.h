#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace chsc6x {

// Register 0x02 is the start of the touch data block:
//   [0]      = number of active touch points (0, 1, or 2)
//   [1..4]   = point 0: X-high, X-low, Y-high, Y-low (12-bit each, masked with 0x0FFF)
//   [5..6]   = reserved (weight/size, always 0)
//   [7..10]  = point 1: X-high, X-low, Y-high, Y-low
static const uint8_t CHSC6X_REG_POINT_DATA = 0x02;
static const uint8_t CHSC6X_DATA_LEN = 11;
static const uint8_t CHSC6X_MAX_POINTS = 2;

// Writing 0x5A to register 0x5A enables hardware interrupt mode.
static const uint8_t CHSC6X_REG_INT_MODE = 0x5A;
static const uint8_t CHSC6X_INT_MODE_ENABLE = 0x5A;

class CHSC6XTouchscreen : public touchscreen::Touchscreen, public i2c::I2CDevice {
 public:
  void setup() override;
  void update_touches() override;
  void dump_config() override;

  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }

 protected:
  InternalGPIOPin *interrupt_pin_{};
};

}  // namespace chsc6x
}  // namespace esphome
