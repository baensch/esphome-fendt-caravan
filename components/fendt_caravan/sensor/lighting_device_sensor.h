#pragma once
#ifdef USE_ESP32

#include "esphome/components/fendt_caravan/caravan_device_component.h"
#include "esphome/components/fendt_caravan/fendt_caravan.h"
#include "esphome/components/fendt_caravan/light/fendt_light_output.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "device_decoders.h"
#include "device_commands.h"
#include <vector>

namespace esphome::fendt_caravan {

class LightingDeviceSensor : public CaravanDeviceComponent, public sensor::Sensor, public Parented<FendtCaravan> {
 public:
  void setup() override;
  void dump_config() override;

  void register_light_output(FendtLightOutput *light_output) {
    this->light_outputs_.push_back(light_output);
  }

  bool decode(const std::string &name, const std::string &value) override;
  void on_state_change_command(const std::string &tag, const std::string &command) override;

 protected:
  std::vector<FendtLightOutput *> light_outputs_;

 private:
  Variable<uint8_t> *create_sw_variable_(const std::string &name);
  Variable<uint8_t> *create_dim_variable_(const std::string &name);
};
}  // namespace esphome::fendt_caravan
#endif
