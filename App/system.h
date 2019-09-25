/**
  ******************************************************************************
  * @file    drv_system.h
  * @author  Merle
  * @version V1.0.0
  * @date    25-January-2017
  * @brief   .
  ******************************************************************************
  */
#ifndef __DRV_SYSTEM_H
#define __DRV_SYSTEM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "board.h"
#include <stdint.h>
#include <stdbool.h>

	 	
typedef enum {
	SYSTEM_IN_STANDBY_STATUS_NORMAL 		= 0,
	SYSTEM_IN_STANDBY_STATUS_WAKE_UP 		= 1,
	SYSTEM_IN_STANDBY_STATUS_POWER_OFF 		= 2,
	SYSTEM_IN_STANDBY_STATUS_SYSTEM_ON 		= 3,	
	SYSTEM_IN_STANDBY_STATUS_BATTERY_LOW  	= 4,
} SYSTEM_IN_STANDBY_STATUS_T;
	 
void SYSTEM_DCDC5V_PowerTurnOn(void);
void SYSTEM_DCDC5V_PowerTurnOff(void);

void SYSTEM_LED_PowerTurnOn(void);
void SYSTEM_LED_PowerTurnOff(void);

void SYSTEM_PowerOff(void);
void SYSTEM_PowerOn(void);
void SYSTEM_PowerCtlInit(void);

void SYSTEM_DevicePowerInit(void);
void SYSTEM_DevicePowerDeInit(void);

void SYSTEM_DevicePowerOn(void);
void SYSTEM_DevicePowerOff(void);	 

//void SYSTEM_ChargeInShutdown(void);
//void SYSTEM_Startup(bool enforce_upon);
//void SYSTEM_Standby(bool enforce_upon);
//void SYSTEM_Shutdown(bool is_save_env);
//bool SYSTEM_WakeUp(bool enforce_upon);
//void SYSTEM_CommonDeInit(void);
//	
void SYSTEM_DealWithReset(void);
void SYSTEM_LogicManage(void);
	
void SYSTEM_SaveParameter(bool is_save_env);
void SYSTEM_CommonDeInit(void);

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

 
#ifdef __cplusplus
}
#endif

#endif /* __DRV_SYSTEM_H */

/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
