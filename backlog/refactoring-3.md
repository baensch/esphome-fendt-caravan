# Refactoring 3: Eliminating Repetition via Pragmatic Abstractions

## Problem Statement
Several components duplicate identical initialization, decoding, and wrapping logic. By using targeted, lightweight abstractions—without resorting to complex metaprogramming—we can eliminate tens of lines of boilerplate and prevent desynchronization between disparate implementations of the same logic.

## Target Areas & Code Changes

### 1. Centralize Repeated Entity `setup()` in `CaravanComponentBase<T>`
* **Files**:
  - [`components/fendt_caravan/caravan_component_base.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/caravan_component_base.h#L10-L35)
  - [`components/fendt_caravan/text_sensor/fendt_text_sensor.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/text_sensor/fendt_text_sensor.h#L17-L27)
  - [`components/fendt_caravan/sensor/fendt_sensor.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/fendt_sensor.h#L13-L23)
  - [`components/fendt_caravan/switch/fendt_switch.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/switch/fendt_switch.h#L14-L24)
  - [`components/fendt_caravan/select/fendt_select.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/select/fendt_select.h#L14-L24)
* **Issue**:
  [`CaravanComponentBase<T>`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/caravan_component_base.h#L29) declares `void setup() override = 0;`. This forces every derived class to duplicate the exact same setup and registration boilerplate:
  ```cpp
  if (this->key_name_.empty()) return;
  auto *variable = static_cast<Variable<T> *>(this->get_parent()->get_variable(this->key_name_));
  if (variable != nullptr) {
    this->set_variable(variable);
    if (variable->is_active()) {
      this->publish_state(variable->get_value());
    }
  }
  ```
* **Refactoring**:
  Implement `setup()` directly in [`CaravanComponentBase<T>`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/caravan_component_base.h#L10) and call `this->on_decoded(variable->get_value())` when active.
  ```cpp
  void setup() override {
    if (this->key_name_.empty() || this->parent_ == nullptr)
      return;
    auto *variable = static_cast<Variable<T> *>(this->get_parent()->get_variable(this->key_name_));
    if (variable != nullptr) {
      this->set_variable(variable);
      if (variable->is_active()) {
        this->on_decoded(variable->get_value());
      }
    }
  }
  ```
  Remove the identical `setup()` overrides from [`FendtTextSensor`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/text_sensor/fendt_text_sensor.h#L17), [`FendtSensor`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/fendt_sensor.h#L13), [`FendtSwitch`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/switch/fendt_switch.h#L14), and [`FendtSelect`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/select/fendt_select.h#L14).

### 2. Unify Truthy / Boolean / Availability Decoding
* **Files**:
  - [`components/fendt_caravan/sensor/device_decoders.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/device_decoders.h#L19-L23)
  - [`components/fendt_caravan/variable.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/variable.h#L70-L83)
  - [`components/fendt_caravan/water_heater/truma_water_heater.cpp`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/water_heater/truma_water_heater.cpp#L18-L37)
  - [`components/fendt_caravan/climate/truma_climate.cpp`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/climate/truma_climate.cpp#L18-L24)
* **Issue**:
  Multiple variations of truthy string parsing exist:
  - `DeviceDecoders::decode_bool` checks `{"01", "true", "1", "on"}`.
  - `Variable::get_bool_value` checks `{"1", "01", "true", "on", "available", "connected", "enable", "enabled"}`.
  - `TrumaWaterHeater` and `TrumaClimate` manually check `val == "Available" || val == "1" || val == "true"`.
* **Refactoring**:
  - Expand [`DeviceDecoders::decode_bool`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/device_decoders.h#L19) to be the single canonical truthy checker:
    ```cpp
    static bool decode_bool(const std::string &value) {
      std::string decode = value;
      std::transform(decode.begin(), decode.end(), decode.begin(), [](unsigned char c) { return std::tolower(c); });
      return (decode == "1" || decode == "01" || decode == "true" || decode == "on" ||
              decode == "available" || decode == "connected" || decode == "enable" || decode == "enabled");
    }
    ```
  - In `Variable<T>::get_bool_value()`, `TrumaWaterHeater::setup()`, and `TrumaClimate::setup()`, delegate all boolean checks directly to `DeviceDecoders::decode_bool`.

### 3. Consolidate Redundant Light Output Headers
* **Files**:
  - [`components/fendt_caravan/light/fendt_float_light_output.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/light/fendt_float_light_output.h)
  - [`components/fendt_caravan/light/fendt_binary_light_output.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/light/fendt_binary_light_output.h)
  - [`components/fendt_caravan/light/fendt_light_output.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/light/fendt_light_output.h)
* **Issue**:
  `fendt_float_light_output.h` and `fendt_binary_light_output.h` each declare a single 1-line subclass that only passes a `ColorMode` into the constructor of [`FendtLightOutput`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/light/fendt_light_output.h#L9).
* **Refactoring**:
  Consolidate these into [`fendt_light_output.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/light/fendt_light_output.h) and delete the redundant files (or configure python codegen to instantiate `FendtLightOutput` with the appropriate mode).

## Acceptance Criteria
- [ ] `CaravanComponentBase<T>` provides default `setup()` implementation; derived text sensor, sensor, switch, and select classes have removed duplicated setup bodies.
- [ ] Boolean string parsing across variables, climate, and water heater uses `DeviceDecoders::decode_bool`.
- [ ] Light output classes are consolidated, reducing unnecessary file sprawl.
