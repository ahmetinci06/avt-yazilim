# VCU State Machine Module (`vcu_state`)

## Overview

The state machine controls the VCU's operating mode and ensures safe transitions between states.

---

## States

```
    ┌─────┐         ┌───────┐         ┌───────┐
    │ OFF │────────▶│ READY │────────▶│ DRIVE │
    └─────┘ Power   └───────┘ Start   └───────┘
       ▲     On         │      +Brake     │
       │                │     Released    │
       │    ┌───────────┴─────────────────┤
       │    │                             │
       │    ▼                             ▼
    ┌──┴────────────────────────────────────┐
    │              FAULT                    │
    │  (Any fault triggers transition)      │
    └───────────────────────────────────────┘
```

| State     | Torque      | LED Pattern | Description              |
| --------- | ----------- | ----------- | ------------------------ |
| **OFF**   | ❌ Disabled | Off         | System idle              |
| **READY** | ❌ Disabled | Slow blink  | Pre-charge done, waiting |
| **DRIVE** | ✅ Active   | Solid on    | Torque commands enabled  |
| **FAULT** | ❌ Disabled | Fast blink  | Critical failure         |

---

## Files

| File   | Location               |
| ------ | ---------------------- |
| Header | `Core/Inc/vcu_state.h` |
| Source | `Core/Src/vcu_state.c` |

---

## Fault Codes

| Code                     | Bit   | Description               |
| ------------------------ | ----- | ------------------------- |
| `FAULT_PEDAL_WIRE_BREAK` | 0x01  | Pedal ADC < min threshold |
| `FAULT_PEDAL_SHORT`      | 0x02  | Pedal ADC > max threshold |
| `FAULT_BMS_TIMEOUT`      | 0x04  | No BMS message in 500ms   |
| `FAULT_BMS_OVERVOLT`     | 0x08  | Battery overvoltage       |
| `FAULT_BMS_UNDERVOLT`    | 0x10  | Battery undervoltage      |
| `FAULT_BMS_OVERTEMP`     | 0x20  | Battery overtemperature   |
| `FAULT_BMS_OVERCURRENT`  | 0x40  | Battery overcurrent       |
| `FAULT_VESC_TIMEOUT`     | 0x80  | No VESC response in 200ms |
| `FAULT_VESC_ERROR`       | 0x100 | VESC reported error       |

---

## API Functions

### Initialization

```c
void VCU_State_Init(StateMachine_t* sm);
```

### Main Loop

```c
void VCU_State_Update(StateMachine_t* sm, uint32_t current_tick);
```

### State Requests

```c
bool VCU_State_Request(StateMachine_t* sm, VCU_StateRequest_t request);
// Requests: VCU_REQUEST_POWER_ON, VCU_REQUEST_START, VCU_REQUEST_STOP, etc.
```

### Fault Management

```c
void VCU_State_SetFault(StateMachine_t* sm, uint32_t fault);
void VCU_State_ClearFault(StateMachine_t* sm, uint32_t fault);
bool VCU_State_HasFault(const StateMachine_t* sm);
```

### Status Checks

```c
bool VCU_State_IsTorqueAllowed(const StateMachine_t* sm);
VCU_State_t VCU_State_GetCurrent(const StateMachine_t* sm);
const char* VCU_State_GetName(VCU_State_t state);
```

### Heartbeat Updates

```c
void VCU_State_UpdateBMSHeartbeat(StateMachine_t* sm, uint32_t current_tick);
void VCU_State_UpdateVESCHeartbeat(StateMachine_t* sm, uint32_t current_tick);
```

---

## Usage Example

```c
#include "vcu_state.h"
#include "vcu_pedal.h"

static StateMachine_t state_machine;
static PedalInput_t pedal;

int main(void) {
    VCU_State_Init(&state_machine);
    VCU_Pedal_Init(&pedal);

    // Power on
    VCU_State_Request(&state_machine, VCU_REQUEST_POWER_ON);

    while (1) {
        uint32_t tick = HAL_GetTick();

        // Update brake state
        state_machine.brake_pressed = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);

        // Read pedal
        PedalFault_t pedal_status = VCU_Pedal_Read(&pedal, adc_value);

        // Set fault if pedal error
        if (pedal_status == PEDAL_FAULT_WIRE_BREAK) {
            VCU_State_SetFault(&state_machine, FAULT_PEDAL_WIRE_BREAK);
        }

        // Update state machine
        VCU_State_Update(&state_machine, tick);

        // Only output torque if allowed
        if (VCU_State_IsTorqueAllowed(&state_machine)) {
            float torque = VCU_Pedal_GetPercent(&pedal);
            // Send to VESC...
        } else {
            // Send zero torque
        }

        HAL_Delay(10);
    }
}
```

---

## State Transition Rules

| From  | To    | Condition                                        |
| ----- | ----- | ------------------------------------------------ |
| OFF   | READY | `VCU_REQUEST_POWER_ON` + no faults               |
| READY | DRIVE | `VCU_REQUEST_START` + brake released + no faults |
| DRIVE | READY | `VCU_REQUEST_STOP` + brake pressed               |
| READY | OFF   | `VCU_REQUEST_POWER_OFF`                          |
| ANY   | FAULT | Any fault flag set                               |
| FAULT | OFF   | `VCU_REQUEST_CLEAR_FAULT` + all faults cleared   |

---

## Timeout Configuration

| Parameter              | Value  | Description                     |
| ---------------------- | ------ | ------------------------------- |
| `TIMEOUT_BMS_MS`       | 500ms  | Max time between BMS messages   |
| `TIMEOUT_VESC_MS`      | 200ms  | Max time between VESC responses |
| `TIMEOUT_PRECHARGE_MS` | 5000ms | Pre-charge max duration         |
