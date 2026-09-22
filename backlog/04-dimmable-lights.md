# Story 04: Dimmable Light Channels (0–15 Range)

**Status**: Done

## Target Device Type & Platform
* **Platform**: ESPHome `light`
* **Entity Type**: Dimmable Light (`light::ColorMode::BRIGHTNESS`)

## Problem Statement
The caravan features up to five dimmable lighting channels (`LIGHT_DIM0` .. `LIGHT_DIM4`), plus a status variable `LIGHT_BUSY`. In CI-BUS, brightness values range from 0 to 15 rather than 0–100% or 0–255. 

Additionally, like switched lights, only those dimming channels physically wired on a particular caravan model should be exposed as entities in Home Assistant.

## Observed Caravan Variables
* `LIGHT_DIM0`: `0` .. `15`
* `LIGHT_DIM1`: `0` .. `15`
* `LIGHT_DIM2`: `0` .. `15`
* `LIGHT_DIM3`: `0` .. `15`
* `LIGHT_DIM4`: `0` .. `15`
* `LIGHT_BUSY`: `0` / `1` (indicates whether the caravan lighting MCU is actively performing a fade transition)

## Known Protocol Commands
* **Toggle Power**: `cmd-tgl:LIGHT_DIM<n>`
* **Set Dimming Level**: `cmd-set:LIGHT_DIM<n>=<level>` where `<level>` is an integer between `0` and `15`

## Acceptance Criteria
1. Dimmable lights are exposed as ESPHome `light` entities supporting brightness adjustments (0.0 to 1.0 mapped to 0–15).
2. Toggling the light on/off sends `cmd-tgl:LIGHT_DIM<n>`.
3. Changing brightness sends `cmd-set:LIGHT_DIM<n>=<val>`.
4. Notifications for `LIGHT_DIM<n>` accurately update the light's reported state and brightness level.
5. Only dimming channels that are reported in the caravan's `net-BT_VARS` variable dump are registered and exposed.
6. The component respects `LIGHT_BUSY` to prevent race conditions during rapid slider adjustments.
