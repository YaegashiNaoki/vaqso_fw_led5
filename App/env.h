/**
  ******************************************************************************
  * @file    env.h
  * @author  Merle
  * @version V1.0.0
  * @date    22-Feburary-2017
  * @brief   .
  ******************************************************************************
  */
  
#ifndef __ENV_H
#define __ENV_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "board.h"
#include <stdint.h>
#include <stdbool.h>
	 
void ENV_Init(void);
	 
void ENV_Load(void);
void ENV_Save(void);
void ENV_Reset(void);

	 
	 
#ifdef __cplusplus
}
#endif

#endif /* __ENV_H */

/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
