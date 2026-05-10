import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import CONF_TYPE, STATE_CLASS_MEASUREMENT

from . import Gamepad

DEPENDENCIES = ["gamepad"]

CONF_GAMEPAD_ID = "gamepad_id"

SENSOR_TYPES = {
    # Rumble motor level (0–255)
    "weak": "set_rumble_weak_level_sensor",
    "strong": "set_rumble_strong_level_sensor",
    # Player indicator sent by host (raw player_leds byte)
    "player_number": "set_player_number_sensor",
    # Adaptive trigger effect mode: 0=off, 1=feedback, 2=weapon, 3=vibration
    "left_trigger_effect": "set_left_trigger_effect_sensor",
    "right_trigger_effect": "set_right_trigger_effect_sensor",
    # Per-zone trigger strength/amplitude (0=inactive, 1–8=active), zones 0–9
    **{
        f"left_trigger_zone_{i}": f"set_left_trigger_zone_sensor_{i}" for i in range(10)
    },
    **{
        f"right_trigger_zone_{i}": f"set_right_trigger_zone_sensor_{i}"
        for i in range(10)
    },
    # Audio volumes from host output report (0–255)
    "headphone_volume": "set_headphone_volume_sensor",
    "speaker_volume": "set_speaker_volume_sensor",
    "mic_volume": "set_mic_volume_sensor",
    # Lightbar LED brightness from host: 0=high, 1=medium, 2=low
    "led_brightness": "set_led_brightness_sensor",
}

CONFIG_SCHEMA = sensor.sensor_schema(
    accuracy_decimals=0,
    state_class=STATE_CLASS_MEASUREMENT,
).extend(
    {
        cv.GenerateID(CONF_GAMEPAD_ID): cv.use_id(Gamepad),
        cv.Required(CONF_TYPE): cv.one_of(*SENSOR_TYPES, lower=True),
    }
)


async def to_code(config):
    gamepad = await cg.get_variable(config[CONF_GAMEPAD_ID])
    var = await sensor.new_sensor(config)
    cg.add(getattr(gamepad, SENSOR_TYPES[config[CONF_TYPE]])(var))
