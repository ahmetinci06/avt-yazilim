/**
  ******************************************************************************
  * @file    esc_telemetry.c
  * @brief   Telemetry — data collection + UART CSV output + SD stub
  ******************************************************************************
  * @note    Collects data from all VCU/ESC modules and outputs as CSV over
  *          UART for RF telemetry or wired dashboard connection.
  *
  *          SD card logging is stubbed — implementation depends on SPI/SDIO
  *          peripheral and FatFS middleware (to be added in CubeMX).
  *
  *          UART output format (CSV, one line per frame):
  *          tick,state,pedal,brake,duty,mA,mT,bV,bA,soc,W,Wh,rWh,kmh,km,whkm,hall,fault
  ******************************************************************************
  */

#include "esc_telemetry.h"
#include "vcu_state.h"
#include "vcu_pedal.h"
#include "vcu_can.h"
#include "esc_commutation.h"
#include "esc_motor_control.h"
#include "esc_sensors.h"
#include "esc_safety.h"
#include "esc_energy.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

/* ---------- External handles ----------------------------------------------- */
extern UART_HandleTypeDef huart2;  /* USART2 — Nucleo ST-LINK VCP / RF module */

/* ---------- External state (from main.c) ----------------------------------- */
extern PedalInput_t pedal;
extern StateMachine_t state_machine;

/* ---------- Private state -------------------------------------------------- */
static TelemetryFrame_t last_frame;
static uint32_t frame_count;
static uint32_t last_telemetry_tick;

/* UART transmit buffer */
static char uart_buf[TELEMETRY_FRAME_MAX];

/* ---------- Private: collect all data into frame --------------------------- */
static void CollectFrame(TelemetryFrame_t *frame, uint32_t tick)
{
  /* Timing */
  frame->timestamp_ms = tick;

  /* VCU state */
  frame->vcu_state   = (uint8_t)VCU_State_GetCurrent(&state_machine);
  frame->fault_flags = state_machine.fault_code | ESC_Safety_GetFaultFlags();

  /* Driver inputs */
  frame->pedal_percent = VCU_Pedal_GetPercent(&pedal);
  frame->brake_active  = (uint8_t)state_machine.brake_pressed;

  /* Motor */
  frame->motor_duty      = ESC_MotorControl_GetDuty();
  frame->motor_current_a = ESC_MotorControl_GetFilteredCurrent();
  frame->motor_temp_c    = ESC_Sensors_GetTemperatureC();
  frame->hall_state      = ESC_Commutation_GetLastHall();

  /* Battery from BMS CAN */
  BMS_Data_t *bms = VCU_CAN_GetBMSData();
  frame->batt_voltage_v = bms->pack_voltage;
  frame->batt_current_a = bms->pack_current;
  frame->batt_soc       = bms->soc_percent;

  /* Energy */
  const EnergyData_t *energy = ESC_Energy_GetData();
  frame->power_w         = energy->power_w;
  frame->energy_wh       = energy->energy_wh;
  frame->energy_regen_wh = energy->energy_regen_wh;
  frame->speed_kmh       = energy->speed_kmh;
  frame->distance_km     = energy->distance_km;
  frame->efficiency_whkm = energy->efficiency_whkm;
}

/* ---------- Private: format and send UART CSV ------------------------------ */
static void SendUART(const TelemetryFrame_t *frame)
{
  int len = snprintf(uart_buf, sizeof(uart_buf),
    "%lu,%u,%.1f,%u,%u,%.1f,%.1f,%.1f,%.1f,%.0f,%.1f,%.2f,%.2f,%.1f,%.3f,%.1f,%u,0x%08lX\r\n",
    (unsigned long)frame->timestamp_ms,
    (unsigned)frame->vcu_state,
    frame->pedal_percent,
    (unsigned)frame->brake_active,
    (unsigned)frame->motor_duty,
    frame->motor_current_a,
    frame->motor_temp_c,
    frame->batt_voltage_v,
    frame->batt_current_a,
    frame->batt_soc,
    frame->power_w,
    frame->energy_wh,
    frame->energy_regen_wh,
    frame->speed_kmh,
    frame->distance_km,
    frame->efficiency_whkm,
    (unsigned)frame->hall_state,
    (unsigned long)frame->fault_flags
  );

  if (len > 0 && len < (int)sizeof(uart_buf))
  {
    /* Non-blocking UART transmit — if busy, frame is dropped (acceptable) */
    (void)HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, (uint16_t)len, 5);
  }
}

/* ---------- Private: SD card stub ------------------------------------------ */
/*
 * TODO: Implement when SPI/SDIO + FatFS is configured in CubeMX.
 *
 * Plan:
 * 1. MX_SPI2_Init() or MX_SDMMC1_Init() in CubeMX
 * 2. Add FatFS middleware
 * 3. On init: f_open("telemetry.csv", FA_WRITE | FA_CREATE_ALWAYS)
 * 4. Write CSV header
 * 5. On each frame: f_write() same CSV line
 * 6. On flush/poweroff: f_sync() + f_close()
 * 7. Circular: check file size, wrap at TELEMETRY_SD_BUFFER_FRAMES
 */
static void WriteSD(const TelemetryFrame_t *frame)
{
  (void)frame;  /* Not yet implemented */
}

/* ========== Public API ===================================================== */

void ESC_Telemetry_Init(void)
{
  memset(&last_frame, 0, sizeof(last_frame));
  frame_count = 0U;
  last_telemetry_tick = 0U;

  /* Send CSV header over UART */
  const char *header = "tick,state,pedal,brake,duty,mA,mT,bV,bA,soc,W,Wh,rWh,kmh,km,whkm,hall,fault\r\n";
  (void)HAL_UART_Transmit(&huart2, (uint8_t *)header, (uint16_t)strlen(header), 10);
}

void ESC_Telemetry_Update(uint32_t current_tick)
{
  /* Rate limit */
  if (last_telemetry_tick > 0U &&
      (current_tick - last_telemetry_tick) < TELEMETRY_INTERVAL_MS)
  {
    return;
  }

  /* Collect data */
  CollectFrame(&last_frame, current_tick);

  /* Output */
  SendUART(&last_frame);
  WriteSD(&last_frame);

  frame_count++;
  last_telemetry_tick = current_tick;
}

const TelemetryFrame_t* ESC_Telemetry_GetLastFrame(void)
{
  return &last_frame;
}

uint32_t ESC_Telemetry_GetFrameCount(void)
{
  return frame_count;
}

void ESC_Telemetry_FlushSD(void)
{
  /* TODO: f_sync() + f_close() when FatFS is available */
}
