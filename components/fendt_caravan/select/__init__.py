import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import CONF_OPTIONS, CONF_TYPE

from .. import CONF_KEY_NAME, CONF_PARENT_ID, CaravanDeviceComponent, fendt_caravan_ns

FendtSelect = fendt_caravan_ns.class_(
    "FendtSelect",
    select.Select,
    cg.Component,
    cg.Parented.template(CaravanDeviceComponent),
)


def _select_schema(icon: str = cv.UNDEFINED, key_name_=cv.UNDEFINED) -> cv.Schema:
    return select.select_schema(FendtSelect).extend(
        {
            cv.Required(CONF_PARENT_ID): cv.use_id(CaravanDeviceComponent),
            cv.Optional(CONF_KEY_NAME, default=key_name_): cv.string,
        }
    )


CONFIG_SCHEMA = cv.typed_schema(
    {
        "alde_heater_electricity": _select_schema(key_name_="HEATER_EL").extend(
            {
                cv.Optional(
                    CONF_OPTIONS, default=["Off", "1 kW", "2 kW", "3 kW"]
                ): cv.ensure_list(cv.string_strict)
            }
        )
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    var = await select.new_select(config, options=config[CONF_OPTIONS])
    if CONF_KEY_NAME in config:
        cg.add(var.set_key_name(config[CONF_KEY_NAME]))
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    cg.add(getattr(parent, f"set_{config[CONF_TYPE]}_select")(var))
