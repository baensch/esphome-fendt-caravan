
#pragma once
#ifdef USE_ESP32
#include "esphome/components/fendt_caravan/caravan_component_base.h"
#include "esphome/components/light/light_output.h"

namespace esphome::fendt_caravan {

class FendtLightOutput : public light::LightOutput, public CaravanComponentBase<uint8_t> {
 public:
  FendtLightOutput() { this->color_mode_ = light::ColorMode::ON_OFF; };
  FendtLightOutput(light::ColorMode color_mode) { this->color_mode_ = color_mode; };

 public:
  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({this->color_mode_});
    return traits;
  }
  void setup() override;
  void write_state(light::LightState *state) override;
  void setup_state(light::LightState *state) override { this->light_state_ = state; }

 protected:
  void on_decoded(const uint8_t &value) override;
  light::LightState *light_state_{nullptr};

 private:
  light::ColorMode color_mode_ = light::ColorMode::ON_OFF;
};
}  // namespace esphome::fendt_caravan
#endif
