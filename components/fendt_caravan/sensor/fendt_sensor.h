#pragma once

#ifdef USE_ESP32
#include "esphome/components/fendt_caravan/caravan_component_base.h"
#include "esphome/components/fendt_caravan/variable.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/log.h"

namespace esphome::fendt_caravan {

class FendtSensor : public CaravanComponentBase<float>, public sensor::Sensor {
 protected:
  void on_decoded(const float &value) override;

 private:
};
}  // namespace esphome::fendt_caravan
#endif
