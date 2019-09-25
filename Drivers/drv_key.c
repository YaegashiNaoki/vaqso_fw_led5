/**
  ******************************************************************************
  * @file    drv_key.c
  * @author  Merle
  * @version V1.0.0
  * @date    12-Octobal-2016
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"

//#define DBG_KEY
#ifdef DBG_KEY
#define KEY_DMESG		RTT_Printf
#else
#define KEY_DMESG(...)
#endif


#define BUTTON_1_PIN                BOARD_KEY

#define BUTTONn  1

typedef enum
{ 
	BUTTON_1N = 0,	// The first must be 0.
} Button_TypeDef;


// 最多5个按键
#define DOWN_CLICK(buttonX)			(1 	<< (buttonX << 3))	//(1 << (8 * buttonX))
#define UP_CLICK(buttonX)			(2 	<< (buttonX << 3))	//(2 << (8 * buttonX))
//#define SINGLE_CLICK(buttonX)		(4 	<< (buttonX << 3))	//(1 << (8 * buttonX))
//#define DOUBLE_CLICK(buttonX)		(8 	<< (buttonX << 3))	//(2 << (8 * buttonX))
#define LONG_LONG_PRESS(buttonX)	(16 << (buttonX << 3))	//(4 << (8 * buttonX))
#define LONG_PRESS(buttonX)			(32 << (buttonX << 3))	//(8 << (8 * buttonX))


const uint16_t BUTTON_PIN[BUTTONn] = {
	BUTTON_1_PIN,
}; 

const uint8_t BUTTON_VALUE[BUTTONn] = {
	0x02,
}; 



static keyFIFO_t s_tKey;			/* 按键FIFO变量,结构体 */
static uint8_t b_key_init = 0;		/* 按键初始化标志 */

/*
*********************************************************************************************************
*	函 数 名: KEY_Put
*	功能说明: 将1个键值压入按键FIFO缓冲区。可用于模拟一个按键。
*	形    参: key : 按键代码
*	返 回 值: 无
*********************************************************************************************************
*/
static void KEY_Put(uint8_t key)
{
	s_tKey.Buf[s_tKey.Write] = key;
    
	if ((++s_tKey.Write) >= KEY_FIFO_SIZE)
	{
		s_tKey.Write = 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: KEY_Get
*	功能说明: 从按键FIFO缓冲区读取一个键值。
*	形    参:  无
*	返 回 值: 按键代码
*********************************************************************************************************
*/
uint8_t KEY_Get(void)
{
	uint8_t ret;

	if (s_tKey.Read == s_tKey.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_tKey.Buf[s_tKey.Read];

		if ((++s_tKey.Read) >= KEY_FIFO_SIZE)
		{
			s_tKey.Read = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*	函 数 名: KEY_Peek
*	功能说明: 从按键FIFO缓冲区读取一个键值。独立的读指针。
*	形    参:  无
*	返 回 值: 按键代码
*********************************************************************************************************
*/
uint8_t KEY_Peek(void)
{
	uint8_t ret;

	if (s_tKey.Read2 == s_tKey.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_tKey.Buf[s_tKey.Read2];

		if ((++s_tKey.Read2) >= KEY_FIFO_SIZE)
		{
			s_tKey.Read2 = 0;
		}
		return ret;
	}
}


/*
*********************************************************************************************************
*	函 数 名: KEY_FIFO_Flush
*	功能说明: 重置读写指针。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void KEY_FIFO_Flush(void)
{
	s_tKey.Read = 0;
	s_tKey.Write = 0;
	s_tKey.Read2 = 0;
}


/*
*********************************************************************************************************
*	函 数 名: key_gpiote_event_handler
*	功能说明: 按键中断处理函数。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void key_gpiote_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    //static uint16_t i ;

	switch (action)
	{
		case NRF_GPIOTE_POLARITY_LOTOHI:
			
			break;
		
		case NRF_GPIOTE_POLARITY_HITOLO:
			break;
		
		case NRF_GPIOTE_POLARITY_TOGGLE:
			break;
		
		default:
			break;
	}
}



/*
*********************************************************************************************************
*	函 数 名: KEY_GPIOTE_Init
*	功能说明: 按键中断初始化。
*	形    参:  无
*	返 回 值: err_code
*********************************************************************************************************
*/
static uint32_t KEY_GPIOTE_Init(void)
{
    uint32_t err_code;
    
    if (!nrf_drv_gpiote_is_init())
	{
		err_code = nrf_drv_gpiote_init();
		
		if (err_code != NRF_SUCCESS)
		{
			return err_code;
		}
	}
    
    nrf_drv_gpiote_pin_t key_irq_pin = BUTTON_1_PIN;	
    nrf_drv_gpiote_in_config_t key_irq_pin_cfg = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    err_code = nrf_drv_gpiote_in_init(key_irq_pin, &key_irq_pin_cfg, key_gpiote_event_handler);
	   
    if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	
    nrf_drv_gpiote_in_event_enable(key_irq_pin, true);
	
	return err_code;
}


/*
*********************************************************************************************************
*	函 数 名: KEY_GPIOTE_DeInit
*	功能说明: 按键中断去除初始化
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void KEY_GPIOTE_DeInit(void)
{
	nrf_drv_gpiote_pin_t key_irq_pin = BUTTON_1_PIN;	
	nrf_drv_gpiote_in_event_disable(key_irq_pin);
	nrf_drv_gpiote_in_uninit(key_irq_pin);
	nrf_gpio_cfg_default(key_irq_pin);	
}


/*
*********************************************************************************************************
*	函 数 名: KEY_GetState
*	功能说明: 获取按键的状态
*	形    参:  是哪个按键
*	返 回 值: 1  or 0 表示按键是否被按下
*********************************************************************************************************
*/
uint8_t KEY_GetState(Button_TypeDef button)
{
	return nrf_gpio_pin_read(BUTTON_PIN[button]);
}

uint8_t KEY_ReadIOState(void)
{
	if (KEY_GetState((Button_TypeDef)0))
	{
		return 1;
	}
	else
		return 0;
}

/*
*********************************************************************************************************
*	函 数 名: KEY_Scan
*	功能说明: 按键扫描
*	形    参:  无
*	返 回 值: 所有按键的键值
*********************************************************************************************************
*/

static uint8_t releaseKeyValue = 0xff;	// The key number scan must less then 8.
static uint8_t lastKeyValue= 0x00;
static uint8_t sKeyPressedDown[BUTTONn] = {0};
static uint8_t sKeyPressedUp[BUTTONn] = {0};
static uint8_t sKeyPressedLong[BUTTONn] = {0};
static uint8_t sKeyPressedLongCnt[BUTTONn] = {0};
uint32_t KEY_Scan(void)
{
	// Scan every 30ms.
	uint32_t retKeyValue = 0;
	uint8_t pressKeyValue = 0;
//	static uint8_t releaseKeyValue = 0xff;	// The key number scan must less then 8.
//	static uint8_t lastKeyValue= 0x00;
//	static uint8_t sKeyPressedDown[BUTTONn] = {0};
//	static uint8_t sKeyPressedUp[BUTTONn] = {0};
//	static uint8_t sKeyPressedLong[BUTTONn] = {0};
//	static uint8_t sKeyPressedLongCnt[BUTTONn] = {0};
	/*
	 * Get the key value.
	 */
	for (uint8_t btn = 0; btn < BUTTONn; btn++)
	{
		if (!KEY_GetState((Button_TypeDef)btn))
		{ 
			pressKeyValue |= BUTTON_VALUE[btn];
			
			if ((sKeyPressedLong[btn] == 0) && (++sKeyPressedLongCnt[btn] == 40)) // 80//50// 1.5s
			{
				sKeyPressedLong[btn] = 1;
				retKeyValue |= LONG_PRESS(btn);
			}
			
			if (sKeyPressedLong[btn] != 0)
			{
				if (++sKeyPressedLongCnt[btn] > 80) //252
				{
					if (sKeyPressedLongCnt[btn] == 81) //253
					{
						// 超长时间按下 3s
						retKeyValue |= LONG_LONG_PRESS(btn);  
					}
					sKeyPressedLongCnt[btn] = 82;
				}
			}
		} 
		else
		{
			releaseKeyValue |= BUTTON_VALUE[btn];
			sKeyPressedUp[btn] = 0;
			sKeyPressedLongCnt[btn] = 0;
			
			if (sKeyPressedDown[btn])
			{
				sKeyPressedDown[btn] = 0;
				retKeyValue |= UP_CLICK(btn);
				sKeyPressedLong[btn] = 0;
			}
		}
		
		// Check,if key presse down valid.
		if (pressKeyValue && (lastKeyValue & BUTTON_VALUE[btn]))
		{ 
			sKeyPressedDown[btn] = 1;
			
			if (sKeyPressedUp[btn] == 0)
			{
				sKeyPressedUp[btn] = 1;
				retKeyValue |= DOWN_CLICK(btn);
			}
		}
	}
	
	lastKeyValue = pressKeyValue & releaseKeyValue;
	
	return retKeyValue;
}


static uint8_t b_key_update_enable = 1;

void KEY_UpdateEnable(void)
{
    b_key_update_enable = 1;
}

void KEY_UpdateDisable(void)
{
    b_key_update_enable = 0;
}

/*
*********************************************************************************************************
*	函 数 名: KEY_Update
*	功能说明: 按键更新到FIFIO
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void KEY_Update(__IO uint32_t key)
{
	if (key && b_key_update_enable)
	{
		// xx秒钟无按键及触摸操作，进入待机状态.
		//global.timeOfInactivityLevelOne = RTC2_Get125MsCounter() + MENU_INACTIVITY_TIME_LEVEL_ONE;
		
		//KEY_DMESG("<KEY> KEY=0x%08X.\r\n", key);
		
		for (uint8_t btn = 0; btn < BUTTONn; btn++)
		{
			if (key & DOWN_CLICK(btn))
			{
				KEY_DMESG("<KEY> KEY:%d occur down click.\r\n", btn);
				
				if (btn == BUTTON_1N)
				{
					KEY_Put(KEY_1_DOWN);
				}

			}
			if (key & UP_CLICK(btn))
			{
				KEY_DMESG("<KEY> KEY:%d occur up click.\r\n", btn);
				
				if (btn == BUTTON_1N)
				{
					KEY_Put(KEY_1_UP);
				}
			}
			if (key & LONG_PRESS(btn))
			{
				KEY_DMESG("<KEY> KEY:%d occur long press.\r\n", btn);
				if (btn == BUTTON_1N)
				{
					KEY_Put(KEY_1_LONG);
				}
			}
			
			if (key & LONG_LONG_PRESS(btn))
			{
				KEY_DMESG("<KEY> KEY:%d occur long long press.\r\n", btn);
				if (btn == BUTTON_1N)
				{
					KEY_Put(KEY_1_LONG_LONG);
				}
			}
		}
	}
}		


static void  KEY_ParameterInit(void)
{
	releaseKeyValue = 0xff;	// The key number scan must less then 8.
	lastKeyValue= 0x00;
	sKeyPressedDown[BUTTON_1N] = 0;
	sKeyPressedUp[BUTTON_1N] = 0;
	sKeyPressedLong[BUTTON_1N] = 0;
	sKeyPressedLongCnt[BUTTON_1N] = 0;
}


/*
*********************************************************************************************************
*	函 数 名: KEY_Init
*	功能说明: 按键初始化
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void KEY_Init(void)
{
	if (b_key_init == 0)
	{
		b_key_init = 1;
		
		for (uint8_t btn = 0; btn < BUTTONn; btn++)
		{
			nrf_gpio_cfg_input(BUTTON_PIN[btn], NRF_GPIO_PIN_PULLUP);//NRF_GPIO_PIN_NOPULL
		}
		
		KEY_ParameterInit();
		
		KEY_DMESG("<KEY> Init.\r\n");
		//KEY_GPIOTE_Init();
	}
}


/*
*********************************************************************************************************
*	函 数 名: KEY_Init
*	功能说明: 按键去掉初始化
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void KEY_DeInit(void)
{
	if (b_key_init)
	{
		b_key_init = 0;
		
		//KEY_GPIOTE_DeInit();
		
		for (uint8_t btn = 0; btn < BUTTONn; btn++)
		{
			nrf_gpio_cfg_default(BUTTON_PIN[btn]);
		}
		KEY_FIFO_Flush();
		
		KEY_ParameterInit();
		
		KEY_DMESG("<KEY> DeInit.\r\n");
	}
}


/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
