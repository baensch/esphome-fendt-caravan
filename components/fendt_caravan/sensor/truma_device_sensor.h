#pragma once

#ifdef USE_ESP32
#include "esphome/components/fendt_caravan/caravan_device_component.h"
#include "esphome/components/fendt_caravan/fendt_caravan.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/fendt_caravan/variable.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/string_ref.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "device_decoders.h"
#include "device_commands.h"

namespace esphome::fendt_caravan {

class TrumaDeviceSensor : public CaravanDeviceComponent, public sensor::Sensor, public Parented<FendtCaravan> {
 public:
  void setup() override;
  void dump_config() override;

  void set_truma_climate(climate::Climate *c) { this->truma_climate_ = c; }
  void set_truma_climate_climate(climate::Climate *c) { this->truma_climate_ = c; }

  SUB_TEXT_SENSOR(truma_available);
  SUB_SWITCH(truma_heater);

  void on_state_change_command(const std::string &tag, const std::string &command) override;

 protected:
  climate::Climate *truma_climate_{nullptr};
};

}  // namespace esphome::fendt_caravan
#endif
