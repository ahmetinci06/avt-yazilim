# Arduino Nano ESC Integration Guide

> Connecting STM32 VCU to Arduino Nano-based ESC System

---

## 🤔 Is This a Good Idea?

### Pros ✅

- **Familiar platform** - Arduino is easier for prototyping
- **Flexibility** - Can customize ESC behavior
- **Cost effective** - Arduino Nano clones are cheap
- **Debugging** - Serial monitor for ESC debugging

### Cons ⚠️

- **Extra complexity** - Two MCUs to manage
- **Latency** - Additional processing delay
- **Reliability** - More points of failure
- **Power requirements** - Need to power Arduino separately

### Recommendation

For **prototyping/testing**: Arduino Nano ESC is acceptable.

For **competition**: Consider using a commercial VESC or direct motor driver from STM32.

If proceeding with Arduino, use **UART** (simplest) or **I2C** for communication.

---

## 📡 Communication Options

### Option 1: UART (Recommended)

**Best for:** Simple, reliable, fast communication

| STM32 Pin | Arduino Pin | Function           |
| --------- | ----------- | ------------------ |
| PA9 (TX)  | D0 (RX)     | VCU → ESC commands |
| PA10 (RX) | D1 (TX)     | ESC → VCU status   |
| GND       | GND         | Common ground      |

**Protocol:** 115200 baud, 8N1

```
┌─────────────┐         ┌─────────────┐
│   STM32     │  UART   │  Arduino    │
│   VCU       │◄───────►│  Nano ESC   │
│             │ 115200  │             │
│  PA9 (TX)───┼────────►│───D0 (RX)   │
│  PA10 (RX)◄─┼─────────│───D1 (TX)   │
│  GND────────┼─────────│───GND       │
└─────────────┘         └─────────────┘
```

---

### Option 2: I2C

**Best for:** Multiple devices on same bus

| STM32 Pin | Arduino Pin | Function      |
| --------- | ----------- | ------------- |
| PB8       | A4 (SDA)    | Data          |
| PB9       | A5 (SCL)    | Clock         |
| GND       | GND         | Common ground |

**Note:** PB8/PB9 are used for FDCAN, so this conflicts with CAN!

---

### Option 3: CAN Bus (If Arduino has CAN)

Use MCP2515 CAN module with Arduino Nano.

**Best for:** Integration with BMS on same CAN bus.

---

## 📝 Communication Protocol

### Message Format (UART)

```
[START] [CMD] [DATA...] [CHECKSUM] [END]
  0xAA   1B    0-6B        1B       0x55
```

### Commands (VCU → Arduino)

| CMD  | Name        | Data          | Description            |
| ---- | ----------- | ------------- | ---------------------- |
| 0x01 | SET_CURRENT | int16 (mA)    | Motor current command  |
| 0x02 | SET_DUTY    | int16 (0.01%) | PWM duty cycle         |
| 0x03 | SET_RPM     | int32         | Target RPM             |
| 0x10 | ENABLE      | 0/1           | Enable/disable motor   |
| 0x11 | BRAKE       | 0/1           | Activate/release brake |
| 0xF0 | STATUS_REQ  | -             | Request status         |

### Responses (Arduino → VCU)

| CMD  | Name       | Data          | Description          |
| ---- | ---------- | ------------- | -------------------- |
| 0x81 | CURRENT_FB | int16 (mA)    | Actual motor current |
| 0x82 | RPM_FB     | int32         | Actual RPM           |
| 0x83 | TEMP       | int16 (0.1°C) | ESC temperature      |
| 0x8F | STATUS     | uint8         | Fault flags          |
| 0xFF | ACK/NACK   | 0/1           | Command acknowledged |

---

## 🔧 STM32 Implementation

### CubeMX Configuration

1. Enable **USART1** (or LPUART1)
2. Pins: PA9 (TX), PA10 (RX)
3. Baud rate: 115200
4. Enable USART interrupt in NVIC

### Code: vcu_esc.h

```c
#ifndef VCU_ESC_H
#define VCU_ESC_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int16_t current_ma;     // Motor current (mA)
    int32_t rpm;            // Motor RPM
    int16_t temp_c10;       // Temp in 0.1°C
    uint8_t fault_flags;    // ESC faults
    bool enabled;           // Motor enabled
    uint32_t last_rx_tick;  // Last message timestamp
} ESC_Data_t;

void VCU_ESC_Init(void* huart);
void VCU_ESC_SetCurrent(int16_t current_ma);
void VCU_ESC_Enable(bool enable);
void VCU_ESC_ProcessRx(uint8_t* data, uint8_t len);
ESC_Data_t* VCU_ESC_GetData(void);
bool VCU_ESC_IsOnline(uint32_t tick, uint32_t timeout_ms);

#endif
```

### Code: Sending Commands

```c
void VCU_ESC_SetCurrent(int16_t current_ma) {
    uint8_t packet[6];
    packet[0] = 0xAA;           // Start
    packet[1] = 0x01;           // CMD: SET_CURRENT
    packet[2] = (current_ma >> 8) & 0xFF;
    packet[3] = current_ma & 0xFF;
    packet[4] = packet[1] ^ packet[2] ^ packet[3]; // XOR checksum
    packet[5] = 0x55;           // End

    HAL_UART_Transmit(&huart1, packet, 6, 10);
}
```

---

## 🔌 Arduino Implementation

### Receiving Commands

```cpp
#define START_BYTE 0xAA
#define END_BYTE   0x55

uint8_t rx_buffer[10];
uint8_t rx_index = 0;

void loop() {
    while (Serial.available()) {
        uint8_t b = Serial.read();

        if (b == START_BYTE) {
            rx_index = 0;
        }

        rx_buffer[rx_index++] = b;

        if (b == END_BYTE && rx_index >= 5) {
            processCommand(rx_buffer, rx_index);
            rx_index = 0;
        }
    }

    // Motor control loop
    updateMotor();
}

void processCommand(uint8_t* buf, uint8_t len) {
    // Verify checksum
    uint8_t cmd = buf[1];

    switch (cmd) {
        case 0x01: // SET_CURRENT
            int16_t current = (buf[2] << 8) | buf[3];
            setMotorCurrent(current);
            sendAck(true);
            break;

        case 0x10: // ENABLE
            motorEnabled = buf[2];
            sendAck(true);
            break;
    }
}
```

### Sending Status

```cpp
void sendStatus() {
    uint8_t packet[10];
    packet[0] = 0xAA;
    packet[1] = 0x8F;  // STATUS
    packet[2] = faultFlags;
    // ... add more data
    packet[n] = calculateChecksum(packet, n);
    packet[n+1] = 0x55;

    Serial.write(packet, n+2);
}
```

---

## ⚡ Wiring Diagram

```
    ┌─────────────────────────────────────────────────────────┐
    │                    POWER SYSTEM                         │
    │                                                         │
    │   ┌─────────┐     ┌─────────┐     ┌─────────┐          │
    │   │ Battery │────►│   BMS   │────►│ Motor   │          │
    │   │ 48V     │     │         │     │ Driver  │          │
    │   └─────────┘     └────┬────┘     └────┬────┘          │
    │                        │               │                │
    │                        │ CAN           │ PWM/Current    │
    │                        ▼               ▼                │
    │   ┌─────────────────────────────────────────────────┐  │
    │   │                  VCU (STM32)                    │  │
    │   │                                                 │  │
    │   │  PA0: Pedal ADC      PA11: CAN RX (to BMS)     │  │
    │   │  PA1: Brake GPIO     PA12: CAN TX              │  │
    │   │  PA9: UART TX ───────────────────┐             │  │
    │   │  PA10: UART RX ──────────────────┼─┐           │  │
    │   └───────────────────────────────────┼─┼───────────┘  │
    │                                       │ │               │
    │   ┌───────────────────────────────────┼─┼───────────┐  │
    │   │            Arduino Nano ESC       │ │           │  │
    │   │                                   │ │           │  │
    │   │  D0 (RX) ◄───────────────────────┘ │           │  │
    │   │  D1 (TX) ─────────────────────────►│           │  │
    │   │  D9: PWM to Motor Driver                       │  │
    │   │  A0: Current Sensor                            │  │
    │   └─────────────────────────────────────────────────┘  │
    │                                                         │
    └─────────────────────────────────────────────────────────┘
```

---

## 🛡️ Safety Considerations

1. **Watchdog Timer** - Arduino should stop motor if no command received for 100ms
2. **Current Limits** - Implement in both VCU and Arduino
3. **Temperature Protection** - Monitor ESC temperature
4. **Voltage Check** - Don't run if voltage too low/high
5. **Isolation** - Use optocouplers if voltage domains differ

---

## 📋 Integration Checklist

- [ ] Wire UART connections (TX/RX with crossover)
- [ ] Common GND between STM32 and Arduino
- [ ] Configure USART1 in CubeMX
- [ ] Create `vcu_esc.h/c` module
- [ ] Test communication with loopback
- [ ] Implement watchdog on Arduino
- [ ] Test safety shutdown
- [ ] Verify timing (command latency < 10ms)
