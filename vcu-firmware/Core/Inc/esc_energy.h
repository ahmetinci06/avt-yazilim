/**
  ******************************************************************************
  * @file    esc_energy.h
  * @brief   Energy monitoring — real-time Wh tracking + efficiency calculation
  ******************************************************************************
  * @note    TEKNOFEST Efficiency Challenge scoring is based on Wh/km.
  *          Target: < 110 Wh/km for Elektromobil category.
  *
  *          This module integrates:
  *          - Battery voltage + current from BMS (via vcu_can)
  *          - Motor current from ESC sensors
  *          - Distance from wheel speed (hall-based RPM estimation)
  *
  *          All energy values are accumulated using trapezoidal integration
  *          for better accuracy between sample intervals.
  ******************************************************************************
  */

#ifndef ESC_ENERGY_H
#define ESC_ENERGY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ---------- Configuration -------------------------------------------------- */

/**
  * @brief  Wheel/drivetrain parameters — MUST be set for your vehicle
  * @note   Adjust these based on actual wheel diameter and gear ratio.
  *         wheel_circumference_m = π × wheel_diameter
  */
#define ENERGY_WHEEL_CIRCUMFERENCE_M   1.60f   /* ~510mm diameter wheel */
#define ENERGY_MOTOR_POLE_PAIRS        7U      /* Motor pole pairs (from bldc_motor.h) */
#define ENERGY_GEAR_RATIO              1.0f    /* Motor:wheel gear ratio (1.0 = direct) */

/**
  * @brief  Sampling and calculation intervals
  */
#define ENERGY_UPDATE_INTERVAL_MS      100U    /* Energy integration every 100ms */

/* ---------- Data types ----------------------------------------------------- */

/**
  * @brief  Energy monitoring data — all accumulated values
  */
typedef struct {
  /* Instantaneous */
  float voltage_v;          /**< Battery pack voltage (V) — from BMS CAN */
  float current_a;          /**< Battery current (A) — from BMS CAN */
  float power_w;            /**< Instantaneous power (W) = V × I */

  /* Accumulated */
  float energy_wh;          /**< Total energy consumed (Wh) */
  float energy_regen_wh;    /**< Total energy regenerated (Wh) */
  float energy_net_wh;      /**< Net energy = consumed - regen (Wh) */

  /* Distance */
  float distance_m;         /**< Total distance traveled (m) */
  float distance_km;        /**< Total distance traveled (km) */
  float speed_kmh;          /**< Current speed (km/h) */

  /* Efficiency */
  float efficiency_whkm;    /**< Real-time efficiency (Wh/km) — competition metric */
  float avg_power_w;        /**< Average power over session (W) */

  /* Session tracking */
  uint32_t session_start_tick;  /**< Session start timestamp */
  uint32_t last_update_tick;    /**< Last integration timestamp */
  uint32_t elapsed_s;           /**< Total elapsed time (s) */
} EnergyData_t;

/* ---------- Public API ----------------------------------------------------- */

/**
  * @brief  Initialize energy monitoring. Resets all accumulators.
  * @note   Call once at startup after all other modules.
  */
void ESC_Energy_Init(void);

/**
  * @brief  Update energy calculations. Call from main loop.
  * @param  current_tick  HAL_GetTick() value
  * @note   Internally rate-limited to ENERGY_UPDATE_INTERVAL_MS.
  *         Reads voltage/current from BMS CAN data and motor RPM
  *         from hall sensor timing.
  */
void ESC_Energy_Update(uint32_t current_tick);

/**
  * @brief  Get pointer to energy data structure (read-only access).
  * @retval Pointer to EnergyData_t with all current values.
  */
const EnergyData_t* ESC_Energy_GetData(void);

/**
  * @brief  Get current efficiency in Wh/km.
  * @retval Efficiency value, or 0.0f if distance < 100m.
  */
float ESC_Energy_GetEfficiency(void);

/**
  * @brief  Get current speed in km/h.
  * @retval Speed calculated from motor RPM and wheel params.
  */
float ESC_Energy_GetSpeed(void);

/**
  * @brief  Get total distance in km.
  */
float ESC_Energy_GetDistanceKm(void);

/**
  * @brief  Get total net energy consumed in Wh.
  */
float ESC_Energy_GetNetEnergyWh(void);

/**
  * @brief  Reset all energy accumulators (e.g., at race start).
  * @param  current_tick  HAL_GetTick() value for new session start.
  */
void ESC_Energy_Reset(uint32_t current_tick);

/**
  * @brief  Pack energy data into CAN telemetry frame (8 bytes).
  * @param  data  Output buffer (min 8 bytes)
  * @note   Format: [Wh/km×10 (u16)] [speed×10 (u16)] [energy×10 (u16)] [dist×10 (u16)]
  */
void ESC_Energy_PackCANFrame(uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif /* ESC_ENERGY_H */
