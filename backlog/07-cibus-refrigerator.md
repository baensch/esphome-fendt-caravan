# Story 07: CI-BUS Smart Refrigerator Integration

## Target Device Type & Platform
* **Platforms**: ESPHome `switch`, `select`, and `number` (or composite `climate`)
* **Entity Types**: 
  * `switch`: Refrigerator Power (`FRIDGE_ON_OFF`)
  * `select`: Energy Source (`FRIDGE_SOURCE`: e.g. Auto, Gas, 230V, 12V)
  * `select`: Operating Mode (`FRIDGE_MODE`)
  * `number` or `select`: Cooling Level (`FRIDGE_TEMP`: levels 1 to 5)

## Problem Statement
The caravan's smart CI-BUS absorption or compressor refrigerator is connected to the central bus, but `fdt` does not currently provide an integration device for fridges.

## Observed Caravan Variables
* `FRIDGE_AVAILABLE`: `1` (indicates CI-BUS fridge communication is online)
* `FRIDGE_TYPE`: `1`
* `FRIDGE_ON_OFF`: `0` (Off) / `1` (On)
* `FRIDGE_MODE`: `0`
* `FRIDGE_SOURCE`: `0` (Energy source indicator)
* `FRIDGE_TEMP`: `3` (Cooling level preset between 1 and 5)

## Known Protocol Commands
* **Toggle Power**: `cmd-tgl:FRIDGE_ON_OFF`
* **Set Cooling Level**: `cmd-set:FRIDGE_TEMP=<1..5>`

## Acceptance Criteria
1. Introduce a `fridge` device component under `fendt_caravan`.
2. Automatically enable entities only when `FRIDGE_AVAILABLE: 1` is reported.
3. Expose a switch for on/off power state using `cmd-tgl:FRIDGE_ON_OFF`.
4. Expose controls for temperature/cooling steps (1–5) and status monitoring for energy source.
