#include "truma_device_sensor.h"
#include <cmath>

#ifdef USE_ESP32
namespace esphome::fendt_caravan {
static const char *const TAG = "FC.truma";

void TrumaDeviceSensor::setup() {
  auto *th_available = new Variable<std::string>("TH_AVAILABLE", [](const std::string &value) {
    const char *tmp[] = {"Available", "Not available"};
    return DeviceDecoders::decode_bool_str(value, tmp);
  });
  this->add_variable(th_available);

  auto *th_type = new Variable<int>("TH_TYPE", DeviceDecoders::decode_int);
  this->add_variable(th_type);

  auto *th_a_en = new Variable<bool>("TH_A_EN", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(th_a_en);

  auto *th_a_t = new Variable<float>(
      "TH_A_T", DeviceDecoders::decode_temperature,
      [](const std::string &name, float temp) {
        return Commands::update_int(name, (uint16_t) std::round(temp));
      });
  this->add_variable(th_a_t);

  auto *th_es = new Variable<int>("TH_ES", DeviceDecoders::decode_int);
  this->add_variable(th_es);

  auto *th_mm = new Variable<int>("TH_MM", DeviceDecoders::decode_int);
  this->add_variable(th_mm);

  // Hot water boiler variables (Story 02 preparation)
  auto *tt_available = new Variable<std::string>("TT_AVAILABLE", [](const std::string &value) {
    const char *tmp[] = {"Available", "Not available"};
    return DeviceDecoders::decode_bool_str(value, tmp);
  });
  this->add_variable(tt_available);

  auto *th_w_en = new Variable<bool>("TH_W_EN", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(th_w_en);

  auto *th_w_t = new Variable<float>(
      "TH_W_T", DeviceDecoders::decode_temperature,
      [](const std::string &name, float temp) {
        return Commands::update_int(name, (uint16_t) std::round(temp));
      });
  this->add_variable(th_w_t);
}

void TrumaDeviceSensor::dump_config() {
  ESP_LOGCONFIG(TAG, " -Fendt Truma Device-");
  LOG_TEXT_SENSOR(TAG, "  Truma Available", this->truma_available_text_sensor_);
  LOG_SWITCH(TAG, "  Truma Air Heating Switch", this->truma_heater_switch_);
}

void TrumaDeviceSensor::on_state_change_command(const std::string &tag, const std::string &command) {
  if (!command.empty()) {
    ESP_LOGD(TAG, "Truma state changed command: %s", command.c_str());
    this->command_callback_.call(command);
  }
}

}  // namespace esphome::fendt_caravan
#endif
