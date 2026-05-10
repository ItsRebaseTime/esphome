import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import DEVICE_CLASS_SWITCH, ENTITY_CATEGORY_CONFIG, ICON_TIMER

from .. import Gamepad, gamepad_ns
from ..const import CONF_CHANNEL, CONF_GAMEPAD_ID, CONF_TIMING_INFO

DEPENDENCIES = ["gamepad"]

TimingInfoSwitch = gamepad_ns.class_("TimingInfoSwitch", switch.Switch)

CHANNEL_TO_CLASS = {
    CONF_TIMING_INFO: TimingInfoSwitch,
}

CONFIG_SCHEMA = switch.switch_schema(
    TimingInfoSwitch,
    device_class=DEVICE_CLASS_SWITCH,
    entity_category=ENTITY_CATEGORY_CONFIG,
    icon=ICON_TIMER,
).extend(
    cv.Schema(
        {
            cv.GenerateID(CONF_GAMEPAD_ID): cv.use_id(Gamepad),
            cv.Required(CONF_CHANNEL): cv.enum(
                {CONF_TIMING_INFO: CONF_TIMING_INFO}, upper=False
            ),
        }
    )
)


async def to_code(config):
    gamepad_component = await cg.get_variable(config[CONF_GAMEPAD_ID])
    channel = config[CONF_CHANNEL]

    if channel == CONF_TIMING_INFO:
        sw = await switch.new_switch(config, gamepad_component)
        cg.add(gamepad_component.set_timing_info_switch(sw))
