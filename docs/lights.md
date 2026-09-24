# Adding Your Lights to ESPHome

Once your ESP32 is connected to the caravan, adding your interior and exterior lights is straightforward. This guide helps you configure your lighting circuits, test each channel, and map friendly names to your physical fixtures.

---

## 1. Quick Setup: Add the Lighting Hub

To enable lighting, register the `lighting_device` hub under `sensor:` in your YAML, then declare your individual light circuits under `light:`.

```yaml
sensor:
  # Central Lighting Controller Hub
  - platform: fendt_caravan
    type: lighting_device
    id: caravan_lighting
    parent_id: my_caravan

light:
  # Switched circuit (On / Off)
  - platform: fendt_caravan
    type: light_aussen
    name: "Caravan Vorzeltleuchte"
    parent_id: caravan_lighting

  - platform: fendt_caravan
    type: light_kueche
    name: "Caravan Küche"
    parent_id: caravan_lighting

  # Dimmable circuit (Brightness slider 0–100%)
  - platform: fendt_caravan
    type: light_dim_decken
    name: "Caravan Deckenleuchte"
    parent_id: caravan_lighting
```

---

## 2. Discovering What Your Caravan Actually Exposes

Before defining lights in your configuration, use the **Query Caravan Variables** discovery button (`net-BT_VARS`) to determine which circuits your vehicle's Toptron MCU actually broadcasts.

> [!TIP]
> **Avoid Phantom Entities**:
> Different model years and trim packages (such as Hobby De Luxe vs. Prestige/Maxia, or Fendt Bianco vs. Diamant) feature different wiring harnesses. Even though the Toptron protocol supports 15+ potential lighting keys, your caravan might only have a fraction of them physically wired to lamps.
> 
> For example, in a 2022 Hobby De Luxe 560KMFe, only **4 switched circuits** (`LIGHT_AUSSEN`, `LIGHT_KUECHE`, `LIGHT_WASCH`, `LIGHT_DUSCHE`) and **4 dimmers** (`LIGHT_DIM0`..`3`) are physically connected to lamps. Auxiliary circuits such as `LIGHT_AMB1`..`3`, `LIGHT_ZUSATZL`/`R`, or `LIGHT_DIM4` either do not appear in the discovery dump or only click an unpopulated relay on the central PCB without illuminating anything.
> 
> **Discovery Workflow**:
> 1. Press **Query Caravan Variables** in Home Assistant or the ESPHome web dashboard.
> 2. Filter your ESPHome logs for `Notified value: LIGHT_`:
>    ```text
>    [D][fendt_caravan:100]: Notified value: LIGHT_AUSSEN:0
>    [D][fendt_caravan:100]: Notified value: LIGHT_KUECHE:0
>    [D][fendt_caravan:100]: Notified value: LIGHT_WASCH:0
>    [D][fendt_caravan:100]: Notified value: LIGHT_DUSCHE:0
>    [D][fendt_caravan:100]: Notified value: LIGHT_DIM0:0
>    ...
>    ```
> 3. Only configure entities in YAML for the `LIGHT_*` keys present in your log stream to avoid unresponsive or phantom controls in Home Assistant.

---

## 3. Common Pre-Defined Light Circuits

The component provides pre-configured entity types for the most common caravan circuits:

### Switched Circuits (Relay On / Off)
| Entity Type | Default CI-BUS Key | Typical Caravan Zone |
|---|---|---|
| `light_aussen` | `LIGHT_AUSSEN` | Porch / Awning light |
| `light_kueche` | `LIGHT_KUECHE` | Kitchen ceiling or countertop lamp |
| `light_wasch` | `LIGHT_WASCH` | Washroom / mirror light |
| `light_dusche` / `light_toilette` | `LIGHT_DUSCHE` | Shower stall or separate toilet room |
| `light_kinder` | `LIGHT_KINDER` | Bunk bed area |
| `light_bett_sw` | `LIGHT_BETT` | Bed area reading switch |
| `light_amb1` .. `light_amb3` | `LIGHT_AMB1` .. `3` | Switched ambient LED strips |

### Dimmable Circuits (16 PWM Brightness Steps)
| Entity Type | Default CI-BUS Key | Typical Caravan Zone |
|---|---|---|
| `light_dim_decken` | `LIGHT_DIM0` | Main ceiling dimmer |
| `light_dim_amb1` | `LIGHT_DIM1` | Bedroom / overhead ambient dimmer |
| `light_dim_amb2` | `LIGHT_DIM2` | Seating area ceiling dimmer |
| `light_dim_amb3` | `LIGHT_DIM3` | Reading spots dimmer |
| `light_dim_bett` | `LIGHT_DIM4` | Bedside ambient dimmer |

> [!NOTE]
> Any circuit can be converted to dimmable by specifying `dimmable: true`, or restricted to a clean binary toggle with `dimmable: false`.

---

## 4. How to Test and Map Your Caravan's Circuits

In many Fendt and Hobby caravans, factory wiring harnesses do not match the firmware labels (e.g., `light_dim_decken` might physically dim the bedroom lights instead of the living area, or the toilet room might be connected to `light_dusche`).

Use this quick 3-step mapping procedure:

### Step 1: Add a Temporary Test Configuration
Add test lights covering all available channels to your YAML and flash the ESP32:

```yaml
light:
  - platform: fendt_caravan
    type: light_dim_decken
    name: "Test Dimmer 0"
    parent_id: caravan_lighting

  - platform: fendt_caravan
    type: light_dim_amb1
    name: "Test Dimmer 1"
    parent_id: caravan_lighting

  - platform: fendt_caravan
    type: light_dim_amb2
    name: "Test Dimmer 2"
    parent_id: caravan_lighting

  - platform: fendt_caravan
    type: light_aussen
    name: "Test Awning"
    parent_id: caravan_lighting

  - platform: fendt_caravan
    type: light_kueche
    name: "Test Kitchen"
    parent_id: caravan_lighting

  - platform: fendt_caravan
    type: light_wasch
    name: "Test Washroom"
    parent_id: caravan_lighting

  - platform: fendt_caravan
    type: light_dusche
    name: "Test Shower/Toilet"
    parent_id: caravan_lighting
```

### Step 2: Toggle Each Entity in Home Assistant
1. Turn off all physical caravan lights.
2. In Home Assistant, turn on one test entity at a time.
3. Observe what happens:
   - **Lamp turns on**: Note which physical fixture illuminated.
   - **Relay clicks in the wardrobe, but no light illuminates**: The PCB has an active relay output, but your floorplan does not have a lamp wired to it.
   - **No reaction**: The circuit is unused on your caravan.

### Step 3: Set Friendly Names in YAML
Update your configuration with human-friendly names that match your caravan's physical layout:

```yaml
light:
  - platform: fendt_caravan
    type: light_dim_decken
    name: "Elternbett Links"
    parent_id: caravan_lighting

  - platform: fendt_caravan
    type: light_dusche
    name: "Toilette"
    parent_id: caravan_lighting
```

If you need to assign a completely custom CI-BUS key to an entity, use `type: semantic_light` and supply `key_name:` directly:

```yaml
light:
  - platform: fendt_caravan
    type: semantic_light
    key_name: LIGHT_AMB1
    name: "Küche Unterschranklicht"
    dimmable: false
    parent_id: caravan_lighting
```

---

## Next Steps

- **[Add Your Heater & Hot Water](climate.md)**: Integrate Truma Combi or Alde heating.
- **[Add Your Refrigerator](fridge.md)**: Control power, energy source, and temperature.
- **[Back to Getting Started](../README.md)** | **[Full Technical Reference](reference.md)**
