/**
  ******************************************************************************
  * @file    math_algo.h
  * @author  Merle
  * @version V1.0.0
  * @date    25-July-2019.
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#ifndef __MATH_ALGO_H_H
#define __MATH_ALGO_H_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "board.h"
#include <stdint.h>
#include <stdbool.h>

uint8_t MATH_ALGO_u5_rol_(uint8_t value, uint8_t shift);
uint8_t MATH_ALGO_u5_ror_(uint8_t value, uint8_t shift);

uint16_t MATH_ALGO_u15_rol_(uint16_t value, uint8_t shift);
uint16_t MATH_ALGO_u15_ror_(uint16_t value, uint8_t shift);

#ifdef __cplusplus
}
#endif

#endif /* __MATH_ALGO_H_H */

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
