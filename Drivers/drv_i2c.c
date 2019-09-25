/**
  ******************************************************************************
  * @file    drv_i2c.c
  * @author  
  * @version V1.0.0
  * @date    11-Octobal-2016
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"


//#define DBG_I2C
#ifdef DBG_I2C
#define I2C_DMSG	RTT_Printf
#else
#define I2C_DMSG(...)
#endif


//-------------------------------------------------
static SemaphoreHandle_t xSemaphore;

/* Indicates if operation on TWI has ended. */
static volatile bool m_xfer_done2 = false;

/* TWI instance ID. */
#define TWI_INSTANCE_ID2     1
/* TWI instance. */

static const nrf_drv_twi_t m_twi2 = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID2);


///**
// * @brief TWI events handler.
// */
//static void twi2_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
//{
//    switch (p_event->type)
//    {
//        case NRF_DRV_TWI_EVT_DONE:
//            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
//            {
//                ;//data_handler(m_sample);
//            }
//            m_xfer_done = true;
//            break;
//		case NRF_DRV_TWI_EVT_ADDRESS_NACK:	///< Error event: NACK received after sending the address.
//			break;
//		case NRF_DRV_TWI_EVT_DATA_NACK:		///< Error event: NACK received after sending a data byte.
//			break;
//        default:
//            break;
//    }
//}


bool I2C2_Tx(uint8_t addr, uint8_t const * p_data, uint8_t length, bool no_stop)
{
    xSemaphoreTake(xSemaphore, 30000);//portMAX_DELAY

    bool ret = false;
    ret_code_t err_code;
    
    m_xfer_done2 = false;
        
	err_code = nrf_drv_twi_tx(&m_twi2, addr, p_data, length, no_stop);
    
//	while (m_xfer_done == false) {
//		__WFE();
//	}
	if (err_code == NRF_SUCCESS)
	{
		ret = true;
	}
    
    xSemaphoreGive(xSemaphore);
    return ret;
}


bool I2C2_Rx(uint8_t addr, uint8_t const * p_data, uint8_t length)
{
    xSemaphoreTake(xSemaphore, 30000);//portMAX_DELAY
    
    bool ret = false;
    ret_code_t err_code;
    
    m_xfer_done2 = false;
        
	err_code = nrf_drv_twi_rx(&m_twi2, addr, (uint8_t*)p_data, length);
    
//	while (m_xfer_done == false) {
//		__WFE();
//	}
	if (err_code == NRF_SUCCESS)
	{
        ret = true;
	}
    
    xSemaphoreGive(xSemaphore);
    return ret;
}


#define I2C2_PIN_SCL         BOARD_I2C2_SCL   // SCL signal pin
#define I2C2_PIN_SDA         BOARD_I2C2_SDA   // SDA signal pin

static uint8_t b_i2c2_init = 0;

void I2C2_DeInit(void)
{
	if (b_i2c2_init)
	{
		b_i2c2_init = 0;

        nrf_drv_twi_disable(&m_twi2);
		nrf_drv_twi_uninit(&m_twi2);
		
//		nrf_gpio_pin_clear(I2C2_PIN_SCL);	
		nrf_gpio_cfg_default(I2C2_PIN_SCL);
		
//		nrf_gpio_pin_clear(I2C2_PIN_SDA);	
		nrf_gpio_cfg_default(I2C2_PIN_SDA);
        
        
        *(volatile uint32_t *) 0x40004FFC = 0;
        *(volatile uint32_t *) 0x40004FFC;
        *(volatile uint32_t *) 0x40004FFC = 1;	
      

		I2C_DMSG("<I2C2> DeInit.\r\n");
	}
}



void I2C2_Init(void)
{
	if (b_i2c2_init == 0)
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
                
                I2C_DMSG("<I2C2> xSemaphoreCreateMutex OK.\r\n");
            }
            else
            {
                I2C_DMSG("<I2C2> xSemaphoreCreateMutex Failure\r\n");
                delay_ms(10);
                APP_ERROR_CHECK(1);
            }
        }

        
		ret_code_t err_code;
        
        nrf_gpio_cfg_default(I2C2_PIN_SCL);	
		nrf_gpio_cfg_default(I2C2_PIN_SDA);

		const nrf_drv_twi_config_t twi_config = {
		   .scl                = I2C2_PIN_SCL,
		   .sda                = I2C2_PIN_SDA,
		   .frequency          = NRF_DRV_TWI_FREQ_400K,//NRF_TWI_FREQ_400K,
		   .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
		   .clear_bus_init     = true // false Merle20190911
		};

		err_code = nrf_drv_twi_init(&m_twi2, &twi_config, NULL, NULL);
	   // err_code = nrf_drv_twi_init(&m_twi2, &twi_config, twi2_handler, NULL);
		APP_ERROR_CHECK(err_code);

		nrf_drv_twi_enable(&m_twi2);
        
        
//		#define TWI_ADDRESSES 127
//		uint8_t sample_data = 0;
//		for (uint8_t address = 1; address <= TWI_ADDRESSES; address++)
//		{
//			uint32_t err_code = nrf_drv_twi_rx(&m_twi2, address, &sample_data, sizeof(sample_data));
//			if (err_code == NRF_SUCCESS)
//			{
//				I2C_DMSG("TWI2 device detected at address 0x%x.\r\n", address);
//			}
//		}
		
		I2C_DMSG("<I2C2> Init.\r\n");
        
        b_i2c2_init = 1;
	}
}


/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
