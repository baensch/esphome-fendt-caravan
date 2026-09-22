#pragma once
#ifdef USE_ESP32
#include "fendt_light_output.h"
namespace esphome::fendt_caravan {

class FendtBinaryLightOutput : public FendtLightOutput {
 public:
  FendtBinaryLightOutput() : FendtLightOutput(light::ColorMode::ON_OFF){};
};

}  // namespace esphome::fendt_caravan
#endif
