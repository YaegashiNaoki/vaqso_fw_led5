/**
  ******************************************************************************
  * @file    color.h
  * @author  Merle
  * @version V1.0.0
  * @date    18-July-2019
  * @brief   .
  ******************************************************************************
  */
#ifndef __COLOR_H
#define __COLOR_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#pragma pack(1)
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ws2812b_color_t;

//typedef struct {
//    uint8_t r;
//    uint8_t g;
//    uint8_t b;
//    union {
//        uint8_t red;
//        uint8_t green;
//        uint8_t blue;
//    } param;
//} color_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;    
} rgb_t;

#pragma pack()


#ifdef __cplusplus
}
#endif

#endif /* __COLOR_H */

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
