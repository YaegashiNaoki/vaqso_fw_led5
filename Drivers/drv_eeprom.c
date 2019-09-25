/**
  ******************************************************************************
  * @file    drv_eeprom.c
  * @author  Merle
  * @version V1.0.0
  * @date    8-July-2019
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "crc16.h"

#define DBG_EEPROM
#ifdef DBG_EEPROM
#define EEPROM_DMSG	RTT_Printf
#else
#define EEPROM_DMSG(...)
#endif


#define EEPROM_SLAVE_ADDR		(0x50)      // Software_iic_address


#define EEPROM_AT24C02 
//#define EEPROM_AT24C128

#ifdef EEPROM_AT24C02
    #define EEPROM_PAGE_SIZE    (8U)       /** Page size **/
    #define EEPROM_SIZE         (256U)     /** Total capacity **/
    #define EEPROM_ADDR_BYTES   (1U)       /** Slave address num of bytes usage **/
#endif

#ifdef EEPROM_AT24C128
    #define EEPROM_PAGE_SIZE    (64U)        /** Page size **/
    #define EEPROM_SIZE         (16*1024U)   /** Total capacity **/
    #define EEPROM_ADDR_BYTES   (2U)         /** Slave address num of bytes usage **/
#endif


static i2c_bus_t i2c_bus = {&I2C2_Init, &I2C2_Rx, &I2C2_Tx};

static uint8_t b_eeprom_Init = 0;
static SemaphoreHandle_t xSemaphore;

static bool EEPROM_WriteBytes(uint16_t addr, uint8_t const* p_data, uint16_t len)
{
    bool ret = false;
    
    //const uint16_t write_len = len;
    
    uint8_t write_buf[2 + EEPROM_PAGE_SIZE];
    
    if (addr >= EEPROM_SIZE)    
        return false;
    const uint16_t write_len = ((addr + len) <= EEPROM_SIZE) ? len : (EEPROM_SIZE - addr);
    
    if (EEPROM_ADDR_BYTES == 1)
    {
        // less or equa 24C02
        
        uint8_t write_addr8 = 0x00ff & addr;
        
        for (uint16_t i = 0; i < write_len; i = i)
        {
            write_addr8 = addr + i;
            
            // in every page first address.
            if (((write_addr8 & (EEPROM_PAGE_SIZE - 1)) == 0) && ((i + EEPROM_PAGE_SIZE) <= write_len))
            { 
                write_buf[0] = write_addr8;
                memcpy(&write_buf[1], &p_data[i], EEPROM_PAGE_SIZE);
                ret = i2c_bus.write(EEPROM_SLAVE_ADDR, write_buf, EEPROM_PAGE_SIZE + 1, false); // stop
                i += EEPROM_PAGE_SIZE;
            }
            else
            {
                write_buf[0] = write_addr8;
                write_buf[1] = p_data[i];
                ret = i2c_bus.write(EEPROM_SLAVE_ADDR, write_buf, 2, false); // stop
                i += 1;
            }
            
            //delay_ms(10);
            uint8_t read_addr8 = 0;
            uint16_t try_cnt = 0;
            for ( try_cnt = 0; try_cnt < 1000; try_cnt++)
            {
                ret = i2c_bus.write(EEPROM_SLAVE_ADDR, &read_addr8, 1, true); // no stop
                if (ret == true)
                {
                    break;
                }
            }
            
            if (ret == false)
            {
                return false;
            }
        }
        
    }
    else
    {
        // more than 24C02
        
        uint8_t write_addr16 = addr;
        //uint8_t write_addr[2];
        
        for (uint16_t i = 0; i < write_len; i = i)
        {
            write_addr16 = addr + i;
            
            // in every page first address.
            if (((write_addr16 & (EEPROM_PAGE_SIZE - 1)) == 0) && ((i + EEPROM_PAGE_SIZE) <= write_len))
            {
                write_buf[0] = write_addr16 >> 8;
                write_buf[1] = write_addr16 & 0xff;
                memcpy(&write_buf[2], &p_data[i], EEPROM_PAGE_SIZE);
                ret = i2c_bus.write(EEPROM_SLAVE_ADDR, write_buf, EEPROM_PAGE_SIZE + 2, false); // stop
                
                i += EEPROM_PAGE_SIZE;
            }
            else
            {
                write_buf[0] = write_addr16 >> 8;
                write_buf[1] = write_addr16 & 0xff;
                write_buf[2] = p_data[i];
                ret = i2c_bus.write(EEPROM_SLAVE_ADDR, write_buf, 3, false); // stop
                
                i += 1;
            }
            
            //delay_ms(10);
            uint8_t read_addr8[2] = {0, 0};
            uint16_t try_cnt = 0;
            for ( try_cnt = 0; try_cnt < 1000; try_cnt++)
            {
                ret = i2c_bus.write(EEPROM_SLAVE_ADDR, read_addr8, 2, true); // no stop
                if (ret == true)
                {
                    break;
                }
            }
            
            if (ret == false)
            {
                return false;
            }
        }
    }
    
    return ret;
}

static bool EEPROM_ReadBytes(uint16_t addr, uint8_t* p_data, uint16_t len)
{
    bool ret = false;
    
    if (addr >= EEPROM_SIZE)    
            return false;
    
    const uint16_t read_len = ((addr + len) <= EEPROM_SIZE) ? len : (EEPROM_SIZE - addr);
    
    if (EEPROM_ADDR_BYTES == 1)
    {
        // less or equa 24C02
        
        uint8_t read_addr8 = 0x00ff & addr;
        
        //const uint16_t read_len = (len > EEPROM_SIZE) ? EEPROM_SIZE : len; // limit size is 8bits = 0xff.

        uint16_t read_255_cnt = read_len / 255; // nRF52832 iic API is limit read max len is 255(uint8_t);
        uint8_t read_remain_len = read_len % 255;
          
        for (uint16_t i = 0; i < read_255_cnt; i++)
        {
            ret = i2c_bus.write(EEPROM_SLAVE_ADDR, &read_addr8, 1, true); // no stop
            ret = i2c_bus.read(EEPROM_SLAVE_ADDR, p_data, 0xff); 
            read_addr8 += 0xff;
            p_data += 0xff;
        }
        
        if (read_remain_len)
        {
            ret = i2c_bus.write(EEPROM_SLAVE_ADDR, &read_addr8, 1, true); // no stop
            ret = i2c_bus.read(EEPROM_SLAVE_ADDR, p_data, read_remain_len);
        }
    }
    else
    {
        // more than 24C02
        
        uint8_t read_addr16 = addr;
        uint8_t read_addr8[2];
        
        uint16_t read_255_cnt = read_len / 255;  // nRF52832 iic API is limit read max len is 255(uint8_t);
        uint8_t read_remain_len = read_len % 255;
        
        for (uint16_t i = 0; i < read_255_cnt; i++)
        {
            read_addr8[0] = read_addr16 >> 8;
            read_addr8[1] = read_addr16 & 0xff;
            
            ret = i2c_bus.write(EEPROM_SLAVE_ADDR, read_addr8, 2, true); // no stop
            ret = i2c_bus.read(EEPROM_SLAVE_ADDR, p_data, 0xff);
            read_addr16 += 0xff;
            p_data += 0xff;
        }
        
        if (read_remain_len)
        {        
            read_addr8[0] = read_addr16 >> 8;
            read_addr8[1] = read_addr16 & 0xff;
            
            ret = i2c_bus.write(EEPROM_SLAVE_ADDR, read_addr8, 2, true); // no stop
            ret = i2c_bus.read(EEPROM_SLAVE_ADDR, p_data, read_remain_len);
        }
    }
    return ret;
}

bool EEPROM_ReadBytesInChannel(uint8_t channel, uint16_t addr, uint8_t* p_data, uint16_t len)
{
    bool ret;
    
    ret = TCA9548A_ChannelSelect(channel);
    ret = EEPROM_ReadBytes(addr, p_data, len);
    
    return ret;
}


bool EEPROM_WriteBytesInChannel(uint8_t channel, uint16_t addr, uint8_t const* p_data, uint16_t len)
{
    bool ret;
    
    ret = TCA9548A_ChannelSelect(channel);
    ret = EEPROM_WriteBytes(addr, p_data, len);
    
    return ret;
}

uint8_t sum_calc(uint8_t const* buf, uint32_t len)
{
    uint8_t verifySum = 0;
    
    while (len--)
    {
        verifySum ^=  *buf++;
    }

    return  verifySum;
}


//define EEPROM_PAGE_SIZE     256 /**< Size of one flash page. */
#define EEPROM_MAGIC_NUMBER  0x4500                          /**< Magic value to identify if flash contains valid data. */
#define EEPROM_EMPTY_MASK    0xFFFF                          /**< Bit mask that defines an empty address in flash. */


// user can read 256 - 8 = 248 bytes.
bool EEPROM_ReadBytesWithMagic(uint8_t channel, uint16_t read_addr, uint8_t* p_out_array, uint16_t byte_read_max, uint16_t * p_byte_count)
{	
    xSemaphoreTake(xSemaphore, 30000);//portMAX_DELAY
    
    bool ret = false;
    int        byte_count;
    uint16_t   flash_header;
    uint16_t   calc_header;
    uint8_t   calc_crc;
    uint16_t   tmp;

    uint16_t read_half_word[2];
    ret = EEPROM_ReadBytesInChannel(channel, read_addr, (uint8_t *)&read_half_word, 4);

    flash_header = read_half_word[0];
    tmp = flash_header & 0xFF00;

    // 0x45DE0000
    if (tmp != EEPROM_MAGIC_NUMBER)
    {
        if (p_byte_count != NULL)
        {    
            *p_byte_count = 0;
        }
        xSemaphoreGive(xSemaphore);
        return false;
    }

    // Read number of elements
    uint16_t read_len = read_half_word[1];
    if (p_byte_count != NULL)
    {
        *p_byte_count = read_len;
    }

    // Read data
    byte_count = read_len;// (*p_byte_count) * sizeof(uint8_t);

    uint8_t read_buf[byte_count];
    ret = EEPROM_ReadBytesInChannel(channel, read_addr + 4, (uint8_t *)&read_buf, byte_count);

    // Check CRC
    calc_crc = sum_calc(read_buf, byte_count);

    calc_header = EEPROM_MAGIC_NUMBER | (uint16_t)calc_crc;

    if (calc_header != flash_header)
    {
        xSemaphoreGive(xSemaphore);
        return false;
    }

    //
    byte_count = (byte_count > byte_read_max) ? byte_read_max : byte_count;
    memcpy(p_out_array, &read_buf[0], byte_count);

    xSemaphoreGive(xSemaphore);
    return ret;
}		
	


//uint32_t wt_buf[1024];
bool EEPROM_WriteBytesWithMagic(uint8_t channel, uint16_t write_addr, uint8_t const* p_in_array, uint16_t byte_count)
{
    xSemaphoreTake(xSemaphore, 30000);//portMAX_DELAY
    
    bool ret = false;

    uint8_t   in_data_crc;
    // uint16_t   flash_crc;
    uint16_t   flash_header;

    // Calculate CRC of the data to write.
    in_data_crc = sum_calc(p_in_array, byte_count * sizeof(uint8_t));

    flash_header = EEPROM_MAGIC_NUMBER | (uint16_t)in_data_crc;

    uint16_t write_byte_count = (uint16_t)(byte_count);

    typedef union {
        uint16_t data[2];
        uint8_t  data_byte[4];
    } spi_flash_write_data_u;

    uint8_t write_buf[4 + write_byte_count];

    spi_flash_write_data_u write_head_data;

    write_head_data.data[0] = flash_header;
    write_head_data.data[1] = write_byte_count;


    memcpy(&write_buf[0], (uint8_t *)&write_head_data, 4);
    memcpy(&write_buf[4], p_in_array, write_byte_count);

    ret = EEPROM_WriteBytesInChannel(channel, write_addr, &write_buf[0], 4 + write_byte_count);

    xSemaphoreGive(xSemaphore);
    return ret;
}

bool EEPROM_CheckIn(uint8_t channel)
{
    xSemaphoreTake(xSemaphore, 20000);//portMAX_DELAY
    
    bool ret = false;
    
    uint8_t sample_data;
    ret = TCA9548A_ChannelSelect(channel);
    ret = i2c_bus.read(EEPROM_SLAVE_ADDR, &sample_data, sizeof(sample_data));
    
    xSemaphoreGive(xSemaphore);
    return ret;
}


bool EEPROM_Init(void)
{
    bool ret = true;
    
    if (b_eeprom_Init == 0)
	{
        static uint8_t b_semaphore_create = 0;
        if (b_semaphore_create == 0)
        {
            xSemaphore = xSemaphoreCreateMutex();
            if( xSemaphore != NULL )
            {
                /* The semaphore was created successfully and
                can be used. */
                b_semaphore_create = 1;
                
                EEPROM_DMSG("<EEPROM> xSemaphoreCreateMutex OK.\r\n");
            }
            else
            {
                EEPROM_DMSG("<EEPROM> xSemaphoreCreateMutex  Failure\r\n");
                delay_ms(10);
                APP_ERROR_CHECK(1);
            }
        }
        
        i2c_bus.init();
        
        ret = TCA9548A_Init();
        
        EEPROM_DMSG("<EEPROM> Init. ret=%d\r\n", ret);
        
        b_eeprom_Init = 1;
    }

    return ret;
}

void EEPROM_DeInit(void)
{
    if (b_eeprom_Init)
	{
		b_eeprom_Init = 0;
        
        EEPROM_DMSG("<EEPROM> DeInit.\r\n");   
    }
}

/*
    // Read reboot status.
    ret = EEPROM_Init();
    uint16_t real_read_len = 0;
    ret = EEPROM_ReadBytesWithMagic(0, 248, &reboot_status, 1, &real_read_len);
    if (reboot_status == 0xeB)
    {
        SYSTEM_DMSG("<MAIN>System In Reboot.");
        reboot_status = 0x00;
        ret = EEPROM_WriteBytesWithMagic(0, 248, &reboot_status, 1);
    }

    // Write reboot status.
    uint8_t reboot_status = 0xeB;
    bool ret = EEPROM_WriteBytesWithMagic(0, 248, &reboot_status, 1);
    XTCP_DMSG("<XTCP> System In Reboot. ret= %d.\r\n", ret);
*/

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
