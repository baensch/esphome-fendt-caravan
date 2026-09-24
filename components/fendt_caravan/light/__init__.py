import esphome.codegen as cg
from esphome.components import light
import esphome.config_validation as cv
from esphome.const import CONF_DEFAULT_TRANSITION_LENGTH, CONF_OUTPUT_ID

from .. import CONF_KEY_NAME, CONF_PARENT_ID, CaravanDeviceComponent, fendt_caravan_ns

CONF_DIMMABLE = "dimmable"

FendtLightOutput = fendt_caravan_ns.class_(
    "FendtLightOutput",
    light.LightOutput,
    cg.Component,
    cg.Parented.template(CaravanDeviceComponent),
)


def _light_schema(key_name_=cv.UNDEFINED, default_dimmable=False) -> cv.Schema:
    return light.BRIGHTNESS_ONLY_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(FendtLightOutput),
            cv.Required(CONF_PARENT_ID): cv.use_id(CaravanDeviceComponent),
            cv.Optional(CONF_KEY_NAME, default=key_name_): cv.string,
            cv.Optional(CONF_DIMMABLE, default=default_dimmable): cv.boolean,
            cv.Optional(
                CONF_DEFAULT_TRANSITION_LENGTH, default="0s"
            ): cv.positive_time_period_milliseconds,
        }
    ).extend(cv.COMPONENT_SCHEMA)


CONFIG_SCHEMA = cv.typed_schema(
    {
        # Generic semantic light (define any custom key_name, dimmable optional)
        "light_sw": _light_schema(default_dimmable=False),
        "semantic_light": _light_schema(default_dimmable=False),
        "light_dim": _light_schema(default_dimmable=True),

        # Common pre-defined switched semantic circuits (can set dimmable: true if equipped)
        "light_aussen": _light_schema(key_name_="LIGHT_AUSSEN", default_dimmable=False),
        "light_kueche": _light_schema(key_name_="LIGHT_KUECHE", default_dimmable=False),
        "light_kueche2": _light_schema(key_name_="LIGHT_KUECHE2", default_dimmable=False),
        "light_wasch": _light_schema(key_name_="LIGHT_WASCH", default_dimmable=False),
        "light_dusche": _light_schema(key_name_="LIGHT_DUSCHE", default_dimmable=False),
        "light_toilette": _light_schema(key_name_="LIGHT_DUSCHE", default_dimmable=False),
        "light_kinder": _light_schema(key_name_="LIGHT_KINDER", default_dimmable=False),
        "light_bett_sw": _light_schema(key_name_="LIGHT_BETT", default_dimmable=False),
        "light_sw0": _light_schema(key_name_="LIGHT_SW0", default_dimmable=False),
        "light_sw1": _light_schema(key_name_="LIGHT_SW1", default_dimmable=False),
        "light_sw2": _light_schema(key_name_="LIGHT_SW2", default_dimmable=False),
        "light_sw3": _light_schema(key_name_="LIGHT_SW3", default_dimmable=False),
        "light_amb1": _light_schema(key_name_="LIGHT_AMB1", default_dimmable=False),
        "light_amb2": _light_schema(key_name_="LIGHT_AMB2", default_dimmable=False),
        "light_amb3": _light_schema(key_name_="LIGHT_AMB3", default_dimmable=False),
        "light_zusatzl": _light_schema(key_name_="LIGHT_ZUSATZL", default_dimmable=False),
        "light_zusatzr": _light_schema(key_name_="LIGHT_ZUSATZR", default_dimmable=False),

        # Semantic dimmers
        "light_dim_decken": _light_schema(key_name_="LIGHT_DIM0", default_dimmable=True),
        "light_dim_amb1": _light_schema(key_name_="LIGHT_DIM1", default_dimmable=True),
        "light_dim_amb2": _light_schema(key_name_="LIGHT_DIM2", default_dimmable=True),
        "light_dim_amb3": _light_schema(key_name_="LIGHT_DIM3", default_dimmable=True),
        "light_dim_bett": _light_schema(key_name_="LIGHT_DIM4", default_dimmable=True),

        # Channel-indexed dimmers (backwards-compatible with older configs)
        "light_dim0": _light_schema(key_name_="LIGHT_DIM0", default_dimmable=True),
        "light_dim1": _light_schema(key_name_="LIGHT_DIM1", default_dimmable=True),
        "light_dim2": _light_schema(key_name_="LIGHT_DIM2", default_dimmable=True),
        "light_dim3": _light_schema(key_name_="LIGHT_DIM3", default_dimmable=True),
        "light_dim4": _light_schema(key_name_="LIGHT_DIM4", default_dimmable=True),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await light.register_light(var, config)
    if CONF_KEY_NAME in config:
        cg.add(var.set_key_name(config[CONF_KEY_NAME]))
    if CONF_DIMMABLE in config:
        cg.add(var.set_dimmable(config[CONF_DIMMABLE]))
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    cg.add(parent.register_light_output(var))
