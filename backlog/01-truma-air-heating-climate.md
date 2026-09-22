# Story 01: Truma Combi Air Heating Control

## Target Device Type & Platform
* **Platform**: ESPHome `climate`
* **Entity Type**: Thermostat / Climate Entity (e.g. `Caravan Heater`)

## Problem Statement
The current component implementation only supports Alde hydronic heaters (`HEATER_...` variables), leaving Truma Combi units completely unsupported. Most Hobby and Fendt caravans are equipped with Truma Combi heating systems, reporting availability via the `TH_...` variable group.

## Observed Caravan Variables
* `TH_AVAILABLE`: `1` (indicates Truma Combi heating is installed and responding on CI-BUS)
* `TH_TYPE`: `1`
* `TH_A_EN`: `Off` / `On` (Air / room heating power state)
* `TH_A_T`: `0^C` .. `30^C` (Current target air temperature)
* `TH_ES`: `0` (Energy source: Gas / Electric / Mixed)
* `TH_MM`: `0` (Fan / Blower Mode: Eco, High, Boost)
* `TEMP_IN`: `17,5^C` (Inside cabin ambient temperature, reported by the control unit)

## Known Protocol Commands
* **Toggle / Power**: `cmd-tgl:TH_A_EN`
* **Set Target Temperature**: `cmd-set:TH_A_T=<target_temp>` (e.g. `cmd-set:TH_A_T=21` sets target temperature to 21 °C)

## Acceptance Criteria
1. The component introduces a `truma` climate platform (or extends the existing `fendt_climate` platform) driven by the `TH_...` variable family.
2. The entity only becomes available if `TH_AVAILABLE: 1` is detected in the CI-BUS variable stream.
3. Turning the climate entity to `HEAT` issues `cmd-tgl:TH_A_EN` when `TH_A_EN` is `Off`.
4. Turning the climate entity to `OFF` issues `cmd-tgl:TH_A_EN` when `TH_A_EN` is `On`.
5. Setting target temperature sends `cmd-set:TH_A_T=<val>`.
6. Current ambient temperature reflects `TEMP_IN`.
7. Target temperature updates whenever incoming notifications for `TH_A_T` arrive.

## Status
**Done** (tested and verified on hardware and Home Assistant).

## Implementation Details
* Added `TrumaDeviceSensor` (`truma_heater_device` under `sensor:`) decoding `TH_AVAILABLE`, `TH_TYPE`, `TH_A_EN`, `TH_A_T`, `TH_ES`, `TH_MM`, and preparing water heating variables.
* Added `TrumaClimate` (`truma_climate` / `truma` under `climate:`) controlling space heating, tracking `TH_A_EN` and `TH_A_T`, reflecting `TEMP_IN`, and gated on `TH_AVAILABLE`.
* Added optional `truma_available` diagnostic text sensor and `truma_heater` switch.


