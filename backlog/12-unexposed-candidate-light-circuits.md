# Story 12: Exposing All Candidate Lighting Circuits (Toilet / Shower, Bunk, Bed, Auxiliary)

## Problem Statement
The user observed:
> "In my camper, the light for the wash room worked nicely, however the separately-lit toilet did not react to any switch. Are there candidate light switches (non-dimmable) that we did not expose?"

Currently in `wohnwagen.yaml`, only a subset of binary lighting circuits were configured:
- `LIGHT_AUSSEN`
- `LIGHT_KUECHE`
- `LIGHT_WASCH`
- `LIGHT_AMB1`, `LIGHT_AMB2`, `LIGHT_AMB3`
- `LIGHT_ZUSATZL`, `LIGHT_ZUSATZR`

In Hobby and Fendt caravans with split or dedicated bathrooms (separate washbasin and toilet/shower cubicle), the toilet/shower light is wired to an independent circuit that was not configured in `wohnwagen.yaml`.

## Candidate CI-BUS Lighting Circuits
From Toptron CI-BUS protocol analysis and variable discovery streams, the following additional candidate non-dimmable circuits exist:

| Circuit Key | German Label | English Description | Candidate For |
|---|---|---|---|
| `LIGHT_DUSCHE` | Dusche | Shower / Wet Cell | **Primary candidate for the separately-lit toilet / shower room** |
| `LIGHT_KINDER` | Kinder | Bunk Bed / Children Area | Separate bunk or rear washroom lamps |
| `LIGHT_BETT` | Bett | Main Bed / Reading Switch | French bed or twin bed overhead fixtures |
| `LIGHT_KUECHE2` | Küche 2 | Secondary Kitchen Circuit | Worktop / cooktop under-cabinet light |
| `LIGHT_SW0` | Schalter 0 | Generic Aux Relay 0 | Unassigned auxiliary lighting relay |
| `LIGHT_SW1` | Schalter 1 | Generic Aux Relay 1 | Unassigned auxiliary lighting relay |
| `LIGHT_SW2` | Schalter 2 | Generic Aux Relay 2 | Unassigned auxiliary lighting relay |
| `LIGHT_SW3` | Schalter 3 | Generic Aux Relay 3 | Unassigned auxiliary lighting relay |

## Solution
1. Ensure all candidate circuits (`light_dusche`, `light_kinder`, `light_bett`, `light_kueche2`, `light_sw0..3`) have first-class shortcut types in Python schemas (`light/__init__.py`).
2. Add the candidate entities to `wohnwagen.yaml` (especially `LIGHT_DUSCHE` as `Caravan Toilette / Dusche`, plus any others).
3. Test toggling `LIGHT_DUSCHE` to confirm whether it controls the toilet light.

## Acceptance Criteria
- [x] `light_dusche`, `light_kinder`, `light_bett`, and `light_kueche2` are supported and documented.
- [x] Candidate lights are exposed in `wohnwagen.yaml` for testing.
- [x] The physical toilet light can be switched from Home Assistant (confirmed via `LIGHT_DUSCHE`).

Status: **Done**

