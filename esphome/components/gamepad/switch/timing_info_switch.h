#pragma once

#include "esphome/components/switch/switch.h"

namespace esphome {
namespace gamepad {

class Gamepad;

class TimingInfoSwitch : public switch_::Switch {
 public:
  explicit TimingInfoSwitch(Gamepad *parent) : parent_(parent) {}

 protected:
  void write_state(bool state) override;

  Gamepad *parent_;
};

}  // namespace gamepad
}  // namespace esphome
