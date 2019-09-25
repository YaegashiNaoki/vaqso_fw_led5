/**
  ******************************************************************************
  * @file    drv_timer.c
  * @author  
  * @version V1.0.0
  * @date    04-January-2017
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"


//#define DBG_TIMER
#ifdef DBG_TIMER
#define TIMER_DMESG			RTT_Printf
#define SNPRINTF    snprintf
#else
#define TIMER_DMESG(...)
#define SNPRINTF(...)
#endif



const nrf_drv_timer_t tim4 = NRF_DRV_TIMER_INSTANCE(4);

static volatile uint32_t sysTickUptime = 0;
static volatile uint32_t sysTickUptime_InChargeMode = 0;

/**
 * @brief Handler for timer events.(10 ms)
 */
static void tim4_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
			
			sysTickUptime++;
			//sysTickUptime_InChargeMode++;
			
//			static uint32_t key_deadline = 0;
//			if ((int32_t)(sysTickUptime_InChargeMode - key_deadline) >= 0)
//			{
//				key_deadline = sysTickUptime_InChargeMode + 3;	//30ms
				KEY_Update(KEY_Scan());
//			}
			
            //nrf_pwr_mgmt_run();
            
            break;

        default:
            //Do nothing.
            break;
    }
}


static uint8_t b_timer4_init = 0;

void TIM4_DeInit(void)
{
	if (b_timer4_init)
	{
		
		b_timer4_init = 0;
		
		nrf_drv_timer_disable(&tim4);
		nrf_drv_timer_uninit(&tim4);
		sd_nvic_ClearPendingIRQ(TIMER4_IRQn);

		TIMER_DMESG("<TIM4> DeInit.\r\n");
	}
}

/**
 * @brief Function for main application entry.
 */
void TIM4_Init(void)
{
	if (b_timer4_init == 0)
	{
		b_timer4_init = 1;

//		// Merle 20170725
//		uint32_t time_ms_1 = 20;
//		uint32_t time_ticks_1ms;
//		time_ticks_1ms = nrf_drv_timer_ms_to_ticks(&tim4, time_ms_1);
//		nrf_drv_timer_extended_compare(
//			 &tim4, NRF_TIMER_CC_CHANNEL1, time_ticks_1ms, NRF_TIMER_SHORT_COMPARE1_STOP_MASK, true);
		//
		uint32_t time_ms = 25; //25ms 10 Time(in miliseconds) between consecutive compare events.
		uint32_t time_ticks;
		uint32_t err_code = NRF_SUCCESS;

		
		//Configure TIMER_LED for generating simple light effect - leds on board will invert his state one after the other.
		nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
		timer_cfg.frequency = 4;	// 1M,5uA,  16M = 70uA.
		
		err_code = nrf_drv_timer_init(&tim4, &timer_cfg, tim4_event_handler);
		APP_ERROR_CHECK(err_code);

		time_ticks = nrf_drv_timer_ms_to_ticks(&tim4, time_ms);


		nrf_drv_timer_extended_compare(
			 &tim4, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);


		//
		nrf_drv_timer_enable(&tim4);
		
		TIMER_DMESG("<TIM4> Init.\r\n");
	}
}


// Return system uptime in milliseconds * 10
uint32_t millis(void)
{
    return sysTickUptime;
}

uint32_t millis_InChargeMode(void)
{
	return sysTickUptime_InChargeMode;
}

/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
