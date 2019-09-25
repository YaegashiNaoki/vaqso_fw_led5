/**
  ******************************************************************************
  * @file    main.c
  * @author  Merle
  * @version 
  * @date    
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "color.h"



//#define DBG_MAIN
#ifdef DBG_MAIN
#define MAIN_DMSG	RTT_Printf
#define SNPRINTF    snprintf
#else
#define MAIN_DMSG(...)
#define SNPRINTF(...)
#endif

void delay_us(uint32_t us)
{
    nrf_delay_us(us);
}

void delay_ms(uint32_t ms)
{
    nrf_delay_ms(ms);
    //vTaskDelay(ms);
}


/**@brief Function for the Power manager.
 */
static void log_init(void)
{
    uint32_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for initializing the clock.
 */
static void clock_init(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
}

/*
 * Notice1: More power save with the Log disable.
 * Notice2: More power save with the FPU disable.
 * Notice3: Moew power save with the Uart disable.(not quite there yet), notice: the uart will just enable when the usb cable in.
 */
/**@brief Function for application main entry.
 */
int main(void)
{
    SYSTEM_PowerCtlInit();
    SYSTEM_PowerOn();
    delay_ms(500);
    
    // Initialize modules.
    log_init();
    clock_init();
        
    MAIN_DMSG("\r\n---...---\r\n");

    WDT_Init(); 
   
    MAIN_DMSG("<MAIN> System Start.\r\n");
    MAIN_DMSG("<MAIN> Built:%s %s.\r\n", __DATE__, __TIME__);   // Built:Aug 16 2019 17:49:50.
    
    SYSTEM_DealWithReset();

    // Initialize the async SVCI interface to bootloader before any interrupts are enabled.
//    err_code = ble_dfu_buttonless_async_svci_init();
//    APP_ERROR_CHECK(err_code);
    
    // Do not start any interrupt that uses system functions before system initialisation.
    // The best solution is to start the OS before any other initalisation.
    THREAD_Create();
    
    // Activate deep sleep mode.
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    
    //RTC2_Init(); 
    //ENV_Init();
    CARTRIDGE_Init();
    FAN_Init();
    LIGHT_Init();
    LIGHT_SequenceSet(LIGHT_SYSTEM_STARTUP);
//    LIGHT_DeInit();
    BLE_Init(false);

    MAIN_DMSG("FreeRTOS started.\r\n");
    
    THREAD_Start();
    
    for (;;)
    {
        MAIN_DMSG("APP_ERROR_HANDLER...\r\n");
        APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
    }
}
    
/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/

