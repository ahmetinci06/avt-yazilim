/**
 ******************************************************************************
 * @file           : vcu_state.c
 * @brief          : Vehicle state machine implementation
 * @author         : AVT Team
 ******************************************************************************
 * @attention
 *
 * Implements the VCU state machine with:
 * - State transitions with safety validation
 * - Fault detection and handling
 * - Timeout monitoring for BMS/VESC communication
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "vcu_state.h"

/* Private defines -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* State name strings for debugging */
static const char *state_names[] = {"OFF", "READY", "DRIVE", "FAULT"};

/* Private function prototypes -----------------------------------------------*/
static void enter_state(StateMachine_t *sm, VCU_State_t new_state,
                        uint32_t tick);
static void check_timeouts(StateMachine_t *sm, uint32_t current_tick);

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initialize state machine to OFF state
 */
void VCU_State_Init(StateMachine_t *sm) {
  if (sm == NULL) {
    return;
  }

  sm->current = VCU_STATE_OFF;
  sm->previous = VCU_STATE_OFF;
  sm->state_enter_tick = 0;
  sm->fault_code = FAULT_NONE;
  sm->last_bms_tick = 0;
  sm->last_vesc_tick = 0;
  sm->brake_pressed = false;
  sm->start_button = false;
}

/**
 * @brief  Update state machine logic
 * @note   Call this every main loop iteration (100Hz recommended)
 */
void VCU_State_Update(StateMachine_t *sm, uint32_t current_tick) {
  if (sm == NULL) {
    return;
  }

  /* Check communication timeouts */
  check_timeouts(sm, current_tick);

  /* If any fault is active, transition to FAULT state */
  if (sm->fault_code != FAULT_NONE && sm->current != VCU_STATE_FAULT) {
    enter_state(sm, VCU_STATE_FAULT, current_tick);
    return;
  }

  /* State-specific logic */
  switch (sm->current) {
  case VCU_STATE_OFF:
    /* Waiting for power-on request */
    /* No automatic transitions from OFF */
    break;

  case VCU_STATE_READY:
    /*
     * In READY state, waiting for:
     * - Start button + brake released -> DRIVE
     * - Fault -> FAULT
     */
    if (sm->start_button && !sm->brake_pressed &&
        sm->fault_code == FAULT_NONE) {
      enter_state(sm, VCU_STATE_DRIVE, current_tick);
    }
    break;

  case VCU_STATE_DRIVE:
    /*
     * In DRIVE state:
     * - Brake pressed + stop request -> READY
     * - Fault -> FAULT (handled above)
     */
    /* Note: Exit from DRIVE requires explicit request via VCU_State_Request */
    break;

  case VCU_STATE_FAULT:
    /*
     * In FAULT state:
     * - All faults cleared + clear request -> OFF
     */
    /* Note: Recovery requires explicit request via VCU_State_Request */
    break;

  default:
    /* Invalid state - go to FAULT */
    sm->fault_code |= FAULT_NONE; /* Mark as unknown error */
    enter_state(sm, VCU_STATE_FAULT, current_tick);
    break;
  }
}

/**
 * @brief  Request a state transition
 */
bool VCU_State_Request(StateMachine_t *sm, VCU_StateRequest_t request) {
  if (sm == NULL) {
    return false;
  }

  uint32_t tick = sm->state_enter_tick; /* Use last known tick */

  switch (request) {
  case VCU_REQUEST_POWER_ON:
    /* OFF -> READY (if no faults) */
    if (sm->current == VCU_STATE_OFF && sm->fault_code == FAULT_NONE) {
      enter_state(sm, VCU_STATE_READY, tick);
      return true;
    }
    break;

  case VCU_REQUEST_START:
    /* READY -> DRIVE (if brake released and no faults) */
    if (sm->current == VCU_STATE_READY && !sm->brake_pressed &&
        sm->fault_code == FAULT_NONE) {
      sm->start_button = true; /* Set flag for Update() */
      return true;
    }
    break;

  case VCU_REQUEST_STOP:
    /* DRIVE -> READY (if brake pressed) */
    if (sm->current == VCU_STATE_DRIVE && sm->brake_pressed) {
      enter_state(sm, VCU_STATE_READY, tick);
      return true;
    }
    break;

  case VCU_REQUEST_POWER_OFF:
    /* ANY -> OFF (if safe) */
    if (sm->current == VCU_STATE_READY || sm->current == VCU_STATE_OFF) {
      enter_state(sm, VCU_STATE_OFF, tick);
      return true;
    }
    break;

  case VCU_REQUEST_CLEAR_FAULT:
    /* FAULT -> OFF (if all faults cleared) */
    if (sm->current == VCU_STATE_FAULT && sm->fault_code == FAULT_NONE) {
      enter_state(sm, VCU_STATE_OFF, tick);
      return true;
    }
    break;

  case VCU_REQUEST_NONE:
  default:
    break;
  }

  return false;
}

/**
 * @brief  Set a fault flag
 */
void VCU_State_SetFault(StateMachine_t *sm, uint32_t fault) {
  if (sm == NULL) {
    return;
  }

  sm->fault_code |= fault;
}

/**
 * @brief  Clear a specific fault flag
 */
void VCU_State_ClearFault(StateMachine_t *sm, uint32_t fault) {
  if (sm == NULL) {
    return;
  }

  sm->fault_code &= ~fault;
}

/**
 * @brief  Get current state
 */
VCU_State_t VCU_State_GetCurrent(const StateMachine_t *sm) {
  if (sm == NULL) {
    return VCU_STATE_FAULT;
  }

  return sm->current;
}

/**
 * @brief  Check if torque output is allowed
 */
bool VCU_State_IsTorqueAllowed(const StateMachine_t *sm) {
  if (sm == NULL) {
    return false;
  }

  /* Torque only allowed in DRIVE with no faults and brake not pressed */
  return (sm->current == VCU_STATE_DRIVE && sm->fault_code == FAULT_NONE &&
          !sm->brake_pressed);
}

/**
 * @brief  Check if any fault is active
 */
bool VCU_State_HasFault(const StateMachine_t *sm) {
  if (sm == NULL) {
    return true; /* Null pointer is a fault condition */
  }

  return (sm->fault_code != FAULT_NONE);
}

/**
 * @brief  Update BMS heartbeat timestamp
 */
void VCU_State_UpdateBMSHeartbeat(StateMachine_t *sm, uint32_t current_tick) {
  if (sm == NULL) {
    return;
  }

  sm->last_bms_tick = current_tick;

  /* Clear BMS timeout fault if it was set */
  sm->fault_code &= ~FAULT_BMS_TIMEOUT;
}

/**
 * @brief  Update VESC heartbeat timestamp
 */
void VCU_State_UpdateVESCHeartbeat(StateMachine_t *sm, uint32_t current_tick) {
  if (sm == NULL) {
    return;
  }

  sm->last_vesc_tick = current_tick;

  /* Clear VESC timeout fault if it was set */
  sm->fault_code &= ~FAULT_VESC_TIMEOUT;
}

/**
 * @brief  Get state name as string
 */
const char *VCU_State_GetName(VCU_State_t state) {
  if (state <= VCU_STATE_FAULT) {
    return state_names[state];
  }
  return "UNKNOWN";
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Enter a new state
 */
static void enter_state(StateMachine_t *sm, VCU_State_t new_state,
                        uint32_t tick) {
  sm->previous = sm->current;
  sm->current = new_state;
  sm->state_enter_tick = tick;

  /* State entry actions */
  switch (new_state) {
  case VCU_STATE_OFF:
    sm->start_button = false;
    break;

  case VCU_STATE_READY:
    sm->start_button = false;
    break;

  case VCU_STATE_DRIVE:
    /* Reset start button after transition */
    sm->start_button = false;
    break;

  case VCU_STATE_FAULT:
    /* Ensure torque is disabled */
    /* (handled externally by checking IsTorqueAllowed) */
    break;

  default:
    break;
  }
}

/**
 * @brief  Check for communication timeouts
 */
static void check_timeouts(StateMachine_t *sm, uint32_t current_tick) {
  /* Only check timeouts when not in OFF state */
  if (sm->current == VCU_STATE_OFF) {
    return;
  }

  /* BMS timeout check */
  if (sm->last_bms_tick > 0) {
    uint32_t bms_elapsed = current_tick - sm->last_bms_tick;
    if (bms_elapsed > TIMEOUT_BMS_MS) {
      sm->fault_code |= FAULT_BMS_TIMEOUT;
    }
  }

  /* VESC timeout check (only in DRIVE state) */
  if (sm->current == VCU_STATE_DRIVE && sm->last_vesc_tick > 0) {
    uint32_t vesc_elapsed = current_tick - sm->last_vesc_tick;
    if (vesc_elapsed > TIMEOUT_VESC_MS) {
      sm->fault_code |= FAULT_VESC_TIMEOUT;
    }
  }
}
