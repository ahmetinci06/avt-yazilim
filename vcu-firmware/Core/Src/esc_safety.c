/**
  ******************************************************************************
  * @file    esc_safety.c
  * @brief   ESC-level safety — overcurrent + overtemperature monitoring
  ******************************************************************************
  * @origin  Adapted from aybukepamukcu/motor-control-esc safety.c (Apache-2.0)
  *
  * @changes - Removed brake GPIO (handled by vcu_state)
  *          - Added ESC-specific fault flags at 0x1000+ range to avoid
  *            collision with vcu_state FAULT_xxx bits
  *          - Reads from esc_sensors instead of original sensors module
  ******************************************************************************
  */

#include "esc_safety.h"
#include "esc_sensors.h"

/* Thresholds — tune based on motor/ESC specs */
#define MOTOR_TEMP_LIMIT_C     80.0f   /* Motor overtemp threshold (°C) */
#define CURRENT_ADC_LIMIT      3000U   /* Raw ADC overcurrent threshold */

/* Internal state */
static uint32_t fault_flags;
static uint8_t  fault_active;

/* ========== Public API =================================================== */

void ESC_Safety_Init(void)
{
  fault_flags  = ESC_FAULT_NONE;
  fault_active = 0U;
}

void ESC_Safety_Update(void)
{
  fault_flags = ESC_FAULT_NONE;

  /* Check motor/ESC temperature */
  float temp_c = ESC_Sensors_GetTemperatureC();
  if (temp_c > MOTOR_TEMP_LIMIT_C)
  {
    fault_flags |= ESC_FAULT_OVERTEMP;
  }

  /* Check motor phase current */
  uint16_t current_raw = ESC_Sensors_GetCurrentRaw();
  if (current_raw > CURRENT_ADC_LIMIT)
  {
    fault_flags |= ESC_FAULT_OVERCURRENT;
  }

  fault_active = (fault_flags != ESC_FAULT_NONE) ? 1U : 0U;
}

uint8_t ESC_Safety_IsFaultActive(void)
{
  return fault_active;
}

uint32_t ESC_Safety_GetFaultFlags(void)
{
  return fault_flags;
}
