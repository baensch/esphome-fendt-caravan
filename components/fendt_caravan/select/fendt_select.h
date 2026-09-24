#pragma once

#ifdef USE_ESP32

#include "esphome/components/fendt_caravan/caravan_component_base.h"
#include "esphome/components/fendt_caravan/variable.h"
#include "esphome/components/select/select.h"
#include "esphome/core/string_ref.h"
#include "esphome/core/log.h"

namespace esphome::fendt_caravan {
class FendtSelect : public CaravanComponentBase<std::string>, public select::Select {
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
