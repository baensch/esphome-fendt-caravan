#pragma once

#ifdef USE_ESP32
#include "esphome/components/fendt_caravan/caravan_component_base.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/automation.h"
#include "esphome/core/log.h"

namespace esphome::fendt_caravan {

class FendtClimate : public CaravanComponentBase<float>, public climate::Climate {
 public:
  void setup() override;
  void dump_config() override;
  void set_sensor(sensor::Sensor *sensor) { this->sensor_ = sensor; }

  Trigger<> *get_heat_action_trigger();
  Trigger<> *get_off_action_trigger();
  SUB_SWITCH(heater);

 protected:
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;
  void on_decoded(const float &target) override;

 protected:
  sensor::Sensor *sensor_{nullptr};
  Trigger<> heat_action_trigger_;
  Trigger<> off_action_trigger_;
  Trigger<> *prev_action_trigger_{nullptr};
};
}  // namespace esphome::fendt_caravan
#endif
