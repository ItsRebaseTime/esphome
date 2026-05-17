import esphome.codegen as cg
from esphome.components import audio, media_source, psram
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_TASK_STACK_IN_PSRAM
from esphome.types import ConfigType

CODEOWNERS = ["@ItsRebaseTime"]
AUTO_LOAD = ["audio"]
DEPENDENCIES = ["media_source"]

CONF_MOUNT_POINT = "mount_point"

audio_sd_card_ns = cg.esphome_ns.namespace("audio_sd_card")
AudioSDCardMediaSource = audio_sd_card_ns.class_(
    "AudioSDCardMediaSource", cg.Component, media_source.MediaSource
)

CONFIG_SCHEMA = cv.All(
    media_source.media_source_schema(AudioSDCardMediaSource)
    .extend(
        {
            cv.Optional(CONF_MOUNT_POINT, default="/sdcard"): cv.string,
            cv.Optional(CONF_TASK_STACK_IN_PSRAM): cv.All(
                cv.boolean, cv.requires_component(psram.DOMAIN)
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
    cv.only_on_esp32,
)


async def to_code(config: ConfigType) -> None:
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await media_source.register_media_source(var, config)

    cg.add(var.set_mount_point(config[CONF_MOUNT_POINT]))

    if CONF_TASK_STACK_IN_PSRAM in config:
        cg.add(var.set_task_stack_in_psram(config[CONF_TASK_STACK_IN_PSRAM]))

    # Enable all codec support since the file format is only known at runtime
    audio.request_mp3_support()
    audio.request_flac_support()
    audio.request_opus_support()
