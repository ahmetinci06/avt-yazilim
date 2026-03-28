# VCU Pedal Module (`vcu_pedal`)

## Overview

The pedal module handles:

- ADC reading from the accelerator pedal sensor
- Low-pass filtering for noise reduction
- Safety interlocks (wire-break and short-circuit detection)
- Calibration support

---

## Files

| File   | Location               |
| ------ | ---------------------- |
| Header | `Core/Inc/vcu_pedal.h` |
| Source | `Core/Src/vcu_pedal.c` |

---

## Data Structures

### `PedalInput_t`

```c
typedef struct {
    uint16_t raw_value;        // Raw ADC value (0-4095)
    uint16_t min_threshold;    // Wire-break threshold
    uint16_t max_threshold;    // Short-circuit threshold
    uint16_t idle_value;       // Pedal released position
    uint16_t full_value;       // Pedal fully pressed
    float    filtered_value;   // Low-pass filtered ADC value
    float    pedal_percent;    // Mapped position 0.0 - 100.0%
    PedalFault_t fault_status; // Current fault status
} PedalInput_t;
```

### `PedalFault_t`

| Value                       | Description                          |
| --------------------------- | ------------------------------------ |
| `PEDAL_OK`                  | Normal operation                     |
| `PEDAL_FAULT_WIRE_BREAK`    | ADC < min threshold (open circuit)   |
| `PEDAL_FAULT_SHORT_CIRCUIT` | ADC > max threshold (shorted to VCC) |
| `PEDAL_FAULT_OUT_OF_RANGE`  | Value outside calibrated range       |

---

## API Functions

### `VCU_Pedal_Init()`

Initialize pedal structure with default thresholds.

```c
void VCU_Pedal_Init(PedalInput_t* pedal);
```

### `VCU_Pedal_Read()`

Read ADC value, apply filter, check for faults.

```c
PedalFault_t VCU_Pedal_Read(PedalInput_t* pedal, uint16_t adc_raw);
```

### `VCU_Pedal_GetPercent()`

Get current pedal position (0-100%). Returns 0 if fault active.

```c
float VCU_Pedal_GetPercent(const PedalInput_t* pedal);
```

### `VCU_Pedal_HasFault()`

Check if any fault is active.

```c
bool VCU_Pedal_HasFault(const PedalInput_t* pedal);
```

### `VCU_Pedal_SetCalibration()`

Set custom idle/full ADC values for calibration.

```c
void VCU_Pedal_SetCalibration(PedalInput_t* pedal, uint16_t idle, uint16_t full);
```

---

## Usage Example

```c
#include "vcu_pedal.h"

static PedalInput_t pedal;

int main(void) {
    // Initialize
    VCU_Pedal_Init(&pedal);

    // Optionally calibrate
    VCU_Pedal_SetCalibration(&pedal, 300, 3500);

    while (1) {
        // Read ADC
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            uint16_t adc_raw = HAL_ADC_GetValue(&hadc1);

            // Process pedal
            PedalFault_t status = VCU_Pedal_Read(&pedal, adc_raw);

            if (status == PEDAL_OK) {
                float position = VCU_Pedal_GetPercent(&pedal);
                // Use position for torque calculation
            } else {
                // Handle fault - cut torque to 0
            }
        }
    }
}
```

---

## Integration in main.c

The pedal module is integrated into `main.c` with:

1. **Include**: `#include "vcu_pedal.h"` in USER CODE Includes
2. **Variables**: `PedalInput_t pedal` and `brake_pressed` in USER CODE PV
3. **Init**: `VCU_Pedal_Init(&pedal)` + ADC calibration in USER CODE 2
4. **Loop**: Read ADC → process → check faults at 100Hz

See [main.c](../Core/Src/main.c) for full implementation.

---

## Calibration

### Default Thresholds

| Parameter             | Value | Voltage (~3.3V ref) |
| --------------------- | ----- | ------------------- |
| `PEDAL_ADC_MIN_VALID` | 200   | ~0.16V              |
| `PEDAL_ADC_MAX_VALID` | 3800  | ~3.07V              |
| `PEDAL_ADC_IDLE`      | 250   | ~0.20V              |
| `PEDAL_ADC_FULL`      | 3600  | ~2.90V              |

### Calibration Procedure

1. **Idle Position**: Release pedal completely, note ADC value
2. **Full Position**: Press pedal fully, note ADC value
3. Call `VCU_Pedal_SetCalibration(idle_value, full_value)`

---

## Safety Interlocks

| Condition     | Detection     | Action                    |
| ------------- | ------------- | ------------------------- |
| Wire break    | ADC < 200     | Torque = 0, FAULT state   |
| Short circuit | ADC > 3800    | Torque = 0, FAULT state   |
| Brake pressed | GPIO PA1 HIGH | Torque = 0 (in main loop) |

---

## Filter Tuning

The low-pass filter uses exponential moving average:

```
filtered = α × new_value + (1 - α) × old_value
```

Default `PEDAL_FILTER_ALPHA = 0.1` provides strong smoothing.

| α Value | Response                        |
| ------- | ------------------------------- |
| 0.05    | Very smooth, slow response      |
| 0.1     | Balanced (default)              |
| 0.2     | Faster response, less filtering |
| 0.5     | Minimal filtering               |
