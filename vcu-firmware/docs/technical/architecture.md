# VCU System Architecture

> Data flow and component architecture for the AVT Vehicle Control Unit

---

## 🏗 System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                         VCU (STM32G491RE)                       │
│                                                                 │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────────────┐ │
│  │   INPUTS    │    │   LOGIC     │    │      OUTPUTS        │ │
│  │             │    │             │    │                     │ │
│  │ ▪ Pedal ADC │───▶│ ▪ State     │───▶│ ▪ Torque Cmd (CAN)  │ │
│  │ ▪ Brake GPIO│    │   Machine   │    │ ▪ LED Status        │ │
│  │ ▪ BMS (CAN) │    │ ▪ Safety    │    │ ▪ Telemetry (UART)  │ │
│  │             │    │   Checks    │    │                     │ │
│  └─────────────┘    └─────────────┘    └─────────────────────┘ │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📊 Data Flow

```
                    ┌──────────────────────────────────────────┐
                    │              SENSOR INPUTS               │
                    └──────────────────────────────────────────┘
                                        │
                    ┌───────────────────┼───────────────────┐
                    ▼                   ▼                   ▼
            ┌───────────┐       ┌───────────┐       ┌───────────┐
            │  PEDAL    │       │   BRAKE   │       │    BMS    │
            │  (ADC1)   │       │  (GPIO)   │       │   (CAN)   │
            └───────────┘       └───────────┘       └───────────┘
                    │                   │                   │
                    ▼                   ▼                   ▼
            ┌───────────────────────────────────────────────────┐
            │                 SAFETY VALIDATION                 │
            │                                                   │
            │  ▪ Wire-break detection (ADC < 200)              │
            │  ▪ Short-circuit detection (ADC > 3800)          │
            │  ▪ BMS fault flags                               │
            │  ▪ Timeout monitoring                             │
            └───────────────────────────────────────────────────┘
                                        │
                                        ▼
            ┌───────────────────────────────────────────────────┐
            │                  STATE MACHINE                    │
            │                                                   │
            │   ┌─────┐    ┌───────┐    ┌───────┐    ┌───────┐ │
            │   │ OFF │───▶│ READY │───▶│ DRIVE │    │ FAULT │ │
            │   └─────┘    └───────┘    └───────┘    └───────┘ │
            │       ▲                        │           ▲      │
            │       └────────────────────────┴───────────┘      │
            └───────────────────────────────────────────────────┘
                                        │
                                        ▼
            ┌───────────────────────────────────────────────────┐
            │                 TORQUE CONTROLLER                 │
            │                                                   │
            │  pedal_percent ──▶ efficiency_map ──▶ torque_cmd │
            │                                                   │
            │  Safety override: brake || fault ──▶ torque = 0  │
            └───────────────────────────────────────────────────┘
                                        │
                    ┌───────────────────┼───────────────────┐
                    ▼                   ▼                   ▼
            ┌───────────┐       ┌───────────┐       ┌───────────┐
            │   VESC    │       │ TELEMETRY │       │  STATUS   │
            │   (CAN)   │       │  (UART)   │       │   LED     │
            └───────────┘       └───────────┘       └───────────┘
```

---

## 🔄 State Machine

### States

| State     | LED        | Torque | Description                                   |
| --------- | ---------- | ------ | --------------------------------------------- |
| **OFF**   | Off        | 0      | System idle, waiting for power-up             |
| **READY** | Slow blink | 0      | Pre-charge done, safety OK, waiting for start |
| **DRIVE** | Solid      | Active | Torque commands enabled                       |
| **FAULT** | Fast blink | 0      | Critical failure, motor disabled              |

### Transitions

```
  OFF ──────────────▶ READY
       Power-up +
       Safety check OK

  READY ────────────▶ DRIVE
       Start button +
       Brake released

  DRIVE ────────────▶ READY
       Brake + Stop button

  ANY ──────────────▶ FAULT
       BMS fault
       Sensor failure
       CAN timeout
```

---

## 🧩 Module Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                       APPLICATION LAYER                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐ │
│  │ vcu_state   │  │ vcu_energy  │  │ vcu_telemetry       │ │
│  │ (planned)   │  │ (planned)   │  │ (planned)           │ │
│  └─────────────┘  └─────────────┘  └─────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                       MIDDLEWARE LAYER                      │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐ │
│  │ vcu_pedal   │  │ vcu_can     │  │ vcu_safety          │ │
│  │ ✅ DONE     │  │ (planned)   │  │ (planned)           │ │
│  └─────────────┘  └─────────────┘  └─────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                       DRIVER LAYER (HAL)                    │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐ │
│  │ ADC         │  │ FDCAN       │  │ GPIO / UART / TIM   │ │
│  │ ✅ CONFIG   │  │ (planned)   │  │ ✅ CONFIG           │ │
│  └─────────────┘  └─────────────┘  └─────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

---

## ⏱ Timing Requirements

| Task            | Frequency     | Priority |
| --------------- | ------------- | -------- |
| Main loop       | 100 Hz (10ms) | Normal   |
| ADC sampling    | Per loop      | Normal   |
| Safety checks   | Per loop      | High     |
| CAN TX (torque) | 100 Hz        | High     |
| CAN RX (BMS)    | Event-driven  | High     |
| Telemetry TX    | 10-50 Hz      | Low      |

---

## 🔌 CAN Bus Message Map

### Receive (RX)

| ID    | Source | Data                   | Rate  |
| ----- | ------ | ---------------------- | ----- |
| 0x100 | BMS    | Voltage, Current, Temp | 10 Hz |
| 0x101 | BMS    | SOC, Fault flags       | 10 Hz |
| 0x201 | VESC   | Motor RPM, Temp        | 10 Hz |

### Transmit (TX)

| ID    | Dest      | Data           | Rate   |
| ----- | --------- | -------------- | ------ |
| 0x200 | VESC      | Torque command | 100 Hz |
| 0x300 | Telemetry | VCU status     | 10 Hz  |

---

## 📁 File Map

| Module    | Header            | Source            | Status     |
| --------- | ----------------- | ----------------- | ---------- |
| Pedal     | `vcu_pedal.h`     | `vcu_pedal.c`     | ✅ Done    |
| CAN       | `vcu_can.h`       | `vcu_can.c`       | 📋 Planned |
| State     | `vcu_state.h`     | `vcu_state.c`     | 📋 Planned |
| Energy    | `vcu_energy.h`    | `vcu_energy.c`    | 📋 Planned |
| Telemetry | `vcu_telemetry.h` | `vcu_telemetry.c` | 📋 Planned |
