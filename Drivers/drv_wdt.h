/**
  ******************************************************************************
  * @file    drv_wdt.h
  * @author  Merle
  * @version V1.0.0
  * @date    22-Feburary-2017
  * @brief   .
  ******************************************************************************
  */
  
#ifndef __DRV_WDT_H
#define __DRV_WDT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "board.h"
#include <stdint.h>
#include <stdbool.h>

	 
void WDT_Init(void);
void WDT_DeInit(void);	// do nothing
void WDT_Feed(void);


#ifdef __cplusplus
}
#endif

#endif /* __DRV_WDT_H */

/******************* (C) COPYRIGHT 2017 - 2019 All Rights Reserved *****END OF FILE****/
