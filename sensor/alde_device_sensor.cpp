#include "alde_device_sensor.h"

#ifdef USE_ESP32
namespace esphome::fendt_caravan {
static const char *const TAG = "FC.alde";

void AldeDeviceSensor::setup() {
  auto *alde_available = new Variable<std::string>("HEATER_AVAILABLE", [](const std::string &value) {
    const char *tmp[] = {"Available", "Not available"};
    return DeviceDecoders::decode_bool_str(value, tmp);
  });
  this->add_variable(alde_available);

  auto *heater_status = new Variable<bool>("HEATER_ONOFF", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(heater_status);

  auto *heater_temp = new Variable<float>("HEATER_TEMP", DeviceDecoders::decode_temperature, Commands::update_temp_10);
  this->add_variable(heater_temp);

  auto *heater_water = new Variable<bool>("HEATER_WATER", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(heater_water);

  auto *heater_water_temp = new Variable<bool>(
      "HEATER_WATER_TEMP", [](const std::string &data) { return DeviceDecoders::decode_temperature(data) == 65.0f; },
      Commands::update_toggle<bool>);
  this->add_variable(heater_water_temp);

  auto *heater_el =
      new Variable<std::string>("HEATER_EL", DeviceDecoders::decode_heater_el, Commands::update_heater_el);
  this->add_variable(heater_el);

  auto *heater_gas = new Variable<bool>("HEATER_GAS", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(heater_gas);
}

void AldeDeviceSensor::dump_config() {
  ESP_LOGCONFIG(TAG, " -Fendt Alde Device-");
  LOG_TEXT_SENSOR(TAG, "  Alde Sensor", this->alde_available_text_sensor_);
  LOG_SWITCH(TAG, "  Alde Status Switch", this->alde_heater_switch_);
  // LOG_NUMBER(TAG, "  Heater Temperature", this->alde_heater_temperature_number_);
  LOG_SWITCH(TAG, "  Heater Water", this->alde_heater_water_switch_);
  LOG_SWITCH(TAG, "  Water Temperature", this->alde_heater_water_temperature_switch_);
  LOG_SELECT(TAG, "  Heater Electric", this->alde_heater_electricity_select_);
  LOG_SWITCH(TAG, "  Heater Gas", this->alde_heater_gas_switch_);
}

void AldeDeviceSensor::on_state_change_command(const std::string &tag, const std::string &command) {
  if (!command.empty()) {
    ESP_LOGD(TAG, "Switch state changed command:%s", command.c_str());
    this->command_callback_.call(command);
  }
}

}  // namespace esphome::fendt_caravan
#endif
