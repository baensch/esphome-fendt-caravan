# Adding Your Heater & Hot Water to ESPHome

This guide explains how to add native Home Assistant climate control and domestic hot water heating for caravans equipped with **Truma Combi** or **Alde** heating systems.

---

## 1. Truma Combi Heating & Boiler Setup

Caravans with Truma Combi systems (e.g., Combi 4 / 6 / 4E / 6E connected via CI-BUS / iNet) expose space heating as a standard `climate` entity and domestic hot water as a `water_heater` entity.

### Step 1: Add the Truma Hub under `sensor:`
```yaml
sensor:
  # Truma Combi Heating Hub
  - platform: fendt_caravan
    type: truma_heater_device
    id: caravan_truma
    parent_id: my_caravan
```

### Step 2: Add Climate, Boiler, and Availability Entities
```yaml
binary_sensor:
  # Reports whether the Truma system is detected and responding
  - platform: fendt_caravan
    type: truma_available
    name: "Caravan Truma Available"
    parent_id: caravan_truma

climate:
  - platform: fendt_caravan
    type: truma_climate
    name: "Caravan Heater"
    parent_id: caravan_truma
    sensor: caravan_temp_in     # Links to your inside temperature sensor ID

water_heater:
  - platform: fendt_caravan
    type: truma_water_heater
    name: "Caravan Water Heater"
    parent_id: caravan_truma
```

### Operating Features

#### Cabin Space Heating (`truma_climate`)
- **Modes**: `OFF` and `HEAT`.
- **Target Temperature**: 5 °C to 30 °C in 1 °C steps.
- **Ambient Feedback**: The linked `sensor:` ID automatically updates the current temperature on the Home Assistant thermostat card.

#### Domestic Hot Water Boiler (`truma_water_heater`)
- **Supported Modes**:
  - `OFF`: Boiler disabled.
  - `ECO`: Target 40 °C (energy-efficient daily warm water).
  - `PERFORMANCE`: Target 60 °C (hot water for showers and dishes).
  - `HIGH_DEMAND`: Target 70 °C (fast boost and anti-legionella thermal disinfection).

> [!NOTE]
> The climate and water heater entities remain safely marked as *unavailable* in Home Assistant until the caravan's central unit confirms `TH_AVAILABLE: 1` over the CI-BUS.

---

## 2. Alde Hydronic Heating Setup

For caravans equipped with Alde hydronic central heating, declare the `alde_heater_device` hub and its associated controls:

```yaml
sensor:
  - platform: fendt_caravan
    type: alde_heater_device
    id: caravan_alde
    parent_id: my_caravan

switch:
  - platform: fendt_caravan
    type: alde_heater
    id: switch_alde_heater
    name: "Caravan Alde Main Switch"
    parent_id: caravan_alde

  - platform: fendt_caravan
    type: alde_heater_gas
    name: "Caravan Alde Gas Burner"
    parent_id: caravan_alde

  - platform: fendt_caravan
    type: alde_heater_water
    name: "Caravan Alde Water Boiler"
    parent_id: caravan_alde

  - platform: fendt_caravan
    type: alde_heater_water_temperature
    name: "Caravan Alde Hot Water Boost"
    parent_id: caravan_alde

select:
  - platform: fendt_caravan
    type: alde_heater_electricity
    name: "Caravan Alde Electric Heating"
    parent_id: caravan_alde

climate:
  - platform: fendt_caravan
    type: alde_climate
    name: "Caravan Alde Climate"
    parent_id: caravan_alde
    sensor: caravan_temp_in
    heater_switch: switch_alde_heater
```

---

## Next Steps

- **[Add Your Lights](lights.md)**: Add dimmers and switches, test circuits, and map custom floorplans.
- **[Add Your Refrigerator](fridge.md)**: Configure AES energy source selection, cooling levels, and quiet night mode.
- **[Back to Getting Started](../README.md)** | **[Full Technical Reference](reference.md)**
