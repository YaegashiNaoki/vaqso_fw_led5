/**
  ******************************************************************************
  * @file    drv_eeprom.h
  * @author  Merle
  * @version V1.0.0
  * @date    4-July-2019
  * @brief   .
  ******************************************************************************
  */
#ifndef __DRV_EEPROM_H
#define __DRV_EEPROM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>


bool EEPROM_Init(void);
void EEPROM_DeInit(void);

/**
 * @brief Function for xx.
 *
 * @param[in] channel (0~4).
 * @retval true      If the cartridge in the channel was find successful.
 * @retval false     If the cartridge in the channel is none.
 */
bool EEPROM_CheckIn(uint8_t channel);

bool EEPROM_WriteBytesInChannel(uint8_t channel, uint16_t addr, uint8_t const* p_data, uint16_t len);
bool EEPROM_ReadBytesInChannel(uint8_t channel, uint16_t addr, uint8_t* p_data, uint16_t len);

bool EEPROM_WriteBytesWithMagic(uint8_t channel, uint16_t write_addr, uint8_t  const* p_in_array, uint16_t byte_count);
bool EEPROM_ReadBytesWithMagic(uint8_t channel, uint16_t read_addr, uint8_t* p_out_array, uint16_t byte_read_max, uint16_t* p_byte_count);


#ifdef __cplusplus
}
#endif

#endif /* __DRV_I2C_H */

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
