#ifdef USE_ESP32
#include "truma_climate.h"
#include "esphome/components/fendt_caravan/sensor/device_commands.h"
#include <cmath>

namespace esphome::fendt_caravan {

static const char *const TAG = "fc.truma_climate";

void TrumaClimate::setup() {
  this->mode = climate::CLIMATE_MODE_OFF;
  this->action = climate::CLIMATE_ACTION_OFF;

  if (this->parent_ != nullptr) {
    auto *var_available = static_cast<Variable<std::string> *>(this->parent_->get_variable("TH_AVAILABLE"));
    if (var_available != nullptr) {
      var_available->set_on_decode_callback([this](const std::string &val) {
        bool avail = (val == "Available" || val == "1" || val == "true");
        this->on_available_decoded_(avail);
      });
      if (var_available->is_active()) {
        const std::string &val = var_available->get_value();
        bool avail = (val == "Available" || val == "1" || val == "true");
        this->on_available_decoded_(avail);
      }
    }

    auto *var_power = static_cast<Variable<bool> *>(this->parent_->get_variable("TH_A_EN"));
    if (var_power != nullptr) {
      var_power->set_on_decode_callback([this](bool power) {
        this->on_power_decoded_(power);
      });
      if (var_power->is_active()) {
        this->on_power_decoded_(var_power->get_value());
      }
    }

    auto *var_target = static_cast<Variable<float> *>(this->parent_->get_variable("TH_A_T"));
    if (var_target != nullptr) {
      var_target->set_on_decode_callback([this](float target) {
        this->on_target_temperature_decoded_(target);
      });
      if (var_target->is_active()) {
        this->on_target_temperature_decoded_(var_target->get_value());
      }
    }
  }

  if (this->sensor_ != nullptr) {
    this->sensor_->add_on_state_callback([this](float state) {
      this->current_temperature = state;
      if (this->is_available_) {
        this->publish_state();
      }
    });
    if (this->sensor_->has_state()) {
      this->current_temperature = this->sensor_->state;
    }
  }
}

void TrumaClimate::dump_config() {
  LOG_CLIMATE(TAG, "Truma Combi Climate Control", this);
  LOG_SENSOR(TAG, "  Inside Temperature Sensor", this->sensor_);
}

void TrumaClimate::set_available(bool available) {
  this->on_available_decoded_(available);
}

void TrumaClimate::on_available_decoded_(bool available) {
  ESP_LOGD(TAG, "TH_AVAILABLE: %s", available ? "true" : "false");
  bool prev = this->is_available_;
  this->is_available_ = available;
  if (available && !prev) {
    this->publish_state();
  }
}

void TrumaClimate::on_power_decoded_(bool power) {
  ESP_LOGD(TAG, "TH_A_EN decoded: %s", power ? "On" : "Off");
  this->power_state_ = power;
  if (power) {
    this->mode = climate::CLIMATE_MODE_HEAT;
    this->action = climate::CLIMATE_ACTION_HEATING;
  } else {
    this->mode = climate::CLIMATE_MODE_OFF;
    this->action = climate::CLIMATE_ACTION_OFF;
  }
  if (this->is_available_) {
    this->publish_state();
  }
}

void TrumaClimate::on_target_temperature_decoded_(float target) {
  ESP_LOGD(TAG, "TH_A_T decoded: %.1f", target);
  this->target_temperature = target;
  if (this->is_available_) {
    this->publish_state();
  }
}

void TrumaClimate::control(const climate::ClimateCall &call) {
  if (!this->is_available_) {
    ESP_LOGW(TAG, "Truma heater is not available, ignoring control call");
    return;
  }

  if (call.get_mode().has_value()) {
    climate::ClimateMode new_mode = call.get_mode().value();
    if (new_mode == climate::CLIMATE_MODE_HEAT && !this->power_state_) {
      // Turn on: only send cmd-tgl:TH_A_EN if currently Off
      ESP_LOGI(TAG, "Turning Truma heating ON (cmd-tgl:TH_A_EN)");
      this->parent_->on_state_change_command("TH_A_EN", "cmd-tgl:TH_A_EN");
    } else if (new_mode == climate::CLIMATE_MODE_OFF && this->power_state_) {
      // Turn off: only send cmd-tgl:TH_A_EN if currently On
      ESP_LOGI(TAG, "Turning Truma heating OFF (cmd-tgl:TH_A_EN)");
      this->parent_->on_state_change_command("TH_A_EN", "cmd-tgl:TH_A_EN");
    }
  }

  if (call.get_target_temperature().has_value()) {
    float target = call.get_target_temperature().value();
    uint16_t int_target = (uint16_t) std::round(target);
    std::string cmd = Commands::update_int("TH_A_T", int_target);
    ESP_LOGI(TAG, "Setting Truma target temperature: %s", cmd.c_str());
    this->parent_->on_state_change_command("TH_A_T", cmd);
  }
}

climate::ClimateTraits TrumaClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE | climate::CLIMATE_SUPPORTS_ACTION);
  traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT});
  traits.set_visual_min_temperature(5.0f);
  traits.set_visual_max_temperature(30.0f);
  traits.set_visual_temperature_step(1.0f);
  return traits;
}

}  // namespace esphome::fendt_caravan
#endif
