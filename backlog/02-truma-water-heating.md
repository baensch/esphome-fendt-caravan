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

## Status
**Done** (tested and verified on hardware and Home Assistant).
*Note: Further inspection on "power off" responsiveness will be conducted in a follow-up pass.*

## Implementation Details
* Added `TrumaWaterHeater` (`truma_water_heater` / `truma` under `water_heater:`) controlling domestic water heating.
* Availability gated on both `TH_AVAILABLE: 1` and `TT_AVAILABLE: 1`.
* Mode transitions mapped to boiler power toggle `cmd-tgl:TH_W_EN` and presets (Eco=40 °C, Performance=60 °C, High Demand=70 °C via `cmd-set:TH_W_T=<val>`).
* Incoming notifications for `TH_W_EN` and `TH_W_T` dynamically update mode and target temperature.
* Added optional `truma_water_heater` switch (`TH_W_EN`).

