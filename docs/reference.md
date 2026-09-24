# Fendt & Hobby Caravan Technical Reference & Configuration Guide

This guide provides an in-depth technical specification and configuration manual for the `fendt_caravan` ESPHome custom component. It covers Bluetooth Low Energy (BLE) peripheral discovery, CI-BUS protocol variables, a complete 5-device YAML reference, subsystem architectures, and physical PCB wiring diagnostics.

For a quick 5-minute setup guide, refer to the [Overview & Getting Started Guide](../README.md).

For dedicated quickstart feature guides, see:
- [Adding Your Lights](lights.md)
- [Adding Your Heater & Hot Water](climate.md)
- [Adding Your Refrigerator](fridge.md)

---

## 1. Hardware & Capability Discovery

Modern Fendt and Hobby caravans integrate Toptron Central Interface Bus (CI-BUS) master controllers. Because factory options, floorplans, and model years (e.g., Fendt Bianco/Diamant, Hobby De Luxe/Prestige/Maxia) differ significantly, you must identify:
1. The caravan's physical Bluetooth Low Energy MAC address.
2. The active CI-BUS variable table broadcast by your specific vehicle.

### Step 1: Discovering the Caravan Bluetooth MAC Address

The Toptron BLE controller continuously advertises its presence. Depending on your operating system, locate the hardware MAC address using one of the following methods:

#### Android / Linux / Windows (nRF Connect)
1. Install and launch the **nRF Connect for Mobile** app (or a desktop BLE scanner).
2. Scan for nearby advertising peripherals.
3. Locate the device broadcasting with names such as `MyHobby Data`, `Hobby Connect`, `Fendt`, or identifying with a MAC prefix starting with `DE:00:XX:...`.
4. Copy the 6-byte hexadecimal MAC address (e.g., `DE:00:24:50:64:61`).

#### Apple iOS (Resolving CoreBluetooth UUID Masking)
Apple iOS enforces a strict privacy architecture through its CoreBluetooth framework. iOS masks physical 48-bit MAC addresses and replaces them with randomized, per-device UUIDs that cannot be used directly in ESPHome's `ble_client` configuration.

To discover the physical MAC address using an ESP32:
1. Flash the provided dedicated discovery firmware [`discovery-example.yaml`](../discovery-example.yaml) to an ESP32:
   ```bash
   esphome run discovery-example.yaml
   esphome logs discovery-example.yaml
   ```
2. The discovery firmware runs an unconstrained raw BLE scan without connecting as a client, printing unfiltered hardware MAC addresses and advertisement payloads to the serial log:
   ```text
   [I][ble_scan]: Device: DE:00:24:50:64:61 | RSSI: -72 dBm | Name: 'MyHobby Data'
   ```

---

### Step 2: Querying the CI-BUS Variable Table (`net-BT_VARS`)

Upon establishing a BLE connection, the Toptron caravan MCU transmits its complete variable dictionary in response to the command `net-BT_VARS`. 

> [!NOTE]
> Establishing an ESPHome Wi-Fi connection and attaching the serial/wireless logger (`esphome logs`) typically requires 20 to 30 seconds after microcontroller boot. Consequently, the initial startup burst of variable broadcasts is usually missed. Adding a manual query button allows triggering the full dump on demand at any time.

Define a template button in your configuration:

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

#### Diagnostic Execution
1. Connect to the device logs:
   ```bash
   esphome logs caravan-controller.yaml
   ```
2. Trigger the button from Home Assistant or the ESPHome local web server.
3. Inspect the incoming log stream for variable notifications matching:
   ```text
   [D][fendt_caravan:100]: Notified value: <KEY>:<VAL>
   ```

> [!TIP]
> **Discovery Hint for Lighting Circuits**:
> The Toptron MCU only returns variables for circuits enabled in its factory firmware configuration. Filter your log stream for `LIGHT_` (`grep "Notified value: LIGHT_"`) to identify your vehicle's exact circuit footprint.
> 
> Entry- and mid-tier models (such as the maintainer's own Hobby caravan) expose a significantly reduced circuit set compared to top-tier layouts with full ambient packages. If a circuit (such as `LIGHT_AMB1` or `LIGHT_DIM4`) does not appear in your `net-BT_VARS` dump, declaring it in YAML will either produce an unresponsive entity or click an unpopulated PCB relay with no physical fixture attached. See [Trim Level Variations & Reduced Circuit Footprints](#trim-level-variations--reduced-circuit-footprints-the-maintainers-caravan-scenario) for an in-depth explanation.

---

### Step 3: CI-BUS Variable Mapping ("Rosetta Stone")

The following reference table maps discovered raw Toptron CI-BUS variable keys to their respective ESPHome platforms, parent hub devices, and entity types:

| CI-BUS Key | Example Log Value | Platform | Parent Hub | Entity `type:` | Description & Range |
|---|---|---|---|---|---|
| `TEMP_IN` | `21,5^C` | `sensor` | `mcu_device` | `temp_in` | Cabin interior temperature (°C, 0.1 °C resolution) |
| `TEMP_OUT` | `14,0^C` | `sensor` | `mcu_device` | `temp_out` | Exterior ambient temperature (°C, 0.1 °C resolution) |
| `WATER_LEVEL` | `75` | `sensor` | `mcu_device` | `water_level` | Freshwater tank fill level (`0`–`100%`) |
| `LINE_EN` | `1` / `Connected` | `binary_sensor` | `mcu_device` | `shore_power` | 230V AC shore power mains connection (`device_class: plug`) |
| `D_PLUS` | `0` / `1` | `binary_sensor` | `mcu_device` | `d_plus` | Tow vehicle alternator running signal (`device_class: power`) |
| `HS_KEY_STATE` | `1` | `switch` | `mcu_device` | `main_switch` | 12V DC master power relay |
| `ALL_LIGHTS` | `1` | `switch` | `mcu_device` | `all_lights` | Master interior lighting toggle |
| `FLOOR_HEATER_ON` | `0` / `1` | `switch` | `mcu_device` | `floor_heater` | 230V electric underfloor heating relay |
| `SOFTWARE_VERSION` | `V010700` | `text_sensor` | `mcu_device` | `software_version` | Caravan MCU firmware version string |
| `FRIDGE_AVAILABLE` | `1` / `Available` | `binary_sensor` | `fridge_device` | `fridge_available` | Smart CI-BUS refrigerator installed flag |
| `FRIDGE_ON_OFF` | `1` | `switch` | `fridge_device` | `fridge` | Refrigerator DC power state toggle |
| `FRIDGE_SOURCE` | `0`–`3` | `select` | `fridge_device` | `fridge_source` | Energy selector: `0`=Auto, `1`=Gas, `2`=12V, `3`=230V |
| `FRIDGE_TEMP` | `1`–`5` | `select` | `fridge_device` | `fridge_temp` | Cooling intensity setpoint (1 to 5) |
| `FRIDGE_MODE` | `0` / `1` | `select` | `fridge_device` | `fridge_mode` | Operating profile: `0`=Normal, `1`=Night quiet mode |
| `TH_AVAILABLE` | `1` / `Available` | `binary_sensor` | `truma_heater_device` | `truma_available` | Truma Combi heating system detected flag |
| `TH_A_EN` | `1` | `climate` | `truma_heater_device` | `truma_climate` | Truma Combi space heater power state |
| `TH_A_T` | `21` | `climate` | `truma_heater_device` | `truma_climate` | Truma space heating target temperature (5 °C–30 °C) |
| `TH_W_EN` | `1` | `water_heater` | `truma_heater_device` | `truma_water_heater` | Truma Combi boiler power state |
| `TH_W_T` | `40` | `water_heater` | `truma_heater_device` | `truma_water_heater` | Truma boiler target temperature (40 °C, 60 °C, 70 °C) |
| `HEATER_AVAILABLE` | `1` / `Available` | `binary_sensor` | `alde_heater_device` | `alde_available` | Alde central heating system detected flag |
| `HEATER_ONOFF` | `1` | `climate` / `switch` | `alde_heater_device` | `alde_climate` / `alde_heater` | Alde central hydronic heating state |
| `HEATER_GAS` | `0` / `1` | `switch` | `alde_heater_device` | `alde_heater_gas` | Alde LPG burner operation |
| `HEATER_WATER` | `0` / `1` | `switch` | `alde_heater_device` | `alde_heater_water` | Alde hot water boiler activation |
| `HEATER_WATER_TEMP`| `0` / `1` | `switch` | `alde_heater_device` | `alde_heater_water_temperature` | Alde hot water boost temperature toggle |
| `HEATER_EL` | `0`–`3` | `select` | `alde_heater_device` | `alde_heater_electricity` | Electric heating cartridge (`Off`, `1 kW`, `2 kW`, `3 kW`) |
| `LIGHT_AUSSEN` | `0` / `1` | `light` | `lighting_device` | `light_aussen` | Awning / exterior porch light |
| `LIGHT_KUECHE` | `0` / `1` | `light` | `lighting_device` | `light_kueche` | Kitchen ceiling or counter light |
| `LIGHT_WASCH` | `0` / `1` | `light` | `lighting_device` | `light_wasch` | Bathroom washbasin light |
| `LIGHT_DUSCHE` | `0` / `1` | `light` | `lighting_device` | `light_dusche` | Shower stall / toilet room light |
| `LIGHT_KINDER` | `0` / `1` | `light` | `lighting_device` | `light_kinder` | Bunk bed area circuit |
| `LIGHT_BETT` | `0` / `1` | `light` | `lighting_device` | `light_bett_sw` | Master bed area switch circuit |
| `LIGHT_AMB1`..`3` | `0` / `1` | `light` | `lighting_device` | `light_amb1`..`3` | Switched ambient lighting circuits |
| `LIGHT_DIM0`..`4` | `0`–`15` | `light` | `lighting_device` | `light_dim_decken`, etc. | PWM dimmable lighting channels (0 to 15 scale) |
| `LIGHT_SW0`..`3` | `0` / `1` | `light` | `lighting_device` | `light_sw0`..`3` | Unlabeled relay lighting channels |

---

## 2. Complete Reference Configuration

The following production-ready configuration integrates all 5 logical device hubs (`mcu_device`, `lighting_device`, `fridge_device`, `truma_heater_device`, `alde_heater_device`) and all supported ESPHome platforms.

```yaml
substitutions:
  name: "caravan-controller"
  friendly_name: "Caravan Controller"

esphome:
  name: ${name}
  friendly_name: ${friendly_name}

esp32:
  board: esp32dev
  framework:
    type: esp-idf

logger:
  level: DEBUG

api:
  encryption:
    key: !secret api_encryption_key

ota:
  - platform: esphome
    password: !secret ota_password

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

external_components:
  - source: github://lukx/esphome-fendt-caravan
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
# LOGICAL DEVICE HUBS & TELEMETRY SENSORS
# ==============================================================================
sensor:
  # 1. Main Control Unit (MCU) Hub
  - platform: fendt_caravan
    type: mcu_device
    id: hub_mcu
    parent_id: my_caravan

  # 2. Lighting Controller Hub
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

  # 5. Alde Hydronic Heating Hub
  - platform: fendt_caravan
    type: alde_heater_device
    id: hub_alde
    parent_id: my_caravan

  # MCU Telemetry Sensors
  - platform: fendt_caravan
    type: temp_in
    id: caravan_temp_in
    name: "Caravan Inside Temperature"
    parent_id: hub_mcu

  - platform: fendt_caravan
    type: temp_out
    id: caravan_temp_out
    name: "Caravan Outside Temperature"
    parent_id: hub_mcu

  - platform: fendt_caravan
    type: water_level
    name: "Caravan Freshwater Tank"
    parent_id: hub_mcu

# ==============================================================================
# BINARY SENSORS (Power, Tow Signal, Subsystem Availability)
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

  - platform: fendt_caravan
    type: alde_available
    name: "Caravan Alde Available"
    parent_id: hub_alde

# ==============================================================================
# SWITCHES (Master Switch, Circuits, Auxiliary Heaters)
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

  # Alde Switch Controls
  - platform: fendt_caravan
    type: alde_heater
    id: switch_alde_heater
    name: "Caravan Alde Heater Switch"
    parent_id: hub_alde

  - platform: fendt_caravan
    type: alde_heater_gas
    name: "Caravan Alde Gas Burner"
    parent_id: hub_alde

  - platform: fendt_caravan
    type: alde_heater_water
    name: "Caravan Alde Water Boiler"
    parent_id: hub_alde

  - platform: fendt_caravan
    type: alde_heater_water_temperature
    name: "Caravan Alde Water Boost"
    parent_id: hub_alde

# ==============================================================================
# SELECT CONTROLS (Refrigerator & Alde Electric Elements)
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

  - platform: fendt_caravan
    type: alde_heater_electricity
    name: "Caravan Alde Electric Heating"
    parent_id: hub_alde

# ==============================================================================
# LIGHTING CIRCUITS (Switched & Dimmable PWM Channels)
# ==============================================================================
light:
  # Binary Switched Circuits
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

  # Dimmable Circuits (16 Brightness Levels: 0-15)
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
# CLIMATE & WATER HEATING (Truma Combi & Alde)
# ==============================================================================
climate:
  # Truma Combi Space Heating
  - platform: fendt_caravan
    type: truma_climate
    name: "Caravan Truma Climate"
    parent_id: hub_truma
    sensor: caravan_temp_in

  # Alde Space Heating
  - platform: fendt_caravan
    type: alde_climate
    name: "Caravan Alde Climate"
    parent_id: hub_alde
    sensor: caravan_temp_in
    heater_switch: switch_alde_heater

water_heater:
  # Truma Combi Boiler
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

## 3. Subsystem Reference

### 3.1 Refrigerator (`fridge_device`)

The `fridge_device` hub interfaces with CI-BUS enabled absorption and compressor refrigerators (such as Dometic 10-series models integrated with Toptron systems).

#### Entities and Controls
- **`fridge` (`switch`)**: Direct power state control transmitting `cmd-tgl:FRIDGE_ON_OFF`.
- **`fridge_source` (`select`)**: Energy selector enforcing the physical CI-BUS protocol index:
  - `Auto` (`0`): Automatic Energy Selection (AES). Prioritizes 230V AC mains, falls back to 12V DC while towing, then switches to LPG gas.
  - `Gas` (`1`): Forced LPG operation.
  - `12V` (`2`): Forced 12V DC operation (requires tow vehicle engine signal via `D_PLUS` or dedicated auxiliary battery wiring).
  - `230V` (`3`): Forced 230V AC shore power operation.
- **`fridge_temp` (`select`)**: Cooling intensity level from `1` (minimum cooling) to `5` (maximum cooling). Transmits `cmd-set:FRIDGE_TEMP=<VALUE>`.
- **`fridge_mode` (`select`)**: Operating profile between `Normal` (`0`) and `Night` (`1`). In Night mode, compressor and fan speeds are capped to minimize acoustic disturbance.
- **`fridge_available` (`binary_sensor` / `text_sensor`)**: Evaluates `FRIDGE_AVAILABLE`. Reports whether a responsive refrigerator is detected on the CI-BUS.

---

### 3.2 Central Lighting (`lighting_device`)

The lighting controller manages switched relay circuits and PWM dimming channels.

#### Decoupling Entity Name from Physical Circuit Key
Factory wiring harnesses rarely follow static firmware names across different caravan layouts. You can decouple the human-friendly `name:` from the underlying CI-BUS key using the `key_name:` parameter:

```yaml
light:
  - platform: fendt_caravan
    type: semantic_light
    key_name: LIGHT_AMB1
    name: "Caravan Küchenleuchte"
    parent_id: hub_lighting
```

#### Switched vs. Dimmable Operation
Any circuit can be forced into binary switched mode or dimmable mode:
- Add `dimmable: true` to enable the Home Assistant brightness slider for circuits with hardware PWM support.
- Set `dimmable: false` to enforce clean binary toggle behavior without brightness attributes.
The component scales Home Assistant brightness (`0.0`–`1.0`) to the caravan's 16-level scale (`0`–`15`).

#### Hardware Fading and Transition Queueing (`LIGHT_BUSY`)
Toptron controllers execute hardware transitions internally. While fading occurs, the MCU reports `LIGHT_BUSY:1`.
- If subsequent brightness commands are transmitted while `LIGHT_BUSY:1` is active, the MCU drops them, leading to visual stutter and state desynchronization.
- The `fendt_caravan` component monitors `LIGHT_BUSY`. Commands issued during transitions are automatically buffered in memory.
- When the MCU reports `LIGHT_BUSY:0`, the component flushes the latest buffered target state. A 1500 ms safety timeout resets the busy state if an MCU notification packet is lost.

#### Reliable Binary State Tracking
Binary switches in CI-BUS use blind toggle commands (`cmd-tgl:<KEY>`). To prevent inverted states (e.g., clicking Turn Off inadvertently toggling the light back On), the component evaluates Home Assistant's target request (`remote_values.is_on()`) against current hardware state before dispatching toggle commands.

#### Split Bathrooms & Specialized Floorplans
Caravans with separated washrooms and toilet cubicles route the toilet light to `LIGHT_DUSCHE`. Supported semantic shortcuts include:
- `light_dusche` / `light_toilette` (`LIGHT_DUSCHE`): Shower stall or separate toilet cubicle.
- `light_kinder` (`LIGHT_KINDER`): Bunk bed area.
- `light_bett_sw` (`LIGHT_BETT`): Fixed bed reading fixtures.
- `light_sw0` .. `light_sw3`: Auxiliary unlabeled relay channels.

#### Trim Level Variations & Reduced Circuit Footprints (The Maintainer's Caravan Scenario)

Caravan manufacturers install different interior lighting packages and wiring looms across model lines (e.g., Hobby De Luxe vs. Prestige/Maxia, or Fendt Bianco vs. Diamant). Even though Toptron firmware supports a broad range of potential circuits, individual vehicles often expose a reduced subset.

**The Maintainer's Caravan Scenario**:
On the maintainer's own Hobby caravan, only a specific subset of lighting outputs is physically connected to active lamps:
- **Active Switched Circuits (4)**: `LIGHT_AUSSEN` (awning), `LIGHT_KUECHE` (kitchen), `LIGHT_WASCH` (washroom), and `LIGHT_DUSCHE` (toilet).
- **Active Dimmable Circuits (4)**: `LIGHT_DIM0` (left bed), `LIGHT_DIM1` (right bed), `LIGHT_DIM2` (seating ceiling), and `LIGHT_DIM3` (reading spots).
- **Unconnected / Empty Outputs**: Channels such as `LIGHT_AMB1`..`3` (switched ambient), `LIGHT_ZUSATZL` / `LIGHT_ZUSATZR` (auxiliary spots), and `LIGHT_DIM4` (bed dimmer) either do not appear in the `net-BT_VARS` dump or trigger an audible click from an unpopulated relay on the central PCB with no physical fixture attached.

**How to avoid phantom entities**:
1. Run `net-BT_VARS` using the **Query Caravan Variables** button.
2. In your ESPHome log, filter specifically for `Notified value: LIGHT_`.
3. Note only the exact `LIGHT_*` variables that your vehicle reports.
4. When writing your YAML configuration, only declare entities for circuits present in your dump.
5. If you suspect an unlisted circuit might be wired, test it using the [Hardware PCB Wiring & Mapping Procedure](#4-hardware-pcb-wiring--mapping-procedure). If only an audible relay clicks with no lamp turning on, comment out or remove the entity from your YAML to keep your Home Assistant dashboard clean.

---

### 3.3 Heating & Hot Water

#### Truma Combi (`truma_heater_device`)
The Truma Combi integration exposes space heating and domestic water heating as native Home Assistant climate and water heater entities:
- **`truma_climate` (`climate`)**:
  - Supported modes: `OFF` and `HEAT`.
  - Temperature setpoint range: 5 °C to 30 °C in 1 °C increments (`TH_A_T`).
  - Ambient temperature coupling: Supply `sensor: caravan_temp_in` to display live cabin conditions on the climate card.
- **`truma_water_heater` (`water_heater`)**:
  - Supported operation modes:
    - `OFF`: Water boiler disabled (`TH_W_EN: 0`).
    - `ECO`: Target 40 °C (`TH_W_T: 40`). Energy-saving continuous warm water.
    - `PERFORMANCE`: Target 60 °C (`TH_W_T: 60`). Standard hot water for showers.
    - `HIGH_DEMAND`: Target 70 °C (`TH_W_T: 70`). High-temperature boost / anti-legionella cycle.
- **Availability Guard**: Both entities remain unavailable until the MCU validates `TH_AVAILABLE: 1` and `TT_AVAILABLE: 1`.

#### Alde Hydronic System (`alde_heater_device`)
For caravans equipped with Alde hydronic central heating:
- **`alde_climate` (`climate`)**: Controls cabin temperature setpoint (`HEATER_TEMP`) and heating activation. Requires linking a cabin temperature sensor and the primary heater switch.
- **`alde_heater` (`switch`)**: Master toggle for the hydronic heating system (`HEATER_ONOFF`).
- **`alde_heater_gas` (`switch`)**: Controls the LPG burner stage (`HEATER_GAS`).
- **`alde_heater_water` & `alde_heater_water_temperature` (`switch`)**: Hot water boiler activation (`HEATER_WATER`) and high-temperature boost (`HEATER_WATER_TEMP`).
- **`alde_heater_electricity` (`select`)**: Electric cartridge power stage (`Off`, `1 kW`, `2 kW`, `3 kW`).

---

### 3.4 Power & Tank Telemetry

- **`shore_power` (`binary_sensor`)**: Evaluates `LINE_EN`. Configured with `device_class: plug` to report whether the caravan is connected to 230V AC mains electricity.
- **`d_plus` (`binary_sensor`)**: Evaluates `D_PLUS`. Configured with `device_class: power` to report whether the towing vehicle alternator is generating power (engine running).
- **`water_level` (`sensor`)**: Evaluates `WATER_LEVEL`. Reports freshwater tank level as a clean percentage (`0`–`100%`) with `state_class: measurement` and `icon: "mdi:water-percent"`.
- **`software_version` (`text_sensor`)**: Displays the MCU firmware version string (`SOFTWARE_VERSION`) for maintenance and debugging.

---

## 4. Hardware PCB Wiring & Mapping Procedure

On Hobby and Fendt assembly lines, wiring looms are often connected to different physical PCB terminals than the default firmware naming indicates. For example, `LIGHT_DIM0` (labeled "Deckenleuchte") may physically illuminate the master bed, while `LIGHT_DUSCHE` operates the separate toilet room.

Use this step-by-step diagnostic workflow to determine the exact hardware wiring of your caravan:

```
┌────────────────────────────────────────────────────────────────────────┐
│                      STEP 1: PREPARATION & AUDIT                       │
│  Expose all dimmer & switched keys in YAML; turn all cabin lights OFF  │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                   STEP 2: SYSTEMATIC ENTITY TESTING                    │
│      Toggle entities one by one in Home Assistant or ESPHome Web       │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
            ┌───────────────────────┴───────────────────────┐
            ▼                                               ▼
┌───────────────────────────────┐               ┌───────────────────────────────┐
│     PHYSICAL LAMP ILLUMINATES │               │       NO LIGHT ILLUMINATES    │
│  Note fixture location & map  │               │                               │
│  key_name -> descriptive name │               │                               │
└───────────────────────────────┘               └───────────────┬───────────────┘
                                                                │
                                    ┌───────────────────────────┴───────────┐
                                    ▼                                       ▼
                    ┌───────────────────────────────┐       ┌───────────────────────────────┐
                    │      RELAY CLICK AUDIBLE      │       │       NO AUDIBLE RELAY        │
                    │ Unpopulated PCB output for    │       │ Key unsupported or circuit    │
                    │ optional equipment (comment   │       │ disabled by factory firmware. │
                    │ out in final production YAML) │       │ Omit from YAML.               │
                    └───────────────────────────────┘       └───────────────────────────────┘
```

### Diagnostic Procedure

#### 1. Baseline Configuration
Add all available dimmer channels and switch channels under your `lighting_device` hub:
```yaml
light:
  - platform: fendt_caravan
    type: light_dim_decken
    name: "Test Dimmer 0"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_dim_amb1
    name: "Test Dimmer 1"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_dim_amb2
    name: "Test Dimmer 2"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_dim_amb3
    name: "Test Dimmer 3"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_dim_bett
    name: "Test Dimmer 4"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_aussen
    name: "Test Awning"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_kueche
    name: "Test Kitchen"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_wasch
    name: "Test Washroom"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_dusche
    name: "Test Shower/Toilet"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_amb1
    name: "Test Amb 1"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_amb2
    name: "Test Amb 2"
    parent_id: hub_lighting

  - platform: fendt_caravan
    type: light_amb3
    name: "Test Amb 3"
    parent_id: hub_lighting
```

#### 2. System Blackout
Ensure all physical lights in the caravan are switched off via the master switch or manual wall switches.

#### 3. Sequential Entity Auditing
1. Open Home Assistant or the ESPHome local web server.
2. Toggle the first test entity to **ON**.
3. Inspect the caravan:
   - **Lamp illuminates**: Record which physical zone turned on (e.g., "Test Dimmer 0 illuminates the left double bed reading spot").
   - **Audible relay click inside the electrical cabinet, but no lamp turns on**: This PCB output terminal is populated with a switching relay on the Toptron board, but the wiring harness to an optional fixture is not installed in your layout. Mark this channel as unused.
   - **No response and no click**: This channel is inactive or unmapped on your MCU firmware version.
4. Turn the entity **OFF** before testing the next channel.

#### 4. Finalizing Configuration
Update your production YAML:
- Assign clean, human-friendly `name:` strings matching the physical lamps.
- Keep the verified `key_name:` parameter intact.
- Comment out or remove unused relay outputs to prevent UI clutter in Home Assistant.
