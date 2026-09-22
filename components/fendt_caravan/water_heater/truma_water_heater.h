#pragma once

#ifdef USE_ESP32
#include "esphome/components/fendt_caravan/caravan_device_component.h"
#include "esphome/components/fendt_caravan/variable.h"
#include "esphome/components/water_heater/water_heater.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"

namespace esphome::fendt_caravan {

class TrumaWaterHeater : public Component, public Parented<CaravanDeviceComponent>, public water_heater::WaterHeater {
 public:
  void setup() override;
  void dump_config() override;
  void set_sensor(sensor::Sensor *sensor) { this->sensor_ = sensor; }

  void publish_state() override;
  water_heater::WaterHeaterCallInternal make_call() override { return water_heater::WaterHeaterCallInternal(this); }

 protected:
  void control(const water_heater::WaterHeaterCall &call) override;
  water_heater::WaterHeaterTraits traits() override;

  void on_th_available_decoded_(bool available);
  void on_tt_available_decoded_(bool available);
  void on_power_decoded_(bool power);
  void on_target_temperature_decoded_(float target);
  void update_mode_from_target_();
  void update_availability_();

  sensor::Sensor *sensor_{nullptr};
  bool th_available_{false};
  bool tt_available_{false};
  bool is_available_{false};
  bool power_state_{false};
};

}  // namespace esphome::fendt_caravan
#endif
