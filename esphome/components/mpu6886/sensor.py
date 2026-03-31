import esphome.codegen as cg
from esphome.components import i2c, sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_TEMPERATURE,
    CONF_UPDATE_INTERVAL,
    DEVICE_CLASS_TEMPERATURE,
    ICON_BRIEFCASE_DOWNLOAD,
    ICON_SCREEN_ROTATION,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_DEGREE_PER_SECOND,
    UNIT_DEGREES,
    UNIT_METER_PER_SECOND_SQUARED,
)

CODEOWNERS = ["@fabaff"]
DEPENDENCIES = ["i2c"]

CONF_ACCEL_X = "accel_x"
CONF_ACCEL_Y = "accel_y"
CONF_ACCEL_Z = "accel_z"
CONF_GYRO_X = "gyro_x"
CONF_GYRO_Y = "gyro_y"
CONF_GYRO_Z = "gyro_z"
CONF_AHRS = "ahrs"
CONF_YAW = "yaw"
CONF_PITCH = "pitch"
CONF_ROLL = "roll"

mpu6886_ns = cg.esphome_ns.namespace("mpu6886")
MPU6886Component = mpu6886_ns.class_(
    "MPU6886Component", cg.PollingComponent, i2c.I2CDevice
)
AhrsMode = mpu6886_ns.enum("AhrsMode")

AHRS_MODES = {
    "madgwick": AhrsMode.AHRS_MODE_MADGWICK,
}

accel_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_METER_PER_SECOND_SQUARED,
    icon=ICON_BRIEFCASE_DOWNLOAD,
    accuracy_decimals=2,
    state_class=STATE_CLASS_MEASUREMENT,
)
gyro_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_DEGREE_PER_SECOND,
    icon=ICON_SCREEN_ROTATION,
    accuracy_decimals=2,
    state_class=STATE_CLASS_MEASUREMENT,
)
temperature_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
)
attitude_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_DEGREES,
    icon=ICON_SCREEN_ROTATION,
    accuracy_decimals=2,
    state_class=STATE_CLASS_MEASUREMENT,
)


def _validate_config(config):
    has_attitude_output = any(
        key in config for key in (CONF_YAW, CONF_PITCH, CONF_ROLL)
    )
    has_ahrs = CONF_AHRS in config
    if has_attitude_output and not has_ahrs:
        raise cv.Invalid(
            "`yaw`, `pitch` and `roll` outputs require `ahrs` to be configured"
        )

    if CONF_UPDATE_INTERVAL not in config:
        config[CONF_UPDATE_INTERVAL] = cv.update_interval("1ms" if has_ahrs else "60s")

    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MPU6886Component),
            cv.Optional(CONF_ACCEL_X): accel_schema,
            cv.Optional(CONF_ACCEL_Y): accel_schema,
            cv.Optional(CONF_ACCEL_Z): accel_schema,
            cv.Optional(CONF_GYRO_X): gyro_schema,
            cv.Optional(CONF_GYRO_Y): gyro_schema,
            cv.Optional(CONF_GYRO_Z): gyro_schema,
            cv.Optional(CONF_TEMPERATURE): temperature_schema,
            cv.Optional(CONF_YAW): attitude_schema,
            cv.Optional(CONF_PITCH): attitude_schema,
            cv.Optional(CONF_ROLL): attitude_schema,
            cv.Optional(CONF_AHRS): cv.All(
                cv.enum(AHRS_MODES, lower=True),
                cv.only_with_arduino,
            ),
            cv.Optional(CONF_UPDATE_INTERVAL): cv.update_interval,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x68)),
    _validate_config,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_AHRS in config:
        cg.add(var.set_ahrs_mode(config[CONF_AHRS]))
        cg.add_define("USE_MPU6886_AHRS_MADGWICK")
        cg.add_library(
            "MadgwickAHRS",
            None,
            "https://github.com/arduino-libraries/MadgwickAHRS.git",
        )

    for d in ["x", "y", "z"]:
        accel_key = f"accel_{d}"
        if accel_key in config:
            sens = await sensor.new_sensor(config[accel_key])
            cg.add(getattr(var, f"set_accel_{d}_sensor")(sens))
        gyro_key = f"gyro_{d}"
        if gyro_key in config:
            sens = await sensor.new_sensor(config[gyro_key])
            cg.add(getattr(var, f"set_gyro_{d}_sensor")(sens))

    for attitude in [CONF_YAW, CONF_PITCH, CONF_ROLL]:
        if attitude in config:
            sens = await sensor.new_sensor(config[attitude])
            cg.add(getattr(var, f"set_{attitude}_sensor")(sens))

    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(var.set_temperature_sensor(sens))
