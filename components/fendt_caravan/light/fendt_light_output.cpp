#include "fendt_light_output.h"

#ifdef USE_ESP32
namespace esphome::fendt_caravan {

static const char *const TAG = "fc.light";

void FendtLightOutput::setup() {
  if (this->key_name_.empty())
    return;
  auto *variable = static_cast<Variable<uint8_t> *>(this->get_parent()->get_variable(this->key_name_));
  if (variable != nullptr) {
    ESP_LOGD(TAG, "Variable '%s' found at setup", this->key_name_.c_str());
    this->set_variable(variable);
    if (variable->is_active()) {
      this->is_discovered_ = true;
      this->on_decoded(variable->get_value());
    }
  }
}

void FendtLightOutput::write_state(light::LightState *state) {
  std::string command = "";
  if (this->variable_ != nullptr) {
    bool cur_state = this->variable_->get_value() != 0;
    ESP_LOGD(TAG, "'%s' write_state: current=%s, target=%s", this->key_name_.c_str(),
             ONOFF(cur_state), ONOFF(state->current_values.is_on()));
    if (cur_state != state->current_values.is_on()) {
      this->variable_->set_value(cur_state ^ 1);
      command = this->variable_->get_command();
    } else if (this->color_mode_ == light::ColorMode::BRIGHTNESS) {
      this->variable_->set_value((uint8_t) (state->remote_values.get_brightness() * 15.0f));
      command = this->variable_->get_alt_command();
    }
  }
  if (!command.empty()) {
    ESP_LOGI(TAG, "'%s' sending command: %s", this->key_name_.c_str(), command.c_str());
    this->parent_->on_state_change_command(this->key_name_, command);
  }
}

void FendtLightOutput::on_decoded(const uint8_t &state) {
  ESP_LOGD(TAG, "'%s' on_decoded value: %d", this->key_name_.c_str(), state);
  this->is_discovered_ = true;
  if (this->light_state_ != nullptr) {
    this->light_state_->remote_values.set_state(state != 0);
    if (this->color_mode_ == light::ColorMode::BRIGHTNESS) {
      this->light_state_->remote_values.set_brightness((float) (state) / 15.0f);
    }
    this->light_state_->publish_state();
  }
  if (this->variable_ != nullptr) {
    this->variable_->set_value(state);
  }
}

}  // namespace esphome::fendt_caravan
#endif
