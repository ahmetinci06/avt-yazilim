/**
  ******************************************************************************
  * @file    esc_safety.h
  * @brief   ESC-level safety — overcurrent + overtemperature monitoring
  ******************************************************************************
  * @origin  Adapted from aybukepamukcu/motor-control-esc safety.c (Apache-2.0)
  *
  * @note    Brake handling is already in vcu_state (brake GPIO interlock).
  *          This module adds ESC-specific hardware fault detection:
  *          - Motor phase overcurrent (ADC threshold)
  *          - Motor/ESC overtemperature (TMP102 threshold)
  *
  *          Faults from this module feed into vcu_state via SetFault().
  ******************************************************************************
  */

#ifndef ESC_SAFETY_H
#define ESC_SAFETY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Fault flag bits (compatible with vcu_state fault_code OR) */
#define ESC_FAULT_NONE          0x00000000U
#define ESC_FAULT_OVERCURRENT   0x00001000U  /* Motor phase overcurrent */
#define ESC_FAULT_OVERTEMP      0x00002000U  /* Motor/ESC overtemperature */

/**
  * @brief  Initialize ESC safety module.
  */
void ESC_Safety_Init(void);

/**
  * @brief  Update ESC safety checks. Call after ESC_Sensors_Update().
  * @note   Reads current + temp from esc_sensors and sets internal flags.
  */
void ESC_Safety_Update(void);

/**
  * @brief  Check if any ESC fault is active.
  * @retval 1 if fault active, 0 if OK.
  */
uint8_t ESC_Safety_IsFaultActive(void);

/**
  * @brief  Get ESC fault flags (can be OR'd into vcu_state fault_code).
  * @retval Bitmask of ESC_FAULT_xxx flags.
  */
uint32_t ESC_Safety_GetFaultFlags(void);

#ifdef __cplusplus
}
#endif

#endif /* ESC_SAFETY_H */
