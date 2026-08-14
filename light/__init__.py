import esphome.codegen as cg
from esphome.components import light
import esphome.config_validation as cv
from esphome.const import CONF_DEFAULT_TRANSITION_LENGTH, CONF_OUTPUT_ID, CONF_TYPE

from .. import CONF_KEY_NAME, CONF_PARENT_ID, CaravanDeviceComponent, fendt_caravan_ns

FendtBinaryLightOutput = fendt_caravan_ns.class_(
    "FendtBinaryLightOutput",
    light.LightOutput,
    cg.Component,
    cg.Parented.template(CaravanDeviceComponent),
)

FendtFloatLightOutput = fendt_caravan_ns.class_(
    "FendtFloatLightOutput",
    light.LightOutput,
    cg.Component,
    cg.Parented.template(CaravanDeviceComponent),
)


def _binary_output_schema(key_name_=cv.UNDEFINED) -> cv.Schema:
    return light.BINARY_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(FendtBinaryLightOutput),
            cv.Required(CONF_PARENT_ID): cv.use_id(CaravanDeviceComponent),
            cv.Optional(CONF_KEY_NAME, default=key_name_): cv.string,
        }
    ).extend(cv.COMPONENT_SCHEMA)


def _float_output_schemar(key_name_=cv.UNDEFINED) -> cv.Schema:
    return light.BRIGHTNESS_ONLY_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(FendtFloatLightOutput),
            cv.Required(CONF_PARENT_ID): cv.use_id(CaravanDeviceComponent),
            cv.Optional(CONF_KEY_NAME, default=key_name_): cv.string,
            cv.Optional(
                CONF_DEFAULT_TRANSITION_LENGTH, default="0s"
            ): cv.positive_time_period_milliseconds,
        }
    ).extend(cv.COMPONENT_SCHEMA)


CONFIG_SCHEMA = cv.typed_schema(
    {
        "light_sw0": _binary_output_schema(key_name_="LIGHT_SW0"),
        "light_sw1": _binary_output_schema(key_name_="LIGHT_SW1"),
        "light_sw2": _binary_output_schema(key_name_="LIGHT_SW2"),
        "light_sw3": _binary_output_schema(key_name_="LIGHT_SW3"),
        "light_dim0": _float_output_schemar(key_name_="LIGHT_DIM0"),
        "light_dim1": _float_output_schemar(key_name_="LIGHT_DIM1"),
        "light_dim2": _float_output_schemar(key_name_="LIGHT_DIM2"),
        "light_dim3": _float_output_schemar(key_name_="LIGHT_DIM3"),
        "light_dim4": _float_output_schemar(key_name_="LIGHT_DIM4"),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await light.register_light(var, config)
    if CONF_KEY_NAME in config:
        cg.add(var.set_key_name(config[CONF_KEY_NAME]))
    await cg.register_component(var, config)
    await cg.register_parented(var, parent)
    cg.add(getattr(parent, f"set_{config[CONF_TYPE]}_output")(var))
