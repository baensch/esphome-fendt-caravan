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
  if (this->key_name_.empty())
    return;

  bool cur_on = (this->variable_ != nullptr && this->variable_->get_value() != 0);
  bool target_on = state->current_values.is_on();
  std::string command = "";

  if (this->color_mode_ == light::ColorMode::BRIGHTNESS) {
    float brightness = state->remote_values.get_brightness();
    uint8_t target_level = (uint8_t) std::round(brightness * 15.0f);

    ESP_LOGD(TAG, "'%s' write_state dimmable: cur_on=%s (val=%d), target_on=%s, target_level=%d (b=%.2f)",
             this->key_name_.c_str(), ONOFF(cur_on),
             this->variable_ ? this->variable_->get_value() : -1,
             ONOFF(target_on), target_level, brightness);

    if (!target_on) {
      if (cur_on) {
        command = "cmd-tgl:" + this->key_name_;
      }
    } else {
      if (target_level == 0) {
        target_level = 1;
      }
      if (!cur_on) {
        if (this->last_brightness_level_ > 0 && target_level == this->last_brightness_level_) {
          command = "cmd-tgl:" + this->key_name_;
        } else {
          command = "cmd-set:" + this->key_name_ + "=" + std::to_string(target_level);
          this->last_brightness_level_ = target_level;
        }
      } else {
        uint8_t cur_level = this->variable_ ? this->variable_->get_value() : 0;
        if (target_level != cur_level) {
          command = "cmd-set:" + this->key_name_ + "=" + std::to_string(target_level);
          this->last_brightness_level_ = target_level;
        }
      }
    }
  } else {
    ESP_LOGD(TAG, "'%s' write_state binary: cur_on=%s, target_on=%s",
             this->key_name_.c_str(), ONOFF(cur_on), ONOFF(target_on));
    if (cur_on != target_on) {
      command = "cmd-tgl:" + this->key_name_;
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
  if (state > 0) {
    this->last_brightness_level_ = state;
  }
  if (this->light_state_ != nullptr) {
    this->light_state_->remote_values.set_state(state != 0);
    if (this->color_mode_ == light::ColorMode::BRIGHTNESS && state > 0) {
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
