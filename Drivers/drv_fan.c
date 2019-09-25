/**
  ******************************************************************************
  * @file    drv_fan.c
  * @author  Merle
  * @version V1.0.0
  * @date    18-July-2019
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "drv_fan.h"

#define DBG_FAN
#ifdef DBG_FAN
#define FAN_DMSG	RTT_Printf
#else
#define FAN_DMSG(...)
#endif

#define FAN1_CTL_CHANNEL     0
#define FAN2_CTL_CHANNEL     1
#define FAN3_CTL_CHANNEL     2
#define FAN4_CTL_CHANNEL     3
#define FAN5_CTL_CHANNEL     4

#define FAN_POWER_ENABLE_CTL_CHANNEL     5//9

//因为我们释放的是气味，能量化成空气出气量这种模式那肯定最好， unit -> mL/Sec（毫升/秒）
/*
CFM是一种流量单位 
　　cubic feet per minute 立方英尺每分钟 
　　1CFM=28.316846592 L/MIN=0.028CMM 
　　CMM是常用中制流量单位,立方米每分钟
1cfm≈1.7m3/h
*/

static uint8_t b_fan_Init = 0;

static uint8_t air_fans_on_status = 0x00;//0x00 | 0x20;

const uint8_t channal_exchangge[5] = {2, 1, 0, 4, 3};
// 
uint8_t FAN_GetStatus(uint8_t channel)
{
    uint8_t status = (air_fans_on_status >> channel) & 0x01;
    return status;
}

bool FAN_SetPwm(uint8_t channel, uint16_t value)
{
    bool ret;
    if (channel >= 5)
        return false;

    //
    if (value)
    {
        air_fans_on_status |= (1 << channel);
        ret = PCA9685_PowerOn();
    }
    //
    uint8_t channelx = channal_exchangge[channel];
    ret = PCA9685_SetPWM(channelx, 0, value);
    //
    if (value == 0)
    {
        air_fans_on_status &= ~(1 << channel); 
        if (air_fans_on_status == 0x00)
        {
            ret = PCA9685_PowerOff();   //5mA -> 2mA autoPowerOff can save Power 4mA, will use in next design PCBA's FW
        }
    }

    return ret;
}

bool FAN_SetSpeed(uint8_t channel, uint8_t value)
{
    bool ret;
    if (channel >= 5)
        return false;
    //
    if (value)
    {
        air_fans_on_status |= (1 << channel);
        ret = PCA9685_PowerOn();
    }
    //
    uint16_t value_16 = ((uint16_t)value << 4); // 0xFFF, 0xFF.
    uint8_t channelx = channal_exchangge[channel];
    if (value == 0xFF)
    {
        ret = PCA9685_SetPinOn(channelx);
    }
    else
    {    
        ret = PCA9685_SetPWM(channelx, 0, value_16);
    }

    //
    if (value == 0)
    {
        air_fans_on_status &= ~(1 << channel); 
        if (air_fans_on_status == 0x00)
        {
            ret = PCA9685_PowerOff();   //5mA -> 2mA autoPowerOff can save Power 4mA, will use in next design PCBA's FW
        }
    }

    return ret;
}

bool FAN_SetFlow(uint8_t channel, uint8_t value)
{
    bool ret;
    if (channel >= 5)
        return false;
    //
    if (value)
    {
        air_fans_on_status |= (1 << channel);
        ret = PCA9685_PowerOn();
    }
    //
    uint16_t value_16 = ((uint16_t)value << 4);
    uint8_t channelx = channal_exchangge[channel];
    ret = PCA9685_SetPWM(channelx, 0, value_16);
    //
    if (value == 0)
    {
        air_fans_on_status &= ~(1 << channel); 
        if (air_fans_on_status == 0x00)
        {
            ret = PCA9685_PowerOff();   //5mA -> 2mA autoPowerOff can save Power 4mA, will use in next design PCBA's FW
        }
    }
    return ret;
}

bool FAN_SetPinOn(uint8_t channel)
{    
    bool ret;
    if (channel >= 5)
        return false; 
    //
    air_fans_on_status |= (1 << channel);
    ret = PCA9685_PowerOn();
    //
    uint8_t channelx = channal_exchangge[channel];
    ret = PCA9685_SetPinOn(channelx);
    return ret;
}

bool FAN_SetPinOff(uint8_t channel)
{    
    bool ret;
    if (channel >= 5)
        return false;
    //
    uint8_t channelx = channal_exchangge[channel];
    ret = PCA9685_SetPinOff(channelx);
    //
    air_fans_on_status &= ~(1 << channel);
    if (air_fans_on_status == 0x00)
    {
        ret = PCA9685_PowerOff();   // autoPowerOff can save Power 4mA, will use in next design PCBA's FW
    }
    return ret;
}


bool FAN_PowerOn(void)
{
    bool ret;
//    ret = PCA9685_PowerOn();
//    ret = PCA9685_SetPinOn(FAN_POWER_ENABLE_CTL_CHANNEL);
    return ret;
}

bool FAN_PowerOff(void)
{
    bool ret;
//    ret = PCA9685_SetPinOff(FAN_POWER_ENABLE_CTL_CHANNEL);
//    ret = PCA9685_PowerOff(); 
    return ret;
}

bool FAN_Init(void)
{
    bool ret;
    if (b_fan_Init == 0)
	{
		b_fan_Init = 1;
        
        ret = PCA9685_Init();
        //FAN_PowerOff();
        //ret = FAN_PowerOn();
        //ret = FAN_PowerOff();
        FAN_DMSG("<FAN> Init. ret=%d.\r\n", ret);   
    }
    return ret;
}

void FAN_DeInit(void)
{
    if (b_fan_Init)
	{
		b_fan_Init = 0;
        
        //FAN_PowerOff();
        
        PCA9685_DeInit();
        
        FAN_DMSG("<FAN> DeInit.\r\n");   
    }
}

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
