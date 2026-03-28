/**
  ******************************************************************************
  * @file    esc_commutation.c
  * @brief   BLDC 6-step hall-sensored commutation — TIM1 complementary PWM
  ******************************************************************************
  * @origin  Adapted from aybukepamukcu/motor-control-esc (Apache-2.0)
  *          Original: commutation.c — Hall pins PB0-PB2, basic commutation
  *
  * @changes AVT VCU board adaptation:
  *          - Hall pins changed: PB0-2 → PA0-2 (per AVT board schematic)
  *          - Commutation table updated: proper high-side PWM + low-side ON
  *            pattern for 6-step drive with complementary dead-time
  *          - Added safe-off with MOE disable
  ******************************************************************************
  */

#include "esc_commutation.h"
#include "main.h"

/* ---------- Pin definitions (AVT custom board) ----------------------------- */
#define HALL_A_PIN    GPIO_PIN_0   /* PA0 — Hall Sensor A */
#define HALL_B_PIN    GPIO_PIN_1   /* PA1 — Hall Sensor B */
#define HALL_C_PIN    GPIO_PIN_2   /* PA2 — Hall Sensor C */
#define HALL_GPIO     GPIOA

/* ---------- External handles from CubeMX ----------------------------------- */
extern TIM_HandleTypeDef htim1;

/* ---------- Private state -------------------------------------------------- */
static uint8_t last_hall_state;

/* ---------- Helper: read 3-bit hall code ----------------------------------- */
static uint8_t ReadHallGPIO(void)
{
  uint8_t a = (HAL_GPIO_ReadPin(HALL_GPIO, HALL_A_PIN) == GPIO_PIN_SET) ? 1U : 0U;
  uint8_t b = (HAL_GPIO_ReadPin(HALL_GPIO, HALL_B_PIN) == GPIO_PIN_SET) ? 1U : 0U;
  uint8_t c = (HAL_GPIO_ReadPin(HALL_GPIO, HALL_C_PIN) == GPIO_PIN_SET) ? 1U : 0U;
  return (uint8_t)((a << 0) | (b << 1) | (c << 2));
}

/* ---------- Helper: all phases off (safe state) ---------------------------- */
static void SafeOff_AllPhases(void)
{
  /* Zero all compare registers */
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);

  /* Disable main output — forces all high/low side off via OSSR/OSSI */
  __HAL_TIM_MOE_DISABLE_UNCONDITIONALLY(&htim1);
}

/* ---------- Helper: enable a specific high-low phase pair ------------------ */
/*
 * Six-step commutation: one phase driven high (PWM), one held low (ON),
 * third phase floating. TIM1 complementary channels with dead-time handle
 * shoot-through prevention.
 *
 * The table below maps hall code → which phase is HIGH (PWM duty) and
 * which is LOW (100% on via max compare). Floating phases get CCR = 0
 * and their CCER output bits are cleared so neither high nor low drives.
 *
 * NOTE: This table must be verified on the actual motor+board combo.
 * Swap entries if motor spins the wrong direction or vibrates.
 */
static void ApplyCommutation(uint8_t hall, uint16_t duty)
{
  uint32_t ccer;

  /* Start with all outputs disabled in CCER, MOE off */
  __HAL_TIM_MOE_DISABLE_UNCONDITIONALLY(&htim1);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);

  /* Build CCER: enable only the two channels we need */
  /* Base CCER with all channel outputs disabled */
  ccer = htim1.Instance->CCER;
  ccer &= ~(TIM_CCER_CC1E | TIM_CCER_CC1NE |
             TIM_CCER_CC2E | TIM_CCER_CC2NE |
             TIM_CCER_CC3E | TIM_CCER_CC3NE);

  switch (hall)
  {
    case 1: /* U high (PWM), V low (ON) — W float */
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty);
      ccer |= TIM_CCER_CC1E;   /* U high-side PWM */
      ccer |= TIM_CCER_CC2NE;  /* V low-side ON   */
      break;

    case 2: /* W high (PWM), V low (ON) — U float */
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, duty);
      ccer |= TIM_CCER_CC3E;   /* W high-side PWM */
      ccer |= TIM_CCER_CC2NE;  /* V low-side ON   */
      break;

    case 3: /* W high (PWM), U low (ON) — V float */
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, duty);
      ccer |= TIM_CCER_CC3E;   /* W high-side PWM */
      ccer |= TIM_CCER_CC1NE;  /* U low-side ON   */
      break;

    case 4: /* V high (PWM), W low (ON) — U float */
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, duty);
      ccer |= TIM_CCER_CC2E;   /* V high-side PWM */
      ccer |= TIM_CCER_CC3NE;  /* W low-side ON   */
      break;

    case 5: /* U high (PWM), W low (ON) — V float */
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty);
      ccer |= TIM_CCER_CC1E;   /* U high-side PWM */
      ccer |= TIM_CCER_CC3NE;  /* W low-side ON   */
      break;

    case 6: /* V high (PWM), U low (ON) — W float */
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, duty);
      ccer |= TIM_CCER_CC2E;   /* V high-side PWM */
      ccer |= TIM_CCER_CC1NE;  /* U low-side ON   */
      break;

    default: /* 0 or 7 — invalid, stay off */
      return;
  }

  htim1.Instance->CCER = ccer;

  /* Re-enable main output */
  __HAL_TIM_MOE_ENABLE(&htim1);
}

/* ========== Public API ===================================================== */

void ESC_Commutation_Init(void)
{
  last_hall_state = 0U;
  SafeOff_AllPhases();
}

uint8_t ESC_Commutation_ReadHall(void)
{
  return ReadHallGPIO();
}

void ESC_Commutation_Update(uint16_t duty)
{
  uint8_t hall = ReadHallGPIO();
  last_hall_state = hall;

  if (duty == 0U || hall == 0U || hall == 7U)
  {
    SafeOff_AllPhases();
    return;
  }

  ApplyCommutation(hall, duty);
}

void ESC_Commutation_AllOff(void)
{
  SafeOff_AllPhases();
}

uint8_t ESC_Commutation_GetLastHall(void)
{
  return last_hall_state;
}
