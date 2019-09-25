/**
  ******************************************************************************
  * @file    drv_rtc.c
  * @author  Merle
  * @version V1.0.0
  * @date    11-Octobal-2016
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"



//#define DBG_RTC2
#ifdef DBG_RTC2
#define RTC2_DMSG	RTT_Printf
#else
#define RTC2_DMSG(...)
#endif


const nrf_drv_rtc_t rtc2 = NRF_DRV_RTC_INSTANCE(2); /**< Declaring an instance of nrf_drv_rtc for RTC2. */

#define COMPARE_COUNTERTIME  (1UL)//(40UL) //(30UL) //(2UL)                                        /**< Get Compare event COMPARE_TIME seconds after the counter starts from 0. */

static uint32_t RTC_SetCompareCounterTime(uint32_t sec)
{
	uint32_t err_code = NRF_SUCCESS;
	uint32_t compare_counter_time = (NRF_RTC2->COUNTER + sec * 8) & 0x00FFFFFF;	// % 16777215;
	err_code = nrf_drv_rtc_cc_set(&rtc2, 0, compare_counter_time, true);
	return err_code;
}

static uint32_t RTC2_ReferenceTime = 0;
static uint32_t RTC2_TimeOfUpdate = 0;
static uint32_t RTC2_CounterSum = 0;

uint32_t RTC2_GetTime(void)
{
	// deal when the rtc overflow.
//	if (NRF_RTC2->COUNTER < RTC2_ReferenceTime)
//	{
//		RTC2_CounterSum += NRF_RTC2->COUNTER + (16777216 - RTC2_ReferenceTime);
//		RTC2_ReferenceTime = NRF_RTC2->COUNTER;
//	}
//	
	uint32_t time_shift = ((RTC2_CounterSum + (NRF_RTC2->COUNTER - RTC2_ReferenceTime)) / 8);		//((NRF_RTC2->COUNTER / 8) - RTC2_ReferenceTime); //
	uint32_t RTC2_CounterValue = RTC2_TimeOfUpdate + time_shift;
    return RTC2_CounterValue;
}


uint32_t RTC2_Get125MsCounter(void)
{
    volatile uint32_t counter = NRF_RTC2->COUNTER;
    return counter;
}



void RTC2_SetTime(time_t ti)
{
	/**  BUG,RTC超过24天后溢出,这时RTC2_CounterSum=24天左右，
		时间如果再次被设定(手动或者蓝牙更新)时间会不正常，
		Merle20170830修正
	**/
	RTC2_CounterSum = 0;	//add 20170830
	//
	RTC2_ReferenceTime = NRF_RTC2->COUNTER;				//	(NRF_RTC2->COUNTER / 8); //sec.
	RTC2_TimeOfUpdate = ti;
	RTC2_DMSG("<RTC> RTC2_ReferenceTime=%u.\r\n", RTC2_ReferenceTime);
		
	MW_ParameterUpdate();
}

static uint8_t b_rtc2_compare_counter = 0;

static uint8_t b_compareCounterInit = 0;

void RTC2_EnableCompareCounter(void)
{
	if (b_compareCounterInit == 0)
	{
		b_compareCounterInit = 1;
	
		b_rtc2_compare_counter = 1;
		RTC_SetCompareCounterTime(COMPARE_COUNTERTIME);
		RTC2_GetTime();
	}
}


void RTC2_DisableCompareCounter(void)
{
	if (b_compareCounterInit)
	{
		b_compareCounterInit = 0;
		
		b_rtc2_compare_counter = 0;
	}
}

uint8_t RTC2_GetCompare(void)
{
    return b_rtc2_compare_counter;
}


static void rtc2_event_handler(nrf_drv_rtc_int_type_t int_type)
{
	//uint32_t tick = NRF_RTC2->COUNTER;
	switch (int_type)
	{
		case NRF_DRV_RTC_INT_COMPARE0:
		{
			if (b_rtc2_compare_counter)
			{
				RTC_SetCompareCounterTime(COMPARE_COUNTERTIME);
				RTC2_GetTime();
				
				//RTC2_DMSG("<SYSTEM> RTC_INT_COMPARE0\r\n\r\n");
			}
		}
			break;

		case NRF_DRV_RTC_INT_OVERFLOW:
			{
				RTC2_CounterSum += NRF_RTC2->COUNTER + (16777216 - RTC2_ReferenceTime);
				RTC2_ReferenceTime = NRF_RTC2->COUNTER;
				
				uint32_t counter = NRF_RTC2->COUNTER;
				RTC2_DMSG("<RTC> RTC_INT_OVERFLOW.. counter=%u, T=%u.\r\n\r\n",counter, RTC2_GetTime());
			}
			break;	
//		case NRF_DRV_RTC_INT_TICK:
//			break;
		default:
			break;
	}
//	if (int_type == NRF_DRV_RTC_INT_COMPARE0)
//    {
//		
//    }
//    else if (int_type == NRF_DRV_RTC_INT_TICK)
//    {
//	
//    }
//	else if (int_type == NRF_DRV_RTC_INT_OVERFLOW)
//	{
//	
//	
//	}
//	else
//	{
//	
//	}
}



static uint8_t b_rtc2_init = 0;


static void RTC_ClockStart(void)
{
    // Starting the  LFCLK XTAL oscillator. 
	uint32_t err_code = NRF_SUCCESS;
    
    bool ret = nrf_drv_clock_lfclk_is_running();
    RTC2_DMSG("<RTC> rtc is runing? ret1=%u \n", ret);
    if (!ret)
    {
        err_code = nrf_drv_clock_init();
        APP_ERROR_CHECK(err_code);
    }
   
    //ret = nrf_drv_clock_lfclk_is_running();
    //RTC2_DMSG("<RTC> rtc is runing? ret2=%u , ct=%u\n", ret,NRF_RTC2->COUNTER);
    
    nrf_drv_clock_lfclk_request(NULL);
     
    //ret = nrf_drv_clock_lfclk_is_running();
    //RTC2_DMSG("<RTC> rtc is runing? ret3=%u  ct=%u\n", ret,NRF_RTC2->COUNTER);
}

void RTC2_Init(void)
{
	if (b_rtc2_init == 0)
	{
		b_rtc2_init = 1;
			
		uint32_t err_code = NRF_SUCCESS;
		
		RTC_ClockStart();
		
		//Initialize RTC instance
		nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;

		config.prescaler = 	((uint32_t)(((double)32768 * (double)125000) / 1E6 + 0.5) - 1);
        //config.prescaler = 	((uint32_t)(((double)32768 * (double)62500) / 1E6 + 0.5) - 1);
		err_code = nrf_drv_rtc_init(&rtc2, &config, rtc2_event_handler);
		
		// Power on RTC instance
		nrf_drv_rtc_enable(&rtc2);
		
		
		// enable overflow
		nrf_drv_rtc_overflow_enable(&rtc2, true);
		
        delay_ms(200);
		//delay_ms(1000); // ????? //
//		struct tm t;
//		t.tm_year =  2019 - 1900;
//		t.tm_mon = 5 - 1;
//		t.tm_mday = 1;
//		t.tm_hour = 0;
//		t.tm_min = 0;
//		t.tm_sec = 0;

//		RTC2_SetTime(mktime(&t));	
        
        RTC2_SetTime(env.cfg.laseSaveEnvTime + 5);

        RTC2_DMSG("<RTC> Init.\r\n");
	}
}



void RTC2_DeInit(void)
{
	if (b_rtc2_init)
	{
		b_rtc2_init = 0;
		RTC2_DisableCompareCounter();
		nrf_drv_rtc_disable(&rtc2);
		nrf_drv_rtc_tick_disable(&rtc2);
		nrf_drv_rtc_uninit(&rtc2);
        
        RTC2_DMSG("<RTC> DeInit.\r\n");
	}
}


void RTC2_DisableTick(void)
{
	nrf_drv_rtc_cc_disable(&rtc2, 0);
	nrf_drv_rtc_disable(&rtc2);
	nrf_drv_rtc_uninit(&rtc2); 
}

void RTC2_EnableTick(void)
{
	//Initialize RTC instance
	nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
	
	config.prescaler = 	((uint32_t)(((double)32768*(double)125000)/1E6+0.5)-1);
	nrf_drv_rtc_init(&rtc2, &config, rtc2_event_handler);
	
	nrf_drv_rtc_overflow_enable(&rtc2, true);
	
	//Power on RTC instance
	nrf_drv_rtc_enable(&rtc2);
	
}


//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31

/**
  * @brief  Check year is the leap-year or not.
  * @param  _year: the intput year.
  * 	
  * @retval .true,leap-year; false,not leap-year
  */
bool RTC2_IsLeapYear(uint16_t _year)
{			  
	// 必须能被4整除
	if ((_year % 4) == 0) 
    { 
		if ((_year % 100) == 0) 
        { 
								
            // 如果以00结尾,还要能被400整除 	 
			if ((_year % 400) == 0)
            {
				 return true;  
            }
			else 
            {
				return false;   
            }
		} 
        else 
        {
			return true;  
		} 
	} 
    else 
    {
		return false;	
	}
	
}

/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
