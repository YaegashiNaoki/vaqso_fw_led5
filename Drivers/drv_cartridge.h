/**
  ******************************************************************************
  * @file    drv_cartridge.h
  * @author  Merle
  * @version V1.0.0
  * @date    17-July-2019
  * @brief   .
  ******************************************************************************
  */
#ifndef __DRV_CARTRIDGE_H
#define __DRV_CARTRIDGE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#pragma pack(1)
typedef struct date_time_t {
	uint8_t     year;
	uint8_t     mon;
	uint8_t     day;
	uint8_t     hour;
	uint8_t     min;
	uint8_t     sec;
    uint16_t    msec;   // milliissecond.
} date_time_t;

typedef struct date_time2_t {
	uint8_t     year;
	uint8_t     mon;
	uint8_t     day;
	uint8_t     hour;
	uint8_t     min;
	uint8_t     sec;
} date_time2_t;

typedef struct cartridge_info_t {
    uint16_t        id;                     /**< Cartridge ID. */
	date_time_t     dateOfManufacture;      /**< Cartridge date of manufacture.  */
    uint8_t         serialNumber;           /**< Cartridge Manufacture serial number.  */
    //
    uint16_t        expiryDays;             /**< Expiry days that can be used after enabling. (unit= 1day). */
} cartridge_info_t;

typedef struct cartridge_enabling_date_t {
	date_time2_t   enablingDate;           /**< Cartridge .  */
} cartridge_enabling_date_t;

#pragma pack()

bool CARTRIDGE_Init(void);
void CARTRIDGE_DeInit(void);


/**
 * @brief Function for xx.
 *
 * @param[in] channel (0~4).
 * @retval true      If the cartridge in the channel was find successful.
 * @retval false     If the cartridge in the channel is none.
 */
bool CARTRIDGE_CheckIn(uint8_t channel);

bool CARTRIDGE_WriteInfo(uint8_t channel, cartridge_info_t const* cartridge_info);
bool CARTRIDGE_ReadInfo(uint8_t channel, cartridge_info_t* cartridge_info);

// 0~4
bool CARTRIDGE_WriteEnablingDate(uint8_t channel, cartridge_enabling_date_t const* cartridge_enabling_date);
bool CARTRIDGE_ReadEnablingDate(uint8_t channel, cartridge_enabling_date_t* cartridge_enabling_date);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_CARTRIDGE_H */

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
