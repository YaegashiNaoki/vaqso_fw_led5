/**
  ******************************************************************************
  * @file    drv_i2c.c
  * @author  
  * @version V1.0.0
  * @date    11-Octobal-2016
  * @brief   .
  ******************************************************************************
  */
#ifndef __DRV_I2C_H
#define __DRV_I2C_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

typedef void (*p_I2cInitFunc_t)(void);
typedef bool (*p_I2cTxFunc_t)(uint8_t addr, uint8_t const * p_data, uint8_t length, bool no_stop);
typedef bool (*p_I2cRxFunc_t)(uint8_t addr, uint8_t const * p_data, uint8_t length);

typedef struct {
    p_I2cInitFunc_t init;
    p_I2cRxFunc_t read;
	p_I2cTxFunc_t write;
} i2c_bus_t;

void I2C2_Init(void);
void I2C2_DeInit(void);

bool I2C2_Tx(uint8_t addr, uint8_t const * p_data, uint8_t length, bool no_stop);
bool I2C2_Rx(uint8_t addr, uint8_t const * p_data, uint8_t length);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_I2C_H */

/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
