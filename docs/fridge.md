# Adding Your Refrigerator to ESPHome

This guide explains how to add full control over your caravan's CI-BUS smart refrigerator (such as Dometic 10-series absorption and compressor models integrated with Toptron systems).

---

## 1. Quick Setup: Add the Refrigerator Hub & Entities

To control your refrigerator, register the `fridge_device` hub under `sensor:` in your YAML, then declare the power switch, energy selector, and intensity controls.

```yaml
sensor:
  # Refrigerator Controller Hub
  - platform: fendt_caravan
    type: fridge_device
    id: caravan_fridge
    parent_id: my_caravan

binary_sensor:
  # Reports whether a smart CI-BUS refrigerator is detected
  - platform: fendt_caravan
    type: fridge_available
    name: "Caravan Refrigerator Available"
    parent_id: caravan_fridge

switch:
  # Direct Refrigerator Power On/Off
  - platform: fendt_caravan
    type: fridge
    name: "Caravan Refrigerator Power"
    parent_id: caravan_fridge

select:
  # Energy Source Selector (Auto, 230V, 12V, Gas)
  - platform: fendt_caravan
    type: fridge_source
    name: "Caravan Refrigerator Source"
    parent_id: caravan_fridge

  # Cooling Intensity (1 to 5)
  - platform: fendt_caravan
    type: fridge_temp
    name: "Caravan Refrigerator Intensity"
    parent_id: caravan_fridge

  # Operating Profile (Normal vs Night quiet mode)
  - platform: fendt_caravan
    type: fridge_mode
    name: "Caravan Refrigerator Mode"
    parent_id: caravan_fridge
```

---

## 2. Refrigerator Controls Explained

### Energy Source Selector (`fridge_source`)
Allows switching between available power sources:
- **`Auto`**: Automatic Energy Selection (AES). The refrigerator automatically selects the best available source: 230V shore power when plugged in, 12V while towing with engine running, or LPG gas when off-grid.
- **`230V`**: Forces 230V AC shore power mains operation.
- **`12V`**: Forces 12V DC operation from the tow vehicle alternator (or onboard auxiliary battery system).
- **`Gas`**: Forces LPG burner operation.

### Cooling Intensity (`fridge_temp`)
Adjusts the cooling setpoint across 5 discrete levels:
- `1`: Minimum cooling (warmest setting, saves battery and gas in mild weather).
- `3`: Standard default setting.
- `5`: Maximum cooling (coldest setting, ideal for high ambient summer temperatures).

### Operating Profile (`fridge_mode`)
- **`Normal`**: Standard automatic cooling cycle.
- **`Night`**: Quiet mode. Reduces compressor speed and ventilation fan RPM to minimize acoustic disturbance while sleeping.

---

## Next Steps

- **[Add Your Lights](lights.md)**: Add dimmers and switches, test circuits, and map custom floorplans.
- **[Add Your Heater & Hot Water](climate.md)**: Integrate Truma Combi or Alde heating.
- **[Back to Getting Started](../README.md)** | **[Full Technical Reference](reference.md)**
