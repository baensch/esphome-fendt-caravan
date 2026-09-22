#include "fendt_light_output.h"

#ifdef USE_ESP32
namespace esphome::fendt_caravan {

static const char *const TAG = "raw.LightOutput";

void FendtLightOutput::setup() {
  if (this->key_name_.empty())
    return;
  auto *variable = static_cast<Variable<uint8_t> *>(this->get_parent()->get_variable(this->key_name_));
  if (variable != nullptr) {
    ESP_LOGD(TAG, "Variable is found.");
    this->set_variable(variable);
  }
}

void FendtLightOutput::write_state(light::LightState *state) {
  std::string command = "";
  if (this->variable_ != nullptr) {
    bool cur_state = this->variable_->get_value() != 0;
    ESP_LOGD(TAG, "State: %s, brightness: %.2f", ONOFF(cur_state), state->remote_values.get_brightness());
    if (cur_state != state->current_values.is_on()) {
      this->variable_->set_value(cur_state ^ 1);
      command = this->variable_->get_command();
    } else if (this->color_mode_ == light::ColorMode::BRIGHTNESS) {
      this->variable_->set_value((uint8_t) (state->remote_values.get_brightness() * 15.0f));
      command = this->variable_->get_alt_command();
    }
  }
  ESP_LOGD(TAG, "Command :%s", command.c_str());
  this->parent_->on_state_change_command(this->key_name_, command);
}

void FendtLightOutput::on_decoded(const uint8_t &state) {
  ESP_LOGD(TAG, "on_decoded value: %d", state);
  this->light_state_->remote_values.set_state(state != 0);
  if (this->color_mode_ == light::ColorMode::BRIGHTNESS) {
    this->light_state_->remote_values.set_brightness((float) (state) / 15.0f);
  }
  this->light_state_->publish_state();
  this->variable_->set_value(state);
}

}  // namespace esphome::fendt_caravan
#endif
