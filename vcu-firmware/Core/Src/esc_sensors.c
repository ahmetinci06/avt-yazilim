/**
  ******************************************************************************
  * @file    esc_sensors.c
  * @brief   ESC sensor acquisition — ADC DMA (current) + TMP102 I2C (temp)
  ******************************************************************************
  * @origin  Adapted from aybukepamukcu/motor-control-esc sensors.c (Apache-2.0)
  *
  * @changes - Removed throttle ADC channel (handled by vcu_pedal)
  *          - Single ADC channel: motor current on PC1 (ADC1_IN7)
  *          - TMP102 temperature reading kept as-is
  ******************************************************************************
  */

#include "esc_sensors.h"
#include "main.h"

/* External handles from CubeMX -------------------------------------------*/
extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;

/* TMP102 I2C defines ------------------------------------------------------*/
#define TMP102_I2C_ADDR   (0x48U << 1)  /* 7-bit 0x48 → 8-bit for HAL */
#define TMP102_TEMP_REG   0x00U

/* ADC DMA buffer — single channel (motor current) */
static uint16_t adc_current_buf[1];

/* Cached values updated by ESC_Sensors_Update() */
static uint16_t current_raw;
static float temperature_c;

/* ---------- Private: TMP102 read ---------------------------------------- */
static HAL_StatusTypeDef TMP102_ReadTemperature(float *temp_c)
{
  uint8_t reg = TMP102_TEMP_REG;
  uint8_t data[2];
  HAL_StatusTypeDef status;

  if (temp_c == NULL)
    return HAL_ERROR;

  status = HAL_I2C_Master_Transmit(&hi2c1, TMP102_I2C_ADDR, &reg, 1, 50);
  if (status != HAL_OK)
    return status;

  status = HAL_I2C_Master_Receive(&hi2c1, TMP102_I2C_ADDR, data, 2, 50);
  if (status != HAL_OK)
    return status;

  int16_t raw = (int16_t)((data[0] << 8) | data[1]);
  *temp_c = (float)(raw >> 4) * 0.0625f;
  return HAL_OK;
}

/* ========== Public API =================================================== */

void ESC_Sensors_Init(void)
{
  current_raw   = 0U;
  temperature_c = 0.0f;

  /* Start ADC DMA — continuous single-channel conversion for motor current.
   * NOTE: If vcu_pedal already starts ADC for throttle on a different rank,
   * this may need to be unified into a shared multi-channel DMA buffer.
   * For now we assume separate ADC init or that CubeMX is configured with
   * both channels in scan mode.
   */
  (void)HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_current_buf, 1);
}

void ESC_Sensors_Update(void)
{
  /* Copy from DMA buffer */
  current_raw = adc_current_buf[0];

  /* Read TMP102 temperature (non-blocking would be better for production) */
  (void)TMP102_ReadTemperature(&temperature_c);
}

uint16_t ESC_Sensors_GetCurrentRaw(void)
{
  return current_raw;
}

float ESC_Sensors_GetTemperatureC(void)
{
  return temperature_c;
}
