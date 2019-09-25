/**
  ******************************************************************************
  * @file    drv_adc.c
  * @author  
  * @version V1.0.0
  * @date    20-January-2017
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"

//#define DBG_ADC
#ifdef DBG_ADC
#define ADC_DMSG	RTT_Printf
#else
#define ADC_DMSG(...)
#endif

#define SAMPLES_IN_BUFFER 1

static nrf_saadc_value_t       m_buffer_pool[2][SAMPLES_IN_BUFFER];


volatile int32_t m_adc_value = 0;
static uint8_t	m_saadc_done = 0;

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
		ret_code_t err_code;
		
		err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
		APP_ERROR_CHECK(err_code);
		m_adc_value = abs(p_event->data.done.p_buffer[0]);
		m_saadc_done = 1;
    }
}


void ADC_DataUpdateOnce(void)
{
	ret_code_t err_code;
	
	//if (m_saadc_done)
	{	
//		m_saadc_done = 0;

		err_code = nrf_drv_saadc_sample();
		
//		if (err_code == NRF_SUCCESS)
//		{
//			ADC_DMSG("<ADC> Start Ok.\r\n");
//		}
//		else
//		{
//			ADC_DMSG("<ADC> Start Failure =%u.\r\n",err_code);
//		}
	}
}

void ADC_TriggerStop(void)
{
	nrf_saadc_task_trigger(NRF_SAADC_TASK_STOP);
}


bool ADC_GetValue(int32_t *adc_value)
{
	(*adc_value) = m_adc_value;
	if (m_saadc_done)
	{
        m_saadc_done = 0;
		return true;
	}
	else
	{
		return false;
	}
}

static uint8_t b_adc_init = 0;


void saadc_init(void)
{
    ret_code_t err_code;
	
   /*
	* 	The internal reference results in an input range of ¡À0.6 V on the ADC core. VDD as reference results in an
		input range of ¡ÀVDD/4 on the ADC core. The gain block can be used to change the effective input range of
		the ADC.
		Input range = (+- 0.6 V or +-VDD/4)/Gain
	* 	For example, choosing VDD as reference, single ended input (grounded negative input), and a gain of 1/4
		the input range will be:
		Input range = (VDD/4)/(1/4) = VDD
	* 	With internal reference, single ended input (grounded negative input), and a gain of 1/6 the input range will
		be:
		Input range = (0.6 V)/(1/6) = 3.6 V
	* 	The AIN0-AIN7 inputs cannot exceed VDD, or be lower than VSS.
	
    *	Table 88: Acquisition time
		TACQ [¦Ìs]    Maximum source resistance [kOhm]
			3 									10
			5 									40
			10 									100
			15 									200
			20 									400
			40 									800
    */
	//set configuration for saadc channel X
    nrf_saadc_channel_config_t  channel_config;
	channel_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
    channel_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
    channel_config.gain       = NRF_SAADC_GAIN1_5;			//(0.6V / (1/5) = 3.0V =4096)NRF_SAADC_GAIN1_6;
    channel_config.reference  = NRF_SAADC_REFERENCE_INTERNAL;
    channel_config.acq_time   = NRF_SAADC_ACQTIME_40US;		//NRF_SAADC_ACQTIME_3US;
    channel_config.mode       = NRF_SAADC_MODE_SINGLE_ENDED;
    channel_config.pin_p      = (nrf_saadc_input_t)(BOARD_BAT_ADC_INPUT_AIN);
    channel_config.pin_n      = NRF_SAADC_INPUT_DISABLED;

	nrf_drv_saadc_config_t m_saadc_config = NRF_DRV_SAADC_DEFAULT_CONFIG;
	m_saadc_config.interrupt_priority = 3;
	err_code = nrf_drv_saadc_init(&m_saadc_config, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);  //Merle20190521
    //err_code = nrf_drv_saadc_channel_init(4, &channel_config);
    APP_ERROR_CHECK(err_code);
//	m_adc_channel_enabled = 0;
		
	//Configure TIMER mode and sampling rate of 200kHz,,,,,,,16mHz / 0x50 = 16000kHz /80 = 200kHz. max = 2047
	NRF_SAADC->SAMPLERATE = (SAADC_SAMPLERATE_MODE_Timers << SAADC_SAMPLERATE_MODE_Pos);
	NRF_SAADC->SAMPLERATE |= (2000 << SAADC_SAMPLERATE_CC_Pos); //(2000 << SAADC_SAMPLERATE_CC_Pos); // old =0x50    

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
		
	//Start sampling. The internal SAADC timer will then trigger sampling until STOP task is called.
	//nrf_drv_saadc_sample();
}

void ADC_DeInit(void)
{
	if (b_adc_init)
	{
		b_adc_init = 0;
		nrf_saadc_task_trigger(NRF_SAADC_TASK_STOP);
		nrf_drv_saadc_uninit();
		sd_nvic_ClearPendingIRQ(SAADC_IRQn);
		
		//ADC_DMSG("<ADC> DeInit.\r\n");
	}
}


void ADC_Init(void)
{
	if (b_adc_init == 0)
	{
		b_adc_init = 1;
        m_saadc_done = 0;
        
		saadc_init();
		//ADC_DMSG("<ADC> Init.\r\n");
	}
}


/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
