/**
  ******************************************************************************
  * @file    drv_wdt.c
  * @author  Merle
  * @version V1.0.0
  * @date    22-Feburary-2017
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"



//#define DBG_WDT
#ifdef DBG_WDT
#define WDT_DMSG	RTT_Printf
#else
#define WDT_DMSG(...)
#endif


nrf_drv_wdt_channel_id m_channel_id;
static int take_time = 0;

/**
 * @brief WDT events handler.
 */
void wdt_event_handler(void)
{
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
}

void WDT_Feed(void)
{
	nrf_drv_wdt_channel_feed(m_channel_id);
}



void WDT_Init(void)
{
	//return;
	uint32_t err_code = NRF_SUCCESS;
	// <1=> Run in SLEEP, Pause in HALT 
	// <8=> Pause in SLEEP, Run in HALT 
	// <9=> Run in SLEEP and HALT 
	// <0=> Pause in SLEEP and HALT
//	return;
	//Configure WDT.
	nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
	config.behaviour = 1;
	config.reload_value = 10000; //60S  //30s // 10s
	err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
	APP_ERROR_CHECK(err_code);
	err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
	APP_ERROR_CHECK(err_code);
	nrf_drv_wdt_enable();
	
    nrf_drv_wdt_channel_feed(m_channel_id);
	
	WDT_DMSG("<WDT> Init.\r\n");
}




/******************* (C) COPYRIGHT 2017 - 2019 All Rights Reserved *****END OF FILE****/
