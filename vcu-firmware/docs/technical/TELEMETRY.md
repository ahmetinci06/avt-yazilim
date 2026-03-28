# Telemetry System Documentation

> ⚠️ **Note**: Telemetry software is developed in a separate repository. This document is for reference only.

---

## 🎯 System Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           TELEMETRY SYSTEM                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   VEHICLE SIDE                             GROUND STATION                  │
│   ────────────                             ──────────────                  │
│                                                                             │
│   ┌─────────────┐                          ┌─────────────┐                 │
│   │Motor Driver │◄── Hall Sensor           │   Laptop    │                 │
│   │   (ESC)     │    (Speed)               │             │                 │
│   └──────┬──────┘                          │  ┌───────┐  │                 │
│          │ CAN                             │  │  GUI  │  │                 │
│          ▼                                 │  │(C#/Py)│  │                 │
│   ┌─────────────┐     RF Module            │  └───────┘  │                 │
│   │     VCU     │─────────────────────────►│             │                 │
│   │    (AKS)    │    (One-Way)             │  ┌───────┐  │                 │
│   └──────┬──────┘                          │  │ .CSV  │  │                 │
│          │                                 │  │ Log   │  │                 │
│   ┌──────┴──────┐                          │  └───────┘  │                 │
│   │  SD Card    │                          └─────────────┘                 │
│   │  (Backup)   │                                                          │
│   └─────────────┘                                                          │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 📊 Telemetry Data Specification

### 🔴 MANDATORY Data Fields

These 4 values **MUST** be transmitted with timestamps:

| # | Field | Source | Unit | Range | Resolution | Priority |
|---|-------|--------|------|-------|------------|----------|
| 1 | **Vehicle Speed** | Motor driver hall sensor → CAN | km/h | 0 - 100 | 0.1 km/h | Critical |
| 2 | **Battery Pack Temperature** | BMS (highest cell temp) | °C | -20 to +80 | 0.01°C | Critical |
| 3 | **Total Battery Voltage** | BMS | V | 36 - 54 | 0.1 V | Critical |
| 4 | **Remaining Energy** | BMS or VCU calculation | Wh | 0 - 3000 | 1 Wh | Critical |

### 🟡 RECOMMENDED Optional Fields

| # | Field | Source | Unit | Range | Notes |
|---|-------|--------|------|-------|-------|
| 5 | Motor RPM | Motor driver → CAN | RPM | 0 - 6000 | From hall sensors |
| 6 | Motor Temperature | Motor driver → CAN | °C | 0 - 120 | Winding/stator temp |
| 7 | Battery Current | BMS → CAN | A | -60 to +60 | Negative = regen |
| 8 | Battery SOC | BMS → CAN | % | 0 - 100 | State of Charge |
| 9 | Motor Driver Temp | Motor driver → CAN | °C | 0 - 100 | MOSFET temperature |
| 10 | Throttle Position | VCU ADC | % | 0 - 100 | Pedal input |
| 11 | Brake Status | VCU GPIO | bool | 0/1 | Brake pressed |
| 12 | VCU State | VCU | enum | 0-7 | OFF/READY/DRIVE/FAULT |
| 13 | Fault Codes | VCU | hex | 0x0000-0xFFFF | Active faults |
| 14 | Distance Traveled | VCU calculation | km | 0 - 50 | From wheel speed |
| 15 | Efficiency | VCU calculation | Wh/km | 0 - 500 | Real-time |
| 16 | Lap Time | VCU/GPS | seconds | 0 - 3600 | Current lap |
| 17 | GPS Position | GPS module | lat/lon | - | If available |

### 📋 Full Telemetry Packet Structure

```c
// Mandatory fields (12 bytes) - MUST transmit
typedef struct __attribute__((packed)) {
    uint32_t timestamp_ms;        // Timestamp (ms since start)
    uint16_t speed_kmh_x10;       // Speed × 10 (0.1 km/h resolution)
    int16_t  battery_temp_x100;   // Max cell temp × 100 (0.01°C)
    uint16_t battery_voltage_x10; // Voltage × 10 (0.1V)
    uint16_t energy_remaining_wh; // Remaining energy (Wh)
} TelemetryMandatory_t;

// Optional fields (additional 20 bytes)
typedef struct __attribute__((packed)) {
    uint16_t motor_rpm;           // Motor RPM
    int16_t  motor_temp_x10;      // Motor temp × 10 (0.1°C)
    int16_t  battery_current_x10; // Current × 10 (0.1A, signed)
    uint8_t  battery_soc;         // SOC (0-100%)
    int8_t   driver_temp;         // Motor driver temp (°C)
    uint8_t  throttle_percent;    // Throttle position (0-100%)
    uint8_t  brake_status;        // 0=released, 1=pressed
    uint8_t  vcu_state;           // VCU state enum
    uint16_t fault_code;          // Active fault flags
    uint16_t distance_m;          // Distance × 10 (0.1 km)
    uint16_t efficiency_x10;      // Wh/km × 10
    uint16_t lap_time_ms;         // Current lap time (ms)
} TelemetryOptional_t;

// Complete packet (32 bytes)
typedef struct __attribute__((packed)) {
    TelemetryMandatory_t mandatory;
    TelemetryOptional_t optional;
    uint8_t checksum;             // XOR checksum
} TelemetryPacket_t;
```

### 📏 Data Encoding Rules

| Data Type | Encoding | Example |
|-----------|----------|---------|
| Speed 45.3 km/h | `× 10` → `453` | `uint16_t = 453` |
| Temp 32.45°C | `× 100` → `3245` | `int16_t = 3245` |
| Voltage 48.2V | `× 10` → `482` | `uint16_t = 482` |
| Current -15.3A | `× 10` → `-153` | `int16_t = -153` |
| Distance 12.5 km | `× 10` → `125` | `uint16_t = 125` |

> [!NOTE]
> Temperature and voltage values are stored in raw format (multiplied) to avoid floating-point operations on embedded systems.

---

## ⚠️ 60-Second Buffer Rule

> [!CAUTION]
> **This is one of the most critical rules!**

- If connection is lost in a remote corner of the track, **data must NOT be lost**
- The processor must store data locally (SD card or Flash memory)
- Data must be transmitted when connection is restored or after the race
- **Maximum 60 seconds of data** must be stored on the vehicle at any time

### Implementation

```c
#define TELEMETRY_BUFFER_SIZE_SECONDS  60
#define TELEMETRY_SAMPLE_RATE_HZ       10
#define BUFFER_SIZE  (TELEMETRY_BUFFER_SIZE_SECONDS * TELEMETRY_SAMPLE_RATE_HZ)

typedef struct {
    TelemetryPacket_t buffer[BUFFER_SIZE];
    uint16_t write_index;
    uint16_t read_index;
    bool connection_lost;
} TelemetryBuffer_t;

// When connection is lost:
void on_connection_lost(void) {
    telemetry.connection_lost = true;
    // Continue writing to SD card
    sd_card_write(&current_packet);
}

// When connection is restored:
void on_connection_restored(void) {
    // First send buffered data
    while (telemetry.read_index != telemetry.write_index) {
        rf_send(&telemetry.buffer[telemetry.read_index]);
        telemetry.read_index = (telemetry.read_index + 1) % BUFFER_SIZE;
    }
    telemetry.connection_lost = false;
}
```

---

## 🚫 One-Way Communication

> [!IMPORTANT]
> Your software must only transmit data in the **Vehicle → Ground** direction!

| Allowed | Direction | Description |
|---------|-----------|-------------|
| ✅ | Vehicle → Ground | Telemetry data |
| ❌ | Ground → Vehicle | Stop motor, change speed, etc. FORBIDDEN! |
| ✅ | Ground → Vehicle | Only "ping/handshake" (connection check) allowed |

---

## 🖥️ Ground Station GUI

### Requirements
- Display incoming data as **real-time graphs** or **numeric values**
- Language: C#, Python, MATLAB, etc.
- Real-time updates

### Recommended Display Layout

```
┌─────────────────────────────────────────────────────────────┐
│                    AVT TELEMETRY                            │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   SPEED       BATTERY         TEMP          ENERGY         │
│  ┌─────┐     ┌─────────┐     ┌─────────┐   ┌─────────┐    │
│  │ 45  │     │  48.2V  │     │  32°C   │   │ 2450 Wh │    │
│  │km/h │     │ ████████│     │ ████    │   │ ███████ │    │
│  └─────┘     └─────────┘     └─────────┘   └─────────┘    │
│                                                             │
│   [=================GRAPH=================]                 │
│                                                             │
│   Connection: 🟢 OK    Packets: 12,456    Lost: 0          │
└─────────────────────────────────────────────────────────────┘
```

---

## 📁 CSV Logging Format

The ground station software must save incoming data in CSV format for judge review:

```csv
timestamp_ms,speed_kmh,battery_temp_c,battery_voltage_v,energy_wh
0,0.0,25.5,48.2,2800
100,2.5,25.6,48.1,2798
200,5.2,25.8,48.0,2795
...
```

### Logging Rules
- Temperature and voltage data should be stored in **raw format** (e.g., ×100) with comma separator
- Separate file for each race
- Filename: `AVT_RACE_YYYY-MM-DD_HH-MM-SS.csv`

---

## 🔗 Integration Points

### VCU Side

```c
// Collect data from CAN
void telemetry_collect_data(TelemetryPacket_t* packet) {
    packet->timestamp_ms = HAL_GetTick();
    packet->speed_kmh_x10 = motor_driver_get_speed() * 10;
    packet->battery_temp_x100 = bms_get_max_temp() * 100;
    packet->battery_voltage_x10 = bms_get_voltage() * 10;
    packet->energy_remaining_wh = energy_get_remaining_wh();
}

// Send to RF module (10 Hz)
void telemetry_send_task(void) {
    static uint32_t last_send = 0;
    
    if (HAL_GetTick() - last_send >= 100) {  // 10 Hz
        TelemetryPacket_t packet;
        telemetry_collect_data(&packet);
        
        // Backup to SD card
        sd_card_log(&packet);
        
        // Transmit via RF
        rf_transmit(&packet, sizeof(packet));
        
        last_send = HAL_GetTick();
    }
}
```

---

## 📚 Related Documents

- [🏆 Competition Requirements](COMPETITION-REQUIREMENTS.md) - Energy Mgmt & Telemetry requirements
- [💻 Software Architecture](SOFTWARE-ARCHITECTURE.md) - VCU software architecture
- [📡 CAN Protocol](vcu_can.md) - BMS/Motor driver communication

---

## 🔗 Telemetry Repository

> Telemetry software is developed in a separate repository.
> 
> **Repository**: *[Add link here]*
