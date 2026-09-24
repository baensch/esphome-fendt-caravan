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
      if (light_output->is_dimmable() || key_name.rfind("LIGHT_DIM", 0) == 0) {
        auto *var = this->create_dim_variable_(key_name);
        this->add_variable(var);
      } else {
        auto *var = this->create_sw_variable_(key_name);
        this->add_variable(var);
      }
    }
  }
}

void LightingDeviceSensor::loop() {
  if (this->is_busy_ && (millis() - this->busy_timestamp_ > LIGHT_BUSY_TIMEOUT_MS)) {
    ESP_LOGW(TAG, "LIGHT_BUSY timeout (>%ums), resetting busy flag", LIGHT_BUSY_TIMEOUT_MS);
    this->is_busy_ = false;
    this->flush_pending_commands_();
  }
}

void LightingDeviceSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "-Fendt Lighting Device-");
  ESP_LOGCONFIG(TAG, "  Configured Light Outputs: %zu", this->light_outputs_.size());
  for (auto *light_output : this->light_outputs_) {
    ESP_LOGCONFIG(TAG, "    Light Key: %s (dimmable: %s, discovered: %s)",
                  light_output->get_key_name().c_str(),
                  light_output->is_dimmable() ? "yes" : "no",
                  light_output->is_discovered() ? "yes" : "no");
  }
}

bool LightingDeviceSensor::decode(const std::string &name, const std::string &value) {
  if (name == "LIGHT_BUSY") {
    int val = DeviceDecoders::decode_int(value);
    bool was_busy = this->is_busy_;
    this->is_busy_ = (val != 0);
    this->busy_timestamp_ = millis();
    ESP_LOGD(TAG, "LIGHT_BUSY: %d (busy=%s)", val, this->is_busy_ ? "YES" : "NO");
    if (was_busy && !this->is_busy_) {
      this->flush_pending_commands_();
    }
    return true;
  }

  auto *variable = this->get_variable(name);
  if (variable == nullptr && name.rfind("LIGHT_", 0) == 0) {
    ESP_LOGI(TAG, "Dynamically discovered light variable: %s", name.c_str());
    bool dimmable = (name.rfind("LIGHT_DIM", 0) == 0);
    for (auto *output : this->light_outputs_) {
      if (output->get_key_name() == name && output->is_dimmable()) {
        dimmable = true;
        break;
      }
    }
    if (dimmable) {
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
  if (command.empty())
    return;

  if (this->is_busy_ && (millis() - this->busy_timestamp_ > LIGHT_BUSY_TIMEOUT_MS)) {
    ESP_LOGW(TAG, "LIGHT_BUSY timeout (>%ums), resetting busy flag", LIGHT_BUSY_TIMEOUT_MS);
    this->is_busy_ = false;
  }

  if (this->is_busy_) {
    ESP_LOGD(TAG, "MCU is BUSY. Buffering command for '%s': %s", tag.c_str(), command.c_str());
    this->pending_commands_[tag] = command;
    return;
  }

  ESP_LOGD(TAG, "Light state change command: %s", command.c_str());
  this->command_callback_.call(command);
}

void LightingDeviceSensor::flush_pending_commands_() {
  if (this->pending_commands_.empty())
    return;

  auto it = this->pending_commands_.begin();
  if (it != this->pending_commands_.end()) {
    std::string tag = it->first;
    std::string cmd = it->second;
    this->pending_commands_.erase(it);
    ESP_LOGI(TAG, "Flushing buffered command for '%s': %s", tag.c_str(), cmd.c_str());
    this->command_callback_.call(cmd);
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
        return (uint8_t) std::min(15, std::max(0, val));
      },
      Commands::update_toggle<uint8_t>,
      [](const std::string &name, uint8_t state) {
        return Commands::update_int(name, state);
      });
}
}  // namespace esphome::fendt_caravan
#endif
