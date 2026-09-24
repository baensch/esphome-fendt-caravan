---
description: "Fendt & Hobby Caravan ESPHome Component allows ESP32 devices to connect to the Toptron CI-BUS BLE interface for full local Home Assistant integration."
title: "Fendt & Hobby Caravan"
params:
  seo:
    description: Fendt & Hobby Caravan component allows ESP32 devices to connect to the Bluetooth Low Energy (BLE) interface of Fendt and Hobby caravans for native Home Assistant control.
    image: fendt_caravan.svg
---

The **Fendt Caravan** component allows ESP32 microcontrollers to communicate directly with the central **Toptron CI-BUS** Bluetooth Low Energy (BLE) module installed in modern **Fendt** and **Hobby** caravans. It replaces proprietary mobile applications (*Fendt Connect*, *Hobby Connect*) with **100% local, bidirectional Home Assistant control and telemetry** via ESPHome.

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

::: note
**Purpose of this Document**: This document serves as the master technical specification and usage guide for the `fendt_caravan` component. It is structured into:
1. **Quickstart (Basics in 5 Minutes)**
2. **Discovery Guide (Finding Your MAC & Discovering Supported Peripherals)**
3. **Full Reference Configuration**
4. **Subsystem & Entity Deep-Dive**
5. **Hardware PCB Wiring & Mapping Guide**
:::

---

## 1. Quickstart: Basic Configuration in 5 Minutes

For users who want to get an ESP32 connected and reporting basic interior/exterior temperatures and power status immediately:

```yaml
esphome:
  name: caravan-controller

esp32:
  board: esp32dev
  framework:
    type: esp-idf

external_components:
  - source:
      type: local
      path: components
    components: [fendt_caravan]

esp32_ble_tracker:

ble_client:
  - mac_address: "DE:00:24:50:64:61"   # Replace with your caravan's Bluetooth MAC
    id: caravan_ble
    auto_connect: true

fendt_caravan:
  id: my_caravan
  ble_client_id: caravan_ble

sensor:
  # Register the primary Main Control Unit (MCU)
  - platform: fendt_caravan
    type: mcu_device
    id: caravan_mcu
    parent_id: my_caravan

  # Interior Temperature
  - platform: fendt_caravan
    type: temp_in
    name: "Caravan Inside Temperature"
    parent_id: caravan_mcu

  # Exterior Temperature
  - platform: fendt_caravan
    type: temp_out
    name: "Caravan Outside Temperature"
    parent_id: caravan_mcu

switch:
  # Master 12V caravan switch
  - platform: fendt_caravan
    type: main_switch
    name: "Caravan Main Switch"
    parent_id: caravan_mcu

binary_sensor:
  # 230V Shore Power connection
  - platform: fendt_caravan
    type: shore_power
    name: "Caravan Shore Power"
    parent_id: caravan_mcu
```

---

## 2. Discovery Guide: Determining What Your Caravan Supports

Because every caravan model year and layout (e.g., Fendt Bianco/Diamant, Hobby De Luxe/Prestige/Maxia) features different factory equipment, you must identify:
1. Your caravan's BLE MAC address.
2. Which CI-BUS variables your specific vehicle broadcasts.

### Step 1: Discovering the Bluetooth MAC Address
- **Android / PC**: Open the **nRF Connect** app, scan for nearby devices, and locate the device broadcasting as `MyHobby Data`, `Hobby Connect`, `Fendt`, or with MAC prefix `DE:00:XX:...`.
- **Apple iOS**: iOS masks physical Bluetooth MAC addresses with randomized per-device UUIDs. If using an iPhone, run the provided [`discovery-example.yaml`](discovery-example.yaml) on an ESP32:
  ```bash
  esphome run discovery-example.yaml
  esphome logs discovery-example.yaml
  ```
  Watch the log for:
  ```text
  [I][ble_scan]: Device: DE:00:24:50:64:61 | RSSI: -75 dBm | Name: 'MyHobby Data'
  ```

### Step 2: Querying the CI-BUS Variable Table (`net-BT_VARS`)
When the caravan MCU connects over BLE, it broadcasts its complete variable dictionary in response to `net-BT_VARS`. 
Because attaching to `esphome logs` typically takes 20–30 seconds after boot (causing you to miss the initial startup burst), add a **Query Button** to trigger a dump on demand:

```yaml
button:
  - platform: template
    name: "Query Caravan Variables"
    id: query_caravan_variables
    icon: "mdi:bluetooth-audio"
    on_press:
      - lambda: |-
          my_caravan->on_command_send("net-BT_VARS");
```

1. Run `esphome logs config.yaml`.
2. Press **Query Caravan Variables** from Home Assistant or the ESPHome web interface.
3. Observe lines matching `[D][fendt_caravan:100]: Notified value: <KEY>:<VAL>`.

### Step 3: CI-BUS Variable Mapping (The "Rosetta Stone")

Use this reference table to match discovered CI-BUS variables to their corresponding ESPHome platform, parent device, and entity type:

| CI-BUS Key | Example Log Value | Recommended Platform | Device Parent | Entity `type:` | Description |
|---|---|---|---|---|---|
| `TEMP_IN` | `21,5^C` | `sensor:` | `mcu_device` | `temp_in` | Inside ambient temperature (°C) |
| `TEMP_OUT` | `14,0^C` | `sensor:` | `mcu_device` | `temp_out` | Outside temperature (°C) |
| `WATER_LEVEL` | `75` | `sensor:` | `mcu_device` | `water_level` | Freshwater tank fill level (0–100%) |
| `LINE_EN` | `1` / `Connected` | `binary_sensor:` | `mcu_device` | `shore_power` | 230V mains connected (`device_class: plug`) |
| `D_PLUS` | `0` / `1` | `binary_sensor:` | `mcu_device` | `d_plus` | Tow vehicle engine running / alternator |
| `HS_KEY_STATE` | `1` | `switch:` | `mcu_device` | `main_switch` | Caravan 12V master power toggle |
| `ALL_LIGHTS` | `1` | `switch:` | `mcu_device` | `all_lights` | Master toggle for all interior lights |
| `FLOOR_HEATER_ON`| `0` / `1` | `switch:` | `mcu_device` | `floor_heater` | 230V underfloor electric heating |
| `SOFTWARE_VERSION`| `V010700` | `text_sensor:` | `mcu_device` | `software_version`| Caravan MCU firmware version |
| `TH_AVAILABLE` | `1` | `binary_sensor:` | `truma_heater_device` | `truma_available` | Truma Combi heater installed flag |
| `TH_A_EN` / `TH_A_T` | `1` / `21` | `climate:` | `truma_heater_device` | `truma_climate` | Truma Combi space heating climate control |
| `TH_W_EN` / `TH_W_T` | `1` / `40` | `water_heater:` | `truma_heater_device` | `truma_water_heater`| Truma Combi domestic hot water boiler |
| `FRIDGE_AVAILABLE`| `1` | `binary_sensor:` | `fridge_device` | `fridge_available` | Smart CI-BUS refrigerator installed flag |
| `FRIDGE_ON_OFF` | `1` | `switch:` | `fridge_device` | `fridge` | Refrigerator power on/off toggle |
| `FRIDGE_SOURCE` | `0`..`3` | `select:` | `fridge_device` | `fridge_source` | Energy selector (Auto=0, Gas=1, 12V=2, 230V=3) |
| `FRIDGE_TEMP` | `1`..`5` | `select:` | `fridge_device` | `fridge_temp` | Cooling intensity level (1–5) |
| `FRIDGE_MODE` | `0` / `1` | `select:` | `fridge_device` | `fridge_mode` | Mode (Normal=0, Night=1) |
| `HEATER_AVAILABLE`| `1` | `binary_sensor:` | `alde_heater_device` | `alde_available` | Alde heating installed flag |
| `HEATER_ONOFF` | `1` | `climate:` / `switch:` | `alde_heater_device` | `alde_climate` / `alde_heater` | Alde central hydronic heating |
| `LIGHT_AUSSEN` | `0` / `1` | `light:` | `lighting_device` | `light_aussen` | Awning / exterior porch light |
| `LIGHT_KUECHE` | `0` / `1` | `light:` | `lighting_device` | `light_kueche` | Kitchen main light |
| `LIGHT_WASCH` | `0` / `1` | `light:` | `lighting_device` | `light_wasch` | Bathroom / washbasin light |
| `LIGHT_DUSCHE` | `0` / `1` | `light:` | `lighting_device` | `light_dusche` | Toilet room / shower stall light |
| `LIGHT_AMB1`..`3` | `0` / `1` | `light:` | `lighting_device` | `light_amb1`..`3` | Ambient lighting circuits |
| `LIGHT_DIM0`..`4` | `0`..`15` | `light:` | `lighting_device` | `light_dim_decken`, etc. | Dimmable PWM channels (0–15 scale) |

---

## 3. Full Reference Configuration

Below is a complete, production-ready `esphome` configuration incorporating every supported platform and logical device hub:

```yaml
esphome:
  name: caravan-controller

esp32:
  board: esp32dev
  framework:
    type: esp-idf

external_components:
  - source:
      type: local
      path: components
    components: [fendt_caravan]

esp32_ble_tracker:

ble_client:
  - mac_address: !secret caravan_mac
    id: caravan_ble
    auto_connect: true

fendt_caravan:
  id: my_caravan
  ble_client_id: caravan_ble

button:
  - platform: template
    name: "Query Caravan Variables"
    id: query_caravan_variables
    icon: "mdi:bluetooth-audio"
    on_press:
      - lambda: |-
          my_caravan->on_command_send("net-BT_VARS");

# ==============================================================================
# LOGICAL DEVICE HUBS
# ==============================================================================
sensor:
  # 1. Main Caravan Control Unit (MCU)
  - platform: fendt_caravan
    type: mcu_device
    id: hub_mcu
    parent_id: my_caravan

  # 2. Central Lighting Controller Hub
  - platform: fendt_caravan
    type: lighting_device
    id: hub_lighting
    parent_id: my_caravan

  # 3. Smart Refrigerator Hub
  - platform: fendt_caravan
    type: fridge_device
    id: hub_fridge
    parent_id: my_caravan

  # 4. Truma Combi Heating Hub
  - platform: fendt_caravan
    type: truma_heater_device
    id: hub_truma
    parent_id: my_caravan

  # --- MCU Sensors ---
  - platform: fendt_caravan
    type: temp_in
    name: "Caravan Inside Temperature"
    id: caravan_temp_in
    parent_id: hub_mcu

  - platform: fendt_caravan
    type: temp_out
    name: "Caravan Outside Temperature"
    parent_id: hub_mcu

  - platform: fendt_caravan
    type: water_level
    name: "Caravan Freshwater Tank"
    parent_id: hub_mcu

# ==============================================================================
# BINARY SENSORS (Shore Power, Tow Vehicle, Availability)
# ==============================================================================
binary_sensor:
  - platform: fendt_caravan
    type: shore_power
    name: "Caravan Shore Power"
    parent_id: hub_mcu

  - platform: fendt_caravan
    type: d_plus
    name: "Caravan Tow Vehicle Connected"
    parent_id: hub_mcu

  - platform: fendt_caravan
    type: truma_available
    name: "Caravan Truma Available"
    parent_id: hub_truma

  - platform: fendt_caravan
    type: fridge_available
    name: "Caravan Refrigerator Available"
    parent_id: hub_fridge

# ==============================================================================
# SWITCHES (Master Switch, All Lights, Underfloor Heating, Fridge Power)
# ==============================================================================
switch:
  - platform: fendt_caravan
    type: main_switch
    name: "Caravan Main Switch"
    parent_id: hub_mcu

  - platform: fendt_caravan
    type: all_lights
    name: "Caravan All Lights"
    parent_id: hub_mcu

  - platform: fendt_caravan
    type: floor_heater
    name: "Caravan Underfloor Heating"
    parent_id: hub_mcu

  - platform: fendt_caravan
    type: fridge
    name: "Caravan Refrigerator Power"
    parent_id: hub_fridge

# ==============================================================================
# REFRIGERATOR CONTROLS (Energy Source, Intensity, Mode)
# ==============================================================================
select:
  - platform: fendt_caravan
    type: fridge_source
    name: "Caravan Refrigerator Source"
    parent_id: hub_fridge

  - platform: fendt_caravan
    type: fridge_temp
    name: "Caravan Refrigerator Intensity"
    parent_id: hub_fridge

  - platform: fendt_caravan
    type: fridge_mode
    name: "Caravan Refrigerator Mode"
    parent_id: hub_fridge

# ==============================================================================
# LIGHTING CIRCUITS (Switched & Dimmable)
# ==============================================================================
light:
  # Switched Circuits (Binary on/off)
  - platform: fendt_caravan
    type: light_aussen
    name: "Caravan Vorzeltleuchte"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_kueche
    name: "Caravan Küche"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_wasch
    name: "Caravan Waschraum"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_dusche
    name: "Caravan Toilette"
    parent_id: hub_lighting

  # Dimmable Circuits (PWM brightness slider 0-100%)
  - platform: fendt_caravan
    type: light_dim_decken
    name: "Caravan Elternbett Links"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_dim_amb1
    name: "Caravan Elternbett Rechts"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_dim_amb2
    name: "Caravan Deckenleuchte Sitzgruppe"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_dim_amb3
    name: "Caravan Leselampen Sitzgruppe"
    parent_id: hub_lighting

# ==============================================================================
# CLIMATE & WATER HEATING (Truma Combi)
# ==============================================================================
climate:
  - platform: fendt_caravan
    type: truma_climate
    name: "Caravan Heater"
    parent_id: hub_truma
    sensor_id: caravan_temp_in

water_heater:
  - platform: fendt_caravan
    type: truma_water_heater
    name: "Caravan Hot Water Boiler"
    parent_id: hub_truma

# ==============================================================================
# DIAGNOSTICS & SYSTEM INFO
# ==============================================================================
text_sensor:
  - platform: fendt_caravan
    type: software_version
    name: "Caravan Software Version"
    parent_id: hub_mcu
```

---

## 4. Subsystem Deep-Dive

### 4.1 Refrigerator Control
- **`fridge` (`switch:`)**: Direct power on/off control sending `cmd-tgl:FRIDGE_ON_OFF`.
- **`fridge_source` (`select:`)**: Energy selector with correct CI-BUS integer mapping:
  - `Auto` (`0`): Automatic Energy Selection (AES) mode.
  - `Gas` (`1`): LPG burner operation.
  - `12V` (`2`): 12V DC operation (from tow vehicle alternator or onboard battery).
  - `230V` (`3`): 230V AC mains shore power operation.
- **`fridge_temp` (`select:`)**: Cooling intensity level from `1` (lowest) to `5` (coldest).
- **`fridge_mode` (`select:`)**: Operating profile (`Normal` / `Night` quiet mode).
- **`fridge_available` (`binary_sensor:`)**: Indicates whether a CI-BUS compatible refrigerator is installed and responding.

### 4.2 Lighting Subsystem
- **Switched vs. Dimmable Flexibility**: Any light can be made dimmable by adding `dimmable: true` or converted to a strict binary toggle with `dimmable: false`.
- **Hardware Transition Queueing (`LIGHT_BUSY`)**: Toptron controllers perform hardware fading and report `LIGHT_BUSY:1` while transitioning. The component automatically coalesces and queues rapid slider adjustments, flushing the final level when `LIGHT_BUSY:0` is reported.
- **Reliable Binary State Tracking**: Evaluates target commands from Home Assistant's `remote_values` directly, guaranteeing that turn-off commands always transmit without toggle phase inversion.
- **Separately-Lit Toilet & Candidate Circuits**:
  Caravans with separate washrooms and toilet cubicles wire the toilet light to `LIGHT_DUSCHE` (`type: light_dusche` or alias `type: light_toilette`). Other candidate circuits supported include `light_kinder` (`LIGHT_KINDER`), `light_bett_sw` (`LIGHT_BETT`), and `light_sw0` .. `light_sw3`.

### 4.3 Truma Combi Heating & Domestic Hot Water
- **`truma_climate` (`climate:`)**: Controls cabin air heating. Supports `OFF` and `HEAT` modes, integer target temperatures (5 °C – 30 °C), and automatically reflects live ambient conditions by linking `sensor_id` to `temp_in`.
- **`truma_water_heater` (`water_heater:`)**: Native Home Assistant water heater entity. Supports:
  - `OFF`: Boiler disabled.
  - `ECO`: Target 40 °C.
  - `PERFORMANCE`: Target 60 °C.
  - `HIGH_DEMAND`: Target 70 °C (anti-legionella boost).
- **Availability Guard**: Both climate and water heater entities remain safely unavailable until the caravan MCU confirms `TH_AVAILABLE: 1` and `TT_AVAILABLE: 1`.

### 4.4 Fresh Water & Power Telemetry
- **`water_level` (`sensor:`)**: Reports freshwater tank level as a clean percentage (`0`–`100%`) with `state_class: measurement` and `icon: "mdi:water-percent"`.
- **`shore_power` (`binary_sensor:`)**: Natively reports whether 230V shore power is connected (`device_class: plug`).
- **`d_plus` (`binary_sensor:`)**: Reports the 12V `D+` engine-running alternator line from the towing vehicle (`device_class: power`).

---

## 5. Hardware PCB Wiring & Mapping Guide

In Hobby and Fendt caravans, factory assembly lines often connect interior lamps to different PCB relay and dimmer terminals than the static firmware names suggest (e.g., `LIGHT_DIM0` labeled "Deckenleuchte" might physically illuminate the parents' bed, and `LIGHT_DUSCHE` might control the toilet room).

### How to Map Your Caravan's Physical Circuits:
1. Temporarily configure all available semantic types in `wohnwagen.yaml`:
   - Dimmers: `light_dim_decken`, `light_dim_amb1`, `light_dim_amb2`, `light_dim_amb3`, `light_dim_bett`.
   - Switches: `light_aussen`, `light_kueche`, `light_wasch`, `light_dusche`, `light_amb1`..`3`, `light_zusatzl`..`r`.
2. Turn off all physical lights in the caravan.
3. Open Home Assistant or the ESPHome web dashboard and toggle each light entity one by one.
4. Record which physical lamp illuminates (or if only an unpopulated PCB relay clicks with no lamp attached).
5. Update the human-friendly `name:` in `wohnwagen.yaml` to match physical reality, and comment out any unused relay channels.
