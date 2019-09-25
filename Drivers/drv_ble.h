/**
  ******************************************************************************
  * @file    drv_ble.h
  * @author  Merle
  * @version V1.0.0
  * @date    05-December-2016
  * @brief   .
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_BLE_H
#define __DRV_BLE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "app_fifo.h"
#include "ble_nus.h"

void BLE_SetDeviceName(unsigned char *p_name ,unsigned char size);

uint32_t BLE_ChangeAdvName(uint8_t* p_adv_name);
void BLE_Init(bool b_ble_adv_start);
void BLE_DeInit(void);

uint32_t BLE_AdvStart(void * p_erase_bonds);
uint32_t BLE_AdvStop(void);

void BLE_AdvInit(void);

bool BLE_IsActive(void);
uint32_t BLE_Disconnect(void);

uint32_t BLE_Send(uint8_t const* buf, uint16_t len);


uint32_t BLE_SendString(uint8_t const* p_string, uint16_t length);
uint32_t BLE_SendStringSpecial(uint8_t const* p_string, uint16_t length);
uint32_t BLE_SendPkg(uint8_t const* pkg, uint16_t len);

void BLE_PacketSend_GetState2(uint8_t *packet_send_is_finish);
uint8_t BLE_PacketSend_GetState(void);
void BLE_PacketSend_ClearStartState(void);

void BLE_PacketSend_Prepare(uint8_t const* pkg, uint16_t len);
uint32_t BLE_PacketSend_Loop(uint8_t *pkg_send_is_finish);
uint32_t BLE_PacketSend(uint8_t const* packet, uint16_t len, uint8_t *p_packet_is_finish);

	
uint32_t BLE_GetRxFifoLen(void);
app_fifo_t* BLE_GetRxFifoPtr(void);
	 


#ifdef __cplusplus
}
#endif

#endif /* __BLE_H */

/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
