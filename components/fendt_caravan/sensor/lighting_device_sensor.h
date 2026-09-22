#pragma once
#ifdef USE_ESP32

#include "esphome/components/fendt_caravan/caravan_device_component.h"
#include "esphome/components/fendt_caravan/fendt_caravan.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "device_decoders.h"
#include "device_commands.h"

namespace esphome::fendt_caravan {

class LightingDeviceSensor : public CaravanDeviceComponent, public sensor::Sensor, public Parented<FendtCaravan> {
 public:
  void setup();
  void dump_config();

  void set_light_sw0_output(light::LightOutput *light_sw0) { this->light_sw0_ = light_sw0; };
  void set_light_sw1_output(light::LightOutput *light_sw1) { this->light_sw1_ = light_sw1; };
  void set_light_sw2_output(light::LightOutput *light_sw2) { this->light_sw2_ = light_sw2; };
  void set_light_sw3_output(light::LightOutput *light_sw3) { this->light_sw3_ = light_sw3; };

  void set_light_dim0_output(light::LightOutput *light_dim0) { this->light_dim0_ = light_dim0; };
  void set_light_dim1_output(light::LightOutput *light_dim1) { this->light_dim1_ = light_dim1; };
  void set_light_dim2_output(light::LightOutput *light_dim2) { this->light_dim2_ = light_dim2; };
  void set_light_dim3_output(light::LightOutput *light_dim3) { this->light_dim3_ = light_dim3; };
  void set_light_dim4_output(light::LightOutput *light_dim4) { this->light_dim4_ = light_dim4; };

  void on_state_change_command(const std::string &tag, const std::string &command) override;

 protected:
  light::LightOutput *light_sw0_;
  light::LightOutput *light_sw1_;
  light::LightOutput *light_sw2_;
  light::LightOutput *light_sw3_;

  light::LightOutput *light_dim0_;
  light::LightOutput *light_dim1_;
  light::LightOutput *light_dim2_;
  light::LightOutput *light_dim3_;
  light::LightOutput *light_dim4_;

 private:
  Variable<uint8_t> *create_variable_(const std::string &name) {
    auto *dimsw = new Variable<uint8_t>(
        name,
        [](const std::string &data) {
          int value = DeviceDecoders::decode_int(data);
          return (uint8_t) value;
        },
        Commands::update_toggle<uint8_t>,
        [](const std::string &name, uint8_t state) { return Commands::update_int(name, state); });
    return dimsw;
  }
};
}  // namespace esphome::fendt_caravan
#endif
