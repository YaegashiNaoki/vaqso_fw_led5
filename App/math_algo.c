/**
  ******************************************************************************
  * @file    math_algo.c
  * @author  Merle
  * @version V1.0.0
  * @date    25-July-2019.
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"

#define DBG_MATH_ALGO
#ifdef DBG_MATH_ALGO
#define MATH_ALGO_DMSG	RTT_Printf
#else
#define MATH_ALGO_DMSG(...)
#endif

uint8_t MATH_ALGO_u5_rol_(uint8_t value, uint8_t shift)
{
    shift = shift % 5;
    uint8_t temp =  (value >> (5 - shift))|(value << shift);
    temp &= 0x1f;
    return temp;
}

uint8_t MATH_ALGO_u5_ror_(uint8_t value, uint8_t shift)
{
    shift = shift % 5;
    uint8_t temp = (value << (5 - shift))|(value >> shift);
    temp &= 0x1f;
    return temp;
}

uint16_t MATH_ALGO_u15_rol_(uint16_t value, uint8_t shift)
{
    shift = shift % 15;
    uint16_t temp =  (value >> (15 - shift))|(value << shift);
    temp &= 0x7fff;
    return temp;
}

uint16_t MATH_ALGO_u15_ror_(uint16_t value, uint8_t shift)
{
    shift = shift % 15;
    uint16_t temp = (value << (15 - shift))|(value >> shift);
    temp &= 0x7fff;
    return temp;
}


/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
