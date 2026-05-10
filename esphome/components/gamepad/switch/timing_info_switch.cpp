#include "timing_info_switch.h"
#include "../gamepad.h"

namespace esphome {
namespace gamepad {

void TimingInfoSwitch::write_state(bool state) {
  this->parent_->set_timing_info(state);
  this->publish_state(state);
}

}  // namespace gamepad
}  // namespace esphome
