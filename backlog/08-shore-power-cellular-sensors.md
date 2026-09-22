# Story 08: Shore Power & GSM Signal Sensors

## Target Device Type & Platform
* **Platforms**: ESPHome `binary_sensor` and `sensor`
* **Entity Types**:
  * `binary_sensor`: Shore Power / Mains Connected (`LINE_EN`, `device_class: plug` or `power`)
  * `sensor`: Cellular Modem Signal Strength (`GSM_SIGNAL`, icon `mdi:signal-cellular-3`)

## Problem Statement
Currently, `LINE_EN` is exposed only as a string-based `text_sensor` returning `"Connected"` or `"Disconnected"`. In Home Assistant, shore power connection is best represented as a native `binary_sensor` with `device_class: power` or `plug` for power loss automations. Furthermore, `GSM_SIGNAL` is ignored.

## Observed Caravan Variables
* `LINE_EN`: `1` (Mains power connected) / `0` (Disconnected / battery only)
* `GSM_SIGNAL`: `17` (Cellular RSSI / signal level from Hobby Connect modem)

## Acceptance Criteria
1. Expose `line_en` as a native `binary_sensor` with `device_class: plug` or `power`.
2. Expose `gsm_signal` as an optional numeric sensor when modem hardware is fitted and reporting non-zero values.
