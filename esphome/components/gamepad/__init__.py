"""Gamepad component."""

from __future__ import annotations

from typing import Final

import esphome.codegen as cg
from esphome.components import (
    binary_sensor,
    light,
    sensor,
    touchscreen as touchscreen_component,
    uart,
)

# from esphome.components.esp32 import VARIANT_ESP32, get_esp32_variant
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_MANUFACTURER_ID, CONF_NAME, CONF_SETUP_PRIORITY
from esphome.core import CORE

from .const import (
    BUILD_FLAGS,
    COMPONENT_CLASS,
    CONF_A_BUTTON,
    CONF_AX_SENSOR,
    CONF_AY_SENSOR,
    CONF_AZ_SENSOR,
    CONF_B_BUTTON,
    CONF_BATTERY_LEVEL_SENSOR,
    CONF_CHARGING_STATUS_SENSOR,
    CONF_COMPANION_UART,
    CONF_DPAD_DOWN_BUTTON,
    CONF_DPAD_LEFT_BUTTON,
    CONF_DPAD_RIGHT_BUTTON,
    CONF_DPAD_UP_BUTTON,
    CONF_HEADPHONE_MIC_SENSOR,
    CONF_HEADPHONES_PLUGGED_SENSOR,
    CONF_L3_BUTTON,
    CONF_L4_BUTTON,
    CONF_L5_BUTTON,
    CONF_LB_BUTTON,
    CONF_LEFT_THUMB_X_SENSOR,
    CONF_LEFT_THUMB_Y_SENSOR,
    CONF_LEFT_TOUCHPAD,
    CONF_LEFT_TRIGGER_BUTTON,
    CONF_LEFT_TRIGGER_SENSOR,
    CONF_LIGHTBAR_LIGHT,
    CONF_MODE_BUTTON,
    CONF_MUTE_ACTIVE_SENSOR,
    CONF_MUTE_BUTTON,
    CONF_MUTE_LIGHT,
    CONF_MUTE_PULSE_EFFECT,
    CONF_PITCH_SENSOR,
    CONF_R3_BUTTON,
    CONF_R4_BUTTON,
    CONF_R5_BUTTON,
    CONF_RB_BUTTON,
    CONF_RIGHT_THUMB_X_SENSOR,
    CONF_RIGHT_THUMB_Y_SENSOR,
    CONF_RIGHT_TOUCHPAD,
    CONF_RIGHT_TRIGGER_BUTTON,
    CONF_RIGHT_TRIGGER_SENSOR,
    CONF_ROLL_SENSOR,
    CONF_SELECT_BUTTON,
    CONF_SHARE_BUTTON,
    CONF_START_BUTTON,
    CONF_STICK_AXIS_MAX,
    CONF_STICK_AXIS_MIN,
    CONF_TOUCHPAD_BUTTON,
    CONF_TOUCHPAD_SENSOR,
    CONF_TOUCHPAD_TOUCHSCREEN,
    CONF_TOUCHPAD_X_MAX,
    CONF_TOUCHPAD_X_MIN,
    CONF_TOUCHPAD_X_SENSOR,
    CONF_TOUCHPAD_Y_MAX,
    CONF_TOUCHPAD_Y_MIN,
    CONF_TOUCHPAD_Y_SENSOR,
    CONF_TRIGGER_MAX,
    CONF_TRIGGER_MIN,
    CONF_USB_PLUGGED_SENSOR,
    CONF_X_BUTTON,
    CONF_Y_BUTTON,
    CONF_YAW_SENSOR,
    DOMAIN,
    LIBS_ADDITIONAL,
)

CODEOWNERS: Final = ["@ItsRebaseTime"]
AUTO_LOAD: Final = ["binary_sensor", "sensor", "number", "button", "light", "switch"]
DEPENDENCIES: Final = ["uart"]

gamepad_ns = cg.esphome_ns.namespace(DOMAIN)

Gamepad = gamepad_ns.class_(COMPONENT_CLASS, cg.PollingComponent)
GamepadDeviceType = gamepad_ns.enum("GamepadDeviceType")

DPAD_BUTTON_KEYS: Final = (
    CONF_DPAD_UP_BUTTON,
    CONF_DPAD_RIGHT_BUTTON,
    CONF_DPAD_DOWN_BUTTON,
    CONF_DPAD_LEFT_BUTTON,
)


def _validate_dpad(config: dict) -> dict:
    defined_dpad_buttons = [key for key in DPAD_BUTTON_KEYS if key in config]
    if defined_dpad_buttons and len(defined_dpad_buttons) != len(DPAD_BUTTON_KEYS):
        missing_dpad_buttons = [key for key in DPAD_BUTTON_KEYS if key not in config]
        raise cv.Invalid(
            "D-pad buttons must be configured together; missing: "
            + ", ".join(missing_dpad_buttons)
        )
    return config


def _validate_trigger_sources(config: dict) -> dict:
    if CONF_LEFT_TRIGGER_BUTTON in config and CONF_LEFT_TRIGGER_SENSOR in config:
        raise cv.Invalid(
            "Only one of left_trigger_button or left_trigger_sensor can be configured"
        )
    if CONF_RIGHT_TRIGGER_BUTTON in config and CONF_RIGHT_TRIGGER_SENSOR in config:
        raise cv.Invalid(
            "Only one of right_trigger_button or right_trigger_sensor can be configured"
        )
    return config


def _validate_touchpad_block(block: dict, label: str) -> dict:
    if block[CONF_TOUCHPAD_X_MIN] >= block[CONF_TOUCHPAD_X_MAX]:
        raise cv.Invalid(f"{label}: x_min must be less than x_max")
    if block[CONF_TOUCHPAD_Y_MIN] >= block[CONF_TOUCHPAD_Y_MAX]:
        raise cv.Invalid(f"{label}: y_min must be less than y_max")
    return block


def _validate_touchpad(config: dict) -> dict:
    if CONF_LEFT_TOUCHPAD in config:
        _validate_touchpad_block(config[CONF_LEFT_TOUCHPAD], "left_touchpad")
    if CONF_RIGHT_TOUCHPAD in config:
        _validate_touchpad_block(config[CONF_RIGHT_TOUCHPAD], "right_touchpad")
    return config


def _validate_touchpad_source(block: dict) -> dict:
    has_sensor = CONF_TOUCHPAD_SENSOR in block
    has_touchscreen = CONF_TOUCHPAD_TOUCHSCREEN in block
    if has_sensor and has_touchscreen:
        raise cv.Invalid(
            "Only one of 'sensor' or 'touchscreen' may be configured for a touchpad"
        )
    if not has_sensor and not has_touchscreen:
        raise cv.Invalid(
            "Either 'sensor' or 'touchscreen' must be configured for a touchpad"
        )
    return block


TOUCHPAD_SCHEMA: Final = cv.All(
    cv.Schema(
        {
            cv.Optional(CONF_TOUCHPAD_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_TOUCHPAD_TOUCHSCREEN): cv.use_id(
                touchscreen_component.Touchscreen
            ),
            cv.Optional(CONF_TOUCHPAD_X_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_TOUCHPAD_Y_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_TOUCHPAD_X_MIN, default=0.0): cv.float_,
            cv.Optional(CONF_TOUCHPAD_X_MAX, default=1919.0): cv.float_,
            cv.Optional(CONF_TOUCHPAD_Y_MIN, default=0.0): cv.float_,
            cv.Optional(CONF_TOUCHPAD_Y_MAX, default=1079.0): cv.float_,
        }
    ),
    _validate_touchpad_source,
)


CONFIG_SCHEMA: Final = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Gamepad),
            cv.Optional(CONF_NAME, default=COMPONENT_CLASS): cv.Length(min=1),
            cv.Optional(CONF_MANUFACTURER_ID, default=COMPONENT_CLASS): cv.Length(
                min=1
            ),
            cv.Optional(CONF_SETUP_PRIORITY): cv.float_,
            cv.Optional(CONF_A_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_B_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_X_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_Y_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_LB_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_RB_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_LEFT_TRIGGER_BUTTON): cv.use_id(
                binary_sensor.BinarySensor
            ),
            cv.Optional(CONF_RIGHT_TRIGGER_BUTTON): cv.use_id(
                binary_sensor.BinarySensor
            ),
            cv.Optional(CONF_L3_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_R3_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_SHARE_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_MUTE_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_L4_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_R4_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_L5_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_R5_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_MODE_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_START_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_SELECT_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_DPAD_UP_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_DPAD_RIGHT_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_DPAD_DOWN_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_DPAD_LEFT_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_TOUCHPAD_BUTTON): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_BATTERY_LEVEL_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_CHARGING_STATUS_SENSOR): cv.use_id(
                binary_sensor.BinarySensor
            ),
            cv.Optional(CONF_LIGHTBAR_LIGHT): cv.use_id(light.LightState),
            cv.Optional(CONF_MUTE_LIGHT): cv.use_id(light.LightState),
            cv.Optional(CONF_MUTE_PULSE_EFFECT): cv.string,
            # Peripheral status inputs (binary sensor state → status2 bits in input report)
            cv.Optional(CONF_HEADPHONES_PLUGGED_SENSOR): cv.use_id(
                binary_sensor.BinarySensor
            ),
            cv.Optional(CONF_HEADPHONE_MIC_SENSOR): cv.use_id(
                binary_sensor.BinarySensor
            ),
            cv.Optional(CONF_USB_PLUGGED_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_MUTE_ACTIVE_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_LEFT_THUMB_X_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_LEFT_THUMB_Y_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_RIGHT_THUMB_X_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_RIGHT_THUMB_Y_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_LEFT_TRIGGER_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_RIGHT_TRIGGER_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_YAW_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_PITCH_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_ROLL_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_AX_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_AY_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_AZ_SENSOR): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_STICK_AXIS_MIN, default=-4095): cv.int_range(
                min=-32767, max=32767
            ),
            cv.Optional(CONF_STICK_AXIS_MAX, default=4095): cv.int_range(
                min=-32767, max=32767
            ),
            cv.Optional(CONF_TRIGGER_MIN, default=0): cv.int_range(
                min=-32767, max=32767
            ),
            cv.Optional(CONF_TRIGGER_MAX, default=4095): cv.int_range(
                min=-32767, max=32767
            ),
            cv.Optional(CONF_LEFT_TOUCHPAD): TOUCHPAD_SCHEMA,
            cv.Optional(CONF_RIGHT_TOUCHPAD): TOUCHPAD_SCHEMA,
            cv.Optional(CONF_COMPANION_UART): cv.use_id(uart.UARTComponent),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    _validate_dpad,
    _validate_trigger_sources,
    _validate_touchpad,
)


async def to_code(config: dict) -> None:
    """Generate component

    :param config: dict
    """

    if not CORE.is_esp32:
        raise cv.Invalid("The component only supports ESP32.")

    # if not CORE.using_arduino:
    #     raise cv.Invalid("The component only supports the Arduino framework.")

    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_NAME],
        config[CONF_MANUFACTURER_ID],
    )
    await cg.register_component(var, config)

    # Configure all available buttons
    button_configs = [
        (CONF_A_BUTTON, "set_a_button"),
        (CONF_B_BUTTON, "set_b_button"),
        (CONF_X_BUTTON, "set_x_button"),
        (CONF_Y_BUTTON, "set_y_button"),
        (CONF_LB_BUTTON, "set_lb_button"),
        (CONF_RB_BUTTON, "set_rb_button"),
        (CONF_LEFT_TRIGGER_BUTTON, "set_ls_button"),
        (CONF_RIGHT_TRIGGER_BUTTON, "set_rs_button"),
        (CONF_L3_BUTTON, "set_l3_button"),
        (CONF_R3_BUTTON, "set_r3_button"),
        (CONF_SHARE_BUTTON, "set_share_button"),
        (CONF_MUTE_BUTTON, "set_mute_button"),
        (CONF_L4_BUTTON, "set_l4_button"),
        (CONF_R4_BUTTON, "set_r4_button"),
        (CONF_L5_BUTTON, "set_l5_button"),
        (CONF_R5_BUTTON, "set_r5_button"),
        (CONF_MODE_BUTTON, "set_mode_button"),
        (CONF_START_BUTTON, "set_start_button"),
        (CONF_SELECT_BUTTON, "set_select_button"),
        (CONF_DPAD_UP_BUTTON, "set_dpad_up_button"),
        (CONF_DPAD_RIGHT_BUTTON, "set_dpad_right_button"),
        (CONF_DPAD_DOWN_BUTTON, "set_dpad_down_button"),
        (CONF_DPAD_LEFT_BUTTON, "set_dpad_left_button"),
        (CONF_TOUCHPAD_BUTTON, "set_touchpad_button"),
    ]

    for config_key, setter_method in button_configs:
        if config_key in config:
            button = await cg.get_variable(config[config_key])
            cg.add(getattr(var, setter_method)(button))

    if CONF_BATTERY_LEVEL_SENSOR in config:
        battery_level_sensor = await cg.get_variable(config[CONF_BATTERY_LEVEL_SENSOR])
        cg.add(var.set_battery_level_sensor(battery_level_sensor))

    if CONF_CHARGING_STATUS_SENSOR in config:
        charging_status_sensor = await cg.get_variable(
            config[CONF_CHARGING_STATUS_SENSOR]
        )
        cg.add(var.set_charging_status_sensor(charging_status_sensor))

    if CONF_LIGHTBAR_LIGHT in config:
        lightbar_light = await cg.get_variable(config[CONF_LIGHTBAR_LIGHT])
        cg.add(var.set_lightbar_light(lightbar_light))

    if CONF_MUTE_LIGHT in config:
        mute_light = await cg.get_variable(config[CONF_MUTE_LIGHT])
        cg.add(var.set_mute_light(mute_light))

    if CONF_MUTE_PULSE_EFFECT in config:
        cg.add(var.set_mute_pulse_effect(config[CONF_MUTE_PULSE_EFFECT]))

    peripheral_status_configs = [
        (CONF_HEADPHONES_PLUGGED_SENSOR, "set_headphones_plugged_sensor"),
        (CONF_HEADPHONE_MIC_SENSOR, "set_headphone_mic_sensor"),
        (CONF_USB_PLUGGED_SENSOR, "set_usb_plugged_sensor"),
        (CONF_MUTE_ACTIVE_SENSOR, "set_mute_active_sensor"),
    ]
    for config_key, setter_method in peripheral_status_configs:
        if config_key in config:
            sensor_var = await cg.get_variable(config[config_key])
            cg.add(getattr(var, setter_method)(sensor_var))

    thumb_configs = [
        (CONF_LEFT_THUMB_X_SENSOR, "set_left_thumb_x_sensor"),
        (CONF_LEFT_THUMB_Y_SENSOR, "set_left_thumb_y_sensor"),
        (CONF_RIGHT_THUMB_X_SENSOR, "set_right_thumb_x_sensor"),
        (CONF_RIGHT_THUMB_Y_SENSOR, "set_right_thumb_y_sensor"),
        (CONF_LEFT_TRIGGER_SENSOR, "set_left_trigger_sensor"),
        (CONF_RIGHT_TRIGGER_SENSOR, "set_right_trigger_sensor"),
    ]

    for config_key, setter_method in thumb_configs:
        if config_key in config:
            control_sensor = await cg.get_variable(config[config_key])
            cg.add(getattr(var, setter_method)(control_sensor))

    motion_configs = [
        (CONF_YAW_SENSOR, "set_yaw_sensor"),
        (CONF_PITCH_SENSOR, "set_pitch_sensor"),
        (CONF_ROLL_SENSOR, "set_roll_sensor"),
        (CONF_AX_SENSOR, "set_ax_sensor"),
        (CONF_AY_SENSOR, "set_ay_sensor"),
        (CONF_AZ_SENSOR, "set_az_sensor"),
    ]

    for config_key, setter_method in motion_configs:
        if config_key in config:
            motion_sensor = await cg.get_variable(config[config_key])
            cg.add(getattr(var, setter_method)(motion_sensor))

    cg.add(var.set_stick_axis_min(config[CONF_STICK_AXIS_MIN]))
    cg.add(var.set_stick_axis_max(config[CONF_STICK_AXIS_MAX]))
    cg.add(var.set_trigger_min(config[CONF_TRIGGER_MIN]))
    cg.add(var.set_trigger_max(config[CONF_TRIGGER_MAX]))

    for conf_key, setter_prefix in (
        (CONF_LEFT_TOUCHPAD, "left"),
        (CONF_RIGHT_TOUCHPAD, "right"),
    ):
        if conf_key not in config:
            continue
        tp = config[conf_key]
        if CONF_TOUCHPAD_SENSOR in tp:
            touch_s = await cg.get_variable(tp[CONF_TOUCHPAD_SENSOR])
            cg.add(getattr(var, f"set_{setter_prefix}_touch_sensor")(touch_s))
        if CONF_TOUCHPAD_TOUCHSCREEN in tp:
            ts = await cg.get_variable(tp[CONF_TOUCHPAD_TOUCHSCREEN])
            cg.add(getattr(var, f"set_{setter_prefix}_touchscreen")(ts))
        if CONF_TOUCHPAD_X_SENSOR in tp:
            x_s = await cg.get_variable(tp[CONF_TOUCHPAD_X_SENSOR])
            cg.add(getattr(var, f"set_{setter_prefix}_touch_x_sensor")(x_s))
        if CONF_TOUCHPAD_Y_SENSOR in tp:
            y_s = await cg.get_variable(tp[CONF_TOUCHPAD_Y_SENSOR])
            cg.add(getattr(var, f"set_{setter_prefix}_touch_y_sensor")(y_s))
        cg.add(
            getattr(var, f"set_{setter_prefix}_touch_x_min")(tp[CONF_TOUCHPAD_X_MIN])
        )
        cg.add(
            getattr(var, f"set_{setter_prefix}_touch_x_max")(tp[CONF_TOUCHPAD_X_MAX])
        )
        cg.add(
            getattr(var, f"set_{setter_prefix}_touch_y_min")(tp[CONF_TOUCHPAD_Y_MIN])
        )
        cg.add(
            getattr(var, f"set_{setter_prefix}_touch_y_max")(tp[CONF_TOUCHPAD_Y_MAX])
        )

    if CONF_COMPANION_UART in config:
        uart_var = await cg.get_variable(config[CONF_COMPANION_UART])
        cg.add(var.set_companion_uart(uart_var))

    for lib in LIBS_ADDITIONAL:  # type: ignore
        cg.add_library(*lib)

    cg.add_build_flag(BUILD_FLAGS)
