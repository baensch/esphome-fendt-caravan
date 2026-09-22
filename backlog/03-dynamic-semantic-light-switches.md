# Story 03: Dynamic Semantic Light Switches & Availability Discovery

**Status**: Done

## Target Device Type & Platform
* **Platform**: ESPHome `light`
* **Entity Type**: Binary Light (`light::ColorMode::ON_OFF`)

## Problem Statement
Currently, [`lighting_device_sensor.cpp`](../components/fendt_caravan/sensor/lighting_device_sensor.cpp) hardcodes four binary lights: `LIGHT_SW0`, `LIGHT_SW1`, `LIGHT_SW2`, and `LIGHT_SW3`. 

In real caravans (both Hobby and Fendt), the CI-BUS MCU does not use `LIGHT_SWx`. Instead, it advertises semantic circuit names matching the specific interior layout of the vehicle:
* `LIGHT_AUSSEN` (Awning / Outside)
* `LIGHT_KUECHE` & `LIGHT_KUECHE2` (Kitchen circuits)
* `LIGHT_WASCH` (Washroom)
* `LIGHT_DUSCHE` (Shower)
* `LIGHT_AMB1`, `LIGHT_AMB2`, `LIGHT_AMB3` (Ambient accent zones)
* `LIGHT_ZUSATZL` & `LIGHT_ZUSATZR` (Left / right auxiliary spots)

Because different caravan floorplans have different lighting packages, static hardcoded schemas result in broken/unresponsive entities or missing channels.

## Observed Caravan Variables
* Switched circuits emit notifications with integer status `0` (Off) or `1` (On), e.g.:
  ```text
  [fendt_caravan:100]: Notified value: LIGHT_DUSCHE:0
  [fendt_caravan:100]: Notified value: LIGHT_WASCH:0
  [fendt_caravan:100]: Notified value: LIGHT_AMB1:0
  [fendt_caravan:100]: Notified value: LIGHT_AMB2:0
  [fendt_caravan:100]: Notified value: LIGHT_AMB3:0
  [fendt_caravan:100]: Notified value: LIGHT_ZUSATZL:0
  [fendt_caravan:100]: Notified value: LIGHT_ZUSATZR:0
  [fendt_caravan:100]: Notified value: LIGHT_KUECHE:0
  [fendt_caravan:100]: Notified value: LIGHT_KUECHE2:0
  [fendt_caravan:100]: Notified value: LIGHT_AUSSEN:0
  ```

## Known Protocol Commands
* **Toggle Light**: `cmd-tgl:<KEY_NAME>` (e.g. `cmd-tgl:LIGHT_AUSSEN`, `cmd-tgl:LIGHT_KUECHE`)

## Acceptance Criteria
1. The lighting subsystem supports arbitrary `key_name` strings prefixed with `LIGHT_`, registering a dynamic `Variable<int>` for each.
2. The integration dynamically discovers and exposes **only** the lighting entities that are actually returned by the caravan during `net-BT_VARS` variable discovery.
3. Caravans missing certain optional fixtures (such as `LIGHT_DUSCHE`) will not expose orphaned or "unavailable" entities in Home Assistant.
4. Toggling an entity sends `cmd-tgl:<KEY_NAME>` to the BLE characteristic.
5. Incoming notifications `LIGHT_<NAME>:0` and `LIGHT_<NAME>:1` immediately update the respective entity's state.
