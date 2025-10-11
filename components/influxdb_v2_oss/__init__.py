import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_SENSORS,
    CONF_TIME_ID,
    CONF_URL,
)
from esphome.components.http_request import (
    HttpRequestComponent,
)
from esphome.components.time import RealTimeClock
from esphome.components import binary_sensor, sensor, text_sensor
from esphome.cpp_types import EntityBase
from esphome.cpp_generator import MockObj, MockObjClass

CODEOWNERS = ["@kpfleming"]

CONF_BACKLOG_DRAIN_BATCH = "backlog_drain_batch"
CONF_BACKLOG_MAX_DEPTH = "backlog_max_depth"
CONF_BUCKET = "bucket"
CONF_HTTP_REQUEST_ID = "http_request_id"
CONF_MEASUREMENT = "measurement"
CONF_ORGANIZATION = "organization"
CONF_TAGS = "tags"
CONF_TOKEN = "token"
CONF_PUBLISH_ALL = "publish_all"
CONF_DEFAULT_NAME_POLICY = "default_name_policy"
CONF_IGNORE = "ignore"

influxdb_ns = cg.esphome_ns.namespace("influxdb")
InfluxDB = influxdb_ns.class_("InfluxDB", cg.Component)
InfluxDBStatics = influxdb_ns.namespace("InfluxDB")
Field = influxdb_ns.class_("Field")
BinarySensorField = influxdb_ns.class_("BinarySensorField")
NumericSensorField = influxdb_ns.class_("NumericSensorField")
TextSensorField = influxdb_ns.class_("TextSensorField")
IgnoredField = influxdb_ns.class_("IgnoredField")
NamePolicies = influxdb_ns.enum("DefaultNamePolicy")

DEFAULT_NAME_POLICIES = {
    "id":           NamePolicies.ENTITY_ID,
    "name":         NamePolicies.ENTITY_NAME,
    "device_class": NamePolicies.ENTITY_DEVICE_CLASS,
}

def valid_identifier(value):
    value = cv.string_strict(value)

    if value[0] == "_":
        raise cv.Invalid(f"Identifiers cannot begin with '_': {value}")

    return value

SENSOR_SCHEMA = cv.Schema(
    {
        cv.use_id(EntityBase): cv.Schema( {
            cv.GenerateID(): cv.declare_id(Field),
            cv.Optional(CONF_IGNORE, default=False): cv.boolean,
            cv.Optional(CONF_NAME): valid_identifier,
            cv.Optional(CONF_MEASUREMENT): cv.string,
            cv.Optional(CONF_TAGS, default={}): cv.Schema({valid_identifier: cv.string}),
        } )
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(InfluxDB),
        cv.GenerateID(CONF_HTTP_REQUEST_ID): cv.use_id(HttpRequestComponent),
        cv.GenerateID(CONF_TIME_ID): cv.use_id(RealTimeClock),
        cv.Required(CONF_URL): cv.url,
        cv.Optional(CONF_ORGANIZATION): cv.string,
        cv.Optional(CONF_TOKEN): cv.string,
        cv.Optional(CONF_BUCKET): cv.string,
        cv.Optional(CONF_MEASUREMENT, default="esphome"): cv.string,
        cv.Optional(CONF_TAGS): cv.Schema({valid_identifier: cv.string}),
        cv.Optional(CONF_BACKLOG_MAX_DEPTH, default=100): cv.int_range(min=1, max=200),
        cv.Optional(CONF_BACKLOG_DRAIN_BATCH, default=5): cv.int_range(min=1, max=20),
        cv.Optional(CONF_PUBLISH_ALL, default=True): cv.boolean,
        cv.Optional(CONF_DEFAULT_NAME_POLICY, default="name"): cv.enum( DEFAULT_NAME_POLICIES ),
        cv.Optional(CONF_SENSORS, default={}): SENSOR_SCHEMA,
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    db = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(db, config)

    http = await cg.get_variable(config[CONF_HTTP_REQUEST_ID])
    cg.add(db.set_http_request(http))

    url = config[CONF_URL]
    if url[-1] == "/":
        url = url[0:-1]

    url += f"/api/v2/write?precision=s"
    if org := config.get(CONF_ORGANIZATION):
        url += "&org=" + org
    if bucket := config.get(CONF_BUCKET):
        url += "&bucket=" + bucket

    cg.add(db.set_url(f"{url}"))
    cg.add(db.set_publish_all(config[CONF_PUBLISH_ALL]))
    default_measurement = config[CONF_MEASUREMENT]
    cg.add(db.set_measurement(default_measurement))
    cg.add(db.set_default_name_policy(config[CONF_DEFAULT_NAME_POLICY]))

    if token := config.get(CONF_TOKEN):
        cg.add(db.set_token(token))

    clock = await cg.get_variable( config.get(CONF_TIME_ID) )
    cg.add(db.set_clock(clock))

    if backlog_max_depth := config.get(CONF_BACKLOG_MAX_DEPTH):
        cg.add(db.set_backlog_max_depth(backlog_max_depth))

        if backlog_drain_batch := config.get(CONF_BACKLOG_DRAIN_BATCH):
            cg.add(db.set_backlog_drain_batch(backlog_drain_batch))

    global_tags: dict = config.get(CONF_TAGS)
    if tags := global_tags:
        for key, value in tags.items():
            cg.add(db.add_tag(key, value))


    for sensor_id, sensor_config in config[CONF_SENSORS].items():
        sensor_var: MockObj = await cg.get_variable(sensor_id)
        if sensor_config[CONF_IGNORE]:
            var = cg.Pvariable(sensor_config[CONF_ID], IgnoredField.new(), IgnoredField)
            cg.add(db.add_field(var))
            cg.add(var.set_sensor(sensor_var))
            cg.add(var.set_field_name( f"{sensor_id} [ignored]" ))
        else:
            sensor_var_type: MockObjClass = sensor_var.base.type

            var: MockObj = None
            if (sensor_var_type.inherits_from( binary_sensor.BinarySensor )):
                cls = BinarySensorField
                var = cg.Pvariable(sensor_config[CONF_ID], cls.new(), cls)
            elif (sensor_var_type.inherits_from( sensor.Sensor )):
                cls = NumericSensorField
                var = cg.Pvariable(sensor_config[CONF_ID], cls.new(), cls)
                cg.add(sensor_var.add_on_state_callback(cg.RawExpression(
                    f"[](float state) {{ {db}->queue( std::move({var}->to_entry( {config[CONF_ID]}->get_url() )) ); }}"
                )))

            elif (sensor_var_type.inherits_from( text_sensor.TextSensor )):
                cls = TextSensorField
                var = cg.Pvariable(sensor_config[CONF_ID], cls.new(), cls)
            else:
                raise ValueError(f"Could not determine the proper field type to create for this entity type! '{sensor_var}' of type '{sensor_var_type}'")

            cg.add(db.add_field(var))
            cg.add(var.set_sensor(sensor_var))
            cg.add(var.set_measurement(sensor_config.get( CONF_MEASUREMENT, default_measurement )))
            if name := sensor_config.get(CONF_NAME):
                cg.add(var.set_field_name(name))

            tags = global_tags.copy() | sensor_config[CONF_TAGS]
            for key, value in tags.items():
                cg.add(var.add_tag(key, value))
