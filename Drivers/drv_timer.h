#ifndef __DRV_TIMER_H
#define __DRV_TIMER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "board.h"
#include <stdlib.h>
#include <stdint.h>

uint32_t get_tick_count(void);


	 
void TIM4_Init(void); 
void TIM4_DeInit(void);
	 
uint32_t millis(void);
uint32_t millis_InChargeMode(void);

	 
	 
#ifdef __cplusplus
}
#endif

#endif /* __DRV_SYSTEM_H */

/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
