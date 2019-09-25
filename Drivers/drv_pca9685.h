/**
  ******************************************************************************
  * @file    drv_pca9685.h
  * @author  Merle
  * @version V1.0.0
  * @date    28-June-2019
  * @brief   .
  ******************************************************************************
  */
#ifndef __DRV_PCA9685_H
#define __DRV_PCA9685_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "board.h"
#include <stdint.h>
#include <stdbool.h>


bool PCA9685_Init(void); 
void PCA9685_DeInit(void);
	 
bool PCA9685_Reset(void);

bool PCA9685_PowerOn(void);
bool PCA9685_PowerOff(void);

bool PCA9685_SetPWMFreq(float freq);

bool PCA9685_SetPWM(uint8_t num, uint16_t on_cnt, uint16_t off_cnt);

bool PCA9685_SetPWMValue(uint8_t num, uint16_t val, bool invert);

bool PCA9685_SetPinOff(uint8_t num);
bool PCA9685_SetPinOn(uint8_t num);

//
bool PCA9685_SetAllPWM(uint16_t on_cnt, uint16_t off_cnt);  
bool PCA9685_SetAllPWMValue(uint16_t val, bool invert);   
bool PCA9685_SetAllPinOff(void );
bool PCA9685_SetAllPinOn(void);

bool PCA9685_SetPinValue(uint32_t total_pin_value, uint8_t start_index, uint8_t end_index);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_PCA9685_H */

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
