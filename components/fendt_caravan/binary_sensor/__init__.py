import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_TYPE,
    DEVICE_CLASS_CONNECTIVITY,
    DEVICE_CLASS_PLUG,
    DEVICE_CLASS_POWER,
    ENTITY_CATEGORY_DIAGNOSTIC,
)

from .. import CONF_KEY_NAME, CONF_PARENT_ID, CaravanDeviceComponent, fendt_caravan_ns

FendtBinarySensor = fendt_caravan_ns.class_(
    "FendtBinarySensor",
    binary_sensor.BinarySensor,
    cg.Component,
    cg.Parented.template(CaravanDeviceComponent),
)


def _binary_sensor_schema(
    device_class: str = cv.UNDEFINED,
    entity_category: str = cv.UNDEFINED,
    icon: str = cv.UNDEFINED,
    key_name=cv.UNDEFINED,
) -> cv.Schema:
    return binary_sensor.binary_sensor_schema(
        FendtBinarySensor,
        device_class=device_class,
        entity_category=entity_category,
        icon=icon,
    ).extend(
        {
            cv.Required(CONF_PARENT_ID): cv.use_id(CaravanDeviceComponent),
            cv.Optional(CONF_KEY_NAME, default=key_name): cv.string,
        }
    )


CONFIG_SCHEMA = cv.typed_schema(
    {
        "shore_power": _binary_sensor_schema(
            device_class=DEVICE_CLASS_PLUG, icon="mdi:power-plug", key_name="LINE_EN"
        ),
        "line_en": _binary_sensor_schema(
            device_class=DEVICE_CLASS_PLUG, icon="mdi:power-plug", key_name="LINE_EN"
        ),
        "truma_available": _binary_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon="mdi:radiator",
            key_name="TH_AVAILABLE",
        ),
        "fridge_available": _binary_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon="mdi:fridge-outline",
            key_name="FRIDGE_AVAILABLE",
        ),
        "alde_available": _binary_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon="mdi:air-filter",
            key_name="HEATER_AVAILABLE",
        ),
        "d_plus": _binary_sensor_schema(
            device_class=DEVICE_CLASS_POWER,
            icon="mdi:car-electric",
            key_name="D_PLUS",
        ),
        "generic": _binary_sensor_schema(),
        "binary_sensor": _binary_sensor_schema(),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    var = await binary_sensor.new_binary_sensor(config)
    if CONF_KEY_NAME in config and config[CONF_KEY_NAME]:
        cg.add(var.set_key_name(config[CONF_KEY_NAME]))
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    setter = f"set_{config[CONF_TYPE]}_binary_sensor"
    if hasattr(parent, setter):
        cg.add(getattr(parent, setter)(var))
