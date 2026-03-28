/**
  ******************************************************************************
  * @file    esc_motor_control.h
  * @brief   ESC motor control — throttle-to-duty mapping + ramp limiter
  ******************************************************************************
  * @origin  Adapted from aybukepamukcu/motor-control-esc motor_control.c
  *
  * @note    Takes pedal percentage (0-100%) from vcu_pedal and converts to
  *          a duty cycle command for esc_commutation. Includes:
  *          - Throttle-to-duty linear mapping
  *          - Ramp rate limiter (smooth acceleration/deceleration)
  *          - Motor current exponential moving average filter
  *          - Immediate stop on brake/fault
  ******************************************************************************
  */

#ifndef ESC_MOTOR_CONTROL_H
#define ESC_MOTOR_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
  * @brief  Initialize motor control state. Call once at startup.
  */
void ESC_MotorControl_Init(void);

/**
  * @brief  Update motor control loop.
  * @param  pedal_percent  Pedal position 0.0-100.0% (from VCU_Pedal_GetPercent)
  * @param  brake_active   1 = brake pressed (immediate duty = 0)
  * @param  fault_active   1 = any safety fault (immediate duty = 0)
  * @note   Call periodically from main loop after sensor updates.
  */
void ESC_MotorControl_Update(float pedal_percent, uint8_t brake_active, uint8_t fault_active);

/**
  * @brief  Get current duty cycle command for commutation (0..MAX_DUTY).
  */
uint16_t ESC_MotorControl_GetDuty(void);

/**
  * @brief  Get filtered motor current value (EMA of ADC readings).
  */
float ESC_MotorControl_GetFilteredCurrent(void);

#ifdef __cplusplus
}
#endif

#endif /* ESC_MOTOR_CONTROL_H */
