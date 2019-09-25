/**
  ******************************************************************************
  * @file    drv_ws2812b.h
  * @author  Merle
  * @version V1.0.0
  * @date    11-July-2019
  * @brief   .
  ******************************************************************************
  */
#ifndef __DRV_WS2812B_H
#define __DRV_WS2812B_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

//#pragma pack(1)
//typedef struct {
//    uint8_t r;
//    uint8_t g;
//    uint8_t b;
//} ws2812b_color_t;
//typedef struct {
//    uint8_t r;
//    uint8_t g;
//    uint8_t b;
//} rgb_led_t;	
//#pragma pack()	 

void WS2812B_Init(void);
void WS2812B_DeInit(void);

/**
 * @brief Function for xx.
 *
 * @param[in] pixel_index (0~17).
 * @param[in] p_color, point to the color_t data.
 */
void WS2812B_SetPixelColor(uint16_t pixel_index, color_t const* p_color);
void WS2812B_GetPixelColor(uint16_t pixel_index, color_t* p_color);

void WS2812B_FrameReset(void);

void WS2812B_Refresh(void);

void WS2812B_PixelClear(void);

void WS2812B_FillColor(color_t const* p_color, uint16_t first, uint16_t count);

void WS2812B_SetTotalPixel(uint32_t total_pixel_value, uint8_t start_index, uint8_t end_index, color_t const* p_color);

uint8_t WS2812B_GetPixelNum(void);

uint8_t WS2812B_GetPowerStatus(void);

bool WS2812B_CheckPixelColorIsAllBalck(void);

void WS2812B_PowerTurnOn(void);
void WS2812B_PowerTurnOff(void);
    
#ifdef __cplusplus
}
#endif

#endif /* __DRV_WS2812B_H */

/******************* (C) COPYRIGHT 2019 *****END OF FILE****/
