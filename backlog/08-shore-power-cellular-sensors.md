# Story 08: Shore Power & GSM Signal Sensors

## Target Device Type & Platform
* **Platforms**: ESPHome `binary_sensor` and `sensor`
* **Entity Types**:
  * `binary_sensor`: Shore Power / Mains Connected (`LINE_EN`, `device_class: plug` or `power`)

## Problem Statement
Currently, `LINE_EN` is exposed only as a string-based `text_sensor` returning `"Connected"` or `"Disconnected"`. In Home Assistant, shore power connection is best represented as a native `binary_sensor` with `device_class: power` or `plug` for power loss automations.

## Observed Caravan Variables
* `LINE_EN`: `1` (Mains power connected) / `0` (Disconnected / battery only)

## Acceptance Criteria
- [x] Expose `line_en` / `shore_power` as a native `binary_sensor` with `device_class: plug` or `power`.
- [x] Create native idiomatic `binary_sensor` platform supporting diagnostic availability sensors (`truma_available`, `fridge_available`, `alde_available`, `d_plus`).
- [x] Verified live on production caravan ESP32 (`192.168.1.204`).

## Implementation Summary
* Added `IVariable::get_bool_value()` and `add_bool_callback()` for universal, RTTI-free boolean observation.
* Added `binary_sensor` platform with `FendtBinarySensor` and Python bindings.
* Configured `shore_power`, `truma_available`, and `fridge_available` in `wohnwagen.yaml`.
* Status: **Done**

