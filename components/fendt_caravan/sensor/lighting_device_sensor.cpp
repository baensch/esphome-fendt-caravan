#include "lighting_device_sensor.h"

#ifdef USE_ESP32
namespace esphome::fendt_caravan {
static const char *const TAG = "fc.lighting_device";

void LightingDeviceSensor::setup() {
  for (auto *light_output : this->light_outputs_) {
    const std::string &key_name = light_output->get_key_name();
    if (key_name.empty())
      continue;

    if (this->get_variable(key_name) == nullptr) {
      if (key_name.rfind("LIGHT_DIM", 0) == 0) {
        auto *var = this->create_dim_variable_(key_name);
        this->add_variable(var);
      } else {
        auto *var = this->create_sw_variable_(key_name);
        this->add_variable(var);
      }
    }
  }
}

void LightingDeviceSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "-Fendt Lighting Device-");
  ESP_LOGCONFIG(TAG, "  Configured Light Outputs: %zu", this->light_outputs_.size());
  for (auto *light_output : this->light_outputs_) {
    ESP_LOGCONFIG(TAG, "    Light Key: %s (discovered: %s)",
                  light_output->get_key_name().c_str(),
                  light_output->is_discovered() ? "yes" : "no");
  }
}

bool LightingDeviceSensor::decode(const std::string &name, const std::string &value) {
  auto *variable = this->get_variable(name);
  if (variable == nullptr && name.rfind("LIGHT_", 0) == 0) {
    ESP_LOGI(TAG, "Dynamically discovered light variable: %s", name.c_str());
    if (name.rfind("LIGHT_DIM", 0) == 0) {
      variable = this->create_dim_variable_(name);
    } else {
      variable = this->create_sw_variable_(name);
    }
    this->add_variable(variable);

    for (auto *output : this->light_outputs_) {
      if (output->get_key_name() == name) {
        output->set_variable(static_cast<Variable<uint8_t> *>(variable));
      }
    }
  }

  if (variable != nullptr) {
    variable->decode(value);
    this->on_data_decoded(variable);
    return true;
  }
  return false;
}

void LightingDeviceSensor::on_state_change_command(const std::string &tag, const std::string &command) {
  if (!command.empty()) {
    ESP_LOGD(TAG, "Light state change command: %s", command.c_str());
    this->command_callback_.call(command);
  }
}

Variable<uint8_t> *LightingDeviceSensor::create_sw_variable_(const std::string &name) {
  return new Variable<uint8_t>(
      name,
      [](const std::string &data) {
        int val = DeviceDecoders::decode_int(data);
        return (uint8_t) (val != 0 ? 1 : 0);
      },
      Commands::update_toggle<uint8_t>);
}

Variable<uint8_t> *LightingDeviceSensor::create_dim_variable_(const std::string &name) {
  return new Variable<uint8_t>(
      name,
      [](const std::string &data) {
        int val = DeviceDecoders::decode_int(data);
        return (uint8_t) val;
      },
      Commands::update_toggle<uint8_t>,
      [](const std::string &name, uint8_t state) {
        return Commands::update_int(name, state);
      });
}
}  // namespace esphome::fendt_caravan
#endif
