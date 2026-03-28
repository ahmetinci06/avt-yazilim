/**
  ******************************************************************************
  * @file    esc_commutation.h
  * @brief   BLDC 6-step hall-sensored commutation for custom ESC
  ******************************************************************************
  * @note    Adapted from aybukepamukcu/motor-control-esc
  *          Pin mapping updated for AVT VCU custom board schematic.
  *
  *          Hall sensors: PA0 (A), PA1 (B), PA2 (C)
  *          TIM1: CH1/CH1N = Phase U, CH2/CH2N = Phase V, CH3/CH3N = Phase W
  ******************************************************************************
  */

#ifndef ESC_COMMUTATION_H
#define ESC_COMMUTATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
  * @brief  Initialize commutation module. Call after MX_TIM1_Init().
  *         Sets all phases to safe-off state.
  */
void ESC_Commutation_Init(void);

/**
  * @brief  Read hall sensor state from PA0/PA1/PA2.
  * @retval 3-bit hall code (0-7). Valid: 1-6; Invalid: 0, 7.
  */
uint8_t ESC_Commutation_ReadHall(void);

/**
  * @brief  Perform 6-step commutation based on current hall state.
  * @param  duty  PWM compare value (0 = coast, up to TIM1 ARR = full).
  * @note   If duty == 0 or hall is invalid (0/7), all phases are turned off.
  */
void ESC_Commutation_Update(uint16_t duty);

/**
  * @brief  Force all phases off immediately (safe state).
  */
void ESC_Commutation_AllOff(void);

/**
  * @brief  Get last valid hall state read by Update().
  * @retval Last hall code (1-6), or 0 if never read.
  */
uint8_t ESC_Commutation_GetLastHall(void);

#ifdef __cplusplus
}
#endif

#endif /* ESC_COMMUTATION_H */
