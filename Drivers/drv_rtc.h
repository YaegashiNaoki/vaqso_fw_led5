/**
  ******************************************************************************
  * @file    drv_rtc.h
  * @author  Merle
  * @version V1.0.0
  * @date    11-Octobal-2016
  * @brief   .
  ******************************************************************************
  */
  
#ifndef __DRV_RTC_H
#define __DRV_RTC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
	 
void RTC2_Init(void);
void RTC2_DeInit(void);
	 
uint32_t RTC2_GetTime(void);
void RTC2_SetTime(time_t ti);

	 
uint32_t RTC2_Get125MsCounter(void);
	 
void RTC2_EnableTick(void);
void RTC2_DisableTick(void);
	 
void RTC2_EnableCompareCounter(void);
void RTC2_DisableCompareCounter(void);
	 
//extern struct tm tmTime;
uint8_t RTC2_GetCompare(void);

bool RTC_IsLeapYear(uint16_t _year);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_RTC_H */

/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
