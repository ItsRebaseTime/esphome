import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_TYPE

from . import Gamepad

DEPENDENCIES = ["gamepad"]

CONF_GAMEPAD_ID = "gamepad_id"

BINARY_SENSOR_TYPES = {
    # Rumble motor active (on/off)
    "weak": "set_rumble_weak_sensor",
    "strong": "set_rumble_strong_sensor",
    # Mute LED commanded by host
    "mute": "set_mute_output",
    # True when the host has set an active adaptive trigger effect on L2/R2
    "left_trigger_effect_active": "set_left_trigger_effect_active_sensor",
    "right_trigger_effect_active": "set_right_trigger_effect_active_sensor",
}

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema().extend(
    {
        cv.GenerateID(CONF_GAMEPAD_ID): cv.use_id(Gamepad),
        cv.Required(CONF_TYPE): cv.one_of(*BINARY_SENSOR_TYPES, lower=True),
    }
)


async def to_code(config):
    gamepad = await cg.get_variable(config[CONF_GAMEPAD_ID])
    var = await binary_sensor.new_binary_sensor(config)
    cg.add(getattr(gamepad, BINARY_SENSOR_TYPES[config[CONF_TYPE]])(var))
