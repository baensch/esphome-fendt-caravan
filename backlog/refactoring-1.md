# Refactoring 1: Safety, Robustness & Low-Level C++ Bug Fixes

## Problem Statement
The initial component implementation introduced several low-level C++ safety hazards, including memory buffer overflow vulnerabilities on incoming BLE notifications, substring slicing bugs in multi-chunk command transmission, command loss from improper queue management, and missing virtual destructors.

## Target Areas & Code Changes

### 1. BLE Notification Buffer Overflow
* **File**: [`components/fendt_caravan/fendt_caravan.cpp`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/fendt_caravan.cpp#L87-L90)
* **Issue**:
  ```cpp
  char buffer[25];
  memset(buffer, 0, param->notify.value_len + 1);
  memcpy(buffer, param->notify.value, param->notify.value_len);
  ```
  `buffer` is fixed at 25 bytes. If a BLE notify event delivers $\ge 25$ bytes (e.g., standard or negotiated MTU), this smashes the stack.
* **Refactoring**: Replace raw buffer operations with safe `std::string` construction:
  ```cpp
  std::string result(reinterpret_cast<const char *>(param->notify.value), param->notify.value_len);
  ```

### 2. Command Chunking Substring Bug & Index Overflow
* **File**: [`components/fendt_caravan/fendt_caravan.cpp`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/fendt_caravan.cpp#L108-L128)
* **Issue**:
  - `chunk = cmd.substr(0, 17);` in [`add_command_`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/fendt_caravan.cpp#L116) always extracts from offset `0` rather than `start_index`. Commands split into $> 2$ chunks repeat the first chunk.
  - `int8_t` is used for indices (`start_index`, `end_index`, `last_index`), causing signed overflow on commands $> 127$ bytes.
* **Refactoring**: Use `size_t` and correct substring slicing:
  ```cpp
  void FendtCaravan::add_command_(const std::string &cmd) {
    constexpr size_t CHUNK_SIZE = 17;
    size_t start_index = 0;
    while (start_index < cmd.length()) {
      size_t remaining = cmd.length() - start_index;
      if (remaining > CHUNK_SIZE) {
        this->commands_.push_back(cmd.substr(start_index, CHUNK_SIZE) + "@");
        start_index += CHUNK_SIZE;
      } else {
        this->commands_.push_back(cmd.substr(start_index));
        break;
      }
    }
  }
  ```

### 3. Command Queue Loss via `std::remove`
* **File**: [`components/fendt_caravan/fendt_caravan.cpp`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/fendt_caravan.cpp#L40)
* **Issue**:
  ```cpp
  this->commands_.erase(std::remove(this->commands_.begin(), this->commands_.end(), cmd), this->commands_.end());
  ```
  `std::remove` wipes **all** instances of `cmd` currently in the queue. If the same command (e.g. `cmd-tgl:LIGHT_WASCH`) is queued twice in rapid succession, the second command is erased before execution.
* **Refactoring**: Use strict FIFO removal for the head element:
  ```cpp
  this->commands_.erase(this->commands_.begin());
  ```
  (Or migrate `commands_` to `std::deque<std::string>` and call `pop_front()`).

### 4. Missing Virtual Destructor on Interface
* **File**: [`components/fendt_caravan/variable.h`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/variable.h#L13-L29)
* **Issue**: [`IVariable`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/variable.h#L13) declares pure virtual methods but lacks a virtual destructor. Deleting polymorphic variables via base pointer causes undefined behavior.
* **Refactoring**: Add `virtual ~IVariable() = default;` to [`IVariable`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/variable.h#L13).

### 5. Exception Safety in Temperature Decoding
* **File**: [`components/fendt_caravan/sensor/device_decoders.cpp`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/sensor/device_decoders.cpp#L23)
* **Issue**: `std::stof(value)` throws unhandled `std::invalid_argument` or `std::out_of_range` on malformed packets, aborting firmware execution.
* **Refactoring**: Trim and parse using `parse_data<float>` (as done in `decode_voltage`), returning `NAN` or `0.0f` on failure:
  ```cpp
  float DeviceDecoders::decode_temperature(const std::string &data) {
    std::string value = data;
    size_t start = value.find("^C");
    if (start != std::string::npos)
      value.replace(start, 2, "");
    start = value.find(',');
    if (start != std::string::npos)
      value.replace(start, 1, ".");
    trim(value);
    auto result = parse_data<float>(value);
    if (!result) {
      ESP_LOGE(TAG, "Temperature parse error. Data: '%s'", data.c_str());
      return NAN;
    }
    return result.value();
  }
  ```

## Acceptance Criteria
- [ ] Large BLE notification frames ($\ge 25$ bytes) do not cause buffer overflows or stack memory corruption.
- [ ] Multi-chunk BLE commands slice consecutive 17-byte blocks correctly with `@` continuation indicators.
- [ ] Repeated commands in `FendtCaravan::commands_` are processed in FIFO order without duplicate drops.
- [ ] Interface [`IVariable`](file:///Users/lukas/development/esphome/fendt-component/fdt/components/fendt_caravan/variable.h#L13) has a virtual destructor.
- [ ] Corrupt temperature strings log an error without throwing uncaught exceptions.
