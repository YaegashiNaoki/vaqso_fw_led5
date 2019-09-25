/**
  ******************************************************************************
  * @file    drv_uart.h
  * @author  Merle
  * @version V1.0.0
  * @date    4-July-2019
  * @brief   .
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_UART_H
#define __DRV_UART_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "app_fifo.h"
	
void UART_Init(void);
void UART_DeInit(void);

void UART_RxIrq_DeInit(void);
void UART_RxIrq_Init(void);

uint8_t UART_GetInitStatus(void);
uint8_t UART_GetActiveStatus(void);
void UART_ClearActiveStatus(void);

void UART_Put(uint8_t const* p_buffer, uint8_t len);
uint32_t UART_Get(uint8_t* p_byte);
     
uint32_t UART_FifoFlush(void);
uint32_t UART_GetRxFifoLen(void);
app_fifo_t* UART_GetRxFifoPtr(void);

void UART_PacketSend_Prepare(uint8_t const* p_send_buf, uint16_t len);
uint32_t UART_PacketSend_Loop(uint8_t* p_packet_is_finish);

uint32_t UART_PacketSend(uint8_t const* p_packet, uint16_t len, uint8_t* p_packet_is_finish);


#ifdef __cplusplus
}
#endif

#endif /* __UART_H */

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
