/**
 ******************************************************************************
 * @file           : vcu_pedal.h
 * @brief          : Pedal input processing and safety validation
 * @author         : AVT Team
 ******************************************************************************
 * @attention
 *
 * Pedal ADC reading with safety interlocks for wire-break and short-circuit
 * detection. Uses hardware oversampling (16x) for noise reduction.
 *
 ******************************************************************************
 */

#ifndef VCU_PEDAL_H
#define VCU_PEDAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stddef.h> /* For NULL */
#include <stdint.h>


/* Defines -------------------------------------------------------------------*/

/**
 * @brief ADC calibration thresholds
 * @note  Tune these during calibration with actual pedal sensor
 *        Assuming 12-bit ADC (0-4095) with 3.3V reference
 */
#define PEDAL_ADC_MIN_VALID 200  /**< Below this = wire break (~0.16V) */
#define PEDAL_ADC_MAX_VALID 3800 /**< Above this = short circuit (~3.07V) */

#define PEDAL_ADC_IDLE 250  /**< Pedal released position */
#define PEDAL_ADC_FULL 3600 /**< Pedal fully pressed position */

#define PEDAL_FILTER_ALPHA 0.1f /**< Low-pass filter coefficient (0.0-1.0) */

/* Typedefs ------------------------------------------------------------------*/

/**
 * @brief Pedal fault status enumeration
 */
typedef enum {
  PEDAL_OK = 0,
  PEDAL_FAULT_WIRE_BREAK,    /**< ADC value too low - open circuit */
  PEDAL_FAULT_SHORT_CIRCUIT, /**< ADC value too high - shorted to VCC */
  PEDAL_FAULT_OUT_OF_RANGE   /**< Value outside calibrated range */
} PedalFault_t;

/**
 * @brief Pedal input data structure
 */
typedef struct {
  uint16_t raw_value;        /**< Raw ADC value (0-4095) */
  uint16_t min_threshold;    /**< Calibration: wire-break threshold */
  uint16_t max_threshold;    /**< Calibration: short-circuit threshold */
  uint16_t idle_value;       /**< Calibration: pedal released position */
  uint16_t full_value;       /**< Calibration: pedal fully pressed */
  float filtered_value;      /**< Low-pass filtered ADC value */
  float pedal_percent;       /**< Mapped pedal position 0.0 - 100.0% */
  PedalFault_t fault_status; /**< Current fault status */
} PedalInput_t;

/* Function Prototypes -------------------------------------------------------*/

/**
 * @brief  Initialize pedal module with default calibration values
 * @param  pedal: Pointer to PedalInput_t structure
 * @retval None
 */
void VCU_Pedal_Init(PedalInput_t *pedal);

/**
 * @brief  Read ADC and update pedal structure
 * @param  pedal: Pointer to PedalInput_t structure
 * @param  adc_raw: Raw ADC value from HAL_ADC_GetValue()
 * @retval PedalFault_t: Current fault status
 */
PedalFault_t VCU_Pedal_Read(PedalInput_t *pedal, uint16_t adc_raw);

/**
 * @brief  Get filtered pedal percentage
 * @param  pedal: Pointer to PedalInput_t structure
 * @retval float: Pedal position 0.0 - 100.0%
 */
float VCU_Pedal_GetPercent(const PedalInput_t *pedal);

/**
 * @brief  Check if pedal has any active fault
 * @param  pedal: Pointer to PedalInput_t structure
 * @retval bool: true if fault detected, false if OK
 */
bool VCU_Pedal_HasFault(const PedalInput_t *pedal);

/**
 * @brief  Set custom calibration values
 * @param  pedal: Pointer to PedalInput_t structure
 * @param  idle: ADC value at pedal idle position
 * @param  full: ADC value at pedal full position
 * @retval None
 */
void VCU_Pedal_SetCalibration(PedalInput_t *pedal, uint16_t idle,
                              uint16_t full);

#ifdef __cplusplus
}
#endif

#endif /* VCU_PEDAL_H */
