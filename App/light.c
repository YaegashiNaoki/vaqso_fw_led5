/**
  ******************************************************************************
  * @file    light.c
  * @author  Merle
  * @version V1.0.0
  * @date    26-July-2019
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "color.h"
#include "math_algo.h"

#define DBG_LIGHT
#ifdef DBG_LIGHT
#define LIGHT_DMSG	RTT_Printf
#else
#define LIGHT_DMSG(...)
#endif


static const uint8_t light_standard_brightness = 255;
static const uint8_t light_battery_status_brightness = 50;

static light_sequence_type_t light_sequence_last_type = 0;
static light_sequence_type_t light_sequence_last_set_type = 0;
static light_sequence_type_t light_sequence_current_set_type = 0;

void LIGHT_TurnOn(color_t const* color, uint16_t first, uint16_t count, uint16_t until_m_scond)
{
    WS2812B_FillColor(color, first, count);
    WS2812B_Refresh();
    delay_ms(until_m_scond);
    
    WS2812B_PixelClear();
    WS2812B_Refresh();
}



//  
#define KNIGHT_RIDER_LIGHT_NUM          (15U)
#define KNIGHT_RIDER_LIGHT_TIMER_SHAFT  (29U)
static const uint8_t KnightRiderlights_buffer[KNIGHT_RIDER_LIGHT_NUM][KNIGHT_RIDER_LIGHT_TIMER_SHAFT] =
{
// timer_shaft   0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39
// interval time 100ms.   
/*LED-0*/       {0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
/*LED-1*/       {0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
/*LED-2*/       {0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
/*LED-3*/       {0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
/*LED-4*/       {0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
/*LED-5*/       {0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
/*LED-6*/       {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
/*LED-7*/       {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
/*LED-8*/       {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
/*LED-9*/       {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
/*LED-10*/      {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
/*LED-11*/      {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
/*LED-12*/      {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00,0x00},
/*LED-13*/      {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00,0x00},
/*LED-14*/      {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00,0x00},
///*LED-15*/      {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00,0x00},
///*LED-16*/      {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00,0x00},
///*LED-17*/      {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x10,0x30,0x80,0x80,0x30,0x10,0x05,0x02,0x00},
};

//  
void LIGHT_ShowKnightRiderlights(void)
{
    int8_t time_shaft = 0;
    uint8_t b_direct = 0; 
    uint8_t b_loop = 1;
    
    color_t color = {0};
   
    while(b_loop)
    {
        for (uint8_t index = 0; index < KNIGHT_RIDER_LIGHT_NUM; index++ )
        {
            color.r = KnightRiderlights_buffer[index][time_shaft];
            color.g = KnightRiderlights_buffer[index][time_shaft];
            color.b = KnightRiderlights_buffer[index][time_shaft];
            
            WS2812B_SetPixelColor(index, &color); // 
        }
        
        WS2812B_Refresh(); // update
        
        vTaskDelay(75);
        
        if (b_direct == 0)
        {
            time_shaft++;
            if (time_shaft >= (KNIGHT_RIDER_LIGHT_TIMER_SHAFT - 1)) 
            {
                time_shaft = KNIGHT_RIDER_LIGHT_TIMER_SHAFT - 1;
                b_direct = 1;
            }
        }
        else
        {
            time_shaft--;
            if (time_shaft < 0) 
            {
                time_shaft = 0;
                b_direct = 0;
                b_loop = 0;
            }
        }
    }
}

static uint8_t b_light_show_is_ble_adv = 0;
static bool b_kokyu_led = false;

void LIGHT_TurnOff(void)
{
    WS2812B_PixelClear();
    WS2812B_Refresh();
}

static color_t light_current_color = {.r = 0, .g =0, .b = 0};

void LIGHT_SetColor(color_t const* color)
{
    light_current_color.r = color->r;
    light_current_color.g = color->g;
    light_current_color.b = color->b;
}

void LIGHT_TurnOnWithTheSettingColor(void)
{
    WS2812B_FillColor(&light_current_color, 0, 15);
    WS2812B_Refresh();
}


#define LIGHT_SEQUENCE_SIZE     (16U) //4 // 8 // 16 // 32 // 64
static app_fifo_t light_sequence_fifo; /**< RX FIFO buffer for storing data received on the UART until the application fetches them using app_uart_get(). */
static uint8_t light_sequence_buffer[LIGHT_SEQUENCE_SIZE];

uint32_t LIGHT_SequenceInit(void)
{
    ret_code_t err_code;
    err_code = app_fifo_init(&light_sequence_fifo, light_sequence_buffer, LIGHT_SEQUENCE_SIZE);
    APP_ERROR_CHECK(err_code);
    return err_code;
}

uint32_t LIGHT_SequenceFlush(void)
{
    b_light_show_is_ble_adv = 0; 
    
    uint32_t err_code;
    err_code = app_fifo_flush(&light_sequence_fifo);
    VERIFY_SUCCESS(err_code);
    
    return err_code;
}

uint32_t LIGHT_SequenceSet(light_sequence_type_t type)
{
    uint32_t err_code;
    
    LIGHT_DMSG("<LIGHT> set sequence type=%d.\r\n", type);
    b_kokyu_led = false;

    if (((light_sequence_current_set_type == LIGHT_BLE_ADV_START) && (type == LIGHT_BLE_BONDS_SUCCESS)) ||
        ((light_sequence_current_set_type == LIGHT_BLE_BONDS_SUCCESS) && (type == LIGHT_BLE_ADV_START))
        )
    {
        err_code = app_fifo_flush(&light_sequence_fifo);
        LIGHT_DMSG("<LIGHT> light_sequence_fifo\r\n");
    }
    
    if ((light_sequence_current_set_type == LIGHT_BLE_ADV_START) && (type == LIGHT_BLE_ADV_START))
    {
        LIGHT_DMSG("<LIGHT> Return,because last type is BLE_Adv.\r\n");
        return NRF_ERROR_INVALID_STATE;
    }
    
    
    light_sequence_current_set_type = type;
//    if ((type >= LIGHT_BAT_DISCHARGE) && (type <= LIGHT_BAT_CHARGING_IRREGULAR)
//         && 
//        ((light_sequence_last_type < LIGHT_BAT_DISCHARGE) || 
//        (light_sequence_last_type > LIGHT_BAT_CHARGING_IRREGULAR))
//       )
//    {
//        LIGHT_DMSG("<LIGHT> return,because_last_type=%d.\r\n", light_sequence_last_type);
//        return  NRF_ERROR_BUSY;
//    }
//    light_sequence_last_type = type;
    
    
//    if (type == LIGHT_BAT_DISCHARGE)
//    {
//        if (f.BLE_CONNECT_STATUS == BLE_STATUS_FAST_ADV)
//        {
//            return NRF_ERROR_BUSY;
//        }
//    }

    if (type == LIGHT_BLE_ADV_START)
    {
        b_light_show_is_ble_adv = 1;
    }
    else
    {
       b_light_show_is_ble_adv = 0;
    }

   
    err_code = app_fifo_put(&light_sequence_fifo, type);
    //VERIFY_SUCCESS(err_code);
    
    if (err_code != NRF_SUCCESS)
    {
        LIGHT_DMSG("<LIGHT> set sequence type,err_code= %d.\r\n", err_code);
    }
    return err_code;
}

uint8_t LIGHT_SequenceGetLastType(void)
{
    uint8_t type = light_sequence_last_type;
    return type;
}


#define LIGHT_NUM_IS_15

#ifdef LIGHT_NUM_IS_15


void LIGHT_ShowLowBattery(void)
{
    color_t color;
    //
    for (uint8_t i = 0; i < 4; i++)
    {
        color.r = light_battery_status_brightness;
        color.g = light_battery_status_brightness;
        color.b = 0;
        WS2812B_SetPixelColor(14, &color);
        WS2812B_Refresh();
        vTaskDelay(500);
        color.r = 0;
        color.g = 0;
        color.b = 0;
        WS2812B_SetPixelColor(14, &color);
        WS2812B_Refresh();
        vTaskDelay(500);
    }
}

void LIGHT_ShowFanGivesOffSmell(void)
{    
    color_t color;
    //
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
    WS2812B_SetPixelColor(0, &color);
    WS2812B_Refresh();
    vTaskDelay(6000);
        
//    WS2812B_PixelClear();
//    WS2812B_Refresh();
}

void LIGHT_ShowAirFanOn(uint8_t channel)
{    
    color_t color;
    //
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
    WS2812B_SetPixelColor(1 + 3 * channel, &color);
    WS2812B_Refresh(); 
}

void LIGHT_ShowAirFanOff(uint8_t channel)
{    
    color_t color;
    //
    color.r = 0;
    color.g = 0;
    color.b = 0;
    WS2812B_SetPixelColor(1 + 3 * channel, &color);
    WS2812B_Refresh();
    
    if (channel == 4)
    {
        if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
        {
            LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
        }
        else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
        {
            LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
        }
        else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
        {
            LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
        }
        else
        {
            ;
        }
    }
}

static const uint8_t lights_cartridges_blink_buffer[23] =
{
// timer_shaft   0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39
// interval time 100ms.   
/*LED*/          0x00,0x02,0x05,0x10,0x20,0x30,0x50,0x60,0x80,0x90,0xB0,0xFF,0xFF,0xB0,0x90,0x80,0x50,0x30,0x20,0x10,0x05,0x02,0x00
};
static const uint8_t lights_cartridges_blink_buffer_show[64] =
{
// timer_shaft   0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39
// interval time 100ms.   
/*LED*/          0x00,0x02,0x04,0x08,0x10,0x18,0x20,0x28,0x30,0x38,0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78,0x80,0x88,0x90,0x98,0xA0,0xA8,0xB0,0xb8,0xC0,0xC8,0xE0,0xE8,0xF0,0xFF,
0xFF,0xF0,0xE8,0xE0,0xC8,0xb8,0xB0,0xA8,0xA0,0x98,0x90,0x88,0x80,0x78,0x70,0x68,0x60,0x58,0x50,0x48,0x40,0x38,0x30,0x28,0x20,0x18,0x10,0x08,0x04,0x04,0x02,0x00

//0x30,0x50,0x60,0x80,0x90,0xB0,0xFF,0xFF,0xB0,0x90,0x80,0x50,0x30,0x20,0x10,0x05,0x02,0x00
};
void LIGHT_ShowCartridgeDetach(uint8_t channel)
{    
    color_t color = {0};
    //
    for (uint8_t j = 0; j < 23; j ++) //2250ms;
    {
        color.r = lights_cartridges_blink_buffer[j];
        color.g = lights_cartridges_blink_buffer[j];
        WS2812B_SetPixelColor(1 + 3 * channel, &color);
        WS2812B_Refresh();
        if (j < 22)
        {
            vTaskDelay(100);
        }
    }
    
    uint8_t fan_status = FAN_GetStatus(channel);
    if (fan_status)
    {
        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + channel);
    }
    else
    {
        if (channel == 4)
        {
            if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
            }
            else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
            {
                LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
            }
            else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
            }
            else
            {
                ;
            }
        }
    }
}

void LIGHT_ShowCartridgeAttach(uint8_t channel)
{    
    color_t color = {0};
    //
    for (uint8_t j = 0; j < 23; j ++)
    {
        color.r = 0;
        color.g = lights_cartridges_blink_buffer[j];
        WS2812B_SetPixelColor(1 + 3 * channel, &color);
        WS2812B_Refresh();
        if (j < 22)
        {
            vTaskDelay(100);
        }
    }

    uint8_t fan_status = FAN_GetStatus(channel);
    if (fan_status)
    {
        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + channel);
    }
    else
    {
        if (channel == 4)
        {
            if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
            }
            else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
            {
                LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
            }
            else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
            }
            else
            {
                ;
            }
        }
    }
}

void LIGHT_ShowCartridgeExpiration(uint8_t channel)
{    
    color_t color = {0};
    //
    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 0; j < 23; j ++)
        {
            color.r = lights_cartridges_blink_buffer[j];
            color.g = lights_cartridges_blink_buffer[j];
            WS2812B_SetPixelColor(1 + 3 * channel, &color);
            WS2812B_Refresh();
            vTaskDelay(43);
        }
    }

    uint8_t fan_status = FAN_GetStatus(channel);
    if (fan_status)
    {
        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + channel);
    }
    else
    {
        if (channel == 4)
        {
            if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
            }
            else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
            {
                LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
            }
            else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
            }
            else
            {
                ;
            }
        }
    }
}

void LIGHT_ShowBatteryDischarge(void)
{    
//    uint8_t fan_status = FAN_GetStatus(4);
//    if (fan_status)
//    {
//        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + 4);
//    }
    //else
    {
        color_t color;
        //
        color.r = 0;
        color.g = 0;
        color.b = 0;
        WS2812B_SetPixelColor(14, &color);
        WS2812B_Refresh();
    }
}

void LIGHT_ShowBatteryChargingIrregular(void)
{   
//    uint8_t fan_status = FAN_GetStatus(4);
//    if (fan_status)
//    {
//        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + 4);
//    }
    //else
    {
        color_t color;
        //
        for (uint8_t i = 0; i < 4; i++)
        {
            color.r = light_battery_status_brightness;
            color.g = 0;
            color.b = 0;
            WS2812B_SetPixelColor(14, &color);
            WS2812B_Refresh();
            vTaskDelay(500);
            WS2812B_PixelClear();
            WS2812B_Refresh();
            vTaskDelay(500);
        }
    }
}


void LIGHT_ShowBatteryChargingComplete(void)
{
//    uint8_t fan_status = FAN_GetStatus(4);
//    if (fan_status)
//    {
//        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + 4);
//    }
    //else
    {
        color_t color;
        color.r = 0;
        color.g = 0;
        color.b = light_battery_status_brightness;
        WS2812B_SetPixelColor(14, &color);
        WS2812B_Refresh();
        vTaskDelay(500);
    }
}

void LIGHT_ShowBatteryCharging(void)
{
//    uint8_t fan_status = FAN_GetStatus(4);
//    if (fan_status)
//    {
//        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + 4);
//    }
    //else
    {
        color_t color;
        color.r = light_battery_status_brightness;
        color.g = 0;
        color.b = 0;
        WS2812B_SetPixelColor(14, &color);
        WS2812B_Refresh();
        vTaskDelay(500);
    }
}

void LIGHT_ShowBleBondsFailure(void)
{    
    color_t color;
    //
    for (uint8_t i = 0; i < 5; i++)
    {
        color.r = light_standard_brightness;
        color.g = light_standard_brightness;
        color.b = 0;

        for (uint8_t i = 0; i < 5;  i++)
        {
            WS2812B_SetPixelColor(1 + 3 * i, &color);
        }
        WS2812B_Refresh();
        vTaskDelay(500);
        //WS2812B_PixelClear();
        color.r = 0;
        color.g = 0;
        color.b = 0;
        for (uint8_t i = 0; i < 5;  i++)
        {
            WS2812B_SetPixelColor(1 + 3 * i, &color);
        }
        WS2812B_Refresh();
        vTaskDelay(500);
    }
    
    //
    for (uint8_t i = 0; i < 5; i++)
    {
        uint8_t fan_status = FAN_GetStatus(i);
        if (fan_status)
        {
            LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + i);
        }
    }
    //
    if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
    {
        LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
    }
    else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
    {
        LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
    }
    else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
    {
        LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
    }
    else
    {
        ;
    }
}

void LIGHT_ShowBleBondsSuccess(void)
{    
    color_t color;
    //
    color.r = 0;
    color.g = 0;
    color.b = light_standard_brightness;
    for (uint8_t i = 0; i < 5;  i++)
    {
         WS2812B_SetPixelColor(1 + 3 * i, &color);
    }
    WS2812B_Refresh();
    vTaskDelay(1500);
    //
    color.r = 0;
    color.g = 0;
    color.b = 0;
    for (uint8_t i = 0; i < 5;  i++)
    {
         WS2812B_SetPixelColor(1 + 3 * i, &color);
    }
    WS2812B_Refresh();
    
    //
    for (uint8_t i = 0; i < 5; i++)
    {
        uint8_t fan_status = FAN_GetStatus(i);
        if (fan_status)
        {
            LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + i);
        }
    }
    //
    if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
    {
        LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
    }
    else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
    {
        LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
    }
    else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
    {
        LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
    }
    else
    {
        ;
    }
}

void LIGHT_ShowBleAdvStart(void)
{    
    color_t color;
    color.r = 0;
    color.g = 0;
    color.b = light_standard_brightness;
    volatile uint16_t total_pixel_value = 0x0001;
    uint16_t num = 14;//5 * 3 - 1;
    do
    {
        for (uint16_t i = 0; (i < num) && b_light_show_is_ble_adv; i++)
        {
            //total_pixel_value &= 0x1f;
            uint32_t pixel32_bit_value = total_pixel_value;
            WS2812B_SetTotalPixel(pixel32_bit_value, 0, 15, &color);
            total_pixel_value = MATH_ALGO_u15_rol_(total_pixel_value, 1);
            WS2812B_Refresh();
            if (b_light_show_is_ble_adv)
            {
                vTaskDelay(100);
            }
        }
    }
    while(b_light_show_is_ble_adv);
    
    if (//(light_sequence_last_type != LIGHT_BLE_ADV_START) && 
        (f.BLE_CONNECT_STATUS == BLE_STATUS_FAST_ADV) &&
        (light_sequence_current_set_type != LIGHT_BLE_BONDS_SUCCESS) &&
        (light_sequence_current_set_type != LIGHT_BLE_BONDS_FAILURE))
    {
        //
        if ((light_sequence_current_set_type >= LIGHT_BAT_DISCHARGE) && 
            (light_sequence_current_set_type <= LIGHT_BAT_POWER_LOW))
        {
            LIGHT_SequenceSet(LIGHT_BLE_ADV_START);
        }
        
        //
        if ((light_sequence_current_set_type >= LIGHT_CARTRIDGE_ATTACH_1) && 
            (light_sequence_current_set_type <= LIGHT_CARTRIDGE_ATTACH_5))
        {
            LIGHT_SequenceSet(LIGHT_BLE_ADV_START);
        }
        
        //
        if ((light_sequence_current_set_type >= LIGHT_CARTRIDGE_DETACH_1) && 
            (light_sequence_current_set_type <= LIGHT_CARTRIDGE_DETACH_5))
        {
            LIGHT_SequenceSet(LIGHT_BLE_ADV_START);
        }
        
        //
        if ((light_sequence_current_set_type >= LIGHT_CARTRIDGE_ECPIRATION_1) && 
            (light_sequence_current_set_type <= LIGHT_CARTRIDGE_ECPIRATION_5))
        {
            LIGHT_SequenceSet(LIGHT_BLE_ADV_START);
        }
    }
    
    //WS2812B_PixelClear();
    color.r = 0;
    color.g = 0;
    color.b = 0;
    WS2812B_FillColor(&color, 0, 15);
    WS2812B_Refresh();
   // vTaskDelay(1850);
    
    // restore the light.
    if (light_sequence_current_set_type == LIGHT_DO_NOTHING)
    {
        //
        for (uint8_t i = 0; i < 5; i++)
        {
            uint8_t fan_status = FAN_GetStatus(i);
            if (fan_status)
            {
                LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + i);
            }
        }
        //
        if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
        {
            LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
        }
        else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
        {
            LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
        }
        else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
        {
            LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
        }
        else
        {
            ;
        }
    }
}


//void LIGHT_ShowBleAdvStart(void)
//{    
//    color_t color;
//    color.r = 0;
//    color.g = 0;
//    color.b = light_standard_brightness;
//    volatile uint8_t total_pixel_value = 0x01;
//    uint16_t num = 5 * 3 - 1;
//    for (uint16_t i = 0; i < num; i++)
//    {
//        total_pixel_value = MATH_ALGO_u5_rol_(total_pixel_value, 1);
//        WS2812B_SetTotalPixel(total_pixel_value, 0, 5, &color);
//        WS2812B_Refresh();
//        vTaskDelay(300);
//    }
//    WS2812B_PixelClear();
//    WS2812B_Refresh();
//    vTaskDelay(1850);
//}
//SemaphoreHandle_t xSemaphore;

//  
void LIGHT_ShowKnightRiderlights2(void)
{
    int8_t time_shaft = 0;
    uint8_t b_direct = 0; 
    uint8_t b_loop = 1;
    
    color_t color = {0};
   
    while(b_loop)
    {
        for (uint8_t index = 0; index < 15; index++ )
        {
            color.r = KnightRiderlights_buffer[index][time_shaft];
            color.g = KnightRiderlights_buffer[index][time_shaft];
            color.b = KnightRiderlights_buffer[index][time_shaft];
            
            WS2812B_SetPixelColor(index, &color); // 
        }
        
        WS2812B_Refresh(); // update
        
        vTaskDelay(85); //2.25s
        
        if (b_direct == 0)
        {
            time_shaft++;
            if (time_shaft >= (26 - 1)) //16
            {
                time_shaft = 26 - 1;//16
                b_direct = 1;
            }
        }
        else
        {
            time_shaft--;
            if (time_shaft < 0) 
            {
                time_shaft = 0;
                b_direct = 0;
                b_loop = 0;
            }
        }
    }
}


void LIGHT_ShowSystemStartup(void)
{ 
    WS2812B_PixelClear();
    
    LIGHT_ShowKnightRiderlights2();
    vTaskDelay(500);
    
    color_t color;
    //
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
    //WS2812B_FillColor(&color, 0, 15);
    for (uint8_t i = 0; i < 5;  i++)
    {
         WS2812B_SetPixelColor(1 + 3 * i, &color);
    }
  
    WS2812B_Refresh();
    vTaskDelay(1000);
    
    //
    //WS2812B_PixelClear();
    color.r = 0;
    color.g = 0;
    color.b = 0;
    for (uint8_t i = 0; i < 5;  i++)
    {
         WS2812B_SetPixelColor(1 + 3 * i, &color);
    }
    WS2812B_Refresh();
}

void LIGHT_ShowSystemShutdown(void)
{
    WS2812B_PixelClear();
    
    color_t color;
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
    for (uint8_t i = 0; i < 5;  i++)
    {
         WS2812B_SetPixelColor(1 + 3 * i, &color);
    }
    WS2812B_Refresh(); 
    vTaskDelay(3000);
    //
    color.r = 0;
    color.g = 0;
    color.b = 0;
    for (uint8_t i = 0; i < 5;  i++)
    {
         WS2812B_SetPixelColor(1 + 3 * i, &color);
    }
    WS2812B_Refresh();
    //vTaskDelay(3000);
}

void LIGHT_ShowWhite_LED1_Test(void)
{
    WS2812B_PixelClear();
    
    color_t color;
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
   
         WS2812B_SetPixelColor(1, &color);
    WS2812B_Refresh(); 
	
}

void LIGHT_ShowWhite_LED2_Test(void)
{
    WS2812B_PixelClear();
    
    color_t color;
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
   
         WS2812B_SetPixelColor(4, &color);
   
    WS2812B_Refresh(); 
	
}

void LIGHT_ShowWhite_LED3_Test(void)
{
    WS2812B_PixelClear();
    
    color_t color;
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
   
         WS2812B_SetPixelColor(7, &color);
    
    WS2812B_Refresh(); 
	
}

void LIGHT_ShowWhite_LED4_Test(void)
{
    WS2812B_PixelClear();
    
    color_t color;
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
    
         WS2812B_SetPixelColor(10, &color);
    
    WS2812B_Refresh(); 
	
}

void LIGHT_ShowWhite_LED5_Test(void)
{
    WS2812B_PixelClear();
    
    color_t color;
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
   
         WS2812B_SetPixelColor(13, &color);
    WS2812B_Refresh(); 
	
}
void LIGHT_ShowWhite_Test(void)
{
    WS2812B_PixelClear();
    
    color_t color;
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
    for (uint8_t i = 0; i < 5;  i++)
    {
         WS2812B_SetPixelColor(1 + 3 * i, &color);
    }
    WS2812B_Refresh(); 
	
}

void LIGHT_all_off_Test(void)
{
	color_t color;
    color.r = 0;
    color.g = 0;
    color.b = 0;
    for (uint8_t i = 0; i < 5;  i++)
    {
         WS2812B_SetPixelColor(1 + 3 * i, &color);
    }
    WS2812B_Refresh();

    //vTaskDelay(3000);
}
void LIGHT_Kokyu()
{    
    color_t color = {0};
	if(b_kokyu_led){
	 b_kokyu_led=false;
     vTaskDelay(100);
	 }
    b_kokyu_led = true;
	do{
    //
		for (uint8_t j = 0; j < 23; j ++) //2250ms;
		{
			color.r = lights_cartridges_blink_buffer[j];
			color.g = lights_cartridges_blink_buffer[j];
			color.b  = lights_cartridges_blink_buffer[j];
			WS2812B_SetPixelColor(1, &color);
            WS2812B_SetPixelColor(4, &color);
            WS2812B_SetPixelColor(7, &color);
            WS2812B_SetPixelColor(10, &color);
            WS2812B_SetPixelColor(13, &color);
			WS2812B_Refresh();
			if (j < 22)
			{
				vTaskDelay(100);
			}
		}
    }while(b_kokyu_led);
    
}

void LIGHT_Kokyu_slow()
{    
    color_t color = {0};
	if(b_kokyu_led){
	 b_kokyu_led=false;
     vTaskDelay(100);
	 }
    b_kokyu_led = true;
	do{
    //
		for (uint8_t j = 0; j < 64; j ++) //2250ms;
		{
			color.r = lights_cartridges_blink_buffer_show[j];
			color.g = lights_cartridges_blink_buffer_show[j];
			color.b  = lights_cartridges_blink_buffer_show[j];
			WS2812B_SetPixelColor(1, &color);
            WS2812B_SetPixelColor(4, &color);
            WS2812B_SetPixelColor(7, &color);
            WS2812B_SetPixelColor(10, &color);
            WS2812B_SetPixelColor(13, &color);
			WS2812B_Refresh();
			if (j < 63)
			{
				vTaskDelay(100);
			}
		}
    }while(b_kokyu_led);
    
}



#else

void LIGHT_ShowLowBattery(void)
{
    color_t color;
    //
    for (uint8_t i = 0; i < 4; i++)
    {
        color.r = light_standard_brightness;
        color.g = light_standard_brightness;
        color.b = 0;
        WS2812B_SetPixelColor(4, &color);
        WS2812B_Refresh();
        vTaskDelay(500);
        WS2812B_PixelClear();
        WS2812B_Refresh();
        vTaskDelay(500);
    }
}

void LIGHT_ShowFanGivesOffSmell(void)
{    
    color_t color;
    //
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
    WS2812B_SetPixelColor(0, &color);
    WS2812B_Refresh();
    vTaskDelay(6000);
        
//    WS2812B_PixelClear();
//    WS2812B_Refresh();
}

void LIGHT_ShowAirFanOn(uint8_t channel)
{    
    color_t color;
    //
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
    WS2812B_SetPixelColor(channel, &color);
    WS2812B_Refresh(); 
}

void LIGHT_ShowAirFanOff(uint8_t channel)
{    
    color_t color;
    //
    color.r = 0;
    color.g = 0;
    color.b = 0;
    WS2812B_SetPixelColor(channel, &color);
    WS2812B_Refresh();
    
    if (channel == 4)
    {
        if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
        {
            LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
        }
        else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
        {
            LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
        }
        else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
        {
            LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
        }
        else
        {
            ;
        }
    }
}

static const uint8_t lights_cartridges_blink_buffer[23] =
{
// timer_shaft   0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39
// interval time 100ms.   
/*LED*/          0x00,0x02,0x05,0x10,0x20,0x30,0x50,0x60,0x80,0x90,0xB0,0xFF,0xFF,0xB0,0x90,0x80,0x50,0x30,0x20,0x10,0x05,0x02,0x00
};

void LIGHT_ShowCartridgeDetach(uint8_t channel)
{    
    color_t color = {0};
    //
    for (uint8_t j = 0; j < 23; j ++) //2250ms;
    {
        color.r = lights_cartridges_blink_buffer[j];
        color.g = lights_cartridges_blink_buffer[j];
        WS2812B_SetPixelColor(channel, &color);
        WS2812B_Refresh();
        vTaskDelay(100);
    }
    
    uint8_t fan_status = FAN_GetStatus(channel);
    if (fan_status)
    {
        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + channel);
    }
    else
    {
        if (channel == 4)
        {
            if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
            }
            else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
            {
                LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
            }
            else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
            }
            else
            {
                ;
            }
        }
    }
}

void LIGHT_ShowCartridgeAttach(uint8_t channel)
{    
    color_t color = {0};
    //
    for (uint8_t j = 0; j < 23; j ++)
    {
        color.r = 0;
        color.g = lights_cartridges_blink_buffer[j];
        WS2812B_SetPixelColor(channel, &color);
        WS2812B_Refresh();
        vTaskDelay(100);
    }

    uint8_t fan_status = FAN_GetStatus(channel);
    if (fan_status)
    {
        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + channel);
    }
    else
    {
        if (channel == 4)
        {
            if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
            }
            else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
            {
                LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
            }
            else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
            }
            else
            {
                ;
            }
        }
    }
}

void LIGHT_ShowCartridgeExpiration(uint8_t channel)
{    
    color_t color = {0};
    //
    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 0; j < 23; j ++)
        {
            color.r = lights_cartridges_blink_buffer[j];
            color.g = lights_cartridges_blink_buffer[j];
            WS2812B_SetPixelColor(channel, &color);
            WS2812B_Refresh();
            vTaskDelay(43);
        }
    }

    uint8_t fan_status = FAN_GetStatus(channel);
    if (fan_status)
    {
        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + channel);
    }
    else
    {
        if (channel == 4)
        {
            if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
            }
            else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
            {
                LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
            }
            else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
            }
            else
            {
                ;
            }
        }
    }
}

void LIGHT_ShowBatteryDischarge(void)
{    
    uint8_t fan_status = FAN_GetStatus(4);
    if (fan_status)
    {
        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + 4);
    }
    else
    {
        color_t color;
        //
        color.r = 0;
        color.g = 0;
        color.b = 0;
        WS2812B_SetPixelColor(4, &color);
        WS2812B_Refresh();
    }
}

void LIGHT_ShowBatteryChargingIrregular(void)
{   
    uint8_t fan_status = FAN_GetStatus(4);
    if (fan_status)
    {
        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + 4);
    }
    else
    {
        color_t color;
        //
        for (uint8_t i = 0; i < 4; i++)
        {
            color.r = light_standard_brightness;
            color.g = 0;
            color.b = 0;
            WS2812B_SetPixelColor(4, &color);
            WS2812B_Refresh();
            vTaskDelay(500);
            WS2812B_PixelClear();
            WS2812B_Refresh();
            vTaskDelay(500);
        }
    }
}


void LIGHT_ShowBatteryChargingComplete(void)
{
    uint8_t fan_status = FAN_GetStatus(4);
    if (fan_status)
    {
        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + 4);
    }
    else
    {
        color_t color;
        color.r = 0;
        color.g = 0;
        color.b = light_standard_brightness;
        WS2812B_SetPixelColor(4, &color);
        WS2812B_Refresh();
        vTaskDelay(500);
    }
}

void LIGHT_ShowBatteryCharging(void)
{
    uint8_t fan_status = FAN_GetStatus(4);
    if (fan_status)
    {
        LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + 4);
    }
    else
    {
        color_t color;
        color.r = light_standard_brightness;
        color.g = 0;
        color.b = 0;
        WS2812B_SetPixelColor(4, &color);
        WS2812B_Refresh();
        vTaskDelay(500);
    }
}

void LIGHT_ShowBleBondsFailure(void)
{    
    color_t color;
    //
    for (uint8_t i = 0; i < 5; i++)
    {
        color.r = light_standard_brightness;
        color.g = light_standard_brightness;
        color.b = 0;
        for (uint8_t j = 0; j < 5; j++)
        {
            WS2812B_SetPixelColor(j, &color);
        }
        WS2812B_Refresh();
        vTaskDelay(500);
        //WS2812B_PixelClear();
        color.r = 0;
        color.g = 0;
        color.b = 0;
        WS2812B_FillColor(&color, 0, 5);
        WS2812B_Refresh();
        vTaskDelay(500);
    }
    
    //
    for (uint8_t i = 0; i < 5; i++)
    {
        uint8_t fan_status = FAN_GetStatus(i);
        if (fan_status)
        {
            LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + i);
        }
    }
    //
    if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
    {
        LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
    }
    else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
    {
        LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
    }
    else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
    {
        LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
    }
    else
    {
        ;
    }
}

void LIGHT_ShowBleBondsSuccess(void)
{    
    color_t color;
    //
    color.r = 0;
    color.g = 0;
    color.b = light_standard_brightness;
    for (uint8_t j = 0; j < 5; j++)
    {
        WS2812B_SetPixelColor(j, &color);
    }
    WS2812B_Refresh();
    vTaskDelay(1500);
    //
    color.r = 0;
    color.g = 0;
    color.b = 0;
    WS2812B_FillColor(&color, 0, 5);
    WS2812B_Refresh();
    
    //
    for (uint8_t i = 0; i < 5; i++)
    {
        uint8_t fan_status = FAN_GetStatus(i);
        if (fan_status)
        {
            LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + i);
        }
    }
    //
    if (bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
    {
        LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
    }
    else if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
    {
        LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
    }
    else if (bat.chargeState == BAT_CHARGE_STATUS_ERROR)
    {
        LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
    }
    else
    {
        ;
    }
}

void LIGHT_ShowBleAdvStart(void)
{    
    color_t color;
    color.r = 0;
    color.g = 0;
    color.b = light_standard_brightness;
    volatile uint8_t total_pixel_value = 0x01;
    uint16_t num = 5 * 3 - 1;
    do
    {
        for (uint16_t i = 0; (i < num) && b_light_show_is_ble_adv; i++)
        {
            //total_pixel_value &= 0x1f;
            uint32_t pixel32_bit_value = total_pixel_value;
            WS2812B_SetTotalPixel(pixel32_bit_value, 0, 5, &color);
            total_pixel_value = MATH_ALGO_u5_rol_(total_pixel_value, 1);
            WS2812B_Refresh();
            vTaskDelay(100);
        }
    }
    while(b_light_show_is_ble_adv);
    
    //
    if ((light_sequence_current_set_type >= LIGHT_BAT_DISCHARGE) && 
        (light_sequence_current_set_type <= LIGHT_BAT_POWER_LOW))
    {
        LIGHT_SequenceSet(LIGHT_BLE_ADV_START);
    }
    
    //
    if ((light_sequence_current_set_type >= LIGHT_CARTRIDGE_ATTACH_1) && 
        (light_sequence_current_set_type <= LIGHT_CARTRIDGE_ATTACH_5))
    {
        LIGHT_SequenceSet(LIGHT_BLE_ADV_START);
    }
    
    //
    if ((light_sequence_current_set_type >= LIGHT_CARTRIDGE_DETACH_1) && 
        (light_sequence_current_set_type <= LIGHT_CARTRIDGE_DETACH_5))
    {
        LIGHT_SequenceSet(LIGHT_BLE_ADV_START);
    }
    
    //
    if ((light_sequence_current_set_type >= LIGHT_CARTRIDGE_ECPIRATION_1) && 
        (light_sequence_current_set_type <= LIGHT_CARTRIDGE_ECPIRATION_5))
    {
        LIGHT_SequenceSet(LIGHT_BLE_ADV_START);
    }
    
    
    LIGHT_DMSG("<LIGHT> BLE_Adv break up with =%d.\r\n", light_sequence_current_set_type);
    //WS2812B_PixelClear();
    color.r = 0;
    color.g = 0;
    color.b = 0;
    WS2812B_FillColor(&color, 0, 5);
    WS2812B_Refresh();
   // vTaskDelay(1850);
}

//void LIGHT_ShowBleAdvStart(void)
//{    
//    color_t color;
//    color.r = 0;
//    color.g = 0;
//    color.b = light_standard_brightness;
//    volatile uint8_t total_pixel_value = 0x01;
//    uint16_t num = 5 * 3 - 1;
//    for (uint16_t i = 0; i < num; i++)
//    {
//        total_pixel_value = MATH_ALGO_u5_rol_(total_pixel_value, 1);
//        WS2812B_SetTotalPixel(total_pixel_value, 0, 5, &color);
//        WS2812B_Refresh();
//        vTaskDelay(300);
//    }
//    WS2812B_PixelClear();
//    WS2812B_Refresh();
//    vTaskDelay(1850);
//}
//SemaphoreHandle_t xSemaphore;

//  
void LIGHT_ShowKnightRiderlights2(void)
{
    int8_t time_shaft = 0;
    uint8_t b_direct = 0; 
    uint8_t b_loop = 1;
    
    color_t color = {0};
   
    while(b_loop)
    {
        for (uint8_t index = 0; index < 5; index++ )
        {
            color.r = KnightRiderlights_buffer[index][time_shaft];
            color.g = KnightRiderlights_buffer[index][time_shaft];
            color.b = KnightRiderlights_buffer[index][time_shaft];
            
            WS2812B_SetPixelColor(index, &color); // 
        }
        
        WS2812B_Refresh(); // update
        
        vTaskDelay(75);
        
        if (b_direct == 0)
        {
            time_shaft++;
            if (time_shaft >= (16 - 1)) 
            {
                time_shaft = 16 - 1;
                b_direct = 1;
            }
        }
        else
        {
            time_shaft--;
            if (time_shaft < 0) 
            {
                time_shaft = 0;
                b_direct = 0;
                b_loop = 0;
            }
        }
    }
}


void LIGHT_ShowSystemStartup(void)
{ 
    WS2812B_PixelClear();
    
    LIGHT_ShowKnightRiderlights2();
    vTaskDelay(500);
    
    color_t color;
    //
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
    WS2812B_FillColor(&color, 0, 5);
    WS2812B_Refresh();
    vTaskDelay(2000);
    
    //
    //WS2812B_PixelClear();
    color.r = 0;
    color.g = 0;
    color.b = 0;
    WS2812B_FillColor(&color, 0, 5);
    WS2812B_Refresh();
}

void LIGHT_ShowSystemShutdown(void)
{
    WS2812B_PixelClear();
    
    color_t color;
    color.r = light_standard_brightness;
    color.g = light_standard_brightness;
    color.b = light_standard_brightness;
    WS2812B_FillColor(&color, 0, 5);
    WS2812B_Refresh(); 
    vTaskDelay(3000);
    //
    //WS2812B_PixelClear();
    color.r = 0;
    color.g = 0;
    color.b = 0;
    WS2812B_FillColor(&color, 0, 5);
    WS2812B_Refresh();
    //vTaskDelay(3000);
}

#endif




/*
 * Set Light sequence scheduler type
 */
void LIGHT_SequenceScheduler(void)
{
    uint8_t light_sequence_type = 0;

    uint32_t err_code = app_fifo_get(&light_sequence_fifo, &light_sequence_type);
    if (err_code == NRF_SUCCESS)
    {
        LIGHT_DMSG("<LIGHT> get light_sequence_type=%d.\r\n", light_sequence_type);
        
//        if ((light_sequence_type >= LIGHT_BAT_DISCHARGE) && (light_sequence_type <= LIGHT_BAT_CHARGING_IRREGULAR)
//            && 
//            ((light_sequence_last_type < LIGHT_BAT_DISCHARGE) || 
//            (light_sequence_last_type > LIGHT_BAT_CHARGING_IRREGULAR))
//        )
//        {
//            LIGHT_DMSG("<LIGHT> return,because_last_type=%d.\r\n", light_sequence_last_type);
//            return;
//        }
        if (light_sequence_type == LIGHT_BLE_ADV_START) // 3
        {
            if (f.BLE_CONNECT_STATUS == BLE_STATUS_STOP)  
            {
                return;
            }
            
            if (light_sequence_last_type == LIGHT_BLE_ADV_START)
            {
                LIGHT_DMSG("<LIGHT> light_sequence_last_type == LIGHT_BLE_ADV_START.\r\n");
                return;
            }
        }
        light_sequence_last_type = light_sequence_type;
        
        switch(light_sequence_type)
        {
            case LIGHT_OFF:
                LIGHT_TurnOff();
                break;
            
            case LIGHT_SYSTEM_STARTUP:
                {
                    LIGHT_ShowSystemStartup();
                    
                    uint8_t resp_pkg[16];
                    uint8_t param_data = PARAM_NOTIFY_POWER_ON;
                    uint16_t param_data_len = sizeof(param_data) / sizeof(uint8_t);

                    uint16_t resp_pkg_len = XTCP_MakePackage(CMD_NOTIFY, CMD_NOTIFY_POWER_STATUS, param_data_len, &param_data, resp_pkg);
                    err_code = UART_PacketSend(resp_pkg, resp_pkg_len, NULL);
                    err_code = BLE_PacketSend(resp_pkg, resp_pkg_len, NULL);
                    KEY_FIFO_Flush();
                    f.SYSTEM_RUN_STATUS = SYSTEM_RUN_STATUS_NORMAL;
                }
                break;
                
            case LIGHT_SYSTEM_SHUTDOWN:
                {
                    uint8_t resp_pkg[16];
                    uint8_t param_data = PARAM_NOTIFY_POWER_OFF;
                    uint16_t param_data_len = sizeof(param_data) / sizeof(uint8_t);
                    
                    uint16_t resp_pkg_len = XTCP_MakePackage(CMD_NOTIFY, CMD_NOTIFY_POWER_STATUS, param_data_len, &param_data, resp_pkg);
                    err_code = UART_PacketSend(resp_pkg, resp_pkg_len, NULL);
                    err_code = BLE_PacketSend(resp_pkg, resp_pkg_len, NULL);
                    
                    LIGHT_ShowSystemShutdown();
                    
                    env.cfg.reboot_status = 0x00;
                    ENV_Save();
                    
                    SYSTEM_PowerOff();
                    f.SYSTEM_RUN_STATUS = SYSTEM_RUN_STATUS_POWER_OFF;
                    delay_ms(2000);
                    NVIC_SystemReset();
                }
                break;
                
            case LIGHT_BLE_ADV_START:
                //LIGHT_ShowBleAdvStart();
                break;
            
            case LIGHT_BLE_BONDS_SUCCESS:
                //LIGHT_ShowBleBondsSuccess();
                break;
            
            case LIGHT_BLE_BONDS_FAILURE:
                //LIGHT_ShowBleBondsFailure();
                break;
            
            //-- BAT
            case LIGHT_BAT_DISCHARGE:
               // LIGHT_ShowBatteryDischarge();
                break;
             
            case LIGHT_BAT_CHARGING:
                //LIGHT_ShowBatteryCharging();
                break;
            
            case LIGHI_BAT_CHARGING_COMPLETE:
                //LIGHT_ShowBatteryChargingComplete();
                break;
            
            case LIGHT_BAT_CHARGING_IRREGULAR:
                //LIGHT_ShowBatteryChargingIrregular();
                break;
            
            case LIGHT_BAT_POWER_LOW:
                //LIGHT_ShowLowBattery();
                break;
            
            //-- CARTRIDGE_ECPIRATION
            case LIGHT_CARTRIDGE_ECPIRATION_1:
                //LIGHT_ShowCartridgeExpiration(0);
                break;
            
            case LIGHT_CARTRIDGE_ECPIRATION_2:
                //LIGHT_ShowCartridgeExpiration(1);
                break;
            
            case LIGHT_CARTRIDGE_ECPIRATION_3:
                //LIGHT_ShowCartridgeExpiration(2);
                break;
            
            case LIGHT_CARTRIDGE_ECPIRATION_4:
                //LIGHT_ShowCartridgeExpiration(3);
                break;
            
            case LIGHT_CARTRIDGE_ECPIRATION_5:
                //LIGHT_ShowCartridgeExpiration(4);
                break;
            
            //-- CARTRIDGE_ATTACH
            case LIGHT_CARTRIDGE_ATTACH_1:
                //LIGHT_ShowCartridgeAttach(0);
                break;
            
            case LIGHT_CARTRIDGE_ATTACH_2:
                //LIGHT_ShowCartridgeAttach(1);
                break;
            
            case LIGHT_CARTRIDGE_ATTACH_3:
                //LIGHT_ShowCartridgeAttach(2);
                break;
            
            case LIGHT_CARTRIDGE_ATTACH_4:
                //LIGHT_ShowCartridgeAttach(3);
                break;
            case LIGHT_CARTRIDGE_ATTACH_5:
                //LIGHT_ShowCartridgeAttach(4);
                break;
            
            //-- CARTRIDGE_DETACH
            case LIGHT_CARTRIDGE_DETACH_1:
                //LIGHT_ShowCartridgeDetach(0);
                break;
            
            case LIGHT_CARTRIDGE_DETACH_2:
                //LIGHT_ShowCartridgeDetach(1);
                break;
            
            case LIGHT_CARTRIDGE_DETACH_3:
                //LIGHT_ShowCartridgeDetach(2);
                break;
            
            case LIGHT_CARTRIDGE_DETACH_4:
               // LIGHT_ShowCartridgeDetach(3);
                break;
            
            case LIGHT_CARTRIDGE_DETACH_5:
                //LIGHT_ShowCartridgeDetach(4);
                break;
            
            //-- AIR_FAN_TURN_ON
            case LIGHT_AIR_FAN1_TURN_ON:
                LIGHT_ShowAirFanOn(0);
                break;
            
            case LIGHT_AIR_FAN2_TURN_ON:
                LIGHT_ShowAirFanOn(1);
                break;
            
            case LIGHT_AIR_FAN3_TURN_ON:
                LIGHT_ShowAirFanOn(2);
                break;
            
            case LIGHT_AIR_FAN4_TURN_ON:
                LIGHT_ShowAirFanOn(3);
                break;
            
            case LIGHT_AIR_FAN5_TURN_ON:
                LIGHT_ShowAirFanOn(4);
                break;
            
            //-- AIR_FAN_TURN_OFF
            case LIGHT_AIR_FAN1_TURN_OFF:
                LIGHT_ShowAirFanOff(0);
                break;
            
            case LIGHT_AIR_FAN2_TURN_OFF:
                LIGHT_ShowAirFanOff(1);
                break;
            
            case LIGHT_AIR_FAN3_TURN_OFF:
                LIGHT_ShowAirFanOff(2);
                break;
            
            case LIGHT_AIR_FAN4_TURN_OFF:
                LIGHT_ShowAirFanOff(3);
                break;
            
            case LIGHT_AIR_FAN5_TURN_OFF:
                LIGHT_ShowAirFanOff(4);
                break;
            
            case LIGHT_FAN_GIVES_OFF_SMELL:
                //LIGHT_ShowFanGivesOffSmell();
                break;
            //////////////////////////
            
            case LIGHT_TURN_ON_WITH_THE_SETTING_COLOR:
                LIGHT_TurnOnWithTheSettingColor();
                break;
            

			case TEST1:
            	 LIGHT_ShowWhite_LED1_Test();
				   break;

			case TEST2:
				LIGHT_ShowWhite_LED2_Test();
						   break;
			case TEST3:
				LIGHT_ShowWhite_LED3_Test();
						   break;
			case TEST4:
					LIGHT_ShowWhite_LED4_Test();
						   break;
			case TEST5:
					LIGHT_ShowWhite_LED5_Test();
						   break;
			case TEST6:
				LIGHT_ShowWhite_Test();
						   break;
			case TEST7:
				LIGHT_Kokyu();
							   break;
		   case TEST8:
				LIGHT_Kokyu_slow();
							   break;
		   case TEST9:
				LIGHT_all_off_Test();
				for(int i =0;i<5;i++) FAN_SetPinOff(i);
				FAN_SetSpeed(0,0xff);
							   break;
		  case TEST10:
				for(int i =0;i<5;i++) FAN_SetPinOff(i);
				FAN_SetSpeed(1,0xff);
						   break;
		 case TEST11:
				for(int i =0;i<5;i++) FAN_SetPinOff(i);
				FAN_SetSpeed(2,0xff);
						   break;
		case TEST12:
				for(int i =0;i<5;i++) FAN_SetPinOff(i);
				FAN_SetSpeed(3,0xff);
				break;
		case TEST13:
        for(int i =0;i<5;i++) FAN_SetPinOff(i);
				FAN_SetSpeed(4,0xff);
		        
            	break;
		
		case TEST14:
        for(int i =0;i<5;i++) FAN_SetPinOff(i);
				LIGHT_all_off_Test();
		      	
        		break;
            default:
                LIGHT_DMSG("<LIGHT> not the light sequence type.\r\n");
                break;
        }
    }
}

static uint8_t b_light_init = 0;

void LIGHT_Init(void)
{   
    if (b_light_init == 0)
    {
        b_light_init = 1;
        LIGHT_SequenceInit();
        WS2812B_Init();    
    }

}

void LIGHT_DeInit(void)
{
    if (b_light_init)
    {
        b_light_init = 0;
        WS2812B_DeInit();
    }
}



/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
