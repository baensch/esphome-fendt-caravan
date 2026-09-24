# Refactoring 2: Test Bench Shortcuts & Magic Numbers

> [!NOTE]
> **User Excluded**: The hardcoded Bluetooth MAC address (`c0:ee:fb:90:b0:a7`) in `FendtCaravan::gattc_event_handler` is being investigated directly by the user and is excluded from this refactoring task.

## Problem Statement
Several hard-coded shortcuts and un-named magic numbers exist in the codebase from the initial test bench prototyping. These include exact floating-point equality comparisons for switch state logic, hardcoded brightness limits duplicated across several files, and magic values for heater presets.

## Target Areas & Code Changes

### 1. Float Equality Comparison in Alde Water Switch
* **File**: [`components/fendt_caravan/sensor/alde_device_sensor.cpp`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/alde_device_sensor.cpp#L23-L26)
* **Issue**:
  ```cpp
  auto *heater_water_temp = new Variable<bool>(
      "HEATER_WATER_TEMP", [](const std::string &data) { return DeviceDecoders::decode_temperature(data) == 65.0f; },
      Commands::update_toggle<bool>);
  ```
  Comparing `== 65.0f` on floating point numbers decoded from strings can fail due to precision artifacts, leading to dropped switch state updates.
* **Refactoring**: Use a threshold or tolerance check:
  ```cpp
  auto *heater_water_temp = new Variable<bool>(
      "HEATER_WATER_TEMP",
      [](const std::string &data) {
        float temp = DeviceDecoders::decode_temperature(data);
        return !std::isnan(temp) && temp >= 60.0f;
      },
      Commands::update_toggle<bool>);
  ```

### 2. Centralize Lighting Magic Numbers
* **Files**:
  - [`components/fendt_caravan/light/fendt_light_output.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/light/fendt_light_output.h)
  - [`components/fendt_caravan/light/fendt_light_output.cpp`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/light/fendt_light_output.cpp#L32)
  - [`components/fendt_caravan/sensor/lighting_device_sensor.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/lighting_device_sensor.h)
  - [`components/fendt_caravan/sensor/lighting_device_sensor.cpp`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/lighting_device_sensor.cpp#L26)
* **Issue**:
  - Scale factor `15.0f` / `15` is hardcoded across multiple calculations for dimming scale translation.
  - The MCU busy timeout of `1500` ms is duplicated in both [`LightingDeviceSensor::loop`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/lighting_device_sensor.cpp#L26) and [`LightingDeviceSensor::on_state_change_command`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/lighting_device_sensor.cpp#L93).
* **Refactoring**:
  - Declare constants in [`fendt_light_output.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/light/fendt_light_output.h):
    ```cpp
    static constexpr uint8_t MAX_BRIGHTNESS_LEVEL = 15;
    ```
  - Declare constant in [`lighting_device_sensor.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/lighting_device_sensor.h):
    ```cpp
    static constexpr uint32_t LIGHT_BUSY_TIMEOUT_MS = 1500;
    ```

### 3. Centralize Water Heater Temperature Presets & Mode Thresholds
* **File**: [`components/fendt_caravan/water_heater/truma_water_heater.cpp`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/water_heater/truma_water_heater.cpp#L143-L188)
* **Issue**:
  Values `40.0f`, `60.0f`, `70.0f` and boundary thresholds `45.0f`, `65.0f` are scattered inline across `update_mode_from_target_` and `control`.
* **Refactoring**: Define clear constants at the top of [`truma_water_heater.cpp`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/water_heater/truma_water_heater.cpp):
  ```cpp
  static constexpr float TEMP_ECO = 40.0f;
  static constexpr float TEMP_PERFORMANCE = 60.0f;
  static constexpr float TEMP_HIGH_DEMAND = 70.0f;
  static constexpr float THRESHOLD_PERFORMANCE = 45.0f;
  static constexpr float THRESHOLD_HIGH_DEMAND = 65.0f;
  ```

## Acceptance Criteria
- [ ] No raw float equality (`== 65.0f`) is used in `AldeDeviceSensor`.
- [ ] Brightness levels and MCU busy timeouts reference centralized constants.
- [ ] Truma water heater mode mapping and presets reference named constants.
