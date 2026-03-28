/**
  ******************************************************************************
  * @file    esc_motor_control.c
  * @brief   ESC motor control — pedal-to-duty mapping with ramp limiter
  ******************************************************************************
  * @origin  Adapted from aybukepamukcu/motor-control-esc motor_control.c
  *
  * @changes - Input is now pedal_percent (0-100%) from vcu_pedal instead of
  *            raw ADC throttle. This eliminates the throttle ADC filtering
  *            (already done in vcu_pedal) and simplifies the mapping.
  *          - Current EMA filtering retained for telemetry/protection.
  *          - MAX_DUTY matched to TIM1 ARR value (9999).
  ******************************************************************************
  */

#include "esc_motor_control.h"
#include "esc_sensors.h"

/* ---------- Configuration -------------------------------------------------- */
#define MAX_DUTY          9999U   /* Must match TIM1 Auto-Reload (htim1.Init.Period) */
#define RAMP_STEP         20U    /* Duty increment per update cycle */

/* Current EMA: filtered = alpha*new + (1-alpha)*prev */
#define CURRENT_ALPHA     0.1f

/* ---------- Private state -------------------------------------------------- */
static float filtered_current;
static uint16_t duty;

/* ========== Public API ===================================================== */

void ESC_MotorControl_Init(void)
{
  filtered_current = 0.0f;
  duty = 0U;
}

void ESC_MotorControl_Update(float pedal_percent, uint8_t brake_active, uint8_t fault_active)
{
  uint16_t current_raw;
  uint32_t target_duty;

  /* 1. Update current EMA from ESC sensors */
  current_raw = ESC_Sensors_GetCurrentRaw();
  filtered_current = CURRENT_ALPHA * (float)current_raw
                     + (1.0f - CURRENT_ALPHA) * filtered_current;

  /* 2. Brake or fault: immediate stop, bypass ramp */
  if (brake_active != 0U || fault_active != 0U)
  {
    duty = 0U;
    return;
  }

  /* 3. Map pedal percentage (0-100%) to target duty (0-MAX_DUTY) */
  if (pedal_percent <= 0.0f)
  {
    target_duty = 0U;
  }
  else if (pedal_percent >= 100.0f)
  {
    target_duty = MAX_DUTY;
  }
  else
  {
    target_duty = (uint32_t)(pedal_percent * (float)MAX_DUTY / 100.0f);
  }

  /* 4. Ramp limiter — smooth duty transitions */
  if (target_duty > (uint32_t)duty)
  {
    duty += RAMP_STEP;
    if (duty > target_duty)
      duty = (uint16_t)target_duty;
  }
  else if (target_duty < (uint32_t)duty)
  {
    if (duty > RAMP_STEP)
      duty -= RAMP_STEP;
    else
      duty = 0U;

    if (duty < target_duty)
      duty = (uint16_t)target_duty;
  }
}

uint16_t ESC_MotorControl_GetDuty(void)
{
  return duty;
}

float ESC_MotorControl_GetFilteredCurrent(void)
{
  return filtered_current;
}
