import esphome.codegen as cg
from esphome.components import sensor, water_heater
import esphome.config_validation as cv
from esphome.const import CONF_SENSOR, CONF_TYPE

from .. import CONF_PARENT_ID, CaravanDeviceComponent, fendt_caravan_ns

TrumaWaterHeater = fendt_caravan_ns.class_(
    "TrumaWaterHeater",
    water_heater.WaterHeater,
    cg.Component,
    cg.Parented.template(CaravanDeviceComponent),
)

CONFIG_SCHEMA = cv.typed_schema(
    {
        "truma_water_heater": water_heater.water_heater_schema(TrumaWaterHeater).extend(
            {
                cv.Required(CONF_PARENT_ID): cv.use_id(CaravanDeviceComponent),
                cv.Optional(CONF_SENSOR): cv.use_id(sensor.Sensor),
            }
        ),
        "truma": water_heater.water_heater_schema(TrumaWaterHeater).extend(
            {
                cv.Required(CONF_PARENT_ID): cv.use_id(CaravanDeviceComponent),
                cv.Optional(CONF_SENSOR): cv.use_id(sensor.Sensor),
            }
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    var = await water_heater.new_water_heater(config)
    if CONF_SENSOR in config:
        sens = await cg.get_variable(config[CONF_SENSOR])
        cg.add(var.set_sensor(sens))
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    if hasattr(parent, f"set_{config[CONF_TYPE]}_water_heater"):
        cg.add(getattr(parent, f"set_{config[CONF_TYPE]}_water_heater")(var))
