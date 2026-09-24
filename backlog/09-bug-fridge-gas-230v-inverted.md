# Story 09: Bug - Inverted Refrigerator Energy Sources (Gas and 230V)

## Problem Statement
During hardware testing of the CI-BUS refrigerator controls (Story 07), the user observed that selecting **Gas** in Home Assistant sets the physical refrigerator to **230V**, and selecting **230V** sets the physical refrigerator to **Gas**. 

Currently, `FridgeDeviceSensor` decodes `FRIDGE_SOURCE` with the following mapping:
- `0`: Auto
- `1`: 230V (Incorrect)
- `2`: 12V
- `3`: Gas (Incorrect)

## Root Cause Analysis
In the CI-BUS specification for Dometic / Thetford AES (Automatic Energy Selector) refrigerators, the integer index ordering is:
* `0`: Auto (`AES`)
* `1`: Gas
* `2`: 12V DC (Battery / Tow Vehicle)
* `3`: 230V AC (Mains)

Indices `1` and `3` are currently swapped in `fridge_device_sensor.cpp`.

## Acceptance Criteria
- [x] When `FRIDGE_SOURCE` receives `1`, the select entity displays `Gas`.
- [x] When `FRIDGE_SOURCE` receives `3`, the select entity displays `230V`.
- [x] Selecting `Gas` in Home Assistant transmits `cmd-set:FRIDGE_SOURCE=1`.
- [x] Selecting `230V` in Home Assistant transmits `cmd-set:FRIDGE_SOURCE=3`.
- [x] Selections for `Auto` (`0`) and `12V` (`2`) remain unchanged.
- [x] Verified on live vehicle hardware by the user.

Status: **Done**

