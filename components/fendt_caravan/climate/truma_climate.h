#pragma once

#ifdef USE_ESP32
#include "esphome/components/fendt_caravan/caravan_device_component.h"
#include "esphome/components/fendt_caravan/variable.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"

namespace esphome::fendt_caravan {

class TrumaClimate : public Component, public Parented<CaravanDeviceComponent>, public climate::Climate {
 public:
  void setup() override;
  void dump_config() override;
  void set_sensor(sensor::Sensor *sensor) { this->sensor_ = sensor; }

  void set_available(bool available);

 protected:
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  void on_available_decoded_(bool available);
  void on_power_decoded_(bool power);
  void on_target_temperature_decoded_(float target);

  sensor::Sensor *sensor_{nullptr};
  bool is_available_{false};
  bool power_state_{false};
};

}  // namespace esphome::fendt_caravan
#endif
