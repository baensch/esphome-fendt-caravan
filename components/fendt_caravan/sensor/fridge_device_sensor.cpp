#include "fridge_device_sensor.h"

#ifdef USE_ESP32
namespace esphome::fendt_caravan {
static const char *const TAG = "FC.fridge";

void FridgeDeviceSensor::setup() {
  auto *fridge_available = new Variable<std::string>("FRIDGE_AVAILABLE", [](const std::string &value) {
    const char *tmp[] = {"Available", "Not available"};
    return DeviceDecoders::decode_bool_str(value, tmp);
  });
  this->add_variable(fridge_available);

  auto *fridge_type = new Variable<int>("FRIDGE_TYPE", DeviceDecoders::decode_int);
  this->add_variable(fridge_type);

  auto *fridge_on_off =
      new Variable<bool>("FRIDGE_ON_OFF", DeviceDecoders::decode_bool, Commands::update_toggle<bool>);
  this->add_variable(fridge_on_off);

  auto *fridge_source = new Variable<std::string>(
      "FRIDGE_SOURCE",
      [](const std::string &value) -> std::string {
        int idx = DeviceDecoders::decode_int(value);
        switch (idx) {
          case 0:
            return "Auto";
          case 1:
            return "230V";
          case 2:
            return "12V";
          case 3:
            return "Gas";
          default:
            return value;
        }
      },
      [](const std::string &name, const std::string &value) -> std::string {
        std::string val_str = "0";
        if (value == "Auto" || value == "auto" || value == "0") {
          val_str = "0";
        } else if (value == "230V" || value == "240V" || value == "230 V" || value == "240 V" || value == "1") {
          val_str = "1";
        } else if (value == "12V" || value == "12 V" || value == "2") {
          val_str = "2";
        } else if (value == "Gas" || value == "gas" || value == "3") {
          val_str = "3";
        } else {
          val_str = value;
        }
        return "cmd-set:" + name + "=" + val_str;
      });
  this->add_variable(fridge_source);

  auto *fridge_temp = new Variable<std::string>(
      "FRIDGE_TEMP",
      [](const std::string &value) -> std::string {
        std::string v = value;
        v.erase(std::remove_if(v.begin(), v.end(), ::isspace), v.end());
        return v;
      },
      [](const std::string &name, const std::string &value) -> std::string {
        return "cmd-set:" + name + "=" + value;
      });
  this->add_variable(fridge_temp);

  auto *fridge_mode = new Variable<std::string>(
      "FRIDGE_MODE",
      [](const std::string &value) -> std::string {
        int idx = DeviceDecoders::decode_int(value);
        return (idx == 1) ? "Night" : "Normal";
      },
      [](const std::string &name, const std::string &value) -> std::string {
        std::string val_str = (value == "Night" || value == "Quiet" || value == "1") ? "1" : "0";
        return "cmd-set:" + name + "=" + val_str;
      });
  this->add_variable(fridge_mode);
}

void FridgeDeviceSensor::dump_config() {
  ESP_LOGCONFIG(TAG, " -Fendt Refrigerator Device-");
  LOG_TEXT_SENSOR(TAG, "  Fridge Available", this->fridge_available_text_sensor_);
  LOG_BINARY_SENSOR("  ", "Fridge Available", this->fridge_available_binary_sensor_);
  LOG_SWITCH(TAG, "  Fridge Switch", this->fridge_switch_);
  LOG_SELECT(TAG, "  Fridge Source", this->fridge_source_select_);
  LOG_SELECT(TAG, "  Fridge Intensity", this->fridge_temp_select_);
  LOG_SELECT(TAG, "  Fridge Mode", this->fridge_mode_select_);
}

void FridgeDeviceSensor::on_state_change_command(const std::string &tag, const std::string &command) {
  if (!command.empty()) {
    ESP_LOGD(TAG, "Fridge state changed command: %s", command.c_str());
    this->command_callback_.call(command);
  }
}

}  // namespace esphome::fendt_caravan
#endif
