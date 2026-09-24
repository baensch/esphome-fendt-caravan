---
description: "Fendt & Hobby Caravan ESPHome Component allows ESP32 devices to connect to the Toptron CI-BUS BLE interface for full local Home Assistant integration."
title: "Fendt & Hobby Caravan"
params:
  seo:
    description: Fendt & Hobby Caravan component allows ESP32 devices to connect to the Bluetooth Low Energy (BLE) interface of Fendt and Hobby caravans for native Home Assistant control.
    image: fendt_caravan.svg
---

# Fendt & Hobby Caravan Integration for ESPHome

The `fendt_caravan` component enables an ESP32 microcontroller to interface directly with the central **Toptron CI-BUS** Bluetooth Low Energy (BLE) module installed in modern **Fendt** and **Hobby** caravans. It replaces proprietary mobile applications (*Fendt Connect*, *Hobby Connect*) with **100% local, bidirectional Home Assistant control and telemetry**.

```
┌─────────────────────────┐               BLE               ┌──────────────────────────┐
│   Caravan CI-BUS MCU    │ ◄─────────────────────────────► │       ESP32 Board        │
│ (Toptron Central Unit)  │   net-BT_VARS / cmd-tgl / set   │   (fendt_caravan comp.)  │
└─────────────────────────┘                                 └────────────┬─────────────┘
                                                                         │ Wi-Fi / API
                                                            ┌────────────▼─────────────┐
                                                            │      Home Assistant      │
                                                            │ (Climate, Water Heater,  │
                                                            │  Lights, Fridge, Power)  │
                                                            └──────────────────────────┘
```

---

## Prerequisites

- **Microcontroller**: An ESP32 development board with Bluetooth Low Energy (BLE) support (ESP32, ESP32-S3, or ESP32-C3). The `esp-idf` framework is recommended for BLE stability.
- **Physical Placement**: Position the ESP32 inside the caravan within stable radio range of the Toptron CI-BUS central module (typically installed in the main wardrobe or electrical cabinet).
- **Software**: ESPHome 2024.x or later.

---

## Architecture & Component Hierarchy

The integration operates using a three-tier architecture:

1. **BLE Transport Layer**:
   - `esp32_ble_tracker`: Actively scans and tracks Bluetooth Low Energy advertisements.
   - `ble_client`: Manages the GATT connection and maintains the link to the caravan's Bluetooth MAC address.
2. **Central Hub (`fendt_caravan`)**:
   - Interacts with the `ble_client` to parse incoming CI-BUS notifications, manage command queues, and handle connection lifecycles.
3. **Logical Device Controllers & Entities**:
   - Platform entities register under designated parent device hubs:
     - `mcu_device`: Central unit telemetry (temperatures, water tank, shore power, master switch).
     - `lighting_device`: Switched and dimmable lighting channels.
     - `fridge_device`: CI-BUS smart refrigerator controls.
     - `truma_heater_device`: Truma Combi space heating and domestic hot water.
     - `alde_heater_device`: Alde hydronic central heating and hot water.

---

## Quickstart: Get Online in Under 5 Minutes

Use this minimal, functional configuration to establish a BLE connection and expose cabin temperatures, freshwater level, shore power status, and master power switching to Home Assistant:

```yaml
esphome:
  name: caravan-controller

esp32:
  board: esp32dev
  framework:
    type: esp-idf

logger:
  level: DEBUG

api:

ota:
  - platform: esphome

wifi:
  ssid: "YOUR_WIFI_SSID"
  password: "YOUR_WIFI_PASSWORD"

# Load the local fendt_caravan component
external_components:
  - source:
      type: local
      path: components
    components: [fendt_caravan]

# BLE Scanner and Client
esp32_ble_tracker:

ble_client:
  - mac_address: "DE:00:24:50:64:61"   # Replace with your caravan's Bluetooth MAC address
    id: caravan_ble
    auto_connect: true

# Core Caravan Hub
fendt_caravan:
  id: my_caravan
  ble_client_id: caravan_ble

sensor:
  # Register the Main Control Unit (MCU) Device
  - platform: fendt_caravan
    type: mcu_device
    id: caravan_mcu
    parent_id: my_caravan

  # Cabin Interior Temperature
  - platform: fendt_caravan
    type: temp_in
    name: "Caravan Inside Temperature"
    parent_id: caravan_mcu

  # Cabin Exterior Temperature
  - platform: fendt_caravan
    type: temp_out
    name: "Caravan Outside Temperature"
    parent_id: caravan_mcu

  # Freshwater Tank Level (0-100%)
  - platform: fendt_caravan
    type: water_level
    name: "Caravan Freshwater Tank"
    parent_id: caravan_mcu

switch:
  # Master 12V Caravan Power Switch
  - platform: fendt_caravan
    type: main_switch
    name: "Caravan Main Switch"
    parent_id: caravan_mcu

binary_sensor:
  # 230V Shore Power Connection Status
  - platform: fendt_caravan
    type: shore_power
    name: "Caravan Shore Power"
    parent_id: caravan_mcu
```

### Installation Steps

1. **Obtain Caravan MAC**: Scan for your caravan's Bluetooth MAC address (`DE:00:XX:...`). If using iOS, see the [Discovery Guide](docs/reference.md#step-1-discovering-the-caravan-bluetooth-mac-address) for instructions on using [`discovery-example.yaml`](discovery-example.yaml) to bypass Apple CoreBluetooth UUID masking.
2. **Flash ESP32**:
   ```bash
   esphome run caravan-controller.yaml
   ```
3. **Verify Logs**: Attach to device logs:
   ```bash
   esphome logs caravan-controller.yaml
   ```
   Confirm that `ble_client` establishes a connection and `fendt_caravan` decodes incoming variables.

---

## Next Steps: Add Your Lights, Heater, and Fridge

Once your ESP32 is connected and reporting basic telemetry, proceed with bringing the rest of your caravan's subsystems into Home Assistant:

1. **[Add Your Lights](docs/lights.md)**:
   - Add switched circuits (awning, kitchen, washroom, toilet) and dimmable channels (ceiling, bedroom, ambient).
   - Follow the simple 3-step audit procedure to test each circuit and map friendly names to your physical fixtures.
2. **[Add Your Heater & Hot Water](docs/climate.md)**:
   - Configure **Truma Combi** space heating (`climate`) and boiler (`water_heater`) with temperature setpoints, Eco/Performance/High Demand modes, and linked indoor temperature feedback.
   - Configure **Alde** hydronic heating, gas burners, and electric heating cartridge stages.
3. **[Add Your Refrigerator](docs/fridge.md)**:
   - Enable direct DC power switching, Automatic Energy Selection (AES / 230V / 12V / Gas), cooling intensity (1–5), and night quiet mode.

---

## Technical Reference & Discovery

For advanced diagnostics, full specifications, and hardware troubleshooting:

- **[Variable "Rosetta Stone" Table](docs/reference.md#step-3-ci-bus-variable-mapping-rosetta-stone)**: Complete dictionary mapping raw CI-BUS keys (`TH_A_EN`, `FRIDGE_SOURCE`, `LINE_EN`, etc.) to ESPHome entity types.
- **[Querying CI-BUS Variables (`net-BT_VARS`)](docs/reference.md#step-2-querying-the-ci-bus-variable-table-net-bt_vars)**: How to dump your caravan's variable table on demand.
- **[Complete Reference Configuration](docs/reference.md#2-complete-reference-configuration)**: Full production-ready YAML demonstrating all 5 logical devices and 8 supported platforms.
- **[Hardware PCB Wiring & Mapping Procedure](docs/reference.md#4-hardware-pcb-wiring--mapping-procedure)**: Detailed workflow for diagnosing factory wiring variations across different model years and layouts.

