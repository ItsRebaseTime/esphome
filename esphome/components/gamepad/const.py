"""Gamepad component const."""

# pylint: disable=line-too-long

from __future__ import annotations

from typing import Final

DOMAIN: Final = "gamepad"

# Button configuration constants
CONF_A_BUTTON: Final = "a_button"
CONF_B_BUTTON: Final = "b_button"
CONF_X_BUTTON: Final = "x_button"
CONF_Y_BUTTON: Final = "y_button"
CONF_LB_BUTTON: Final = "lb_button"
CONF_RB_BUTTON: Final = "rb_button"
CONF_LEFT_TRIGGER_BUTTON: Final = "left_trigger_button"
CONF_RIGHT_TRIGGER_BUTTON: Final = "right_trigger_button"
CONF_L3_BUTTON: Final = "l3_button"
CONF_R3_BUTTON: Final = "r3_button"
CONF_MODE_BUTTON: Final = "mode_button"
CONF_START_BUTTON: Final = "start_button"
CONF_SELECT_BUTTON: Final = "select_button"
CONF_ANDROID_LAYOUT: Final = "android_layout"
CONF_SHARE_BUTTON: Final = "share_button"
CONF_MUTE_BUTTON: Final = "mute_button"
CONF_L4_BUTTON: Final = "l4_button"
CONF_R4_BUTTON: Final = "r4_button"
CONF_L5_BUTTON: Final = "l5_button"
CONF_R5_BUTTON: Final = "r5_button"
CONF_DPAD_UP_BUTTON: Final = "dpad_up_button"
CONF_DPAD_RIGHT_BUTTON: Final = "dpad_right_button"
CONF_DPAD_DOWN_BUTTON: Final = "dpad_down_button"
CONF_DPAD_LEFT_BUTTON: Final = "dpad_left_button"
CONF_BATTERY_LEVEL_SENSOR: Final = "battery_level_sensor"
CONF_CHARGING_STATUS_SENSOR: Final = "charging_status_sensor"
CONF_LIGHTBAR_LIGHT: Final = "lightbar_light"
CONF_MUTE_LIGHT: Final = "mute_light"
CONF_MUTE_PULSE_EFFECT: Final = "mute_pulse_effect"

# Peripheral status sensors (binary sensor inputs → set status2 bits in input report)
CONF_HEADPHONES_PLUGGED_SENSOR: Final = "headphones_plugged_sensor"
CONF_HEADPHONE_MIC_SENSOR: Final = "headphone_mic_sensor"
CONF_USB_PLUGGED_SENSOR: Final = "usb_plugged_sensor"
CONF_MUTE_ACTIVE_SENSOR: Final = "mute_active_sensor"

# Thumbstick/trigger range configuration constants
CONF_STICK_AXIS_MIN: Final = "stick_axis_min"
CONF_STICK_AXIS_MAX: Final = "stick_axis_max"
CONF_TRIGGER_MIN: Final = "trigger_min"
CONF_TRIGGER_MAX: Final = "trigger_max"

CONF_GAMEPAD_ID: Final = "gamepad_id"

# Switch configuration constants
CONF_CHANNEL: Final = "channel"
CONF_TIMING_INFO: Final = "timing_info"

# Thumbstick/trigger sensor configuration constants
CONF_LEFT_THUMB_X_SENSOR: Final = "left_thumb_x_sensor"
CONF_LEFT_THUMB_Y_SENSOR: Final = "left_thumb_y_sensor"
CONF_RIGHT_THUMB_X_SENSOR: Final = "right_thumb_x_sensor"
CONF_RIGHT_THUMB_Y_SENSOR: Final = "right_thumb_y_sensor"
CONF_LEFT_TRIGGER_SENSOR: Final = "left_trigger_sensor"
CONF_RIGHT_TRIGGER_SENSOR: Final = "right_trigger_sensor"

# Touchpad configuration constants
CONF_TOUCHPAD_BUTTON: Final = "touchpad_button"
CONF_TOUCHPAD_SPLIT: Final = "touchpad_split"
CONF_TOUCH_SENSOR: Final = "touch_sensor"
CONF_TOUCH_X_SENSOR: Final = "touch_x_sensor"
CONF_TOUCH_Y_SENSOR: Final = "touch_y_sensor"
CONF_TOUCH_X_MIN: Final = "touch_x_min"
CONF_TOUCH_X_MAX: Final = "touch_x_max"
CONF_TOUCH_Y_MIN: Final = "touch_y_min"
CONF_TOUCH_Y_MAX: Final = "touch_y_max"
CONF_TOUCH2_SENSOR: Final = "touch2_sensor"
CONF_TOUCH2_X_SENSOR: Final = "touch2_x_sensor"
CONF_TOUCH2_Y_SENSOR: Final = "touch2_y_sensor"
CONF_TOUCH2_X_MIN: Final = "touch2_x_min"
CONF_TOUCH2_X_MAX: Final = "touch2_x_max"
CONF_TOUCH2_Y_MIN: Final = "touch2_y_min"
CONF_TOUCH2_Y_MAX: Final = "touch2_y_max"

# Motion control sensor configuration constants
CONF_YAW_SENSOR: Final = "yaw_sensor"
CONF_PITCH_SENSOR: Final = "pitch_sensor"
CONF_ROLL_SENSOR: Final = "roll_sensor"
CONF_AX_SENSOR: Final = "ax_sensor"
CONF_AY_SENSOR: Final = "ay_sensor"
CONF_AZ_SENSOR: Final = "az_sensor"

COMPONENT_CLASS: Final = "Gamepad"

"""Libraries"""
LIBS_DEFAULT: Final = [
    # ("ESP32 BLE Arduino", "1.0.1", None),
]

LIBS_ADDITIONAL: Final = [
    # (
    #     "h2zero/NimBLE-Arduino",
    #     "2.3.6",
    #     None,
    # ),
    (
        "ESP32-BLE-CompositeHID",
        None,
        "https://github.com/ItsRebaseTime/ESP32-BLE-CompositeHID.git#dualsense_input_output_field_info",
    ),
]

BUILD_FLAGS: Final = "-D USE_NIMBLE"
