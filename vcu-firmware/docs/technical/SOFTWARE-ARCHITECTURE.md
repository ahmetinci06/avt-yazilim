# VCU Software Architecture

## 🎯 Overview

This document describes the software architecture for the AVT VCU firmware, designed for the TÜBİTAK Elektrikli Araç Yarışları.

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          Application Layer                               │
│   ┌─────────────────────────────────────────────────────────────────┐   │
│   │                     Main Control Loop                            │   │
│   │    (vcu_main_task: 1ms cycle time)                              │   │
│   └─────────────────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────────────────┤
│                           State Machine (FSM)                           │
│   ┌─────────────────────────────────────────────────────────────────┐   │
│   │   OFF  ──►  READY  ──►  DRIVE  ──►  COAST/REGEN  ──►  FAULT    │   │
│   └─────────────────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────────────────┤
│                         Middleware Services                             │
│   ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐              │
│   │  Energy  │  │  Motor   │  │  Safety  │  │   Energy Mgmt   │              │
│   │  Mgmt    │  │  Ctrl    │  │  Monitor │  │ Features │              │
│   └──────────┘  └──────────┘  └──────────┘  └──────────┘              │
├─────────────────────────────────────────────────────────────────────────┤
│                       Hardware Abstraction Layer                        │
│   ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐              │
│   │   ADC    │  │  FDCAN   │  │   PWM    │  │   GPIO   │              │
│   │  (Pedal) │  │ (BMS/MC) │  │ (Motor)  │  │ (Status) │              │
│   └──────────┘  └──────────┘  └──────────┘  └──────────┘              │
├─────────────────────────────────────────────────────────────────────────┤
│                        STM32 HAL/LL Drivers                             │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 📦 Module Descriptions

### 1. Core Modules

#### Main Control Loop
```c
// main.c - Main control loop structure
void vcu_main_task(void) {
    while(1) {
        // 1ms tasks
        vcu_state_update();       // State machine
        vcu_pedal_process();      // Input processing
        vcu_motor_control();      // Torque command
        vcu_safety_check();       // Safety monitoring
        vcu_can_process();        // CAN communication
        
        // 10ms tasks
        if (tick_10ms) {
            vcu_energy_monitor();  // Energy calculations
        }
        
        // 100ms tasks
        if (tick_100ms) {
            vcu_telemetry_send();  // Data transmission
        }
    }
}
```

---

### 2. State Machine

#### State Definitions
```c
typedef enum {
    VCU_STATE_OFF,       // System powered off
    VCU_STATE_INIT,      // Initialization
    VCU_STATE_IDLE,      // Ready, no drive
    VCU_STATE_PRECHARGE, // Precharging capacitors
    VCU_STATE_READY,     // Ready to drive
    VCU_STATE_DRIVE,     // Normal driving
    VCU_STATE_COAST,     // Coasting (no power)
    VCU_STATE_REGEN,     // Regenerative braking
    VCU_STATE_FAULT      // Fault condition
} VCU_State_t;
```

#### State Transition Diagram
```
              ┌─────────────┐
              │     OFF     │◄────────────────────────────┐
              └──────┬──────┘                             │
                     │ Power On                           │ Clear Fault
                     ▼                                    │ + Key Off
              ┌─────────────┐                        ┌────┴────┐
              │    INIT     │                        │  FAULT  │
              └──────┬──────┘                        └────▲────┘
                     │ Init Complete                      │
                     ▼                                    │
              ┌─────────────┐                            │
              │    IDLE     │───────────────────────────►│
              └──────┬──────┘  Any Critical Fault        │
                     │ Start Request                      │
                     ▼                                    │
              ┌─────────────┐                            │
              │  PRECHARGE  │───────────────────────────►│
              └──────┬──────┘  Timeout/Fault             │
                     │ Precharge Done                     │
                     ▼                                    │
              ┌─────────────┐                            │
              │    READY    │───────────────────────────►│
              └──────┬──────┘  Any Fault                 │
                     │ Pedal + No Brake                   │
                     ▼                                    │
              ┌─────────────┐                            │
         ┌───►│    DRIVE    │◄───┐                       │
         │    └──────┬──────┘    │                       │
         │           │           │                       │
  Pedal  │    Brake  │   Pedal   │                       │
  Input  │      ▼    │    Input  │                       │
         │    ┌──────┴──────┐    │                       │
         │    │    COAST    │    │                       │
         │    └──────┬──────┘    │                       │
         │           │           │                       │
         │           ▼           │                       │
         │    ┌─────────────┐    │                       │
         └────│    REGEN    │────┘                       │
              └──────┬──────┘                            │
                     │ Any Fault                          │
                     └────────────────────────────────────┘
```

---

### 3. Fault Codes

```c
typedef enum {
    // Pedal Faults (0x0001 - 0x000F)
    FAULT_PEDAL_WIRE_BREAK    = 0x0001,
    FAULT_PEDAL_SHORT_CIRCUIT = 0x0002,
    FAULT_PEDAL_IMPLAUSIBLE   = 0x0004,
    
    // CAN Faults (0x0010 - 0x00F0)
    FAULT_CAN_BMS_TIMEOUT     = 0x0010,
    FAULT_CAN_VESC_TIMEOUT    = 0x0020,
    FAULT_CAN_BUS_OFF         = 0x0040,
    
    // BMS Faults (0x0100 - 0x0F00)
    FAULT_BMS_OVERVOLTAGE     = 0x0100,
    FAULT_BMS_UNDERVOLTAGE    = 0x0200,
    FAULT_BMS_OVERCURRENT     = 0x0400,
    FAULT_BMS_OVERTEMP        = 0x0800,
    
    // Motor Faults (0x1000 - 0xF000)
    FAULT_MOTOR_OVERTEMP      = 0x1000,
    FAULT_MOTOR_OVERCURRENT   = 0x2000,
    FAULT_MOTOR_STALL         = 0x4000
} VCU_FaultCode_t;
```

---

### 4. Communication Protocol

#### CAN Message Definitions
```c
// CAN Message IDs
#define CAN_ID_VCU_STATUS    0x100  // VCU → All: Status broadcast
#define CAN_ID_VCU_COMMAND   0x101  // VCU → Motor Driver
#define CAN_ID_BMS_STATUS    0x200  // BMS → VCU: Battery data
#define CAN_ID_BMS_CELLS     0x201  // BMS → VCU: Cell voltages
#define CAN_ID_MOTOR_STATUS  0x300  // Motor → VCU: Motor data
#define CAN_ID_TELEMETRY     0x400  // VCU → Telemetry
#define CAN_ID_Energy Mgmt_DATA     0x500  // Energy Mgmt → VCU
```

#### Message Structures
```c
// VCU Status Message (0x100)
typedef struct __attribute__((packed)) {
    uint8_t  state;           // VCU state
    uint8_t  fault_low;       // Fault code (low byte)
    uint8_t  fault_high;      // Fault code (high byte)
    uint8_t  pedal_percent;   // Pedal position (0-100)
    uint16_t motor_rpm;       // Motor speed
    uint16_t reserved;
} CAN_VCU_Status_t;

// Motor Command Message (0x101)
typedef struct __attribute__((packed)) {
    int16_t  torque_request;  // Torque in 0.1 Nm
    uint8_t  mode;            // 0=Off, 1=Torque, 2=Speed
    uint8_t  enable;          // Drive enable
    uint16_t speed_limit;     // Speed limit in RPM
    uint16_t reserved;
} CAN_Motor_Command_t;
```

---

## 📂 File Organization

```
Core/
├── Inc/
│   ├── main.h                # Main configuration
│   ├── vcu_config.h          # System constants
│   ├── vcu_types.h           # Common type definitions
│   │
│   ├── vcu_pedal.h           # Pedal input module
│   ├── vcu_state.h           # State machine
│   ├── vcu_can.h             # CAN communication
│   ├── vcu_energy.h          # Energy management
│   ├── vcu_safety.h          # Safety systems
│   └── vcu_adas.h            # energy management
│
└── Src/
    ├── main.c                # Entry point & main loop
    ├── stm32g4xx_it.c        # Interrupt handlers
    │
    ├── vcu_pedal.c           # ✅ Implemented
    ├── vcu_state.c           # ✅ Implemented
    ├── vcu_can.c             # ✅ Implemented
    ├── vcu_energy.c          # 📋 Planned
    ├── vcu_safety.c          # 📋 Planned
    └── vcu_adas.c            # 📋 Planned

motor_driver/                 # 🚧 Six-Step Motor Control
├── bldc_motor.h              # Motor control API
├── bldc_motor.c              # 6-step commutation logic
└── README.md                 # Module documentation

docs/
├── README.md                 # Documentation index
├── meeting/                  # Meeting reports
├── architecture.md           # Detailed architecture
├── vcu_pedal.md             # Pedal module docs
├── vcu_state.md             # State machine docs
├── vcu_can.md               # CAN protocol docs
└── PROJECT-MASTER-PLAN.md   # Motor control planning
```

---

## ⏱️ Timing Requirements

| Task | Period | Priority | WCET |
|------|--------|----------|------|
| ADC Reading | 100 µs | High | 10 µs |
| FOC Loop | 50 µs | Critical | 20 µs |
| State Machine | 1 ms | High | 50 µs |
| CAN Processing | 1 ms | Medium | 100 µs |
| Energy Calc | 10 ms | Low | 200 µs |
| Telemetry | 100 ms | Low | 500 µs |

---

## 🔧 Configuration Constants

```c
// vcu_config.h

// System Timing
#define MAIN_LOOP_PERIOD_MS     1
#define CAN_TIMEOUT_BMS_MS      500
#define CAN_TIMEOUT_VESC_MS     200

// Motor Limits
#define MAX_MOTOR_CURRENT_A     50.0f
#define MAX_MOTOR_RPM           4000
#define MAX_MOTOR_TEMP_C        80.0f

// Energy Targets
#define TARGET_EFFICIENCY_WHKM  110.0f
#define BATTERY_CAPACITY_WH     2800.0f
#define REGEN_MAX_CURRENT_A     20.0f

// Safety Limits
#define BATTERY_MAX_VOLTAGE_V   52.0f
#define BATTERY_MIN_VOLTAGE_V   36.0f
#define BATTERY_MAX_CURRENT_A   60.0f
```

---

## 📚 Related Documents

- [📋 Project Roadmap](PROJECT-ROADMAP.md)
- [🔌 CAN Protocol](vcu_can.md)
- [🎛️ Pedal Module](vcu_pedal.md)
- [🔄 State Machine](vcu_state.md)
