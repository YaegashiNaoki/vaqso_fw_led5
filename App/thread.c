 /**
  ******************************************************************************
  * @file    thread.c
  * @author  Merle
  * @version V1.0.0
  * @date    25-July-2019
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"

//#define DBG_THREAD
#ifdef DBG_THREAD
#define THREAD_DMSG	    RTT_Printf
#define SNPRINTF        snprintf
#else
#define THREAD_DMSG(...)
#define SNPRINTF(...)
#endif

#if NRF_LOG_ENABLED
static TaskHandle_t m_logger_thread;                                /**< Definition of Logger thread. */
#endif

#if NRF_LOG_ENABLED
/**@brief Thread for handling the logger.
 *
 * @details This thread is responsible for processing log entries if logs are deferred.
 *          Thread flushes all log entries and suspends. It is resumed by idle task hook.
 *
 * @param[in]   arg   Pointer used for passing some arbitrary information (context) from the
 *                    osThreadCreate() call to the thread.
 */
static void logger_thread(void * arg)
{
    UNUSED_PARAMETER(arg);

    while (1)
    {
        NRF_LOG_FLUSH();
        
        vTaskSuspend(NULL); // Suspend myself
    }
}
#endif //NRF_LOG_ENABLED


/**@brief A function which is hooked to idle task.
 * @note Idle hook must be enabled in FreeRTOS configuration (configUSE_IDLE_HOOK).
 */
void vApplicationIdleHook( void )
{
#if NRF_LOG_ENABLED
    vTaskResume(m_logger_thread);
#endif
}


static TaskHandle_t m_light_sequence_execute_thread; 
static void light_sequence_scheduler_thread(void * arg)
{
    UNUSED_PARAMETER(arg);

    THREAD_DMSG("<THREAD> light_thread start.\r\n");
    
    while(1)
    {
        LIGHT_SequenceScheduler();

        nrf_pwr_mgmt_run();
        vTaskDelay(35);//50./10
    }
}

static TaskHandle_t m_xtcp_thread;
static void xtcp_thread(void * arg)
{
    UNUSED_PARAMETER(arg);

    THREAD_DMSG("<THREAD> xtcp_thread start.\r\n");
    
    while (1)
    {
        XTCP_Loop();
        
        nrf_pwr_mgmt_run();
        vTaskDelay(10);//50
    }
}

static TaskHandle_t m_key_scan_thread;
static void key_scan_thread(void * arg)
{
    UNUSED_PARAMETER(arg);

    KEY_Init();

    THREAD_DMSG("<THREAD> key_scan_thread start.\r\n");
    
    uint8_t key_value;
    
    while (1)
    {
        KEY_Update(KEY_Scan());
        /*
        key_value = KEY_Get();

        if (key_value == KEY_1_LONG_LONG)
        {
            if (f.SYSTEM_RUN_STATUS == SYSTEM_RUN_STATUS_NORMAL)
            {
                LIGHT_SequenceSet(LIGHT_SYSTEM_SHUTDOWN);
            }
        }
            
        if (key_value == KEY_1_DOWN)
        {
            if (f.SYSTEM_RUN_STATUS == SYSTEM_RUN_STATUS_NORMAL)
            {
                BLE_AdvStart(false);
            }
        }
		*/
        nrf_pwr_mgmt_run();
        vTaskDelay(35);
    }
}

static TaskHandle_t m_battery_level_update_thread;
static void battery_level_update_thread(void * arg)
{
    UNUSED_PARAMETER(arg);
    
    BAT_Init();
    
    THREAD_DMSG("<THREAD> battery_check_thread start.\r\n");
     
    uint8_t current_battery_state = 0xff;
    /*
        صС20%ʱÿ2ʾû͵
        صС5%ʱ̹ػ
    */   
    #define BAT_POWER_LOW_NOTICE_TIMES   (120) // 120s = 2min.
    #define BAT_POWER_LOW_NOTICE_LEVEL   (200) // 20% (X10)
    #define BAT_POWER_LOW_SHUTDOEN_LEVEL  (50) // 5%  (X10)
    //
    uint8_t b_bat_power_low = 0;
    uint16_t bat_power_low_times_cnt = 0;
    uint16_t bat_power_indicate_shutdown_times_cnt = 0;
    //
    uint8_t resp_pkg[16];
    uint32_t err_code;
    uint16_t resp_pkg_len;
    uint8_t param_data;
    uint16_t param_data_len = sizeof(param_data) / sizeof(uint8_t);
    //
    while (1)
    {
        BAT_LevelUpdate();
        
        // Check only in discharge mode.
        if (bat.chargeState == BAT_CHARGE_STATUS_DISCHARGE)
        {
            // If the battery power below the 20%, execute the light sequence notice it.
            if (bat.percentage < BAT_POWER_LOW_NOTICE_LEVEL) 
            {
                THREAD_DMSG("<THREAD> bat_power_low_times_cnt=%d. \r\n",bat_power_low_times_cnt);
                if (++bat_power_low_times_cnt > BAT_POWER_LOW_NOTICE_TIMES)
                {
                    bat_power_low_times_cnt = 0;
                    LIGHT_SequenceSet(LIGHT_BAT_POWER_LOW);
                    //
                    param_data = PARAM_NOTIFY_BATTERY_POWER_LOW;
                    resp_pkg_len = XTCP_MakePackage(CMD_NOTIFY, CMD_NOTIFY_BATTERY_STATUS, param_data_len, &param_data, resp_pkg);
                    err_code = UART_PacketSend(resp_pkg, resp_pkg_len, NULL);
                    err_code = BLE_PacketSend(resp_pkg, resp_pkg_len, NULL);   
                }
                
                // If the battery power below the 5%, shutdown the system.
                if (bat.percentage < BAT_POWER_LOW_SHUTDOEN_LEVEL)
                {
                    if (++bat_power_indicate_shutdown_times_cnt)
                    {
                        if (bat_power_indicate_shutdown_times_cnt > 10) // Jitter processing logic.
                        {
                            bat_power_indicate_shutdown_times_cnt = 0;
                            
                            LIGHT_SequenceSet(LIGHT_SYSTEM_SHUTDOWN); // After the light execute over, than the system will be shutdown.
                        }
                    } 
                }
                else
                {
                    bat_power_indicate_shutdown_times_cnt = 0;
                }
            }
            else
            {
                bat_power_low_times_cnt = 0;
            }
        }
        else
        {
            bat_power_low_times_cnt = 0;
        }
        
        //
        if (current_battery_state != bat.chargeState)
        {
            if(bat.chargeState == BAT_CHARGE_STATUS_CHARGE)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING);
                param_data = PARAM_NOTIFY_BATTERY_CHARGING;
                resp_pkg_len = XTCP_MakePackage(CMD_NOTIFY, CMD_NOTIFY_BATTERY_STATUS, param_data_len, &param_data, resp_pkg);
            }
            else if(bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
            {
                LIGHT_SequenceSet(LIGHI_BAT_CHARGING_COMPLETE);
                
                param_data = PARAM_NOTIFY_BATTERY_CHARGING_COMPLETE;
                resp_pkg_len = XTCP_MakePackage(CMD_NOTIFY, CMD_NOTIFY_BATTERY_STATUS, param_data_len, &param_data, resp_pkg);
            }
            else if(bat.chargeState == BAT_CHARGE_STATUS_ERROR)
            {
                LIGHT_SequenceSet(LIGHT_BAT_CHARGING_IRREGULAR);
                
                param_data = PARAM_NOTIFY_BATTERY_CHARGING_IRREGULAR;
                resp_pkg_len = XTCP_MakePackage(CMD_NOTIFY, CMD_NOTIFY_BATTERY_STATUS, param_data_len, &param_data, resp_pkg);
            }
            else
            {
                // discharge
                LIGHT_SequenceSet(LIGHT_BAT_DISCHARGE);
                param_data = PARAM_NOTIFY_BATTERY_DISCHARGE;
                resp_pkg_len = XTCP_MakePackage(CMD_NOTIFY, CMD_NOTIFY_BATTERY_STATUS, param_data_len, &param_data, resp_pkg);
            }
            
            //PARAM_NOTIFY_BATTERY_TOO_LOW
            //resp_param = PARAM_NOTIFY_BATTERY_TOO_LOW;
            //packet_len = XTCP_PacketMake(CMD_NOTIFY, CMD_NOTIFY_BATTERY_STATUS, resp_param_len, &resp_param, resp_buf);
            
            err_code = UART_PacketSend(resp_pkg, resp_pkg_len, NULL);
            err_code = BLE_PacketSend(resp_pkg, resp_pkg_len, NULL);   
        }
        current_battery_state = bat.chargeState;

        nrf_pwr_mgmt_run();
        vTaskDelay(1000);
    }
}


static cartridge_info_t cartridge_info = {0};
static cartridge_enabling_date_t cartridge_enabling_date = {0};


static TaskHandle_t m_cartridge_detect_thread;
static void cartridge_detect_thread(void * arg)
{
    UNUSED_PARAMETER(arg);
    
    //CARTRIDGE_Init();
    uint32_t err_code;
    bool ret;
    uint8_t cartridge_last_check_state[5] = {0};
    
    while (1)
    {
        for (uint8_t i = 0; i < 5; i++)
        {
            ret = CARTRIDGE_CheckIn(i);
            if (ret)
            {
                //THREAD_DMSG("<THREAD> CartridgeCheckIn ch = %d.\r\n", i);
                ret = CARTRIDGE_ReadInfo(i, &cartridge_info);
                //if (ret)
                {
                    
                    THREAD_DMSG("<THREAD> CartridgeInfo read is: ret=%s, id= 0x%04X, %04d-%02d-%02d,%02d:%02d:%02d.%03d, ExpiryDays=%d, serialNum= %d.\r\n", 
                                ret ? "success" : "failure",
                                cartridge_info.id,
                                cartridge_info.dateOfManufacture.year + 2000,
                                cartridge_info.dateOfManufacture.mon,
                                cartridge_info.dateOfManufacture.day,
                                cartridge_info.dateOfManufacture.hour,
                                cartridge_info.dateOfManufacture.min,
                                cartridge_info.dateOfManufacture.sec,
                                cartridge_info.dateOfManufacture.msec,
                                cartridge_info.expiryDays,
                                cartridge_info.serialNumber);
                }
                
                ret = CARTRIDGE_ReadEnablingDate(i, &cartridge_enabling_date);
                //if (ret)
                {
                    
                    THREAD_DMSG("<THREAD> CartridgeEnablingDate read is: ret=%s, %04d-%02d-%02d,%02d:%02d:%02d.\r\n", 
                                ret ? "success" : "failure",
                                cartridge_enabling_date.enablingDate.year + 2000,
                                cartridge_enabling_date.enablingDate.mon,
                                cartridge_enabling_date.enablingDate.day,
                                cartridge_enabling_date.enablingDate.hour,
                                cartridge_enabling_date.enablingDate.min,
                                cartridge_enabling_date.enablingDate.sec);
                }
                
                //
                if (cartridge_last_check_state[i] == 0)
                {
                    LIGHT_SequenceSet(LIGHT_CARTRIDGE_ATTACH_1 + i);
                    
                    uint8_t resp_pkg[16];
                    uint8_t param_data[2];
                    param_data[0] = i + 1;  // channel.
                    param_data[1] = PARAM_NOTIFY_CARTRIDGE_ATTACH;
                    uint16_t param_data_len = sizeof(param_data) / sizeof(uint8_t);
                    uint16_t resp_pkg_len = XTCP_MakePackage(CMD_NOTIFY, CMD_NOTIFY_CARTRIDGE_STATUS, param_data_len, &param_data[0], resp_pkg);
                    err_code = UART_PacketSend(resp_pkg, resp_pkg_len, NULL);
                    err_code = BLE_PacketSend(resp_pkg, resp_pkg_len, NULL);
                }
                cartridge_last_check_state[i] = 1;
            }
            else
            {
                //
                if (cartridge_last_check_state[i])
                {
                    LIGHT_SequenceSet(LIGHT_CARTRIDGE_DETACH_1 + i);      
                    
                    uint8_t resp_pkg[16];
                    uint8_t param_data[2];
                    param_data[0] = i + 1; // channel.
                    param_data[1] = PARAM_NOTIFY_CARTRIDGE_DETACH;
                    uint16_t param_data_len = sizeof(param_data) / sizeof(uint8_t);
                    uint16_t resp_pkg_len = XTCP_MakePackage(CMD_NOTIFY, CMD_NOTIFY_CARTRIDGE_STATUS, param_data_len, &param_data[0], resp_pkg);
                    err_code = UART_PacketSend(resp_pkg, resp_pkg_len, NULL);
                    err_code = BLE_PacketSend(resp_pkg, resp_pkg_len, NULL);                    
                }
                cartridge_last_check_state[i] = 0;
            }
        }
        
        nrf_pwr_mgmt_run();
        vTaskDelay(500);
    }
}

//#define BOARD_UART_AUTO_STANBY_CTL

static TaskHandle_t m_main_thread;
static int btn_count = 37;
static void main_thread(void * arg)
{
    UNUSED_PARAMETER(arg);

    RTC2_Init(); 
    
    UART_Init(); // 2.2mA. Notice: Init when the usb cable put in.

    THREAD_DMSG("<THREAD> main_thread start.\r\n");
    
    uint8_t usb_connnect_status = 0xff;
    uint16_t uart_not_active_cnt = 0;
    
    //unsigned portBASE_TYPE uxHighWaterMark; 
    
    while (1)
    {
        WDT_Feed();
        
        time_t aTime = RTC2_GetTime();
        //global.tmTime = *localtime(&aTime);
         
		uint8_t key_value;
        key_value = KEY_Get();
        
		if (key_value == KEY_1_LONG_LONG)
        {
            if (f.SYSTEM_RUN_STATUS == SYSTEM_RUN_STATUS_NORMAL)
            {
                LIGHT_SequenceSet(LIGHT_SYSTEM_SHUTDOWN);
            }
        }

        if (key_value == KEY_1_DOWN)
        {
           btn_count++;
		   THREAD_DMSG("btn_count = %d.\r\n",btn_count);
           THREAD_DMSG("TEST1 = %d.\r\n",TEST1);
           THREAD_DMSG("TEST14 = %d.\r\n",TEST14);
		   LIGHT_SequenceSet(btn_count);
		   if(btn_count == TEST14) btn_count = 37;
        }
        vTaskDelay(200);
    }
}


void THREAD_Create(void)
{
#if NRF_LOG_ENABLED
    // Start execution.
    if (pdPASS != xTaskCreate(logger_thread, "LOGGER", 256, NULL, 1, &m_logger_thread))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
#endif

    if (pdPASS != xTaskCreate(main_thread, "MAIN", 256, NULL, 1, &m_main_thread)) //256 * 4Bytes(uint32_t) = 1K
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
    
    if (pdPASS != xTaskCreate(light_sequence_scheduler_thread, "LIGHT", 256, NULL, 1, &m_light_sequence_execute_thread))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
    
    if (pdPASS != xTaskCreate(xtcp_thread, "XTCP", 256, NULL, 1, &m_xtcp_thread))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }    
    
    if (pdPASS != xTaskCreate(key_scan_thread, "KEY_SCAN", 256, NULL, 1, &m_key_scan_thread))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }    
    
    if (pdPASS != xTaskCreate(cartridge_detect_thread, "CARTRIDGE", 256, NULL, 1, &m_cartridge_detect_thread))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
    
    if (pdPASS != xTaskCreate(battery_level_update_thread, "BATTERY", 256, NULL, 1, &m_battery_level_update_thread))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
}


void THREAD_Start(void)
{
    // Start FreeRTOS scheduler.
    vTaskStartScheduler();
}


void THREAD_Init(void)
{

}



		
/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
