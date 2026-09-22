# Story 07: CI-BUS Smart Refrigerator Integration (Done)

## Target Device Type & Platform
* **Platforms**: ESPHome `sensor` (`fridge_device`), `switch` (`fridge` / `fridge_switch`), `select` (`fridge_source`, `fridge_temp`, `fridge_mode`), `text_sensor` (`fridge_available`)
* **Entity Types**: 
  * `switch`: Refrigerator Power (`FRIDGE_ON_OFF`) via `cmd-tgl:FRIDGE_ON_OFF`
  * `select`: Energy Source (`FRIDGE_SOURCE`: Auto, 230V, 12V, Gas) via `cmd-set:FRIDGE_SOURCE=<0..3>`
  * `select`: Operating Mode (`FRIDGE_MODE`: Normal, Night) via `cmd-set:FRIDGE_MODE=<0..1>`
  * `select`: Cooling Intensity (`FRIDGE_TEMP`: levels 1 to 5) via `cmd-set:FRIDGE_TEMP=<1..5>`
  * `text_sensor`: Refrigerator Availability (`FRIDGE_AVAILABLE`)

## Problem Statement
The caravan's smart CI-BUS absorption or compressor refrigerator is connected to the central bus, but `fdt` previously did not provide an integration device for fridges.

## Observed Caravan Variables
* `FRIDGE_AVAILABLE`: `1` (indicates CI-BUS fridge communication is online)
* `FRIDGE_TYPE`: `1`
* `FRIDGE_ON_OFF`: `0` (Off) / `1` (On)
* `FRIDGE_MODE`: `0` (Normal)
* `FRIDGE_SOURCE`: `0` (Auto / AES energy source)
* `FRIDGE_TEMP`: `3` (Cooling level preset between 1 and 5)

## Protocol Commands
* **Toggle Power**: `cmd-tgl:FRIDGE_ON_OFF`
* **Set Cooling Level**: `cmd-set:FRIDGE_TEMP=<1..5>`
* **Set Energy Source**: `cmd-set:FRIDGE_SOURCE=<0..3>` (`0` = Auto, `1` = 230V/240V, `2` = 12V, `3` = Gas)
* **Set Mode**: `cmd-set:FRIDGE_MODE=<0..1>` (`0` = Normal, `1` = Night)

## Implementation Summary
1. Introduced `FridgeDeviceSensor` (`type: fridge_device`) under `sensor: fendt_caravan`.
2. Routed CI-BUS key/value notifications through `fridge_device_` in `FendtCaravan`.
3. Added `fridge` switch platform toggling power using `cmd-tgl:FRIDGE_ON_OFF` with state change guards.
4. Added `fridge_source` select platform supporting Auto, 230V, 12V, and Gas.
5. Added `fridge_temp` select platform with 1–5 intensity steps.
6. Added `fridge_mode` select platform with Normal and Night options.
7. Added `fridge_available` diagnostic text sensor.
8. Deployed to production caravan ESP32 and verified live state.
