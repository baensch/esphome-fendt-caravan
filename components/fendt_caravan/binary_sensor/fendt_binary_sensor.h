#pragma once

#ifdef USE_ESP32

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/fendt_caravan/caravan_device_component.h"
#include "esphome/components/fendt_caravan/variable.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <string>
#include <utility>

namespace esphome::fendt_caravan {

class FendtBinarySensor : public Component, public Parented<CaravanDeviceComponent>, public binary_sensor::BinarySensor {
 public:
  void set_key_name(std::string key_name) { this->key_name_ = std::move(key_name); }
  const std::string &get_key_name() const { return this->key_name_; }

  void setup() override;
  void dump_config() override;

 protected:
  std::string key_name_;
};

}  // namespace esphome::fendt_caravan
#endif
