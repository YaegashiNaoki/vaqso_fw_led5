/**
  ******************************************************************************
  * @file    drv_bat.h
  * @author  
  * @version V1.0.0
  * @date    20-January-2017
  * @brief   .
  ******************************************************************************
  */

#ifndef __DRV_BAT_H
#define __DRV_BAT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "board.h"
#include <stdint.h>
#include <stdbool.h>


typedef enum {
	BAT_CHARGE_STATUS_DISCHARGE  = 0,
	BAT_CHARGE_STATUS_CHARGE = 1,
	BAT_CHARGE_STATUS_COMPLETE = 2,
	BAT_CHARGE_STATUS_ERROR = 3,
} BAT_CHARGE_STATUS_E; 
	 
	 
void BAT_Init(void);
void BAT_DeInit(void);

void BAT_IO_Ctl_Init(void);
void BAT_IO_Ctl_DeInit(void);

uint8_t BAT_GetUSBConnectStatus(void);

BAT_CHARGE_STATUS_E BAT_GetChargeStatus(void);
void BAT_ResetRefrence(void);
void BAT_UpdatePercentageWithDelay(void);
void BAT_PowerLowCheckParameterReset(void);

int BAT_Update(void);
bool BAT_WakeUpBatteryCheck(bool update_bat);
bool BAT_InStandBy_GetVoltage(void);

void BAT_UpdateBatInfo(void);
uint32_t BAT_CreateTimer(void);
uint32_t BAT_CalcTimerStart(uint32_t tick_ms);
uint32_t BAT_CalcTimerStop(void);
void BAT_DataCompute(void);

void BAT_LevelUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_BAT_H */

/******************* (C) COPYRIGHT 2017  *****END OF FILE****/
