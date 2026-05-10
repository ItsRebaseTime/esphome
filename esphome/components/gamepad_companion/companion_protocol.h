#pragma once

#include <cstdint>

namespace esphome {
namespace gamepad_companion {

namespace cproto {
static constexpr uint8_t SYNC_1 = 0xAA;
static constexpr uint8_t SYNC_2 = 0x55;

static constexpr uint8_t MSG_CAP_REQUEST = 0x01;
static constexpr uint8_t MSG_STATE_REQUEST = 0x02;
static constexpr uint8_t MSG_OUTPUT_UPDATE = 0x03;
static constexpr uint8_t MSG_CAP_RESPONSE = 0x81;
static constexpr uint8_t MSG_INPUT_UPDATE = 0x82;

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

static constexpr uint8_t IN_BATTERY = 32;
static constexpr uint8_t IN_LEFT_THUMB_X = 33;
static constexpr uint8_t IN_LEFT_THUMB_Y = 34;
static constexpr uint8_t IN_RIGHT_THUMB_X = 35;
static constexpr uint8_t IN_RIGHT_THUMB_Y = 36;
static constexpr uint8_t IN_LEFT_TRIGGER = 37;
static constexpr uint8_t IN_RIGHT_TRIGGER = 38;
static constexpr uint8_t IN_YAW = 39;
static constexpr uint8_t IN_PITCH = 40;
static constexpr uint8_t IN_ROLL = 41;
static constexpr uint8_t IN_AX = 42;
static constexpr uint8_t IN_AY = 43;
static constexpr uint8_t IN_AZ = 44;
static constexpr uint8_t IN_LEFT_TOUCH_X = 45;
static constexpr uint8_t IN_LEFT_TOUCH_Y = 46;
static constexpr uint8_t IN_RIGHT_TOUCH_X = 47;
static constexpr uint8_t IN_RIGHT_TOUCH_Y = 48;

static constexpr uint8_t ANALOG_OFFSET = 32;
static constexpr uint8_t ANALOG_COUNT = 17;

static constexpr uint8_t OUT_RUMBLE_WEAK_LEVEL = 0;
static constexpr uint8_t OUT_RUMBLE_STRONG_LEVEL = 1;
static constexpr uint8_t OUT_LIGHTBAR_R = 2;
static constexpr uint8_t OUT_LIGHTBAR_G = 3;
static constexpr uint8_t OUT_LIGHTBAR_B = 4;
static constexpr uint8_t OUT_MUTE_LED = 5;
static constexpr uint8_t OUT_PLAYER_LEDS = 6;
static constexpr uint8_t OUT_LEFT_TRIGGER_EFFECT = 7;
static constexpr uint8_t OUT_RIGHT_TRIGGER_EFFECT = 8;
static constexpr uint8_t OUT_HEADPHONE_VOL = 9;
static constexpr uint8_t OUT_SPEAKER_VOL = 10;
static constexpr uint8_t OUT_MIC_VOL = 11;
static constexpr uint8_t OUT_LED_BRIGHTNESS = 12;
static constexpr uint8_t OUT_LEFT_ZONE_BASE = 13;
static constexpr uint8_t OUT_RIGHT_ZONE_BASE = 23;
static constexpr uint8_t OUTPUT_FIELD_COUNT = 33;
static constexpr uint8_t OUTPUT_MASK_BYTES = 5;
static constexpr uint8_t MAX_FRAME_SIZE = 512;
}  // namespace cproto

}  // namespace gamepad_companion
}  // namespace esphome
