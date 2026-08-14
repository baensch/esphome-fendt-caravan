#pragma once

#ifdef USE_ESP32

#include "esphome/components/fendt_caravan/caravan_component_base.h"
#include "esphome/components/fendt_caravan/variable.h"
#include "esphome/components/select/select.h"
#include "esphome/core/string_ref.h"
#include "esphome/core/log.h"

namespace esphome::fendt_caravan {
class FendtSelect : public CaravanComponentBase<std::string>, public select::Select {
 public:
  void setup() override {
    if (this->key_name_.empty())
      return;
    auto *variable = static_cast<Variable<std::string> *>(this->get_parent()->get_variable(this->key_name_));
    if (variable != nullptr) {
      this->set_variable(variable);
    }
  }

 protected:
  void control(const std::string &value) override {
    std::string command = "";
    if (this->variable_ != nullptr) {
      this->variable_->set_value(value);
      command = this->variable_->get_command();
    }
    this->parent_->on_state_change_command(this->key_name_, command);
  }
  void on_decoded(const std::string &value) override { this->publish_state(value); }

 private:
};

}  // namespace esphome::fendt_caravan
#endif
