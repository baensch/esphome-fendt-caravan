#include "lighting_device_sensor.h"

#ifdef USE_ESP32
namespace esphome::fendt_caravan {
static const char *const TAG = "FC.lights";

void LightingDeviceSensor::setup() {
  if (this->light_sw0_ != nullptr) {
    auto *light_sw0 = new Variable<int>("LIGHT_SW0", DeviceDecoders::decode_int, Commands::update_toggle<bool>);
    this->add_variable(light_sw0);
  }

  if (this->light_sw1_ != nullptr) {
    auto *light_sw1 = new Variable<int>("LIGHT_SW1", DeviceDecoders::decode_int, Commands::update_toggle<bool>);
    this->add_variable(light_sw1);
  }

  if (this->light_sw2_ != nullptr) {
    auto *light_sw2 = new Variable<int>("LIGHT_SW2", DeviceDecoders::decode_int, Commands::update_toggle<bool>);
    this->add_variable(light_sw2);
  }

  if (this->light_sw3_ != nullptr) {
    auto *light_sw3 = new Variable<int>("LIGHT_SW3", DeviceDecoders::decode_int, Commands::update_toggle<bool>);
    this->add_variable(light_sw3);
  }

  if (this->light_dim0_ != nullptr) {
    auto *light_dim0 = this->create_variable_("LIGHT_DIM0");
    this->add_variable(light_dim0);
  }

  if (this->light_dim1_ != nullptr) {
    auto *light_dim1 = this->create_variable_("LIGHT_DIM1");
    this->add_variable(light_dim1);
  }

  if (this->light_dim2_ != nullptr) {
    auto *light_dim2 = this->create_variable_("LIGHT_DIM2");
    this->add_variable(light_dim2);
  }
  if (this->light_dim3_ != nullptr) {
    auto *light_dim3 = this->create_variable_("LIGHT_DIM3");
    this->add_variable(light_dim3);
  }
  if (this->light_dim4_ != nullptr) {
    auto *light_dim4 = this->create_variable_("LIGHT_DIM4");
    this->add_variable(light_dim4);
  }
}

void LightingDeviceSensor::dump_config() { ESP_LOGCONFIG(TAG, "-Fendt Lighting Device-"); }

void LightingDeviceSensor::on_state_change_command(const std::string &tag, const std::string &command) {
  if (!command.empty()) {
    ESP_LOGD(TAG, "Switch state changed command:%s", command.c_str());
    this->command_callback_.call(command);
  }
}
}  // namespace esphome::fendt_caravan
#endif
