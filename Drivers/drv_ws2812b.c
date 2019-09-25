/**
  ******************************************************************************
  * @file    drv_ws2812b.c
  * @author  Merle
  * @version V1.0.0
  * @date    11-July-2019
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "nrf_drv_i2s.h"
#include "nrf_i2s.h"

#define DBG_WS2812B
#ifdef DBG_WS2812B
#define WS2812B_DMSG	RTT_Printf
#else
#define WS2812B_DMSG(...)
#endif

#define I2S_SDOUT_PIN     BOARD_I2S_SDOUT_PIN
#define I2S_SCK_PIN       BOARD_I2S_SCK_PIN

static volatile bool i2s_xfer_done = true;  /**< Flag used to indicate that SPI instance completed the transfer. */
static uint8_t b_direct = 0;
static uint8_t b_i2s_init = 0;

static SemaphoreHandle_t i2s_xSemaphore;

static uint32_t I2S_SendBuf(uint8_t *p_send_buf, uint32_t len)
{ 
	ret_code_t ret;
	
	if (b_i2s_init)
	{
        xSemaphoreTake(i2s_xSemaphore, 800000);//portMAX_DELAY
        b_direct = 0;
        i2s_xfer_done = false;
        nrf_drv_i2s_buffers_t const initial_buffers = {
                                                .p_tx_buffer = (uint32_t*)p_send_buf,
                                                .p_rx_buffer = NULL
                                                };
        uint16_t send_len = len / 4;
        ret = nrf_drv_i2s_start(&initial_buffers, send_len , 0);
        APP_ERROR_CHECK(ret);
        while (!i2s_xfer_done) 
        {
            __WFE();
        }
        
        xSemaphoreGive(i2s_xSemaphore);
	}

    return ret;
}


static void i2s_data_handler(nrf_drv_i2s_buffers_t const * p_released,
                         uint32_t                      status)
{
    // 'nrf_drv_i2s_next_buffers_set' is called directly from the handler
    // each time next buffers are requested, so data corruption is not
    // expected.
    ASSERT(p_released);

    // When the handler is called after the transfer has been stopped
    // (no next buffers are needed, only the used buffers are to be
    // released), there is nothing to do.
    if (!(status & NRFX_I2S_STATUS_NEXT_BUFFERS_NEEDED))
    {
        return;
    }

    // First call of this handler occurs right after the transfer is started.
    // No data has been transferred yet at this point, so there is nothing to
    // check. Only the buffers for the next part of the transfer should be
    // provided.
    if (!p_released->p_rx_buffer)
    {
        
        if (b_direct == 0)
        {
            b_direct = 1;
        }
        else
        {
            b_direct = 0;
        }
    }
    else
    {  
        if (b_direct == 0)
        {
            b_direct = 1;
        }
        else
        {
            b_direct = 0;
            
            nrf_drv_i2s_stop();
            i2s_xfer_done = true;
        }
    }
}



static void I2S_Init(void)
{
    if (b_i2s_init == 0)
    {
        static uint8_t b_i2s_semaphore_create = 0;
        if (b_i2s_semaphore_create == 0)
        {
            i2s_xSemaphore = xSemaphoreCreateMutex();
            if( i2s_xSemaphore != NULL )
            {
                /* The semaphore was created successfully and
                can be used. */
                b_i2s_semaphore_create = 1;
                
                WS2812B_DMSG("<I2S> i2s_SemaphoreCreateMutex OK.\r\n");
            }
            else
            {
                WS2812B_DMSG("<WS2812B> i2s_SemaphoreCreateMutex Failure\r\n");
                delay_ms(10);
                APP_ERROR_CHECK(1);
            }
        }
        
        uint32_t err_code;
        nrf_drv_i2s_config_t config = NRF_DRV_I2S_DEFAULT_CONFIG;
        // In Master mode the MCK frequency and the MCK/LRCK ratio should be
        // set properly in order to achieve desired audio sample rate (which
        // is equivalent to the LRCK frequency).
        // For the following settings we'll get the LRCK frequency equal to
        // 15873 Hz (the closest one to 16 kHz that is possible to achieve).
        
        config.sck_pin = I2S_SCK_PIN;
        config.sdin_pin  = NRF_DRV_I2S_PIN_NOT_USED;//2S_SDIN_PIN;
        config.sdout_pin = I2S_SDOUT_PIN;
        config.lrck_pin = NRF_DRV_I2S_PIN_NOT_USED;
        config.mck_pin = NRF_DRV_I2S_PIN_NOT_USED;
        config.mck_setup = NRF_I2S_MCK_32MDIV6;
        config.ratio     = NRF_I2S_RATIO_32X;
        config.channels  = NRF_I2S_CHANNELS_STEREO;
        //config.irq_priority = APP_IRQ_PRIORITY_HIGH;
        config.mode         = NRF_I2S_MODE_MASTER;
        config.format       = NRF_I2S_FORMAT_I2S;
        config.alignment    = NRF_I2S_ALIGN_LEFT;
        config.sample_width = NRF_I2S_SWIDTH_8BIT;

        err_code = nrf_drv_i2s_init(&config, i2s_data_handler);
        APP_ERROR_CHECK(err_code);

        //
        WS2812B_DMSG("<I2S> Init.\r\n");
        
        b_i2s_init = 1;  
    }
}
 
static void I2S_DeInit(void)
{
    if (b_i2s_init)
    {
        b_i2s_init = 0; 

        // stop transfer
        nrf_drv_i2s_stop();

        // un-initialize i2s
        nrf_drv_i2s_uninit();
        
        WS2812B_DMSG("<I2S> DeInit.\r\n");
    }
}

/*
    g7,g6,g5,g4,g3,g2,g1,g0,r7,r6,r5,r4,r3,r2,r1,r0,b7,b6,b5,b4,b3,b2,b1,b0
    \_____________________________________________________________________/
                           |      _________________...
                           |     /   __________________...
                           |    /   /   ___________________...
                           |   /   /   /
                          RGB,RGB,RGB,RGB,...,STOP
*/

/*
TH+TL =12.5us+-600ns
    Hi  + Lo
0 = 0.4 + 0.85us // 100
1 = 0.8 + 0.45us // 110
low voltage time above 50us // 50 / 1.25 = 40 -> 40 *3bit /8 =15bytes   -> head + tail = 2* 15 = 32字节 .

// 9byte * 500ns/1bit * 8bit/1byte -> 点亮一个灯花费 9 *4us = 36us, 点亮20个灯*36us = 720us + (32 * 4uS) = 720 + 128 = 848us
*/


#define BIT_HI  6 // 110, Bit pattern for data "1".
#define BIT_LO	4 // 100, Bit pattern for data "0" .

typedef struct {
	uint32_t other :  8;
	uint32_t b7 : 3; 
	uint32_t b6 : 3;	
	uint32_t b5 : 3;		
	uint32_t b4 : 3;		
	uint32_t b3 : 3;			
	uint32_t b2 : 3;		
	uint32_t b1 : 3;		
	uint32_t b0 : 3; // 0 or 1
} u32_3bit_magic_t;

typedef union { 
	uint32_t word;
    u32_3bit_magic_t  pixel;
	uint8_t byte[4];
} u32_pixel_magic_t;

static uint8_t u32_pixel_magic_buf[4];
static u32_pixel_magic_t *ws2812b_magic_num = (u32_pixel_magic_t *)u32_pixel_magic_buf;


#define WS2812B_PIXEL_NUM	 (15U)//(18U)//20 //9 20


#define WS2812B_FRAME_RESET_SIZE    (64U)
#define WS2812B_PIXEL_BUFFER_SIZE	 (16 + (9 * WS2812B_PIXEL_NUM) + WS2812B_FRAME_RESET_SIZE)
static uint8_t ws2812b_pixel_buf[WS2812B_PIXEL_BUFFER_SIZE] = {0}; 

static uint8_t b_ws2812b_init = 0;
static SemaphoreHandle_t ws2812b_xSemaphore;    // 4bytes.
//static StaticSemaphore_t ws2812b_xMutexBuffer;  // 72bytes


uint8_t WS2812B_GetPixelNum(void)
{
    uint8_t pixel_num = WS2812B_PIXEL_NUM;
    return pixel_num;
}

/*!
 @param   pixel_index  Index of pixel to set (0 = first).
*/
void WS2812B_SetPixelColor(uint16_t pixel_index, color_t const* p_color)
{ 
    xSemaphoreTake(ws2812b_xSemaphore, 8000);//portMAX_DELAY
    
    if (pixel_index >= WS2812B_PIXEL_NUM)
    {
        xSemaphoreGive(ws2812b_xSemaphore);
        return;
    }
    
    uint16_t pixel_indexX = WS2812B_PIXEL_NUM - 1 - pixel_index; // 0~14
    
    uint16_t array_index = 16 + pixel_indexX * 9;

    uint8_t color_array[3] = {p_color->g, p_color->r, p_color->b};

    for (uint16_t i = 0; i < 3; i++)
    {
        uint16_t bit_index = i * 3;

        ws2812b_magic_num->pixel.b0 = (color_array[i] & 0x80) ? BIT_HI : BIT_LO;
        ws2812b_magic_num->pixel.b1 = (color_array[i] & 0x40) ? BIT_HI : BIT_LO;
        ws2812b_magic_num->pixel.b2 = (color_array[i] & 0x20) ? BIT_HI : BIT_LO;
        ws2812b_magic_num->pixel.b3 = (color_array[i] & 0x10) ? BIT_HI : BIT_LO;
        ws2812b_magic_num->pixel.b4 = (color_array[i] & 0x08) ? BIT_HI : BIT_LO;
        ws2812b_magic_num->pixel.b5 = (color_array[i] & 0x04) ? BIT_HI : BIT_LO;
        ws2812b_magic_num->pixel.b6 = (color_array[i] & 0x02) ? BIT_HI : BIT_LO;
        ws2812b_magic_num->pixel.b7 = (color_array[i] & 0x01) ? BIT_HI : BIT_LO;

        ws2812b_pixel_buf[array_index + 0 + bit_index] = ws2812b_magic_num->byte[3];
        ws2812b_pixel_buf[array_index + 1 + bit_index] = ws2812b_magic_num->byte[2];
        ws2812b_pixel_buf[array_index + 2 + bit_index] = ws2812b_magic_num->byte[1];
    }
    
    xSemaphoreGive(ws2812b_xSemaphore);
}


/*!
 @param   pixel_index  Index of pixel to read (0 = first).
*/
void WS2812B_GetPixelColor(uint16_t pixel_index, color_t* p_color)
{ 
    if (pixel_index >= WS2812B_PIXEL_NUM)
    {
        return;
    }
	
    uint16_t pixel_indexX = WS2812B_PIXEL_NUM - 1 - pixel_index; // 0~14
    uint16_t array_index = 16 + pixel_indexX * 9;
    
    uint8_t color_array[3] = {0, 0, 0};
    
    for (uint16_t i = 0; i < 3; i++)
    {
        uint16_t bit_index = i * 3;

        ws2812b_magic_num->byte[3] = ws2812b_pixel_buf[array_index + 0 + bit_index];
        ws2812b_magic_num->byte[2] = ws2812b_pixel_buf[array_index + 1 + bit_index];
        ws2812b_magic_num->byte[1] = ws2812b_pixel_buf[array_index + 2 + bit_index];

        color_array[i] |= (ws2812b_magic_num->pixel.b0 == BIT_HI) ? 0x80 : 0x00;
        color_array[i] |= (ws2812b_magic_num->pixel.b1 == BIT_HI) ? 0x40 : 0x00;
        color_array[i] |= (ws2812b_magic_num->pixel.b2 == BIT_HI) ? 0x20 : 0x00;
        color_array[i] |= (ws2812b_magic_num->pixel.b3 == BIT_HI) ? 0x10 : 0x00;
        color_array[i] |= (ws2812b_magic_num->pixel.b4 == BIT_HI) ? 0x08 : 0x00;
        color_array[i] |= (ws2812b_magic_num->pixel.b5 == BIT_HI) ? 0x04 : 0x00;
        color_array[i] |= (ws2812b_magic_num->pixel.b6 == BIT_HI) ? 0x02 : 0x00;
        color_array[i] |= (ws2812b_magic_num->pixel.b7 == BIT_HI) ? 0x01 : 0x00;
    }
    
    p_color->g = color_array[0];
    p_color->r = color_array[1]; 
    p_color->b = color_array[2];
}

static uint8_t b_ws2812b_power_on = 0;

void WS2812B_PowerTurnOn(void)
{
    nrf_gpio_pin_set(BOARD_LED_POWER_CTL);
    b_ws2812b_power_on = 1;
    //WS2812B_DMSG("<WS2812B> ws2812b_PowerTurnOn.\r\n");
}

void WS2812B_PowerTurnOff(void)
{
    nrf_gpio_pin_clear(BOARD_LED_POWER_CTL);
    
    b_ws2812b_power_on = 0;
    WS2812B_DMSG("<WS2812B> ws2812b_PowerTurnOff.\r\n");
}

uint8_t WS2812B_GetPowerStatus(void)
{
    uint8_t status = b_ws2812b_power_on;
    return status;
}

bool WS2812B_CheckPixelColorIsAllBalck(void)
{
    bool ret = false;
    volatile uint16_t b_black_led_num = 0;
    
    for (uint16_t i = 0; i < WS2812B_PIXEL_NUM; i++)
    {
        uint16_t bit_index = i * 9;
        if ((ws2812b_pixel_buf[16 + bit_index] == 0x92) &&  // 0b100 100 10
            (ws2812b_pixel_buf[17 + bit_index] == 0x49) &&  // 0b0 100 100 1
            (ws2812b_pixel_buf[18 + bit_index] == 0x24) &&   // 0b00 100 100
        
            (ws2812b_pixel_buf[19 + bit_index] == 0x92) &&  // 0b100 100 10
            (ws2812b_pixel_buf[20 + bit_index] == 0x49) &&  // 0b0 100 100 1
            (ws2812b_pixel_buf[21 + bit_index] == 0x24) &&   // 0b00 100 100
        
            (ws2812b_pixel_buf[22 + bit_index] == 0x92) &&  // 0b100 100 10
            (ws2812b_pixel_buf[23 + bit_index] == 0x49) &&  // 0b0 100 100 1
            (ws2812b_pixel_buf[24 + bit_index] == 0x24))    // 0b00 100 100
        
        {
            b_black_led_num++;
        } 
    }

    if (b_black_led_num == WS2812B_PIXEL_NUM)
    {
        //WS2812B_PowerTurnOff();
        //WS2812B_DMSG("<WS2812B> All RGB_LED Is black.\r\n");
        ret = true;
    }
    
    return ret;
}

void WS2812B_Refresh(void)
{
    //WS2812B_FrameReset();
    xSemaphoreTake(ws2812b_xSemaphore, 800000);//portMAX_DELAY //8000
    
    if (b_ws2812b_power_on == 0)
    {
        WS2812B_PowerTurnOn();
        delay_ms(5);//10
    }
    
    uint32_t ret_code;

    ret_code = I2S_SendBuf(ws2812b_pixel_buf, WS2812B_PIXEL_BUFFER_SIZE);
    
    if (ret_code != NRF_SUCCESS)
    {
       WS2812B_DMSG("<I2S> SendBuf Fault.\r\n");
    }
    //WS2812B_DMSG("<I2S> WS2812B_Refresh.\r\n");
    
    xSemaphoreGive(ws2812b_xSemaphore);
}


void WS2812B_FrameReset(void)
{
    uint32_t ret_code;
    //uint8_t buf[4 * 4] = {0};
    ret_code = I2S_SendBuf(ws2812b_pixel_buf, 4);
    delay_ms(1);
}


void WS2812B_PixelClear(void)
{
    for (uint8_t i = 0; i < WS2812B_PIXEL_NUM; i++)
    {
        color_t color = {.r = 0, .b = 0, .g = 0};
        WS2812B_SetPixelColor(i, &color);
    }
}

/*!
  @brief   Fill all or part of the NeoPixel strip with a color.
  @param   color  rgb color value. Most significant byte is white (for
                  RGBW pixels) or ignored (for RGB pixels), next is red,
                  then green, and least significant byte is blue. If all
                  arguments are unspecified, this will be 0 (off).
  @param   first  Index of first pixel to fill, starting from 0. Must be
                  in-bounds, no clipping is performed. 0 if unspecified.
  @param   count  Number of pixels to fill, as a positive value. Passing
                  0 or leaving unspecified will fill to end of strip.
  @return 
*/
void WS2812B_FillColor(color_t const* p_color, uint16_t first, uint16_t count)
{
    uint16_t i, end;
    if (first >= WS2812B_PIXEL_NUM)
    {
        WS2812B_DMSG("first Num is %d \r\n");
        return ;  // If first LED is past end of strip, nothing to do
    }
    
    // Calculate the index ONE AFTER the last pixel to fill
    if(count == 0) 
    {
        // Fill to end of strip
        end = WS2812B_PIXEL_NUM;
    } 
    else 
    {
        // Ensure that the loop won't go past the last pixel
        end = first + count;
        if(end > WS2812B_PIXEL_NUM) 
        {
            end = WS2812B_PIXEL_NUM;
        }
    }

    for(i = first; i < end; i++) 
    {
        WS2812B_SetPixelColor(i, p_color);
    }
}


void WS2812B_SetTotalPixel(uint32_t total_pixel_value, uint8_t start_index, uint8_t end_index, color_t const* p_color)
{
    color_t color_clear = {0,0,0};
    for (uint8_t i = start_index; i < end_index; i ++)
    {
        if (total_pixel_value & (0x00000001 << i))
        {
            WS2812B_SetPixelColor(i, p_color);
        }
        else
        {
            //color_t color_clear = {0,0,0};
            WS2812B_SetPixelColor(i, &color_clear);
        }
    }
}



void WS2812B_DeInit(void)
{ 
    if (b_ws2812b_init)
    {
        b_ws2812b_init = 0;
        
        WS2812B_PixelClear();
        WS2812B_Refresh();
        
        I2S_DeInit();
        
        WS2812B_PowerTurnOff();
    }
}

void WS2812B_Init(void)
{
    ct_assert(sizeof(SemaphoreHandle_t) < (5)); //72
    
    if (b_ws2812b_init == 0)
    {
        b_ws2812b_init = 1;
        
        WS2812B_PowerTurnOn();
        delay_ms(20);
        
        I2S_Init();
        
        static uint8_t b_ws2812b_semaphore_create = 0;
        if (b_ws2812b_semaphore_create == 0)
        {
            ws2812b_xSemaphore = xSemaphoreCreateMutex();
            if( i2s_xSemaphore != NULL )
            {
                /* The semaphore was created successfully and
                can be used. */
                b_ws2812b_semaphore_create = 1;
                
                WS2812B_DMSG("<WS2812B> ws2812b_xSemaphoreCreateMutex OK.\r\n");
            }
            else
            {
                WS2812B_DMSG("<WS2812B> ws2812b_xSemaphoreCreateMutex Failure\r\n");
                delay_ms(10);
                APP_ERROR_CHECK(1);
            }
        }

        WS2812B_FrameReset();
       
        for (int i = 0; i < WS2812B_FRAME_RESET_SIZE; i++)
        {
            ws2812b_pixel_buf[i] = 0;
            ws2812b_pixel_buf[WS2812B_PIXEL_BUFFER_SIZE - WS2812B_FRAME_RESET_SIZE + i] = 0;
        }
        
        WS2812B_PixelClear();

        WS2812B_Refresh();

        WS2812B_DMSG("<WS2812B> Init.\r\n");
    }
}


/******************* (C) COPYRIGHT 2019 *****END OF FILE****/
