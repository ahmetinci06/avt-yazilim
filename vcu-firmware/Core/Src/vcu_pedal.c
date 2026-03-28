/**
 ******************************************************************************
 * @file           : vcu_pedal.c
 * @brief          : Pedal input processing and safety validation implementation
 * @author         : AVT Team
 ******************************************************************************
 * @attention
 *
 * Implements pedal ADC reading with:
 * - Low-pass filtering for noise reduction
 * - Wire-break detection (ADC too low)
 * - Short-circuit detection (ADC too high)
 * - Configurable calibration thresholds
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "vcu_pedal.h"

/* Private defines -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static float clamp_float(float value, float min, float max);
static float map_range(float value, float in_min, float in_max, float out_min,
                       float out_max);

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initialize pedal module with default calibration values
 */
void VCU_Pedal_Init(PedalInput_t *pedal) {
  if (pedal == NULL) {
    return;
  }

  pedal->raw_value = 0;
  pedal->min_threshold = PEDAL_ADC_MIN_VALID;
  pedal->max_threshold = PEDAL_ADC_MAX_VALID;
  pedal->idle_value = PEDAL_ADC_IDLE;
  pedal->full_value = PEDAL_ADC_FULL;
  pedal->filtered_value = 0.0f;
  pedal->pedal_percent = 0.0f;
  pedal->fault_status = PEDAL_OK;
}

/**
 * @brief  Read ADC and update pedal structure with filtering and fault check
 */
PedalFault_t VCU_Pedal_Read(PedalInput_t *pedal, uint16_t adc_raw) {
  if (pedal == NULL) {
    return PEDAL_FAULT_OUT_OF_RANGE;
  }

  /* Store raw value */
  pedal->raw_value = adc_raw;

  /* Check for wire-break (open circuit - voltage drops to 0) */
  if (adc_raw < pedal->min_threshold) {
    pedal->fault_status = PEDAL_FAULT_WIRE_BREAK;
    pedal->pedal_percent = 0.0f; /* Safety: force to zero on fault */
    return pedal->fault_status;
  }

  /* Check for short-circuit (shorted to VCC - voltage goes to max) */
  if (adc_raw > pedal->max_threshold) {
    pedal->fault_status = PEDAL_FAULT_SHORT_CIRCUIT;
    pedal->pedal_percent = 0.0f; /* Safety: force to zero on fault */
    return pedal->fault_status;
  }

  /* No fault - apply low-pass filter */
  /* Formula: filtered = alpha * new + (1 - alpha) * old */
  pedal->filtered_value = (PEDAL_FILTER_ALPHA * (float)adc_raw) +
                          ((1.0f - PEDAL_FILTER_ALPHA) * pedal->filtered_value);

  /* Map filtered value to percentage */
  pedal->pedal_percent =
      map_range(pedal->filtered_value, (float)pedal->idle_value,
                (float)pedal->full_value, 0.0f, 100.0f);

  /* Clamp to valid range */
  pedal->pedal_percent = clamp_float(pedal->pedal_percent, 0.0f, 100.0f);

  pedal->fault_status = PEDAL_OK;
  return pedal->fault_status;
}

/**
 * @brief  Get filtered pedal percentage
 */
float VCU_Pedal_GetPercent(const PedalInput_t *pedal) {
  if (pedal == NULL) {
    return 0.0f;
  }

  /* Return 0 if there's an active fault */
  if (pedal->fault_status != PEDAL_OK) {
    return 0.0f;
  }

  return pedal->pedal_percent;
}

/**
 * @brief  Check if pedal has any active fault
 */
bool VCU_Pedal_HasFault(const PedalInput_t *pedal) {
  if (pedal == NULL) {
    return true; /* Null pointer is a fault condition */
  }

  return (pedal->fault_status != PEDAL_OK);
}

/**
 * @brief  Set custom calibration values
 */
void VCU_Pedal_SetCalibration(PedalInput_t *pedal, uint16_t idle,
                              uint16_t full) {
  if (pedal == NULL) {
    return;
  }

  pedal->idle_value = idle;
  pedal->full_value = full;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Clamp a float value between min and max
 */
static float clamp_float(float value, float min, float max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

/**
 * @brief  Map a value from one range to another
 * @note   Similar to Arduino map() but with float precision
 */
static float map_range(float value, float in_min, float in_max, float out_min,
                       float out_max) {
  /* Avoid division by zero */
  if ((in_max - in_min) == 0.0f) {
    return out_min;
  }

  return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
