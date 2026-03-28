/**
  ******************************************************************************
  * @file    esc_sensors.h
  * @brief   ESC sensor acquisition — ADC DMA (current sense) + TMP102 (temp)
  ******************************************************************************
  * @origin  Adapted from aybukepamukcu/motor-control-esc (Apache-2.0)
  *
  * @note    In the AVT VCU architecture, throttle/pedal input is handled by
  *          vcu_pedal module. This module provides:
  *          - Motor phase current sensing via ADC (PC1, Channel 7)
  *          - Motor temperature via TMP102 I2C sensor
  *
  *          The VCU pedal module remains the authoritative throttle source.
  ******************************************************************************
  */

#ifndef ESC_SENSORS_H
#define ESC_SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
  * @brief  Initialize ESC sensors and start ADC DMA.
  * @note   Call once after MX_ADC1_Init() and MX_I2C1_Init().
  */
void ESC_Sensors_Init(void);

/**
  * @brief  Update sensor values from ADC DMA buffer and TMP102.
  * @note   Call periodically from main loop (after Sensors init).
  */
void ESC_Sensors_Update(void);

/**
  * @brief  Get raw motor current ADC value (0..4095).
  */
uint16_t ESC_Sensors_GetCurrentRaw(void);

/**
  * @brief  Get motor temperature in Celsius from TMP102.
  * @retval Temperature in °C. Returns 0.0f on I2C error.
  */
float ESC_Sensors_GetTemperatureC(void);

#ifdef __cplusplus
}
#endif

#endif /* ESC_SENSORS_H */
