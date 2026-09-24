#pragma once

#ifdef USE_ESP32
#include "esphome/components/fendt_caravan/caravan_device_component.h"
#include "esphome/components/fendt_caravan/fendt_caravan.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/fendt_caravan/variable.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/water_heater/water_heater.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/string_ref.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/fendt_caravan/device_decoders.h"
#include "esphome/components/fendt_caravan/device_commands.h"

namespace esphome::fendt_caravan {

class TrumaDeviceSensor : public CaravanDeviceComponent, public sensor::Sensor, public Parented<FendtCaravan> {
 public:
  void setup() override;
  void dump_config() override;

  SUB_TEXT_SENSOR(truma_available);
  SUB_BINARY_SENSOR(truma_available);
  SUB_SWITCH(truma_heater);
  SUB_SWITCH(truma_water_heater);

  void on_state_change_command(const std::string &tag, const std::string &command) override;
};

}  // namespace esphome::fendt_caravan
#endif
