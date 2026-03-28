/**
 ******************************************************************************
 * @file           : vcu_state.h
 * @brief          : Vehicle state machine management
 * @author         : AVT Team
 ******************************************************************************
 * @attention
 *
 * State machine for VCU operation modes:
 * - OFF: System idle
 * - READY: Pre-charge complete, waiting for start
 * - DRIVE: Torque commands active
 * - FAULT: Critical failure, motor disabled
 *
 ******************************************************************************
 */

#ifndef VCU_STATE_H
#define VCU_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stddef.h> /* For NULL */
#include <stdint.h>


/* Defines -------------------------------------------------------------------*/

/**
 * @brief Fault code bit definitions
 * @note  Multiple faults can be active simultaneously (OR'd together)
 */
#define FAULT_NONE 0x00000000U
#define FAULT_PEDAL_WIRE_BREAK 0x00000001U /**< Pedal ADC too low */
#define FAULT_PEDAL_SHORT 0x00000002U      /**< Pedal ADC too high */
#define FAULT_BMS_TIMEOUT 0x00000004U      /**< No BMS message received */
#define FAULT_BMS_OVERVOLT 0x00000008U     /**< Battery overvoltage */
#define FAULT_BMS_UNDERVOLT 0x00000010U    /**< Battery undervoltage */
#define FAULT_BMS_OVERTEMP 0x00000020U     /**< Battery overtemperature */
#define FAULT_BMS_OVERCURRENT 0x00000040U  /**< Battery overcurrent */
#define FAULT_VESC_TIMEOUT 0x00000080U     /**< No VESC response */
#define FAULT_VESC_ERROR 0x00000100U       /**< VESC reported error */
#define FAULT_PRECHARGE_FAIL 0x00000200U   /**< Pre-charge timeout */

/**
 * @brief Timeout values in milliseconds
 */
#define TIMEOUT_BMS_MS 500U        /**< BMS heartbeat timeout */
#define TIMEOUT_VESC_MS 200U       /**< VESC response timeout */
#define TIMEOUT_PRECHARGE_MS 5000U /**< Pre-charge max duration */

/* Typedefs ------------------------------------------------------------------*/

/**
 * @brief Vehicle state enumeration
 */
typedef enum {
  VCU_STATE_OFF = 0, /**< System idle, no power */
  VCU_STATE_READY,   /**< Pre-charge done, waiting for start */
  VCU_STATE_DRIVE,   /**< Torque commands active */
  VCU_STATE_FAULT    /**< Critical failure, motor disabled */
} VCU_State_t;

/**
 * @brief State transition request enumeration
 */
typedef enum {
  VCU_REQUEST_NONE = 0,
  VCU_REQUEST_POWER_ON,   /**< Request OFF -> READY */
  VCU_REQUEST_START,      /**< Request READY -> DRIVE */
  VCU_REQUEST_STOP,       /**< Request DRIVE -> READY */
  VCU_REQUEST_POWER_OFF,  /**< Request ANY -> OFF */
  VCU_REQUEST_CLEAR_FAULT /**< Request FAULT -> OFF (if safe) */
} VCU_StateRequest_t;

/**
 * @brief State machine context structure
 */
typedef struct {
  VCU_State_t current;       /**< Current state */
  VCU_State_t previous;      /**< Previous state */
  uint32_t state_enter_tick; /**< Tick when entered current state */
  uint32_t fault_code;       /**< Active fault flags (OR'd) */
  uint32_t last_bms_tick;    /**< Last BMS message timestamp */
  uint32_t last_vesc_tick;   /**< Last VESC message timestamp */
  bool brake_pressed;        /**< Brake input state */
  bool start_button;         /**< Start button state */
} StateMachine_t;

/* Function Prototypes -------------------------------------------------------*/

/**
 * @brief  Initialize state machine to OFF state
 * @param  sm: Pointer to StateMachine_t structure
 * @retval None
 */
void VCU_State_Init(StateMachine_t *sm);

/**
 * @brief  Update state machine logic (call every loop iteration)
 * @param  sm: Pointer to StateMachine_t structure
 * @param  current_tick: Current system tick (HAL_GetTick())
 * @retval None
 */
void VCU_State_Update(StateMachine_t *sm, uint32_t current_tick);

/**
 * @brief  Request a state transition
 * @param  sm: Pointer to StateMachine_t structure
 * @param  request: Requested transition
 * @retval bool: true if request accepted, false if rejected
 */
bool VCU_State_Request(StateMachine_t *sm, VCU_StateRequest_t request);

/**
 * @brief  Set a fault flag
 * @param  sm: Pointer to StateMachine_t structure
 * @param  fault: Fault bit to set
 * @retval None
 */
void VCU_State_SetFault(StateMachine_t *sm, uint32_t fault);

/**
 * @brief  Clear a specific fault flag
 * @param  sm: Pointer to StateMachine_t structure
 * @param  fault: Fault bit to clear
 * @retval None
 */
void VCU_State_ClearFault(StateMachine_t *sm, uint32_t fault);

/**
 * @brief  Get current state
 * @param  sm: Pointer to StateMachine_t structure
 * @retval VCU_State_t: Current state
 */
VCU_State_t VCU_State_GetCurrent(const StateMachine_t *sm);

/**
 * @brief  Check if torque output is allowed
 * @param  sm: Pointer to StateMachine_t structure
 * @retval bool: true if in DRIVE state with no faults
 */
bool VCU_State_IsTorqueAllowed(const StateMachine_t *sm);

/**
 * @brief  Check if any fault is active
 * @param  sm: Pointer to StateMachine_t structure
 * @retval bool: true if any fault flag is set
 */
bool VCU_State_HasFault(const StateMachine_t *sm);

/**
 * @brief  Update BMS heartbeat timestamp
 * @param  sm: Pointer to StateMachine_t structure
 * @param  current_tick: Current system tick
 * @retval None
 */
void VCU_State_UpdateBMSHeartbeat(StateMachine_t *sm, uint32_t current_tick);

/**
 * @brief  Update VESC heartbeat timestamp
 * @param  sm: Pointer to StateMachine_t structure
 * @param  current_tick: Current system tick
 * @retval None
 */
void VCU_State_UpdateVESCHeartbeat(StateMachine_t *sm, uint32_t current_tick);

/**
 * @brief  Get state name as string (for debugging/telemetry)
 * @param  state: State to convert
 * @retval const char*: State name string
 */
const char *VCU_State_GetName(VCU_State_t state);

#ifdef __cplusplus
}
#endif

#endif /* VCU_STATE_H */
