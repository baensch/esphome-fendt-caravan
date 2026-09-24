#ifdef USE_ESP32
#include "truma_water_heater.h"
#include "esphome/components/fendt_caravan/device_commands.h"
#include "esphome/components/fendt_caravan/device_decoders.h"
#include <cmath>

namespace esphome::fendt_caravan {

static const char *const TAG = "fc.truma_wh";

static constexpr float TEMP_ECO = 40.0f;
static constexpr float TEMP_PERFORMANCE = 60.0f;
static constexpr float TEMP_HIGH_DEMAND = 70.0f;
static constexpr float THRESHOLD_PERFORMANCE = 45.0f;
static constexpr float THRESHOLD_HIGH_DEMAND = 65.0f;

void TrumaWaterHeater::setup() {
  this->mode_ = water_heater::WATER_HEATER_MODE_OFF;
  this->set_state_flag_(water_heater::WATER_HEATER_STATE_ON, false);

  if (this->parent_ != nullptr) {
    auto *var_th_avail = static_cast<Variable<std::string> *>(this->parent_->get_variable("TH_AVAILABLE"));
    if (var_th_avail != nullptr) {
      var_th_avail->set_on_decode_callback([this](const std::string &val) {
        this->on_th_available_decoded_(DeviceDecoders::decode_bool(val));
      });
      if (var_th_avail->is_active()) {
        this->on_th_available_decoded_(DeviceDecoders::decode_bool(var_th_avail->get_value()));
      }
    }

    auto *var_tt_avail = static_cast<Variable<std::string> *>(this->parent_->get_variable("TT_AVAILABLE"));
    if (var_tt_avail != nullptr) {
      var_tt_avail->set_on_decode_callback([this](const std::string &val) {
        this->on_tt_available_decoded_(DeviceDecoders::decode_bool(val));
      });
      if (var_tt_avail->is_active()) {
        this->on_tt_available_decoded_(DeviceDecoders::decode_bool(var_tt_avail->get_value()));
      }
    }

    auto *var_power = static_cast<Variable<bool> *>(this->parent_->get_variable("TH_W_EN"));
    if (var_power != nullptr) {
      var_power->set_on_decode_callback([this](bool power) {
        this->on_power_decoded_(power);
      });
      if (var_power->is_active()) {
        this->on_power_decoded_(var_power->get_value());
      }
    }

    auto *var_target = static_cast<Variable<float> *>(this->parent_->get_variable("TH_W_T"));
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
      this->current_temperature_ = state;
      if (this->is_available_) {
        this->publish_state();
      }
    });
    if (this->sensor_->has_state()) {
      this->current_temperature_ = this->sensor_->state;
    }
  }
}

void TrumaWaterHeater::dump_config() {
  LOG_WATER_HEATER("", "Truma Combi Water Heater", this);
  this->dump_traits_(TAG);
}

void TrumaWaterHeater::publish_state() {
  if (!this->is_available_) {
    return;
  }
  water_heater::WaterHeater::publish_state();
}

void TrumaWaterHeater::on_th_available_decoded_(bool available) {
  ESP_LOGD(TAG, "TH_AVAILABLE decoded: %s", available ? "true" : "false");
  this->th_available_ = available;
  this->update_availability_();
}

void TrumaWaterHeater::on_tt_available_decoded_(bool available) {
  ESP_LOGD(TAG, "TT_AVAILABLE decoded: %s", available ? "true" : "false");
  this->tt_available_ = available;
  this->update_availability_();
}

void TrumaWaterHeater::update_availability_() {
  bool prev = this->is_available_;
  this->is_available_ = this->th_available_ && this->tt_available_;
  ESP_LOGD(TAG, "Water heater availability: %s (TH: %s, TT: %s)",
           this->is_available_ ? "available" : "unavailable",
           this->th_available_ ? "true" : "false",
           this->tt_available_ ? "true" : "false");
  if (this->is_available_ && !prev) {
    this->publish_state();
  }
}

void TrumaWaterHeater::on_power_decoded_(bool power) {
  ESP_LOGD(TAG, "TH_W_EN decoded: %s", power ? "On" : "Off");
  this->power_state_ = power;
  this->set_state_flag_(water_heater::WATER_HEATER_STATE_ON, power);
  if (!power) {
    this->mode_ = water_heater::WATER_HEATER_MODE_OFF;
  } else {
    this->update_mode_from_target_();
  }
  if (this->is_available_) {
    this->publish_state();
  }
}

void TrumaWaterHeater::on_target_temperature_decoded_(float target) {
  ESP_LOGD(TAG, "TH_W_T decoded: %.1f", target);
  if (target > 0.0f) {
    this->target_temperature_ = target;
  } else if (!this->power_state_) {
    this->target_temperature_ = NAN;
  }
  if (this->power_state_) {
    this->update_mode_from_target_();
  }
  if (this->is_available_) {
    this->publish_state();
  }
}

void TrumaWaterHeater::update_mode_from_target_() {
  if (!this->power_state_) {
    this->mode_ = water_heater::WATER_HEATER_MODE_OFF;
  } else if (!std::isnan(this->target_temperature_) && this->target_temperature_ > THRESHOLD_HIGH_DEMAND) {
    this->mode_ = water_heater::WATER_HEATER_MODE_HIGH_DEMAND;
  } else if (!std::isnan(this->target_temperature_) && this->target_temperature_ > THRESHOLD_PERFORMANCE) {
    this->mode_ = water_heater::WATER_HEATER_MODE_PERFORMANCE;
  } else {
    this->mode_ = water_heater::WATER_HEATER_MODE_ECO;
  }
}

void TrumaWaterHeater::control(const water_heater::WaterHeaterCall &call) {
  if (!this->is_available_) {
    ESP_LOGW(TAG, "Truma water heater is not available, ignoring control call");
    return;
  }

  bool has_mode = call.get_mode().has_value();
  bool has_on = call.get_on().has_value();
  bool has_target = !std::isnan(call.get_target_temperature());

  optional<bool> want_on;
  if (has_on) {
    want_on = call.get_on().value();
  } else if (has_mode) {
    want_on = (call.get_mode().value() != water_heater::WATER_HEATER_MODE_OFF);
  }

  optional<float> target_temp;
  if (has_target) {
    target_temp = call.get_target_temperature();
    if (!want_on.has_value()) {
      want_on = (target_temp.value() > 0.0f);
    }
  } else if (has_mode) {
    switch (call.get_mode().value()) {
      case water_heater::WATER_HEATER_MODE_ECO:
        target_temp = TEMP_ECO;
        break;
      case water_heater::WATER_HEATER_MODE_PERFORMANCE:
        target_temp = TEMP_PERFORMANCE;
        break;
      case water_heater::WATER_HEATER_MODE_HIGH_DEMAND:
        target_temp = TEMP_HIGH_DEMAND;
        break;
      default:
        break;
    }
  }

  if (want_on.has_value()) {
    if (want_on.value() && !this->power_state_) {
      ESP_LOGI(TAG, "Turning Truma water heater ON (cmd-tgl:TH_W_EN)");
      this->parent_->on_state_change_command("TH_W_EN", "cmd-tgl:TH_W_EN");
    } else if (!want_on.value() && this->power_state_) {
      ESP_LOGI(TAG, "Turning Truma water heater OFF (cmd-tgl:TH_W_EN)");
      this->parent_->on_state_change_command("TH_W_EN", "cmd-tgl:TH_W_EN");
    }
  }

  if (target_temp.has_value() && (!want_on.has_value() || want_on.value())) {
    uint16_t int_target = (uint16_t) std::round(target_temp.value());
    std::string cmd = Commands::update_int("TH_W_T", int_target);
    ESP_LOGI(TAG, "Setting Truma water target temperature: %s", cmd.c_str());
    this->parent_->on_state_change_command("TH_W_T", cmd);
  }
}

water_heater::WaterHeaterTraits TrumaWaterHeater::traits() {
  auto traits = water_heater::WaterHeaterTraits();
  if (this->sensor_ != nullptr) {
    traits.add_feature_flags(water_heater::WATER_HEATER_SUPPORTS_CURRENT_TEMPERATURE);
  }
  traits.add_feature_flags(water_heater::WATER_HEATER_SUPPORTS_OPERATION_MODE |
                           water_heater::WATER_HEATER_SUPPORTS_TARGET_TEMPERATURE |
                           water_heater::WATER_HEATER_SUPPORTS_ON_OFF);
  traits.set_min_temperature(15.0f);
  traits.set_max_temperature(70.0f);
  traits.set_target_temperature_step(1.0f);
  traits.set_supported_modes({
      water_heater::WATER_HEATER_MODE_OFF,
      water_heater::WATER_HEATER_MODE_ECO,
      water_heater::WATER_HEATER_MODE_PERFORMANCE,
      water_heater::WATER_HEATER_MODE_HIGH_DEMAND,
  });
  return traits;
}

}  // namespace esphome::fendt_caravan
#endif
