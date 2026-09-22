# Backlog: Fendt / Hobby Caravan ESPHome Component (`fdt`)

This directory tracks feature requests and implementation stories derived from reverse-engineering the CI-BUS BLE protocol (`net-BT_VARS` discovery dumps).

## Stories

| ID | Title | Target Device Type / Platform | Key Variables | Status |
|---|---|---|---|---|
| [01](01-truma-air-heating-climate.md) | Truma Combi Air Heating Control | `climate` | `TH_AVAILABLE`, `TH_A_EN`, `TH_A_T`, `TH_ES`, `TH_MM` | Done |
| [02](02-truma-water-heating.md) | Truma Combi Hot Water Control | `water_heater` | `TH_W_EN`, `TH_W_T`, `TT_AVAILABLE` | Done |
| [03](03-dynamic-semantic-light-switches.md) | Dynamic Semantic Light Switches & Discovery | `light` (binary) | `LIGHT_<NAME>` (`LIGHT_AUSSEN`, `LIGHT_KUECHE`, etc.) | Backlog |
| [04](04-dimmable-lights.md) | Dimmable Light Channels (0–15 scale) | `light` (brightness) | `LIGHT_DIM0` .. `LIGHT_DIM4`, `LIGHT_BUSY` | Backlog |
| [05](05-truma-therme-boiler-switch.md) | Truma Therme 230V Water Boiler Switch | `switch` | `THERME_CONFIG`, `THERME_ON` | Backlog |
| [06](06-freshwater-level-sensor.md) | Freshwater Tank Level Sensor | `sensor` | `WATER_LEVEL`, `WATER_MEASUREMENT` | Backlog |
| [07](07-cibus-refrigerator.md) | CI-BUS Smart Refrigerator Integration | `switch` / `select` / `sensor` | `FRIDGE_AVAILABLE`, `FRIDGE_ON_OFF`, `FRIDGE_TEMP`, `FRIDGE_SOURCE`, `FRIDGE_MODE` | Backlog |
| [08](08-shore-power-cellular-sensors.md) | Shore Power & GSM Signal Sensors | `binary_sensor` / `sensor` | `LINE_EN`, `GSM_SIGNAL` | Backlog |
