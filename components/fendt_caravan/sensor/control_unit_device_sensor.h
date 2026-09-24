#pragma once

#ifdef USE_ESP32
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/fendt_caravan/caravan_device_component.h"
#include "esphome/components/fendt_caravan/fendt_caravan.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/fendt_caravan/variable.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/string_ref.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/fendt_caravan/device_decoders.h"
#include "esphome/components/fendt_caravan/device_commands.h"
#include <functional>

namespace esphome::fendt_caravan {

class ControlUnitDeviceSensor : public CaravanDeviceComponent, public sensor::Sensor, public Parented<FendtCaravan> {
 public:
  void setup() override;
  void dump_config() override;
  void on_state_change_command(const std::string &tag, const std::string &command) override;
  SUB_SWITCH(main_switch);
  SUB_SENSOR(temp_in);
  SUB_SENSOR(temp_out);
  SUB_SENSOR(water_level);
  SUB_TEXT_SENSOR(power_status);
  SUB_TEXT_SENSOR(software_version);
  SUB_BINARY_SENSOR(shore_power);
  SUB_BINARY_SENSOR(line_en);
  SUB_BINARY_SENSOR(d_plus);
  SUB_SWITCH(all_lights);
  SUB_SWITCH(floor_heater);

 protected:
  void on_data_decoded(IVariable *variable) override;
};
}  // namespace esphome::fendt_caravan
#endif
