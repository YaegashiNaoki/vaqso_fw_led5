/**
  ******************************************************************************
  * @file    drv_tca9548a.c
  * @author  Merle
  * @version V1.0.0
  * @date    5-July-2019
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"


//#define DBG_TCA9548A
#ifdef DBG_TCA9548A
#define TCA9548A_DMSG	RTT_Printf
#else
#define TCA9548A_DMSG(...)
#endif


#define TCA9548A_SLAVE_ADDR		(0x70)      // IIC Address.


static i2c_bus_t i2c_bus = {&I2C2_Init, &I2C2_Rx, &I2C2_Tx};


static uint8_t b_tca9548a_init = 0;


static bool TCA9548A_Read8(uint8_t *p_data) 
{
    bool ret;
    ret = i2c_bus.read(TCA9548A_SLAVE_ADDR, p_data, 1);
    return ret;
}

static bool TCA9548A_Write8(uint8_t data)
{
    bool ret; 
    ret = i2c_bus.write(TCA9548A_SLAVE_ADDR, &data, 1, false);
    return ret;
}


#define TCA9548A_RESET_PIN  BOARD_I2C_RESET

bool TCA9548A_Reset(void)
{
    nrf_gpio_pin_clear(TCA9548A_RESET_PIN);
    delay_us(2);
    nrf_gpio_pin_set(TCA9548A_RESET_PIN);
    
    return true;
}


// channel could be be 0~7
bool TCA9548A_ChannelSelect(uint8_t channel) 
{
    bool ret;

    volatile uint8_t ctl_reg_data = 0;

    ctl_reg_data = 0xff & (0x01 << ((channel > 7) ? 7 : channel));
    
    ret = TCA9548A_Write8(ctl_reg_data);
    //TCA9548A_DMSG("<TCA9548A> 1.write.ret=%d, ctl_reg_data= 0x%02X \r\n", ret, ctl_reg_data);
 
    return ret;
}

bool TCA9548A_ChannelGet(uint8_t* channel)
{
    bool ret;
    uint8_t ctl_reg_data = 0;
    ret = TCA9548A_Read8(&ctl_reg_data);
    
    (*channel) = ctl_reg_data;
    TCA9548A_DMSG("<TCA9548A> 2.read.ret=%d, ctl_reg_data= 0x%02X \r\n", ret, ctl_reg_data);
    
    return ret;
}

void TCA9548A_DeInit(void)
{
    if (b_tca9548a_init)
    {
        b_tca9548a_init = 0;
        
        TCA9548A_Reset();

        TCA9548A_DMSG("<TCA9548A> DeInit.\r\n");
    }
}

bool TCA9548A_Init(void)
{
    bool ret = true;
    
    if (b_tca9548a_init == 0)
    {        
        i2c_bus.init();
        
        nrf_gpio_cfg_output(TCA9548A_RESET_PIN);
        nrf_gpio_pin_set(TCA9548A_RESET_PIN);
        TCA9548A_Reset();
          
        uint8_t ctl_reg_data = 0;
    
        ret = TCA9548A_Read8(&ctl_reg_data);
    
        TCA9548A_DMSG("<TCA9548A> Init. ret=%d, read ctl_reg_data= 0x%02X \r\n", ret, ctl_reg_data);
        
        b_tca9548a_init = 1;
    }
    return ret;
}



/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
