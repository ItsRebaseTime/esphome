"""ESPHome gamepad companion component."""

from __future__ import annotations

import esphome.codegen as cg
from esphome.components import binary_sensor, light, sensor, uart
from esphome.components.gamepad import TOUCHPAD_SCHEMA
from esphome.components.gamepad.const import (
    CONF_A_BUTTON,
    CONF_AX_SENSOR,
    CONF_AY_SENSOR,
    CONF_AZ_SENSOR,
    CONF_B_BUTTON,
    CONF_BATTERY_LEVEL_SENSOR,
    CONF_CHARGING_STATUS_SENSOR,
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
    CONF_TOUCHPAD_SENSOR,
    CONF_TOUCHPAD_TOUCHSCREEN,
    CONF_TOUCHPAD_X_MAX,
    CONF_TOUCHPAD_X_MIN,
    CONF_TOUCHPAD_X_SENSOR,
    CONF_TOUCHPAD_Y_MAX,
    CONF_TOUCHPAD_Y_MIN,
    CONF_TOUCHPAD_Y_SENSOR,
    CONF_USB_PLUGGED_SENSOR,
    CONF_X_BUTTON,
    CONF_Y_BUTTON,
    CONF_YAW_SENSOR,
)
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_UART_ID
import esphome.final_validate as fv

from .const import (
    COMPONENT_CLASS,
    CONF_HEADPHONE_VOLUME_SENSOR,
    CONF_LED_BRIGHTNESS_SENSOR,
    CONF_LEFT_TRIGGER_EFFECT_ACTIVE_SENSOR,
    CONF_LEFT_TRIGGER_EFFECT_SENSOR,
    CONF_LEFT_TRIGGER_ZONE_SENSOR_0,
    CONF_LEFT_TRIGGER_ZONE_SENSOR_1,
    CONF_LEFT_TRIGGER_ZONE_SENSOR_2,
    CONF_LEFT_TRIGGER_ZONE_SENSOR_3,
    CONF_LEFT_TRIGGER_ZONE_SENSOR_4,
    CONF_LEFT_TRIGGER_ZONE_SENSOR_5,
    CONF_LEFT_TRIGGER_ZONE_SENSOR_6,
    CONF_LEFT_TRIGGER_ZONE_SENSOR_7,
    CONF_LEFT_TRIGGER_ZONE_SENSOR_8,
    CONF_LEFT_TRIGGER_ZONE_SENSOR_9,
    CONF_LIGHTBAR_BLUE_SENSOR,
    CONF_LIGHTBAR_GREEN_SENSOR,
    CONF_LIGHTBAR_RED_SENSOR,
    CONF_MIC_VOLUME_SENSOR,
    CONF_MUTE_LED_SENSOR,
    CONF_MUTE_PULSE_EFFECT,
    CONF_PLAYER_NUMBER_SENSOR,
    CONF_RIGHT_TRIGGER_EFFECT_ACTIVE_SENSOR,
    CONF_RIGHT_TRIGGER_EFFECT_SENSOR,
    CONF_RIGHT_TRIGGER_ZONE_SENSOR_0,
    CONF_RIGHT_TRIGGER_ZONE_SENSOR_1,
    CONF_RIGHT_TRIGGER_ZONE_SENSOR_2,
    CONF_RIGHT_TRIGGER_ZONE_SENSOR_3,
    CONF_RIGHT_TRIGGER_ZONE_SENSOR_4,
    CONF_RIGHT_TRIGGER_ZONE_SENSOR_5,
    CONF_RIGHT_TRIGGER_ZONE_SENSOR_6,
    CONF_RIGHT_TRIGGER_ZONE_SENSOR_7,
    CONF_RIGHT_TRIGGER_ZONE_SENSOR_8,
    CONF_RIGHT_TRIGGER_ZONE_SENSOR_9,
    CONF_RUMBLE_STRONG_LEVEL_SENSOR,
    CONF_RUMBLE_STRONG_SENSOR,
    CONF_RUMBLE_WEAK_LEVEL_SENSOR,
    CONF_RUMBLE_WEAK_SENSOR,
    CONF_SPEAKER_VOLUME_SENSOR,
)

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor", "sensor", "light"]

gamepad_companion_ns = cg.esphome_ns.namespace("gamepad_companion")
GamepadCompanion = gamepad_companion_ns.class_(
    COMPONENT_CLASS, cg.Component, uart.UARTDevice
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(GamepadCompanion),
        cv.Optional(CONF_UART_ID): cv.use_id(uart.UARTComponent),
        cv.Optional(CONF_A_BUTTON): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_B_BUTTON): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_X_BUTTON): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_Y_BUTTON): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_LB_BUTTON): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_RB_BUTTON): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_LEFT_TRIGGER_BUTTON): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_RIGHT_TRIGGER_BUTTON): cv.use_id(binary_sensor.BinarySensor),
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
        cv.Optional(CONF_LEFT_TOUCHPAD): TOUCHPAD_SCHEMA,
        cv.Optional(CONF_RIGHT_TOUCHPAD): TOUCHPAD_SCHEMA,
        cv.Optional(CONF_BATTERY_LEVEL_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_CHARGING_STATUS_SENSOR): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_HEADPHONES_PLUGGED_SENSOR): cv.use_id(
            binary_sensor.BinarySensor
        ),
        cv.Optional(CONF_HEADPHONE_MIC_SENSOR): cv.use_id(binary_sensor.BinarySensor),
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
        cv.Optional(CONF_LIGHTBAR_LIGHT): cv.use_id(light.LightState),
        cv.Optional(CONF_MUTE_LIGHT): cv.use_id(light.LightState),
        cv.Optional(CONF_MUTE_PULSE_EFFECT): cv.string,
        cv.Optional(CONF_RUMBLE_WEAK_LEVEL_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_RUMBLE_STRONG_LEVEL_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_RUMBLE_WEAK_SENSOR): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_RUMBLE_STRONG_SENSOR): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_PLAYER_NUMBER_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LEFT_TRIGGER_EFFECT_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_RIGHT_TRIGGER_EFFECT_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LEFT_TRIGGER_EFFECT_ACTIVE_SENSOR): cv.use_id(
            binary_sensor.BinarySensor
        ),
        cv.Optional(CONF_RIGHT_TRIGGER_EFFECT_ACTIVE_SENSOR): cv.use_id(
            binary_sensor.BinarySensor
        ),
        cv.Optional(CONF_HEADPHONE_VOLUME_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_SPEAKER_VOLUME_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MIC_VOLUME_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LED_BRIGHTNESS_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MUTE_LED_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LIGHTBAR_RED_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LIGHTBAR_GREEN_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LIGHTBAR_BLUE_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LEFT_TRIGGER_ZONE_SENSOR_0): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LEFT_TRIGGER_ZONE_SENSOR_1): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LEFT_TRIGGER_ZONE_SENSOR_2): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LEFT_TRIGGER_ZONE_SENSOR_3): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LEFT_TRIGGER_ZONE_SENSOR_4): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LEFT_TRIGGER_ZONE_SENSOR_5): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LEFT_TRIGGER_ZONE_SENSOR_6): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LEFT_TRIGGER_ZONE_SENSOR_7): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LEFT_TRIGGER_ZONE_SENSOR_8): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_LEFT_TRIGGER_ZONE_SENSOR_9): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_RIGHT_TRIGGER_ZONE_SENSOR_0): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_RIGHT_TRIGGER_ZONE_SENSOR_1): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_RIGHT_TRIGGER_ZONE_SENSOR_2): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_RIGHT_TRIGGER_ZONE_SENSOR_3): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_RIGHT_TRIGGER_ZONE_SENSOR_4): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_RIGHT_TRIGGER_ZONE_SENSOR_5): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_RIGHT_TRIGGER_ZONE_SENSOR_6): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_RIGHT_TRIGGER_ZONE_SENSOR_7): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_RIGHT_TRIGGER_ZONE_SENSOR_8): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_RIGHT_TRIGGER_ZONE_SENSOR_9): cv.use_id(sensor.Sensor),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if CONF_UART_ID in config:
        await uart.register_uart_device(var, config)
    else:
        full_config = fv.full_config.get()
        uart_configs = full_config.get("uart", [])
        if len(uart_configs) == 0:
            raise cv.Invalid("gamepad_companion requires a uart bus or uart_id")
        if len(uart_configs) > 1:
            raise cv.Invalid(
                "gamepad_companion requires uart_id when multiple UART buses are configured"
            )
        config[CONF_UART_ID] = uart_configs[0][CONF_ID]
        await uart.register_uart_device(var, config)

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
    ]
    for config_key, setter_method in button_configs:
        if config_key in config:
            button = await cg.get_variable(config[config_key])
            cg.add(getattr(var, setter_method)(button))

    peripheral_status_configs = [
        (CONF_CHARGING_STATUS_SENSOR, "set_charging_status_sensor"),
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

    for conf_key, prefix in (
        (CONF_LEFT_TOUCHPAD, "left"),
        (CONF_RIGHT_TOUCHPAD, "right"),
    ):
        if conf_key not in config:
            continue
        tp = config[conf_key]
        if CONF_TOUCHPAD_SENSOR in tp:
            cg.add(
                getattr(var, f"set_{prefix}_touch_sensor")(
                    await cg.get_variable(tp[CONF_TOUCHPAD_SENSOR])
                )
            )
        if CONF_TOUCHPAD_TOUCHSCREEN in tp:
            cg.add_define("GAMEPAD_USE_TOUCHSCREEN")
            cg.add(
                getattr(var, f"set_{prefix}_touchscreen")(
                    await cg.get_variable(tp[CONF_TOUCHPAD_TOUCHSCREEN])
                )
            )
        if CONF_TOUCHPAD_X_SENSOR in tp:
            cg.add(
                getattr(var, f"set_{prefix}_touch_x_sensor")(
                    await cg.get_variable(tp[CONF_TOUCHPAD_X_SENSOR])
                )
            )
        if CONF_TOUCHPAD_Y_SENSOR in tp:
            cg.add(
                getattr(var, f"set_{prefix}_touch_y_sensor")(
                    await cg.get_variable(tp[CONF_TOUCHPAD_Y_SENSOR])
                )
            )
        cg.add(getattr(var, f"set_{prefix}_touch_x_min")(tp[CONF_TOUCHPAD_X_MIN]))
        cg.add(getattr(var, f"set_{prefix}_touch_x_max")(tp[CONF_TOUCHPAD_X_MAX]))
        cg.add(getattr(var, f"set_{prefix}_touch_y_min")(tp[CONF_TOUCHPAD_Y_MIN]))
        cg.add(getattr(var, f"set_{prefix}_touch_y_max")(tp[CONF_TOUCHPAD_Y_MAX]))

    if CONF_BATTERY_LEVEL_SENSOR in config:
        battery_level_sensor = await cg.get_variable(config[CONF_BATTERY_LEVEL_SENSOR])
        cg.add(var.set_battery_level_sensor(battery_level_sensor))

    if CONF_LIGHTBAR_LIGHT in config:
        lightbar_light = await cg.get_variable(config[CONF_LIGHTBAR_LIGHT])
        cg.add(var.set_lightbar_light(lightbar_light))

    if CONF_MUTE_LIGHT in config:
        mute_light = await cg.get_variable(config[CONF_MUTE_LIGHT])
        cg.add(var.set_mute_light(mute_light))

    if CONF_MUTE_PULSE_EFFECT in config:
        cg.add(var.set_mute_pulse_effect(config[CONF_MUTE_PULSE_EFFECT]))

    for config_key, setter_method in (
        (CONF_RUMBLE_WEAK_LEVEL_SENSOR, "set_rumble_weak_level_sensor"),
        (CONF_RUMBLE_STRONG_LEVEL_SENSOR, "set_rumble_strong_level_sensor"),
        (CONF_RUMBLE_WEAK_SENSOR, "set_rumble_weak_sensor"),
        (CONF_RUMBLE_STRONG_SENSOR, "set_rumble_strong_sensor"),
        (CONF_PLAYER_NUMBER_SENSOR, "set_player_number_sensor"),
        (CONF_LEFT_TRIGGER_EFFECT_SENSOR, "set_left_trigger_effect_sensor"),
        (CONF_RIGHT_TRIGGER_EFFECT_SENSOR, "set_right_trigger_effect_sensor"),
        (
            CONF_LEFT_TRIGGER_EFFECT_ACTIVE_SENSOR,
            "set_left_trigger_effect_active_sensor",
        ),
        (
            CONF_RIGHT_TRIGGER_EFFECT_ACTIVE_SENSOR,
            "set_right_trigger_effect_active_sensor",
        ),
        (CONF_HEADPHONE_VOLUME_SENSOR, "set_headphone_volume_sensor"),
        (CONF_SPEAKER_VOLUME_SENSOR, "set_speaker_volume_sensor"),
        (CONF_MIC_VOLUME_SENSOR, "set_mic_volume_sensor"),
        (CONF_LED_BRIGHTNESS_SENSOR, "set_led_brightness_sensor"),
        (CONF_MUTE_LED_SENSOR, "set_mute_led_sensor"),
        (CONF_LIGHTBAR_RED_SENSOR, "set_lightbar_red_sensor"),
        (CONF_LIGHTBAR_GREEN_SENSOR, "set_lightbar_green_sensor"),
        (CONF_LIGHTBAR_BLUE_SENSOR, "set_lightbar_blue_sensor"),
    ):
        if config_key in config:
            cg.add(
                getattr(var, setter_method)(await cg.get_variable(config[config_key]))
            )

    for index, key in enumerate(
        [
            CONF_LEFT_TRIGGER_ZONE_SENSOR_0,
            CONF_LEFT_TRIGGER_ZONE_SENSOR_1,
            CONF_LEFT_TRIGGER_ZONE_SENSOR_2,
            CONF_LEFT_TRIGGER_ZONE_SENSOR_3,
            CONF_LEFT_TRIGGER_ZONE_SENSOR_4,
            CONF_LEFT_TRIGGER_ZONE_SENSOR_5,
            CONF_LEFT_TRIGGER_ZONE_SENSOR_6,
            CONF_LEFT_TRIGGER_ZONE_SENSOR_7,
            CONF_LEFT_TRIGGER_ZONE_SENSOR_8,
            CONF_LEFT_TRIGGER_ZONE_SENSOR_9,
        ]
    ):
        if key in config:
            cg.add(
                var.set_left_trigger_zone_sensor(
                    index, await cg.get_variable(config[key])
                )
            )

    for index, key in enumerate(
        [
            CONF_RIGHT_TRIGGER_ZONE_SENSOR_0,
            CONF_RIGHT_TRIGGER_ZONE_SENSOR_1,
            CONF_RIGHT_TRIGGER_ZONE_SENSOR_2,
            CONF_RIGHT_TRIGGER_ZONE_SENSOR_3,
            CONF_RIGHT_TRIGGER_ZONE_SENSOR_4,
            CONF_RIGHT_TRIGGER_ZONE_SENSOR_5,
            CONF_RIGHT_TRIGGER_ZONE_SENSOR_6,
            CONF_RIGHT_TRIGGER_ZONE_SENSOR_7,
            CONF_RIGHT_TRIGGER_ZONE_SENSOR_8,
            CONF_RIGHT_TRIGGER_ZONE_SENSOR_9,
        ]
    ):
        if key in config:
            cg.add(
                var.set_right_trigger_zone_sensor(
                    index, await cg.get_variable(config[key])
                )
            )
