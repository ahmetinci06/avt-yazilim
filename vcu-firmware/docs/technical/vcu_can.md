# VCU CAN Module (`vcu_can`)

## Overview

FDCAN communication module for BMS and VESC motor controller.

**Pin Configuration:**

- PA11: FDCAN1_RX
- PA12: FDCAN1_TX
- Bitrate: 500 kbit/s (Classic CAN)

---

## Files

| File   | Location             |
| ------ | -------------------- |
| Header | `Core/Inc/vcu_can.h` |
| Source | `Core/Src/vcu_can.c` |

---

## CAN Message IDs

### BMS Messages (Receive)

| ID    | Name        | Data                          |
| ----- | ----------- | ----------------------------- |
| 0x100 | BMS_STATUS  | Fault flags                   |
| 0x101 | BMS_VOLTAGE | Pack voltage (0.01V units)    |
| 0x102 | BMS_CURRENT | Current (0.01A units, signed) |
| 0x104 | BMS_SOC     | State of charge (%)           |

### VESC Messages

| ID Formula                | Name        | Direction |
| ------------------------- | ----------- | --------- |
| `(0x001 << 8) \| VESC_ID` | SET_CURRENT | TX        |
| `(0x009 << 8) \| VESC_ID` | STATUS_1    | RX        |
| `(0x010 << 8) \| VESC_ID` | STATUS_4    | RX        |

### VCU Telemetry (Transmit)

| ID    | Name       | Data              |
| ----- | ---------- | ----------------- |
| 0x300 | VCU_STATUS | State, fault code |
| 0x301 | VCU_PEDAL  | Pedal %, brake    |

---

## API Functions

### Initialization

```c
bool VCU_CAN_Init(void* hfdcan);   // Pass &hfdcan1
bool VCU_CAN_Start(void);          // Start with filters
```

### Main Loop

```c
void VCU_CAN_Process(uint32_t current_tick);
```

### Torque Control

```c
bool VCU_CAN_SendTorqueCommand(float current_amps);
bool VCU_CAN_SendZeroTorque(void);
```

### Status Check

```c
bool VCU_CAN_IsBMSOnline(uint32_t tick, uint32_t timeout_ms);
bool VCU_CAN_IsVESCOnline(uint32_t tick, uint32_t timeout_ms);
BMS_Data_t* VCU_CAN_GetBMSData(void);
VESC_Data_t* VCU_CAN_GetVESCData(void);
```

---

## Usage Example

```c
#include "vcu_can.h"

extern FDCAN_HandleTypeDef hfdcan1;

void main_init(void) {
    VCU_CAN_Init(&hfdcan1);
    VCU_CAN_Start();
}

void main_loop(void) {
    uint32_t tick = HAL_GetTick();

    VCU_CAN_Process(tick);

    // Check BMS timeout
    if (!VCU_CAN_IsBMSOnline(tick, 500)) {
        VCU_State_SetFault(&state_machine, FAULT_BMS_TIMEOUT);
    }

    // Send torque if allowed
    if (VCU_State_IsTorqueAllowed(&state_machine)) {
        float current = pedal_percent * MAX_CURRENT / 100.0f;
        VCU_CAN_SendTorqueCommand(current);
    } else {
        VCU_CAN_SendZeroTorque();
    }
}

// In stm32g4xx_it.c, add callback:
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    VCU_CAN_RxCallback(hfdcan, RxFifo0ITs);
}
```

---

## CubeMX Configuration Required

1. Enable **FDCAN1**
2. Set pins: **PA11 (RX)**, **PA12 (TX)**
3. Configure **500 kbit/s** bitrate
4. Enable **FDCAN1 interrupt** in NVIC
5. Generate code

---

## Data Structures

### BMS_Data_t

```c
typedef struct {
    float    pack_voltage;   // V
    float    pack_current;   // A (+ = discharge)
    float    soc_percent;    // 0-100%
    uint8_t  fault_flags;    // BMS faults
    bool     is_online;      // Communication active
} BMS_Data_t;
```

### VESC_Data_t

```c
typedef struct {
    float    rpm;            // Motor RPM
    float    current_motor;  // Motor current (A)
    float    temp_mos;       // MOSFET temp (°C)
    float    temp_motor;     // Motor temp (°C)
    bool     is_online;      // Communication active
} VESC_Data_t;
```

---

## BMS Fault Flags

| Flag                    | Bit  | Description          |
| ----------------------- | ---- | -------------------- |
| `BMS_FAULT_OVERVOLT`    | 0x01 | Battery overvoltage  |
| `BMS_FAULT_UNDERVOLT`   | 0x02 | Battery undervoltage |
| `BMS_FAULT_OVERTEMP`    | 0x04 | Overtemperature      |
| `BMS_FAULT_OVERCURRENT` | 0x10 | Overcurrent          |
