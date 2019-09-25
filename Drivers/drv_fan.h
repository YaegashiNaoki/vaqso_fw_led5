/**
  ******************************************************************************
  * @file    drv_fan.h
  * @author  Merle
  * @version V1.0.0
  * @date    18-July-2019
  * @brief   .
  ******************************************************************************
  */
#ifndef __DRV_FAN_H
#define __DRV_FAN_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>



bool FAN_Init(void);
void FAN_DeInit(void);

bool FAN_PowerOn(void);
bool FAN_PowerOff(void);


/**
 * @brief Function for xx.
 *
 * @param[in] channel (0~4).
 * @param[out] value (0~255).
 */
bool FAN_SetPwm(uint8_t channel, uint16_t value);

/**
 * @brief Function for xx.
 *
 * @param[in] channel (0~4).
 * @param[out] value (0~255).
 */
bool FAN_SetSpeed(uint8_t channel, uint8_t value);

/**
 * @brief Function for xx.
 *
 * @param[in] channel (0~4).
 * @param[out] value (0~255).
 */
bool FAN_SetFlow(uint8_t channel, uint8_t value);

/**
 * @brief Function for xx.
 *
 * @param[in] channel (0~4).
 */
bool FAN_SetPinOn(uint8_t channel);

/**
 * @brief Function for xx.
 *
 * @param[in] channel (0~4).
 */
bool FAN_SetPinOff(uint8_t channel);

/**
 * @brief Function for xx.
 *
 * @param[in] channel (0~4).
 * @param[out] if the channel of the air fan status is turn on then return 0x01, or return 0x00.
 */
uint8_t FAN_GetStatus(uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_FAN_H */

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
