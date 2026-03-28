/**
  ******************************************************************************
  * @file    esc_telemetry.h
  * @brief   Telemetry module — SD card logging + UART serial output
  ******************************************************************************
  * @note    Logs vehicle data for post-race analysis and real-time dashboard.
  *
  *          Two output channels:
  *          1. SD Card (SPI) — circular buffer CSV logging
  *          2. UART — serial output for RF telemetry or wired dashboard
  *
  *          Logged data per frame:
  *          - Timestamp (ms), State, Speed (km/h), Distance (km)
  *          - Battery V/I/SOC, Power (W), Energy (Wh), Efficiency (Wh/km)
  *          - Motor duty, current, temperature
  *          - Pedal %, brake, fault flags
  ******************************************************************************
  */

#ifndef ESC_TELEMETRY_H
#define ESC_TELEMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ---------- Configuration -------------------------------------------------- */

/** Telemetry output rate (ms). 200ms = 5 Hz logging. */
#define TELEMETRY_INTERVAL_MS     200U

/** UART telemetry frame size (fixed-length binary or CSV line) */
#define TELEMETRY_FRAME_MAX       128U

/** SD card circular buffer — number of frames before wrap */
#define TELEMETRY_SD_BUFFER_FRAMES  18000U  /* 18000 × 200ms = 60 minutes */

/* ---------- Telemetry frame ------------------------------------------------ */

/**
  * @brief  Single telemetry data point — everything we want to log.
  */
typedef struct {
  /* Timing */
  uint32_t timestamp_ms;      /**< HAL_GetTick() */

  /* Vehicle state */
  uint8_t  vcu_state;         /**< VCU_State_t enum value */
  uint32_t fault_flags;       /**< Combined VCU + ESC fault flags */

  /* Driver inputs */
  float    pedal_percent;     /**< 0-100% */
  uint8_t  brake_active;      /**< 1 = pressed */

  /* Motor */
  uint16_t motor_duty;        /**< ESC duty command (0-9999) */
  float    motor_current_a;   /**< Filtered motor current (A, from ADC) */
  float    motor_temp_c;      /**< Motor/ESC temperature (°C) */
  uint8_t  hall_state;        /**< Current hall code (1-6) */

  /* Battery (from BMS CAN) */
  float    batt_voltage_v;    /**< Pack voltage (V) */
  float    batt_current_a;    /**< Pack current (A) */
  float    batt_soc;          /**< State of charge (%) */

  /* Energy (from esc_energy) */
  float    power_w;           /**< Instantaneous power (W) */
  float    energy_wh;         /**< Total consumed (Wh) */
  float    energy_regen_wh;   /**< Total regenerated (Wh) */
  float    speed_kmh;         /**< Current speed (km/h) */
  float    distance_km;       /**< Total distance (km) */
  float    efficiency_whkm;   /**< Wh/km */
} TelemetryFrame_t;

/* ---------- Public API ----------------------------------------------------- */

/**
  * @brief  Initialize telemetry (UART + SD card if available).
  * @note   Call after all other modules are initialized.
  */
void ESC_Telemetry_Init(void);

/**
  * @brief  Collect data and output telemetry frame.
  * @param  current_tick  HAL_GetTick() value
  * @note   Internally rate-limited to TELEMETRY_INTERVAL_MS.
  *         Reads from vcu_state, vcu_pedal, vcu_can, esc_* modules.
  */
void ESC_Telemetry_Update(uint32_t current_tick);

/**
  * @brief  Get last telemetry frame (for display/dashboard).
  * @retval Pointer to most recent TelemetryFrame_t.
  */
const TelemetryFrame_t* ESC_Telemetry_GetLastFrame(void);

/**
  * @brief  Get total frames logged this session.
  */
uint32_t ESC_Telemetry_GetFrameCount(void);

/**
  * @brief  Force flush SD card buffer (call before power-off).
  */
void ESC_Telemetry_FlushSD(void);

#ifdef __cplusplus
}
#endif

#endif /* ESC_TELEMETRY_H */
