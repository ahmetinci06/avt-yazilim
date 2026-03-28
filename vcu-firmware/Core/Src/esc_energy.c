/**
  ******************************************************************************
  * @file    esc_energy.c
  * @brief   Energy monitoring — Wh tracking, speed, efficiency (Wh/km)
  ******************************************************************************
  * @note    TEKNOFEST Efficiency Challenge: scored on Wh/km.
  *          Uses trapezoidal integration for energy accumulation.
  *
  *          Data sources:
  *          - Battery V/I → BMS CAN (via vcu_can.h)
  *          - Motor RPM → hall timing (via esc_commutation.h)
  *          - Distance → RPM × wheel circumference / gear ratio
  ******************************************************************************
  */

#include "esc_energy.h"
#include "vcu_can.h"
#include "esc_commutation.h"

/* ---------- Constants ------------------------------------------------------ */
#define MS_TO_HOURS     (1.0f / 3600000.0f)   /* ms → hours for Wh calc */
#define MS_TO_SECONDS   (1.0f / 1000.0f)
#define M_TO_KM         (1.0f / 1000.0f)
#define MIN_DISTANCE_M  100.0f  /* Min distance before efficiency is valid */

/* RPM → speed conversion:
 * speed_mps = (RPM / 60) × wheel_circumference / gear_ratio
 * speed_kmh = speed_mps × 3.6
 */
#define RPM_TO_KMH  ((ENERGY_WHEEL_CIRCUMFERENCE_M / ENERGY_GEAR_RATIO) * (3.6f / 60.0f))

/* Hall period → RPM:
 * RPM = 60,000,000 / (hall_period_us × 6 × pole_pairs)
 * But we need hall_period from commutation module.
 * For now, estimate from consecutive hall reads.
 */

/* ---------- Private state -------------------------------------------------- */
static EnergyData_t energy;

/* Previous values for trapezoidal integration */
static float prev_power_w;
static float prev_speed_mps;

/* Hall-based speed tracking */
static uint8_t prev_hall_code;
static uint32_t hall_edge_tick;
static uint32_t hall_period_ms;

/* ---------- Private helpers ------------------------------------------------ */

/**
  * @brief  Estimate motor RPM from hall edge timing.
  * @retval Motor RPM (electrical), or 0 if stalled.
  */
static float EstimateRPM(uint32_t current_tick)
{
  uint8_t hall_now = ESC_Commutation_ReadHall();

  /* Detect hall state change (= one commutation step) */
  if (hall_now != prev_hall_code && hall_now >= 1U && hall_now <= 6U)
  {
    if (hall_edge_tick > 0U)
    {
      hall_period_ms = current_tick - hall_edge_tick;
    }
    hall_edge_tick = current_tick;
    prev_hall_code = hall_now;
  }

  /* Timeout: if no hall edge for 500ms, motor is stopped */
  if (hall_edge_tick > 0U && (current_tick - hall_edge_tick) > 500U)
  {
    hall_period_ms = 0U;
    return 0.0f;
  }

  if (hall_period_ms == 0U)
  {
    return 0.0f;
  }

  /* RPM = 60000 / (hall_period_ms × 6 × pole_pairs)
   * 6 hall edges per electrical revolution */
  float rpm = 60000.0f / ((float)hall_period_ms * 6.0f * (float)ENERGY_MOTOR_POLE_PAIRS);
  return rpm;
}

/* ========== Public API ===================================================== */

void ESC_Energy_Init(void)
{
  /* Zero everything */
  energy.voltage_v       = 0.0f;
  energy.current_a       = 0.0f;
  energy.power_w         = 0.0f;
  energy.energy_wh       = 0.0f;
  energy.energy_regen_wh = 0.0f;
  energy.energy_net_wh   = 0.0f;
  energy.distance_m      = 0.0f;
  energy.distance_km     = 0.0f;
  energy.speed_kmh       = 0.0f;
  energy.efficiency_whkm = 0.0f;
  energy.avg_power_w     = 0.0f;
  energy.session_start_tick = 0U;
  energy.last_update_tick   = 0U;
  energy.elapsed_s          = 0U;

  prev_power_w   = 0.0f;
  prev_speed_mps = 0.0f;
  prev_hall_code = 0U;
  hall_edge_tick = 0U;
  hall_period_ms = 0U;
}

void ESC_Energy_Update(uint32_t current_tick)
{
  /* Session start on first call */
  if (energy.session_start_tick == 0U)
  {
    energy.session_start_tick = current_tick;
    energy.last_update_tick = current_tick;
    return;
  }

  /* Rate limit: only calculate every ENERGY_UPDATE_INTERVAL_MS */
  uint32_t dt_ms = current_tick - energy.last_update_tick;
  if (dt_ms < ENERGY_UPDATE_INTERVAL_MS)
  {
    return;
  }

  /* ---- Read data sources ---- */

  /* Battery voltage & current from BMS CAN */
  BMS_Data_t *bms = VCU_CAN_GetBMSData();
  energy.voltage_v = bms->pack_voltage;
  energy.current_a = bms->pack_current;

  /* Instantaneous power (W) = V × I */
  /* Positive current = discharge (consuming), negative = regen (charging) */
  energy.power_w = energy.voltage_v * energy.current_a;

  /* ---- Energy integration (trapezoidal) ---- */
  /* ΔE = (P_prev + P_now) / 2 × Δt */
  float dt_hours = (float)dt_ms * MS_TO_HOURS;
  float delta_energy_wh = ((prev_power_w + energy.power_w) / 2.0f) * dt_hours;

  if (delta_energy_wh >= 0.0f)
  {
    energy.energy_wh += delta_energy_wh;    /* Consumption */
  }
  else
  {
    energy.energy_regen_wh += (-delta_energy_wh);  /* Regeneration */
  }
  energy.energy_net_wh = energy.energy_wh - energy.energy_regen_wh;

  prev_power_w = energy.power_w;

  /* ---- Speed & Distance ---- */
  float rpm = EstimateRPM(current_tick);
  energy.speed_kmh = rpm * RPM_TO_KMH;

  float speed_mps = energy.speed_kmh / 3.6f;
  float dt_seconds = (float)dt_ms * MS_TO_SECONDS;

  /* Distance integration (trapezoidal): Δd = (v_prev + v_now) / 2 × Δt */
  float delta_distance_m = ((prev_speed_mps + speed_mps) / 2.0f) * dt_seconds;
  energy.distance_m += delta_distance_m;
  energy.distance_km = energy.distance_m * M_TO_KM;

  prev_speed_mps = speed_mps;

  /* ---- Efficiency ---- */
  if (energy.distance_m > MIN_DISTANCE_M)
  {
    energy.efficiency_whkm = energy.energy_net_wh / energy.distance_km;
  }
  else
  {
    energy.efficiency_whkm = 0.0f;  /* Not enough distance for valid reading */
  }

  /* ---- Session stats ---- */
  energy.elapsed_s = (current_tick - energy.session_start_tick) / 1000U;

  if (energy.elapsed_s > 0U)
  {
    /* Average power = total energy / elapsed time */
    float elapsed_h = (float)energy.elapsed_s / 3600.0f;
    energy.avg_power_w = (elapsed_h > 0.0f) ? (energy.energy_net_wh / elapsed_h) : 0.0f;
  }

  energy.last_update_tick = current_tick;
}

const EnergyData_t* ESC_Energy_GetData(void)
{
  return &energy;
}

float ESC_Energy_GetEfficiency(void)
{
  return energy.efficiency_whkm;
}

float ESC_Energy_GetSpeed(void)
{
  return energy.speed_kmh;
}

float ESC_Energy_GetDistanceKm(void)
{
  return energy.distance_km;
}

float ESC_Energy_GetNetEnergyWh(void)
{
  return energy.energy_net_wh;
}

void ESC_Energy_Reset(uint32_t current_tick)
{
  ESC_Energy_Init();
  energy.session_start_tick = current_tick;
  energy.last_update_tick = current_tick;
}

void ESC_Energy_PackCANFrame(uint8_t *data)
{
  if (data == NULL) return;

  /* Pack into 8 bytes for CAN telemetry:
   * [0-1] Efficiency × 10 (Wh/km, uint16, big-endian) → 0-6553.5
   * [2-3] Speed × 10 (km/h, uint16, big-endian)       → 0-6553.5
   * [4-5] Net energy × 10 (Wh, uint16, big-endian)    → 0-6553.5
   * [6-7] Distance × 10 (km, uint16, big-endian)      → 0-6553.5
   */
  uint16_t eff  = (uint16_t)(energy.efficiency_whkm * 10.0f);
  uint16_t spd  = (uint16_t)(energy.speed_kmh * 10.0f);
  uint16_t nrg  = (uint16_t)(energy.energy_net_wh * 10.0f);
  uint16_t dist = (uint16_t)(energy.distance_km * 10.0f);

  data[0] = (uint8_t)(eff >> 8);
  data[1] = (uint8_t)(eff & 0xFF);
  data[2] = (uint8_t)(spd >> 8);
  data[3] = (uint8_t)(spd & 0xFF);
  data[4] = (uint8_t)(nrg >> 8);
  data[5] = (uint8_t)(nrg & 0xFF);
  data[6] = (uint8_t)(dist >> 8);
  data[7] = (uint8_t)(dist & 0xFF);
}
