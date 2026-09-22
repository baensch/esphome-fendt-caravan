# Story 06: Freshwater Tank Level Sensor

**Status**: Done

## Target Device Type & Platform
* **Platform**: ESPHome `sensor`
* **Entity Type**: Percentage Sensor (`device_class: water_level`, `unit_of_measurement: "%"`, `state_class: measurement`, icon `mdi:water-percent`)

## Problem Statement
The caravan CI-BUS sends freshwater level data via `WATER_LEVEL`, but [`control_unit_device_sensor.cpp`](../components/fendt_caravan/sensor/control_unit_device_sensor.cpp) lacks a sensor implementation for this variable. Freshwater monitoring is one of the most critical metrics for campers and RV owners.

## Observed Caravan Variables
* `WATER_LEVEL`: `0` .. `100` (Freshwater tank fill level percentage)
* `WATER_MEASUREMENT`: `0` (Measurement status / calibration)

## Acceptance Criteria
1. Introduce a `water_level` sensor under `sensor: fendt_caravan` (parented to `control_unit` or `mcu_device`).
2. Incoming `WATER_LEVEL:<val>` notifications decode as numeric percentage values and publish to the sensor.
3. Configured with standard Home Assistant attributes (`device_class: water_level`, `unit_of_measurement: "%"`).
