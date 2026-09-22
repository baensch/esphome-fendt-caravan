#ifdef USE_ESP32
#include "fendt_climate.h"

static const char *const TAG = "fc.climate";

namespace esphome::fendt_caravan {

void FendtClimate::setup() {
  this->mode = climate::CLIMATE_MODE_HEAT;
  if (this->heater_switch_ != nullptr) {
    this->heater_switch_->publish_state(true);
    this->heater_switch_->add_on_state_callback([this](bool state) {
      if (state) {
        this->mode = climate::CLIMATE_MODE_HEAT;
        this->action = climate::CLIMATE_ACTION_HEATING;
      } else {
        this->mode = climate::CLIMATE_MODE_OFF;
        this->action = climate::CLIMATE_ACTION_OFF;
      }
      this->publish_state();
    });
  }
  if (!this->key_name_.empty()) {
    auto *variable = static_cast<Variable<float> *>(this->get_parent()->get_variable(this->key_name_));
    if (variable != nullptr) {
      this->set_variable(variable);
    }
  }
  this->sensor_->add_on_state_callback([this](float state) {
    this->current_temperature = state;
    // current temperature changed, publish state
    this->publish_state();
  });
  this->current_temperature = this->sensor_->state;
  this->action = climate::CLIMATE_ACTION_HEATING;
  this->publish_state();
}

void FendtClimate::dump_config() {
  LOG_CLIMATE(TAG, "Caravan Climate Control", this);
  LOG_SENSOR(TAG, "  Temperature sensor", this->sensor_);
}
Trigger<> *FendtClimate::get_heat_action_trigger() { return &this->heat_action_trigger_; }
Trigger<> *FendtClimate::get_off_action_trigger() { return &this->off_action_trigger_; }

void FendtClimate::control(const climate::ClimateCall &call) {
  Trigger<> *trig = &this->heat_action_trigger_;
  bool action_ready = false;
  bool publish_state = false;
  if (call.get_mode().has_value()) {
    climate::ClimateMode mode = call.get_mode().value();
    if (this->mode == mode)
      return;
    this->mode = mode;
    publish_state = true;
    if (mode == climate::CLIMATE_MODE_OFF) {
      this->action = climate::CLIMATE_ACTION_OFF;
      trig = &this->off_action_trigger_;
      action_ready = true;
      if (this->heater_switch_ != nullptr) {
        this->heater_switch_->turn_off();
      }
    } else if (mode == climate::CLIMATE_MODE_HEAT) {
      this->action = climate::CLIMATE_ACTION_HEATING;
      trig = &this->heat_action_trigger_;
      action_ready = true;
      if (this->heater_switch_ != nullptr) {
        this->heater_switch_->turn_on();
      }
    }
    if (action_ready) {
      if (this->prev_action_trigger_ != nullptr) {
        this->prev_action_trigger_->stop_action();
        this->prev_action_trigger_ = nullptr;
      }
      this->prev_action_trigger_ = trig;
      trig->trigger();
    }
  }
  if (call.get_target_temperature().has_value()) {
    float target_temp = call.get_target_temperature().value();
    if (this->variable_) {
      this->variable_->set_value(target_temp);
      this->parent_->on_state_change_command(this->key_name_, this->variable_->get_command());
    }
    ESP_LOGV(TAG, "Control call. Target temperature: %.2f", target_temp);
  }
  if (publish_state)
    this->publish_state();
}

climate::ClimateTraits FendtClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE | climate::CLIMATE_SUPPORTS_ACTION);
  traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::ClimateMode::CLIMATE_MODE_HEAT});
  traits.set_visual_min_temperature(5.0);
  traits.set_visual_max_temperature(30.0);
  traits.set_visual_temperature_step(0.5);
  traits.set_supported_presets({
      climate::CLIMATE_PRESET_HOME,
      climate::CLIMATE_PRESET_AWAY,
  });
  return traits;
}

void FendtClimate::on_decoded(const float &target) {
  ESP_LOGV(TAG, "On Decoded called");
  this->target_temperature = target;
  this->publish_state();
}
}  // namespace esphome::fendt_caravan
#endif
