# Story 02: Truma Combi Hot Water Control

## Target Device Type & Platform
* **Platform**: ESPHome `water_heater` (or `climate` configured for domestic hot water)
* **Entity Type**: Water Heater Entity (e.g. `Caravan Water Heater`)

## Problem Statement
In addition to air heating, the Truma Combi unit heats domestic water for taps and showers. The water boiler operates independently of the space/room heater (essential during warm summer months). Currently, water heating cannot be monitored or triggered from ESPHome / Home Assistant.

## Observed Caravan Variables
* `TH_AVAILABLE`: `1`
* `TT_AVAILABLE`: `1`
* `TH_W_EN`: `Off` / `On` (Water heating active state)
* `TH_W_T`: `0^C` (Target water temperature, typically mapped to presets: Off, Eco / 40 °C, Hot / 60 °C, Boost)

## Known Protocol Commands
* **Toggle Water Boiler**: `cmd-tgl:TH_W_EN`
* **Set Water Target Temperature**: `cmd-set:TH_W_T=<temperature_or_level>`

## Acceptance Criteria
1. The component exposes a `water_heater` entity representing the domestic hot water circuit of the Truma Combi.
2. The entity is conditioned on `TH_AVAILABLE: 1` and `TT_AVAILABLE: 1`.
3. Toggling operation mode between `OFF` and active heating sends `cmd-tgl:TH_W_EN` according to current boiler state.
4. Setting water target temperature updates `TH_W_T` using `cmd-set:TH_W_T=...`.
5. State reflects incoming notifications for `TH_W_EN` and `TH_W_T`.
