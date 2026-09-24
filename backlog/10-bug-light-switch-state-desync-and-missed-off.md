# Story 10: Bug - Binary Light Toggle State Desynchronization & Missed Off Toggle

## Problem Statement
During hardware testing of binary light circuits (specifically `Caravan Waschraum` / `LIGHT_WASCH`):
- Turning the light **ON** triggers reliably.
- Turning the light **OFF** sometimes misses / fails to turn off the physical fixture.
- When an "OFF" toggle fails, pressing **ON** in the Home Assistant UI unexpectedly turns the physical light **OFF**.

## Root Cause Analysis
1. **Target Value Inspection**:
   In [`fendt_light_output.cpp`](../components/fendt_caravan/light/fendt_light_output.cpp), `write_state()` checks:
   ```cpp
   bool target_on = state->current_values.is_on();
   ```
   In ESPHome's light architecture, `current_values` represents the light's current running state, while `remote_values` represents the target state requested by Home Assistant. If `current_values` has not completed its transition or is checked prematurely, `cur_on != target_on` can evaluate incorrectly.
2. **Toggle-Only Inversion Desync**:
   Because binary lighting circuits in CI-BUS use blind toggle commands (`cmd-tgl:<KEY_NAME>`), any dropped BLE transmission or delayed acknowledgment causes an internal phase inversion:
   - User clicks OFF in HA → command sent but dropped/missed by caravan MCU → physical light stays ON.
   - HA / ESPHome optimistically assumes the light is OFF.
   - User sees the physical light still ON, and clicks the switch in HA back to "ON".
   - ESPHome compares internal state (`OFF`) with requested state (`ON`), determines a state change is needed, and sends `cmd-tgl:<KEY_NAME>`.
   - The caravan MCU receives the toggle and switches the light from ON to OFF.
3. **State Confirmation & Retries**:
   The component currently does not verify that the MCU returned `LIGHT_WASCH:0` after sending `cmd-tgl:LIGHT_WASCH`, leading to state desynchronization.

## Scope & Proposed Fix
1. Change `write_state()` to evaluate `state->remote_values.is_on()` instead of `current_values.is_on()`.
2. Investigate whether the Toptron/CI-BUS protocol supports direct state commands (such as `cmd-set:LIGHT_<NAME>=0` and `cmd-set:LIGHT_<NAME>=1`) as used by dimmers, which eliminates toggle race conditions.
3. If only `cmd-tgl:` is supported, introduce state confirmation tracking: if a toggle is sent and the MCU notification does not match the expected state within a timeout window, resynchronize the entity state.

## Acceptance Criteria
- [x] `FendtLightOutput::write_state` evaluates `remote_values.is_on()`.
- [ ] Turning binary lights OFF works reliably and does not leave HA out of phase with physical fixtures (ready for user re-test).
- [ ] If an action fails, the HA entity accurately snaps back to the physical state rather than inverting subsequent toggles.

Status: **In Verification**

