# Refactoring 4: Architecture, Structure & File Organization

## Problem Statement
The codebase exhibits architectural coupling and misnomers stemming from early design stages:
1. [`FendtCaravan`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/fendt_caravan.h#L15) explicitly references every subsystem controller with hardcoded fields and dispatch branches.
2. Protocol-wide utilities are buried inside the `sensor/` directory, requiring awkward cross-subsystem includes.
3. Class and file names show naming inconsistencies (e.g., `fendt_climate` is actually Alde-specific climate, while Truma climate is `truma_climate`).
4. Several controllers maintain dead sub-entity pointers and unnecessary macros.

## Target Areas & Code Changes

### 1. Decouple `FendtCaravan` Routing via `std::vector<CaravanDeviceComponent *>`
* **Files**:
  - [`components/fendt_caravan/fendt_caravan.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/fendt_caravan.h#L24-L46)
  - [`components/fendt_caravan/fendt_caravan.cpp`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/fendt_caravan.cpp#L130-L162)
* **Issue**:
  `FendtCaravan` explicitly defines 5 member pointers (`mcu_device_`, `alde_heater_device_`, `lighting_device_`, `truma_heater_device_`, `fridge_device_`), 5 duplicate setters, and a 25-line dispatch cascade in `on_data_received_`.
* **Refactoring**:
  Replace with a dynamic device registry:
  ```cpp
  // In fendt_caravan.h:
  void register_device(CaravanDeviceComponent *device) {
    this->devices_.push_back(device);
    device->set_command_send_callback([this](const std::string &cmd) { this->on_command_send(cmd); });
  }

  // Deprecate / forward old setters to register_device:
  void set_mcu_device_sensor(CaravanDeviceComponent *d) { this->register_device(d); }
  void set_alde_heater_device_sensor(CaravanDeviceComponent *d) { this->register_device(d); }
  void set_lighting_device_sensor(CaravanDeviceComponent *d) { this->register_device(d); }
  void set_truma_heater_device_sensor(CaravanDeviceComponent *d) { this->register_device(d); }
  void set_fridge_device_sensor(CaravanDeviceComponent *d) { this->register_device(d); }
  ```
  In [`FendtCaravan::on_data_received_`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/fendt_caravan.cpp#L130):
  ```cpp
  void FendtCaravan::on_data_received_(const std::string &data) {
    size_t colon = data.find(':');
    if (colon == std::string::npos) return;
    std::string key = data.substr(0, colon);
    std::string value = data.substr(colon + 1);

    for (auto *device : this->devices_) {
      if (device->decode(key, value)) return;
    }
  }
  ```

### 2. Relocate Protocol Files Out of `sensor/`
* **Files**:
  - `components/fendt_caravan/sensor/device_commands.h` $\to$ `components/fendt_caravan/device_commands.h`
  - `components/fendt_caravan/sensor/device_decoders.h` $\to$ `components/fendt_caravan/device_decoders.h`
  - `components/fendt_caravan/sensor/device_decoders.cpp` $\to$ `components/fendt_caravan/device_decoders.cpp`
* **Issue**:
  Protocol command formatters and decoders are shared by `climate/`, `water_heater/`, and `light/`, yet they reside inside `sensor/`.
* **Refactoring**:
  Move these files to `components/fendt_caravan/` root and clean up includes across all consumers.

### 3. Rename `FendtClimate` to `AldeClimate`
* **Files**:
  - `components/fendt_caravan/climate/fendt_climate.h` $\to$ `components/fendt_caravan/climate/alde_climate.h`
  - `components/fendt_caravan/climate/fendt_climate.cpp` $\to$ `components/fendt_caravan/climate/alde_climate.cpp`
  - [`components/fendt_caravan/climate/__init__.py`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/climate/__init__.py#L9)
* **Issue**:
  In `climate/__init__.py`, `FendtClimate` represents the Alde heating system, while Truma heating is represented by `TrumaClimate`. Naming one `FendtClimate` and the other `TrumaClimate` causes confusion.
* **Refactoring**:
  Rename class `FendtClimate` to `AldeClimate` and rename files accordingly for symmetric consistency.

### 4. Remove Dead Member Pointers in Device Controllers
* **Files**:
  - [`components/fendt_caravan/sensor/truma_device_sensor.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/truma_device_sensor.h#L39-L40)
  - [`components/fendt_caravan/sensor/truma_device_sensor.cpp`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/truma_device_sensor.cpp)
  - [`components/fendt_caravan/sensor/alde_device_sensor.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/alde_device_sensor.h#L35-L44)
  - [`components/fendt_caravan/sensor/fridge_device_sensor.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/fridge_device_sensor.h#L28-L31)
* **Issue**:
  - `TrumaDeviceSensor` stores `truma_climate_{nullptr}` and `truma_water_heater_{nullptr}`, but never interacts with them.
  - Entities register their own callbacks with `Variable<T>`, making many `SUB_*` member pointers dead baggage.
  - `FridgeDeviceSensor` defines adapter methods (`set_fridge_switch_switch`, `set_fridge_temperature_select`) solely to patch name mismatches in Python codegen.
* **Refactoring**:
  Remove unused member variables and streamline setter signatures.

## Acceptance Criteria
- [ ] `FendtCaravan` stores a polymorphic `std::vector<CaravanDeviceComponent *>` rather than individual subsystem fields.
- [ ] Protocol files `device_commands.h` and `device_decoders.*` reside in `components/fendt_caravan/`.
- [ ] `fendt_climate.*` is renamed to `alde_climate.*` (`AldeClimate`).
- [ ] Unused pointers and dead macro declarations in device controllers are cleaned up.
