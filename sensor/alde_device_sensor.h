#pragma once

#ifdef USE_ESP32
#include "esphome/components/fendt_caravan/caravan_device_component.h"
#include "esphome/components/fendt_caravan/fendt_caravan.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/fendt_caravan/variable.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/select/select.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/string_ref.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "device_decoders.h"
#include "device_commands.h"

namespace esphome::fendt_caravan {

using namespace std;

#define SUB_CLIMATE(name) \
 protected: \
  climate::Climate *name##_climate_{nullptr}; \
\
 public: \
  void set_##name##_climate(climate::Climate *c) { this->name##_climate_ = c; }

class AldeDeviceSensor : public CaravanDeviceComponent, public sensor::Sensor, public Parented<FendtCaravan> {
 public:
  void setup() override;
  void dump_config() override;

  SUB_TEXT_SENSOR(alde_available);
  SUB_CLIMATE(alde_climate);
  SUB_SWITCH(alde_heater);
  // FENDT_NUMBER(alde_heater_temperature);
  SUB_SWITCH(alde_heater_water);
  SUB_SWITCH(alde_heater_water_temperature);
  SUB_SELECT(alde_heater_electricity);
  SUB_SWITCH(alde_heater_gas);

  void on_state_change_command(const std::string &tag, const std::string &command) override;

 protected:
 private:
  // void on_switch_state_change(switch_::Switch *sw, bool state, const std::string &command) override;
  //  void on_number_state_change_(FendtNumber *num, float state);
  //  void on_select_state_change_(FendtSelect *sel, std::string state);
};
}  // namespace esphome::fendt_caravan
#endif
