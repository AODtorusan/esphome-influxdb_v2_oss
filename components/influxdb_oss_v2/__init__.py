import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_BINARY_SENSORS,
    CONF_FORMAT,
    CONF_ID,
    CONF_NAME,
    CONF_SENSORS,
    CONF_TEXT_SENSORS,
    CONF_TIME_ID,
    CONF_URL,
)
from esphome.components.http_request import (
    CONF_USERAGENT,
    validate_url,
)
from esphome.components.binary_sensor import BinarySensor
from esphome.components.sensor import Sensor
from esphome.components.text_sensor import TextSensor
from esphome.components.time import RealTimeClock

CODEOWNERS = ["@kpfleming"]

CONF_BUCKET = "bucket"
CONF_MEASUREMENTS = "measurements"
CONF_ORGANIZATION = "organization"
CONF_RAW_VALUE = "raw_value"
CONF_TAGS = "tags"
CONF_TOKEN = "token"

SENSOR_FORMATS = {
    "float": "",
    "integer": "i",
    "unsigned_integer": "u",
}

influxdb_ns = cg.esphome_ns.namespace("influxdb")
InfluxDB = influxdb_ns.class_("InfluxDB", cg.Component)
Measurement = influxdb_ns.struct("Measurement")


def validate_measurement_config(config):
    if (
        (CONF_BINARY_SENSORS not in config)
        and (CONF_SENSORS not in config)
        and (CONF_TEXT_SENSORS not in config)
    ):
        raise cv.Invalid(
            f"At least one of '{CONF_BINARY_SENSORS}', '{CONF_SENSORS}', or '{CONF_TEXT_SENSORS}' must be specified."
        )

    return config


MEASUREMENT_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Measurement),
            cv.Required(CONF_NAME): cv.validate_id_name,
            cv.Optional(CONF_TAGS): cv.Schema({cv.string: cv.string}),
            cv.Optional(CONF_BINARY_SENSORS): cv.ensure_list(
                cv.maybe_simple_value(
                    cv.Schema(
                        {
                            cv.GenerateID(): cv.use_id(BinarySensor),
                            cv.Optional(CONF_NAME): cv.validate_id_name,
                        }
                    ),
                    key=CONF_ID,
                )
            ),
            cv.Optional(CONF_SENSORS): cv.ensure_list(
                cv.maybe_simple_value(
                    cv.Schema(
                        {
                            cv.GenerateID(): cv.use_id(Sensor),
                            cv.Optional(CONF_NAME): cv.validate_id_name,
                            cv.Optional(CONF_FORMAT, default="float"): cv.enum(
                                SENSOR_FORMATS
                            ),
                            cv.Optional(CONF_RAW_VALUE, default=False): cv.boolean,
                        }
                    ),
                    key=CONF_ID,
                )
            ),
            cv.Optional(CONF_TEXT_SENSORS): cv.ensure_list(
                cv.maybe_simple_value(
                    cv.Schema(
                        {
                            cv.GenerateID(): cv.use_id(TextSensor),
                            cv.Optional(CONF_NAME): cv.validate_id_name,
                            cv.Optional(CONF_RAW_VALUE, default=False): cv.boolean,
                        }
                    ),
                    key=CONF_ID,
                )
            ),
        }
    ),
    validate_measurement_config,
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(InfluxDB),
        cv.Required(CONF_URL): cv.All(cv.string, validate_url),
        cv.Required(CONF_ORGANIZATION): cv.string,
        cv.Required(CONF_BUCKET): cv.string,
        cv.Optional(CONF_TOKEN): cv.string,
        cv.Optional(CONF_USERAGENT, "ESPHome"): cv.string,
        cv.Optional(CONF_TIME_ID): cv.use_id(RealTimeClock),
        cv.Optional(CONF_TAGS): cv.Schema({cv.string: cv.string}),
        cv.Required(CONF_MEASUREMENTS): cv.ensure_list(MEASUREMENT_SCHEMA),
    }
).extend(cv.COMPONENT_SCHEMA)
