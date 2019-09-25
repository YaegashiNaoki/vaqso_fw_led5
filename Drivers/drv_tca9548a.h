/**
  ******************************************************************************
  * @file    drv_tca9548a.h
  * @author  Merle
  * @version V1.0.0
  * @date    5-July-2019
  * @brief   .
  ******************************************************************************
  */
#ifndef __DRV_TCA9548A_H
#define __DRV_TCA9548A_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "board.h"
#include <stdint.h>
#include <stdbool.h>
	 
	 
bool TCA9548A_Init(void); 
void TCA9548A_DeInit(void);
	 
bool TCA9548A_Reset(void);

bool TCA9548A_ChannelGet(uint8_t* channel);
bool TCA9548A_ChannelSelect(uint8_t channel);
        

#ifdef __cplusplus
}
#endif

#endif /* __DRV_TCA9685A_H */

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
