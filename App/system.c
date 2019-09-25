/**
  ******************************************************************************
  * @file    drv_system.c
  * @author  Merle
  * @version V1.0.0
  * @date    25-January-2017.
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"

//#define DBG_SYSTEM
#ifdef DBG_SYSTEM
#define SYSTEM_DMSG	RTT_Printf
#else
#define SYSTEM_DMSG(...)
#endif


void SYSTEM_PowerOff(void)
{
    nrf_gpio_pin_clear(BOARD_MIX_POWER_CTL);
    SYSTEM_DMSG("<SYSTEM> PowerOff.\r\n");
}

void SYSTEM_PowerOn(void)
{
    nrf_gpio_pin_set(BOARD_MIX_POWER_CTL);
    SYSTEM_DMSG("<SYSTEM> PowerOn.\r\n");
}

void SYSTEM_PowerCtlInit(void)
{
    nrf_gpio_cfg_output(BOARD_MIX_POWER_CTL);
    //
    nrf_gpio_cfg_output(BOARD_DCDC_5V_POWER_CTL);
    nrf_gpio_pin_clear(BOARD_DCDC_5V_POWER_CTL);
    //
    nrf_gpio_cfg_output(BOARD_LED_POWER_CTL);
    nrf_gpio_pin_clear(BOARD_LED_POWER_CTL);
}

void SYSTEM_DCDC5V_PowerTurnOn(void)
{
    nrf_gpio_pin_set(BOARD_DCDC_5V_POWER_CTL);
    SYSTEM_DMSG("<SYSTEM> 5V_PowerOn.\r\n");
}

void SYSTEM_DCDC5V_PowerTurnOff(void)
{
    nrf_gpio_pin_clear(BOARD_DCDC_5V_POWER_CTL);
    SYSTEM_DMSG("<SYSTEM> 5V_PowerOff.\r\n");
}

//void SYSTEM_LED_PowerTurnOn(void)
//{
//    nrf_gpio_pin_set(BOARD_LED_POWER_CTL);
//}

//void SYSTEM_LED_PowerTurnOff(void)
//{
//    nrf_gpio_pin_clear(BOARD_LED_POWER_CTL);
//}

static const uint8_t light_brightness = 50;

void SYSTEM_DealWithReset(void)
{
    bool ret;

    ENV_Init();
    SYSTEM_DCDC5V_PowerTurnOff();
    FAN_Init();
    FAN_DeInit();
    
    env.cfg.bootTime ++;
    
    if (env.cfg.reboot_status == BOARD_REBOOT_BYTE)
    {
        SYSTEM_DMSG("<MAIN>System In Reboot.\r\n");
        
        env.cfg.reboot_status = 0xFF;
        ENV_Save();
    }
    else
    {
        SYSTEM_DMSG("<MAIN>System In Check. reboot_status=0x%02X.\r\n",env.cfg.reboot_status);
        
        KEY_Init();
        //WS2812B_Init();
        
        #define SYSTEM_START_KEY_CNT    (34U)
        uint8_t  key_down_cnt = 0;
        
        // if power on with the key down loop one second, then system startup normal.
        for (uint16_t i = 0; i < SYSTEM_START_KEY_CNT; i++)
        {
            uint8_t value = KEY_ReadIOState();
            //SYSTEM_DMSG("<SYSTEM> KeyValue Down,i =%d, =%d.\r\n", i, value);
            if (value == 0)
            {
                // key_down
                key_down_cnt++;
            }
            else
            {
                key_down_cnt = 0;
                SYSTEM_PowerOff();  // power on lock itself disable. 
                break;
            }
            delay_ms(30);
        }
        SYSTEM_DMSG("<SYSTEM> KeyValue Down,n =%d, \r\n", key_down_cnt);
        
        
        // else, power on without the valid key put down(), may be in charging.
        if (key_down_cnt < SYSTEM_START_KEY_CNT)
        {
            SYSTEM_DCDC5V_PowerTurnOn();
            WS2812B_Init();
            TIM4_Init();
            BAT_Init();
            
            SYSTEM_PowerOn(); // use to before env save.
            
            uint8_t last_bat_state = 0xff;
            uint8_t key_value;
            uint8_t light_show_battery_low_cnt = 0;
            uint8_t light_show_battery_low_direct = 0;
            uint8_t usb_power_down_cnt = 0;
            
            bool b_loop = true;
            while (b_loop)
            {
                key_value = KEY_Get();

                if (key_value == KEY_1_LONG)
                {
                    SYSTEM_DMSG("<SYSTEM> KEY_1_LONG. bat.percentage =%d.\r\n", bat.percentage);
                    if (bat.percentage > 50) // %5 > can be startup.
                    {
                        // Then the system startup, get out off this loop function.
                        SYSTEM_PowerOn(); 
                        b_loop = false;
                    }
                    else
                    {
                        light_show_battery_low_cnt = 8;
                        light_show_battery_low_direct = 0;
                    }
                }
                
                // Battery Power Is Too Low For System Startup With The RGB_LED Blink Show Logic.
                static uint32_t battery_power_low_Deadline = 0;
                int32_t dtDeadline1 = (int64_t)millis() - battery_power_low_Deadline; // 25ms * n;
                if (dtDeadline1 < 0)
                {
                    if (dtDeadline1 < (-50))
                    {
                        battery_power_low_Deadline = 0;
                    }
                }
                else
                {
                    battery_power_low_Deadline = millis() + 20;	// 25ms * 20 = about 500 ms.
                    
                    if (light_show_battery_low_cnt)
                    {
                        color_t color;
                        if (light_show_battery_low_direct == 0)
                        {
                            light_show_battery_low_direct = 1;
                            
                            color.r = light_brightness;
                            color.g = light_brightness;
                            color.b = 0;
                            WS2812B_SetPixelColor(14, &color);//4
                            WS2812B_Refresh();
                        }
                        else
                        {
                            light_show_battery_low_direct = 0;
                              
                            color.r = 0;
                            color.g = 0;
                            color.b = 0;
                            WS2812B_SetPixelColor(14, &color);//4
                            WS2812B_Refresh();
                        }
                        light_show_battery_low_cnt--;
                        if (light_show_battery_low_cnt == 0)
                        {
                            last_bat_state = 0xFF;  // refresh the battery charge status.
                        }
                    }
                    
                    // USB Cable, Jitter processing logic.
                    uint8_t usb_connect_state = BAT_GetUSBConnectStatus();
                    if (usb_connect_state == 0)
                    {
                        usb_power_down_cnt++;
                        if (usb_power_down_cnt > 5) // 250ms* 5times.
                        {
                            usb_power_down_cnt = 0;
                            ENV_Save();
                            SYSTEM_PowerOff();
                        }   
                    }
                    else
                    {
                        usb_power_down_cnt = 0;
                    }
                }
                
                
                
                static uint32_t battery_update_Deadline = 0;
                int32_t dtDeadline = (int64_t)millis() - battery_update_Deadline; // 25ms * n;
                if (dtDeadline < 0)
                {
                    if (dtDeadline < (-50))
                    {
                        battery_update_Deadline = 0;
                    }
                }
                else
                {
                    battery_update_Deadline = millis() + 40;	// 25ms * 40 = about 1s.
                
                    //
                    BAT_Update();
                
                    // the rgb led show the battery charge status.
                    if (bat.chargeState != last_bat_state)
                    {
                        if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
                        {
                            WS2812B_PixelClear();
                        
                            color_t color;
                            //
                            color.r = light_brightness;
                            color.g = 0;
                            color.b = 0;
                            WS2812B_SetPixelColor(14, &color);//4
                            WS2812B_Refresh();
                        }
                        else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
                        {
                            WS2812B_PixelClear();
                        
                            color_t color;
                            //
                            color.r = 0;
                            color.g = 0;
                            color.b = light_brightness;
                            WS2812B_SetPixelColor(14, &color);//4
                            WS2812B_Refresh();
                        }
                        else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
                        {
                            color_t color;
                            // 
                            WS2812B_PixelClear();
                        
                            color.r = light_brightness;
                            color.g = 0;
                            color.b = 0;
                            for (uint8_t i = 0; i < 4; i++)
                            {
                                color.r = 255;
                                WS2812B_SetPixelColor(14, &color);//4
                                WS2812B_Refresh();
                                delay_ms(500);
                                color.r = 0;
                                WS2812B_SetPixelColor(14, &color);//4
                                WS2812B_Refresh();
                                delay_ms(500);
                            }      
                        }
                        else
                        {
                            WS2812B_PixelClear();
                            WS2812B_Refresh();
                        }
                    }
                    last_bat_state = bat.chargeState;
                    SYSTEM_DMSG("<SYSTEM> chargeState =%d batPercent=%d.vot=%d.\r\n", 
                            bat.chargeState,
                            bat.percentage,
                            bat.voltage);
                    
                    WDT_Feed();
                }

                nrf_pwr_mgmt_run();
                //delay_ms(1000);
                //WDT_Feed();
            }
            TIM4_DeInit();
            KEY_FIFO_Flush();
            WS2812B_DeInit();
            BAT_DeInit();
            SYSTEM_DCDC5V_PowerTurnOff();
            //PCA9685_PowerOff();
        }
        else
        {
            WS2812B_DeInit();
            SYSTEM_PowerOn();
        }
    }

    SYSTEM_DCDC5V_PowerTurnOn();
}












//void HardFault_Handler(void)
//{
//	SYSTEM_SaveParameter(true);
//	SYSTEM_DMSG("HardFault_Handler\r\n.\r\n");
//	delay_ms(50);
//	NVIC_SystemReset();
//}

void DebugMon_Handler(void)
{
	SYSTEM_DMSG("DebugMon_Handler\r\n.\r\n");
	delay_ms(50);
	NVIC_SystemReset();
}

//void PendSV_Handler(void)
//{
//	SYSTEM_DMSG("PendSV_Handler\r\n.\r\n");
//	delay_ms(50);
//	NVIC_SystemReset();
//}

//void SVC_Handler(void)
//{
//	SYSTEM_DMSG("VC_Handler\r\n.\r\n");
//	delay_ms(50);
//	NVIC_SystemReset();
//}

void UsageFault_Handler(void)
{
	//MOTOR_PowerOff();
//	ENV_Save();
	SYSTEM_DMSG("UsageFault_Handler\r\n.\r\n");
	delay_ms(50);
	NVIC_SystemReset();
}

void BusFault_Handler(void)
{
	SYSTEM_DMSG("BusFault_Handler\r\n.\r\n");
	delay_ms(50);
	NVIC_SystemReset();
}

void MemoryManagement_Handler(void)
{
	SYSTEM_DMSG("MemoryManagement_Handler\r\n.\r\n");
	delay_ms(50);
	NVIC_SystemReset();
}


void NMI_Handler(void)
{
	SYSTEM_DMSG("NMI_Handler\r\n.\r\n");
	delay_ms(50);
	NVIC_SystemReset();
}
void SysTick_Handler(void)
{
	SYSTEM_DMSG("SysTick_Handler\r\n.\r\n");
	delay_ms(50);
	NVIC_SystemReset();
}



	
/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
