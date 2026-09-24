# Story 11: Semantic Lighting Hardware PCB Mapping & User Override Guide

## Problem Statement
The user observed that on their Hobby caravan, physical lights do not match the expected semantic names (e.g., a switch labeled `LIGHT_KUECHE` or `LIGHT_AMB1` controls a different physical lamp in the vehicle). 

In Hobby and Fendt caravans equipped with Toptron central electrical modules, factory wiring harnesses are frequently attached to different PCB output terminals depending on vehicle model, floorplan variations, model year revisions, or assembly line practices. While the CI-BUS MCU broadcasts static semantic strings (`LIGHT_AUSSEN`, `LIGHT_KUECHE`, `LIGHT_WASCH`, `LIGHT_AMB1`, etc.), the actual wire running from terminal `X` may illuminate a completely different zone.

## Solution & Design
1. **Config-Level Decoupling**:
   Ensure that every light entity in `fdt` cleanly separates its human-friendly `name:` from its underlying hardware CI-BUS circuit key `key_name:`.
   For example, if the physical kitchen light is wired to the PCB terminal designated `LIGHT_AMB1`:
   ```yaml
   light:
     - platform: fendt_caravan
       type: semantic_light  # or light_sw
       key_name: LIGHT_AMB1
       name: "Caravan Küche"
       parent_id: hobby_lighting
   ```
2. **Interactive Mapping Workflow**:
   Document a straightforward diagnostic procedure allowing users to map their caravan's physical lighting circuits:
   - Turn off all lights in the caravan.
   - In Home Assistant or ESPHome web server, toggle each CI-BUS light key one by one.
   - Note which physical lamp illuminates.
   - Update the `name:` in `wohnwagen.yaml` to reflect the physical reality.

## Acceptance Criteria
- [x] The component supports arbitrary combinations of `name:` and `key_name:` without being constrained by predefined shortcuts.
- [x] User documentation provides clear instructions for mapping and renaming circuits when factory PCB wiring differs from CI-BUS variable labels.
- [x] Applied physical mapping deductions directly to `wohnwagen.yaml`.

Status: **Done**

