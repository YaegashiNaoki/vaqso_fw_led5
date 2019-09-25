/**
  ******************************************************************************
  * @file    drv_cartridge.c
  * @author  Merle
  * @version V1.0.0
  * @date    17-July-2019
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "drv_cartridge.h"
#include "crc16.h"

#define DBG_CARTRIDGE
#ifdef DBG_CARTRIDGE
#define CARTRIDGE_DMSG	RTT_Printf
#else
#define CARTRIDGE_DMSG(...)
#endif

//cartridge_info_t current_cartridge_info = {0};

static uint8_t b_cartridge_Init = 0;

#define CARTRIDGE_INFO_STORE_ADDR           (0)
#define CARTRIDGE_ENABLIND_DATE_STORE_ADDR  (80)

// 0~4
bool CARTRIDGE_WriteInfo(uint8_t channel, cartridge_info_t const* cartridge_info)
{
    bool ret;
    if (b_cartridge_Init == 0)
        return false;
    
    uint8_t ch = channel + 1;
    ret = EEPROM_WriteBytesWithMagic(ch, CARTRIDGE_INFO_STORE_ADDR, (uint8_t*)cartridge_info, sizeof(cartridge_info_t) / sizeof(uint8_t));
    //CARTRIDGE_DMSG("<CARTRIDGE> write ret= %d.\r\n", ret);
    return ret;
}

// 0~4
bool CARTRIDGE_ReadInfo(uint8_t channel, cartridge_info_t* cartridge_info)
{
    bool ret;
    if (b_cartridge_Init == 0)
        return false;
    
    uint16_t read_len = 0;
    uint8_t ch = channel + 1;
    ret = EEPROM_ReadBytesWithMagic(ch, CARTRIDGE_INFO_STORE_ADDR, (uint8_t*)cartridge_info, sizeof(cartridge_info_t) / sizeof(uint8_t), &read_len);
    //CARTRIDGE_DMSG("<CARTRIDGE> read ret= %d.\r\n", ret);
    return ret;
}


// 0~4
bool CARTRIDGE_WriteEnablingDate(uint8_t channel, cartridge_enabling_date_t const* cartridge_enabling_date)
{
    bool ret;
    if (b_cartridge_Init == 0)
        return false;
    
    uint8_t ch = channel + 1;
    ret = EEPROM_WriteBytesWithMagic(ch, CARTRIDGE_ENABLIND_DATE_STORE_ADDR, (uint8_t*)cartridge_enabling_date, sizeof(cartridge_enabling_date_t) / sizeof(uint8_t));
    //CARTRIDGE_DMSG("<CARTRIDGE> write ret= %d.\r\n", ret);
    return ret;
}

// 0~4
bool CARTRIDGE_ReadEnablingDate(uint8_t channel, cartridge_enabling_date_t* cartridge_enabling_date)
{
    bool ret;
    if (b_cartridge_Init == 0)
        return false;
    
    uint16_t read_len = 0;
    uint8_t ch = channel + 1;
    ret = EEPROM_ReadBytesWithMagic(ch, CARTRIDGE_ENABLIND_DATE_STORE_ADDR, (uint8_t*)cartridge_enabling_date, sizeof(cartridge_enabling_date_t) / sizeof(uint8_t), &read_len);
    //CARTRIDGE_DMSG("<CARTRIDGE> read ret= %d.\r\n", ret);
    return ret;
}

bool CARTRIDGE_CheckIn(uint8_t channel)
{
    bool ret;
    if (b_cartridge_Init == 0)
        return false;
    
    uint8_t ch = channel + 1;
    ret = EEPROM_CheckIn(ch);
    //CARTRIDGE_DMSG("<CARTRIDGE> check ret= %d.\r\n", ret);
    return ret;
}


bool CARTRIDGE_Init(void)
{
    ct_assert(sizeof(cartridge_info_t) < (CARTRIDGE_ENABLIND_DATE_STORE_ADDR - (CARTRIDGE_INFO_STORE_ADDR + 8)));
    
    bool ret;
    if (b_cartridge_Init == 0)
	{
		b_cartridge_Init = 1;
        
        ret = EEPROM_Init();
        
        CARTRIDGE_DMSG("<CARTRIDGE> Init.\r\n");   
    }
    return ret;
}

void CARTRIDGE_DeInit(void)
{
    if (b_cartridge_Init)
	{
		b_cartridge_Init = 0;
        
        CARTRIDGE_DMSG("<CARTRIDGE> DeInit.\r\n");   
    }
}

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
