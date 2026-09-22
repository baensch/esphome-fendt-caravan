# Story 05: Truma Therme 230V Water Boiler Switch

## Target Device Type & Platform
* **Platform**: ESPHome `switch` (with icon `mdi:water-boiler`)
* **Entity Type**: Binary Switch (e.g. `Caravan Water Boiler` / `Truma Therme`)

## Problem Statement
Many European caravans with separate heaters or basic layouts feature a standalone 5-liter 230V electric water boiler (Truma Therme). While [`control_unit_device_sensor.cpp`](../components/fendt_caravan/sensor/control_unit_device_sensor.cpp) registers `THERME_CONFIG`, it never implemented an entity to monitor or toggle `THERME_ON`.

## Observed Caravan Variables
* `THERME_CONFIG`: `1` (indicates the caravan is configured with a Truma Therme)
* `THERME_ON`: `0` (Off) / `1` (On)

## Known Protocol Commands
* **Toggle Therme**: `cmd-tgl:THERME_ON`

## Acceptance Criteria
1. The control unit component exposes a switch entity for `THERME_ON` when `THERME_CONFIG: 1`.
2. Turning the switch ON or OFF sends `cmd-tgl:THERME_ON`.
3. The switch state reflects incoming `THERME_ON` notifications.
4. If `THERME_CONFIG: 0`, the entity is hidden or omitted from Home Assistant.
