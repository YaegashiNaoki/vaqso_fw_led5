/**
  ******************************************************************************
  * @file    xTimer.c
  * @author  Merle
  * @version V1.0.0
  * @date    25-July-2019
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"

#define DBG_XTIMER
#ifdef DBG_XTIMER
#define XTIMER_DMSG	    RTT_Printf
#define SNPRINTF        snprintf
#else
#define XTIMER_DMSG(...)
#define SNPRINTF(...)
#endif

#define OSTIMER_WAIT_FOR_QUEUE              2   /**< Number of ticks to wait for the timer queue to be ready */

//static TimerHandle_t m_battery_timer;        



//static void battery_level_update_timeout_handler(TimerHandle_t xTimer)
//{
//    UNUSED_PARAMETER(xTimer);
//}



void XTIMER_Create(void)
{
    // Create timers.
    
//    m_battery_timer = xTimerCreate("BATERRY_LEVLEL_UPDATE",
//                                   1000,
//                                   pdTRUE,
//                                   NULL,
//                                   battery_level_update_timeout_handler);
    
}


void XTIMER_Start(void)
{
    
//    if (pdPASS != xTimerStart(m_battery_timer, OSTIMER_WAIT_FOR_QUEUE))
//    {
//        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
//    }

    // ...
}


void XTIMER_Init(void)
{

}



		
/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
