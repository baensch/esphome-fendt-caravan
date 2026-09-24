#pragma once

#ifdef USE_ESP32
#include "esphome/components/fendt_caravan/caravan_device_component.h"
#include "esphome/components/fendt_caravan/fendt_caravan.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/fendt_caravan/variable.h"
#include "esphome/components/select/select.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/string_ref.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "device_decoders.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "device_commands.h"

namespace esphome::fendt_caravan {

class FridgeDeviceSensor : public CaravanDeviceComponent, public sensor::Sensor, public Parented<FendtCaravan> {
 public:
  void setup() override;
  void dump_config() override;

  SUB_TEXT_SENSOR(fridge_available);
  SUB_BINARY_SENSOR(fridge_available);
  SUB_SWITCH(fridge);
  void set_fridge_switch_switch(switch_::Switch *sw) { this->set_fridge_switch(sw); }
  SUB_SELECT(fridge_source);
  SUB_SELECT(fridge_temp);
  void set_fridge_temperature_select(select::Select *s) { this->set_fridge_temp_select(s); }
  SUB_SELECT(fridge_mode);

  void on_state_change_command(const std::string &tag, const std::string &command) override;

 protected:
  bool is_available_{false};
};

}  // namespace esphome::fendt_caravan
#endif
