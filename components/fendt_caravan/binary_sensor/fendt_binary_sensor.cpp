#ifdef USE_ESP32

#include "fendt_binary_sensor.h"

namespace esphome::fendt_caravan {

static const char *const TAG = "FC.BS";

void FendtBinarySensor::setup() {
  if (this->key_name_.empty())
    return;
  auto *variable = this->get_parent()->get_variable(this->key_name_);
  if (variable != nullptr) {
    variable->add_bool_callback([this](bool state) {
      ESP_LOGV(TAG, "Decoded bool for: %s value: %s", this->key_name_.c_str(), state ? "true" : "false");
      this->publish_state(state);
    });
    if (variable->is_active()) {
      this->publish_state(variable->get_bool_value());
    }
  }
}

void FendtBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "Fendt Caravan Binary Sensor", this);
  ESP_LOGCONFIG(TAG, "  Key: %s", this->key_name_.c_str());
}

}  // namespace esphome::fendt_caravan
#endif
