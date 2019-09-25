/**
  ******************************************************************************
  * @file    drv_adc.h
  * @author  
  * @version V1.0.0
  * @date    20-January-2017
  * @brief   .
  ******************************************************************************
  */
#ifndef __DRV_ADC_H
#define __DRV_ADC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "board.h"
#include <stdint.h>
#include <stdbool.h>
	 

void ADC_Init(void);
void ADC_DeInit(void);
	 
bool ADC_GetValue(int32_t *adc_value);
void ADC_DataUpdateOnce(void);
void ADC_TriggerStop(void);
	 
void ADC_GetBaseChargePercentageEnable(void);	
void ADC_GetBaseCharagePercentageDisable(void);
uint8_t ADC_GetBaseChargePercentageStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_ADC_H */

/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
