---
description: "Fendt Caravan component allows ESP32 devices to connect to the Bluetooth Low Energy (BLE) interface of Fendt and Hobby (most probably) caravans."
title: "Fendt Caravan"
params:
  seo:
    description: Fendt Caravan component allows ESP32 devices to connect to the Bluetooth Low Energy (BLE) interface of Fendt and Hobby (most probably) caravans.
    image: fendt_caravan.svg
---

The **Fendt Caravan** component allows ESP32 devices to connect to the Bluetooth Low Energy (BLE) interface of **Fendt** and **Hobby** caravans and exposes the same data and controls that are available in the *Fendt Connect* mobile application directly to **Home Assistant** using ESPHome.

This enables fully local control and monitoring of caravan systems such as heating, fridge, lighting, temperature sensors, and power status without relying on the official mobile application or any cloud services.

::: warning
This component is **experimental**. The configuration, exposed entities, and behavior may change in future ESPHome releases.
:::

{{< img src="fendt_caravan_ui.png" alt="Image" >}}

{{< anchor "config-fendt-caravan" >}}

## Prerequisites

To use the **Fendt Caravan** component, the `esp32_ble_tracker` and `ble_client` components must be added to the YAML configuration file. For the `ble_client` component, the specified `mac_address` must be the MAC address of the Caravan.

- ESP32 device with BLE support
- Caravan with **Fendt / Hobby BLE interface** powered on

### Finding Your Caravan's Bluetooth MAC Address

1. **Android / PC**: You can scan using the **nRF Connect** app to find the hardware MAC address.
2. **iPhone (iOS)**: CoreBluetooth on iOS masks real hardware MAC addresses with randomized per-device UUIDs, making it impossible to see the real MAC address directly on an iPhone.
3. **Using an ESP32 for Discovery**:
   A dedicated [**`discovery-example.yaml`**](discovery-example.yaml) configuration is provided in this repository. It runs continuous BLE scanning with:
   * **10-Second Startup Delay**: Gives you time to connect to `esphome logs` before discovery output begins.
   * **Cyclic Re-Scan (every 25s)**: Periodically refreshes discovered devices so visible devices are re-printed continuously.
   * **Dedicated Radio Time**: Disables all connected BLE clients to maximize scan throughput.

   Run it on your ESP32:
   ```bash
   esphome run discovery-example.yaml
   esphome logs discovery-example.yaml
   ```

   Look for log lines like:
   ```text
   [I][ble_scan]: Device: DE:00:24:50:64:61 | RSSI: -77 dBm | Name: 'MyHobby Data' | UUIDs: EAFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFF0
   ```
   * **Common Names**: `MyHobby Data`, `Hobby Connect`, `Fendt`, `CI-BUS`, or model codes.
   * **Common MAC Prefixes**: Frequently starts with `DE:00:XX:...`.

### Discovering Available Features & Variables (Querying the Caravan)

Different caravan models and trim levels have different peripherals installed (e.g. discrete light switches, dimmers, underfloor heating, Alde vs. Truma heaters, battery monitors).

The caravan control unit supports the `net-BT_VARS` protocol command, which instructs the caravan to dump its entire variable table and current values via BLE notifications.

#### Why Variables Might Not Appear Automatically in Logs
The ESPHome component automatically sends `net-BT_VARS` during initial Bluetooth service discovery right after boot (~5 seconds after power-on). Because attaching to `esphome logs` typically takes 20–30 seconds after a reboot, this initial burst is usually missed. Once complete, the caravan only notifies live sensors whose values actively change (like outside temperature).

#### Adding a Query Button
You can trigger a fresh dump at any time while streaming logs by exposing a template button:

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

#### How to Inspect Discovered Variables
1. Stream logs from your device:
   ```bash
   esphome logs config.yaml
   ```
2. Press the **Query Caravan Variables** button (from Home Assistant or the ESPHome web dashboard).
3. The caravan will output all supported variables:
   ```text
   [D][fendt_caravan:100]: Notified value: HS_KEY_STATE:1
   [D][fendt_caravan:100]: Notified value: LINE_EN:Connected
   [D][fendt_caravan:100]: Notified value: UBAT:13,2 V
   [D][fendt_caravan:100]: Notified value: TEMP_IN:20,5^C
   [D][fendt_caravan:100]: Notified value: TEMP_OUT:14,0^C
   [D][fendt_caravan:100]: Notified value: LIGHT_AUSSEN:0
   [D][fendt_caravan:100]: Notified value: LIGHT_KUECHE:0
   [D][fendt_caravan:100]: Notified value: LIGHT_WASCH:0
   [D][fendt_caravan:100]: Notified value: LIGHT_AMB1:0
   [D][fendt_caravan:100]: Notified value: LIGHT_DIM0:75
   [D][fendt_caravan:100]: Notified value: FLOOR_HEATER_ON:0
   [D][fendt_caravan:100]: Notified value: TH_AVAILABLE:1
   ```
4. Map the discovered variables into your configuration:
   - `LIGHT_<NAME>` (e.g. `LIGHT_AUSSEN`, `LIGHT_KUECHE`, `LIGHT_WASCH`, etc.) → add corresponding `light_aussen`, `light_kueche`, or generic `light_sw` with `key_name: LIGHT_<NAME>` under `light:`.
   - `LIGHT_DIM0` .. `LIGHT_DIM4` → add corresponding `light_dim0` .. `light_dim4` under `light:`.
   - `FLOOR_HEATER_ON` → add `floor_heater` under `switch:`.
   - `TH_AVAILABLE: 1` / `TH_A_EN` → caravan is equipped with a Truma Combi heating system (`truma_heater_device` under `sensor:`, `truma_climate` under `climate:`, and `truma_water_heater` under `water_heater:`).
   - `HEATER_AVAILABLE: 1` / `HEATER_ONOFF` → caravan is equipped with an Alde heating system (`alde_heater_device`).

## Fendt Caravan Configuration

Add the component via `external_components` pointing to the `components` directory:

```yaml
external_components:
  - source:
      type: local
      path: components
    components: [fendt_caravan]

esp32_ble_tracker:

ble_client:
  - mac_address: !secret fendt_mac
    id: my_ble_client
    auto_connect: true

fendt_caravan:
  id: my_caravan
  ble_client_id: my_ble_client

sensor:
  # 1. Register the parent logical device(s)
  - platform: fendt_caravan
    id: mcu_device
    parent_id: my_caravan
    type: mcu_device

  # 2. Register sensors attached to the device
  - platform: fendt_caravan
    id: caravan_inside_temp
    name: "Caravan Inside Temperature"
    parent_id: mcu_device
    type: temp_in

  - platform: fendt_caravan
    id: caravan_outside_temp
    name: "Caravan Outside Temperature"
    parent_id: mcu_device
    type: temp_out

switch:
  - platform: fendt_caravan
    name: "Main Switch"
    parent_id: mcu_device
    type: main_switch

  - platform: fendt_caravan
    name: "All Lights"
    parent_id: mcu_device
    type: all_lights

  - platform: fendt_caravan
    name: "Floor Heater"
    parent_id: mcu_device
    type: floor_heater

text_sensor:
  - platform: fendt_caravan
    name: "Power Status"
    parent_id: mcu_device
    type: power_status

  - platform: fendt_caravan
    name: "Software Version"
    parent_id: mcu_device
    type: software_version
```

### Component Architecture & Hierarchy

The component models the caravan's CI-BUS network hierarchically:

1. **`fendt_caravan` (Hub)**: Manages the BLE communication session via `ble_client`.
   - **`id`** (**Required**, [ID](/guides/configuration-types#id)): Identifier for the caravan hub.
   - **`ble_client_id`** (**Required**, [ID](/guides/configuration-types#id)): The ID of the configured `ble_client`.

2. **Logical Devices (under `sensor:`)**: Physical / functional subsystems reporting telemetry:
   - `type: mcu_device` — Main Caravan Control Unit (MCU).
   - `type: alde_heater_device` — Alde heating controller (if caravan is equipped).
   - `type: lighting_device` — Central lighting controller.
   - Each device requires `parent_id` set to the `fendt_caravan` hub ID.

3. **Sub-Entities (Sensors, Switches, Text Sensors, Selects, Lights, Climate)**:
   - Each entity requires `parent_id` pointing to its corresponding logical device (e.g., `mcu_device` or `alde_heater_device`).

### Available Platforms & Types

#### Sensors (`sensor:`)
- Attached to `mcu_device`:
  - `type: temp_in`: Inside temperature (°C).
  - `type: temp_out`: Outside temperature (°C).

#### Switches (`switch:`)
- Attached to `mcu_device`:
  - `type: main_switch`: Caravan main power toggle.
  - `type: all_lights`: Master toggle for caravan lights.
  - `type: floor_heater`: Underfloor heating toggle.
- Attached to `alde_heater_device`:
  - `type: alde_heater`: Alde heater main toggle.
  - `type: alde_heater_gas`: Alde gas operation toggle.
  - `type: alde_heater_water`: Alde hot water boost toggle.
  - `type: alde_heater_water_temperature`: Alde hot water temperature toggle.

#### Text Sensors (`text_sensor:`)
- Attached to `mcu_device`:
  - `type: power_status`: Caravan mains / battery power status.
  - `type: software_version`: MCU firmware version.
- Attached to `alde_heater_device`:
  - `type: alde_available`: Reports availability of Alde heating system.

#### Select (`select:`)
- Attached to `alde_heater_device`:
  - `type: alde_heater_electricity`: Electric heating power selector (`Off`, `1 kW`, `2 kW`, `3 kW`).

#### Lights (`light:`)
Attached to `lighting_device`:

Modern European caravans (Fendt, Hobby, etc.) powered by Toptron CI-BUS systems do not use arbitrary numbers (like "switch 1" or "light 2") for lights. Instead, every circuit has a meaningful **semantic name** corresponding to its physical installation in the vehicle.

##### How Light Names Work in CI-BUS

The caravan's central electrical module broadcasts its available lighting circuits over Bluetooth. In your ESPHome log output during startup, you will see lines discovering the active circuits in your caravan:

```text
[lighting_device_sensor]: Discovered dynamic light key: LIGHT_AUSSEN
[lighting_device_sensor]: Discovered dynamic light key: LIGHT_KUECHE
[lighting_device_sensor]: Discovered dynamic light key: LIGHT_WASCH
[lighting_device_sensor]: Discovered dynamic light key: LIGHT_AMB1
```

Each `LIGHT_<CIRCUIT>` key corresponds directly to a physical lighting zone wired to the caravan controller.

##### Configuring Lights in YAML

You can configure each light in your caravan as either a **switched (binary on/off)** light or a **dimmable (brightness slider)** light.

###### 1. Deciding Between Switched and Dimmable (`dimmable: true / false`)

Depending on your caravan's factory equipment and options, certain circuits (like ambient LED strips or main ceiling lights) may be dimmable or simply switched.

You have full control on configuration level:
- Add `dimmable: true` to **any** light entity to expose it as a dimmable light with a brightness slider (0–15 scale in CI-BUS, mapped smoothly to 0–100% in Home Assistant).
- Leave `dimmable: false` (or omit) to expose it as a crisp binary on/off switch.

```yaml
light:
  # Switched circuit (binary on/off):
  - platform: fendt_caravan
    type: light_kueche
    name: "Caravan Kitchen Light"
    parent_id: lighting_device

  # Make any semantic circuit dimmable with dimmable: true!
  - platform: fendt_caravan
    type: light_amb1
    name: "Caravan Ambient 1 (Dimmable)"
    dimmable: true
    parent_id: lighting_device
```

###### 2. Built-in Semantic Shortcut Types

For the most common circuits found in Fendt and Hobby caravans, pre-configured shortcuts are available:

**Switched Circuits (`dimmable: false` by default, can set `dimmable: true`):**
| Type | Circuit / Location | CI-BUS Key | Typical Description |
|---|---|---|---|
| `light_aussen` | Awning / Exterior | `LIGHT_AUSSEN` | Vorzeltleuchte / Exterior door light |
| `light_kueche` | Kitchen Main | `LIGHT_KUECHE` | Primary kitchen counter fixture |
| `light_kueche2` | Kitchen Secondary | `LIGHT_KUECHE2` | Kitchen cabinet or range hood light |
| `light_wasch` | Washroom | `LIGHT_WASCH` | Bathroom ceiling light |
| `light_dusche` | Shower | `LIGHT_DUSCHE` | Dedicated shower stall light |
| `light_amb1` | Ambient Zone 1 | `LIGHT_AMB1` | Ceiling ambient LED strip |
| `light_amb2` | Ambient Zone 2 | `LIGHT_AMB2` | Seating area / dinette ambient light |
| `light_amb3` | Ambient Zone 3 | `LIGHT_AMB3` | Bedroom or entrance accent light |
| `light_zusatzl` | Spot Left | `LIGHT_ZUSATZL` | Left reading light or auxiliary spot |
| `light_zusatzr` | Spot Right | `LIGHT_ZUSATZR` | Right reading light or auxiliary spot |

**Semantic Dimmers (`dimmable: true` by default):**
| Type | Circuit / Location | CI-BUS Key | Typical Description |
|---|---|---|---|
| `light_dim_decken` | Ceiling / Main | `LIGHT_DIM0` | Main ceiling dimmer channel |
| `light_dim_amb1` | Ambient Zone 1 | `LIGHT_DIM1` | Dinette / ceiling ambient dimmer |
| `light_dim_amb2` | Ambient Zone 2 | `LIGHT_DIM2` | Seating accent dimmer |
| `light_dim_amb3` | Ambient Zone 3 | `LIGHT_DIM3` | Bedroom accent dimmer |
| `light_dim_bett` | Bed / Reading | `LIGHT_DIM4` | Reading / bed zone dimmer |

*(Legacy channels `light_dim0` through `light_dim4` remain supported as direct channel index aliases).*

###### 3. Generic Switched and Dimmable Types

For custom circuits discovered in your caravan logs (such as `LIGHT_BETT` or `LIGHT_KINDER`):
```yaml
light:
  # Generic switched binary light:
  - platform: fendt_caravan
    type: light_sw
    key_name: "LIGHT_BETT"
    name: "Caravan Bed Light"
    parent_id: lighting_device

  # Generic dimmable light (with brightness slider):
  - platform: fendt_caravan
    type: light_dim
    key_name: "LIGHT_DIM0"
    name: "Caravan Custom Dimmer"
    parent_id: lighting_device
```

##### Smooth Hardware Fading & `LIGHT_BUSY` Synchronization

The Toptron lighting controller performs smooth hardware transitions internally and broadcasts `LIGHT_BUSY:1` while fading. The ESPHome component automatically buffers and coalesces rapid slider movements during fading, dispatching your final target level the moment the controller reports `LIGHT_BUSY:0`. This prevents command race conditions and erratic flickering.

> [!NOTE]
> Physical wall switches and the caravan's central touch panel remain fully functional. When a light is switched on or off physically in the caravan, its state updates immediately in Home Assistant.

#### Water Heaters (`water_heater:`)
- Attached to `truma_heater_device`:
  - `type: truma_water_heater` / `type: truma`: Domestic hot water boiler control for Truma Combi systems (Off, Eco / 40 °C, Performance / 60 °C, High Demand / 70 °C).

#### Climate (`climate:`)
- Attached to `truma_heater_device`:
  - `type: truma_climate` / `type: truma`: Truma Combi space / air heating climate control.
- Attached to `alde_heater_device`:
  - `type: alde_climate`: Exposes Alde heating as an ESPHome climate device. Requires `sensor` and `heater_switch`.

## Supported Platforms

- ESP32 (BLE required)
- Tested with **ESP32**, **ESP32-C3**, **ESP32-S3**

ESP8266 is **not supported** due to lack of BLE hardware.

## See Also

- ESPHome Bluetooth Tracker
- ESPHome BLE Client
- Home Assistant ESPHome Integration

