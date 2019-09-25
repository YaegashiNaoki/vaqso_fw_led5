/**
  ******************************************************************************
  * @file    drv_ble.c
  * @author  Merle
  * @version V1.0.0
  * @date    04-January-2017
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"


#define DBG_BLE
#ifdef DBG_BLE

#define BLE_DMSG	RTT_Printf
#define SNPRINTF    snprintf
#else
#define BLE_DMSG(...)
#define SNPRINTF(...)
#endif



#define DEVICE_NAME                     "VAQSO_VR_"//"VAQSO_VR_V5"                  /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "Mira"                                      /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                300     //1400                              /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */
#define APP_ADV_DURATION                18000   //0   3000//                               /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(3000, UNIT_10_MS)             /**<4000 Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                  0                                           /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS              0                                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


//MerleAdd
BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */
/////////////////
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                            /**< Handle of the current connection. */
static void advertising_start(void * p_erase_bonds);                                    /**< Forward declaration of advertising start function */

// YOUR_JOB: Use UUIDs for service(s) used in your application.
//static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}};
//static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_NUS_SERVICE, BLE_UUID_TYPE_BLE}};       /**< Universally unique service identifier. */
//#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */
//static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
//{
//    {BLE_UUID_NUS_SERVICE, BLE_UUID_TYPE_BLE}//NUS_SERVICE_UUID_TYPE} // ÓÃNUS_SERVICE_UUID_TYPE¹ã²¥Ãû×Ö»á±ä¶Ì
//};

#define BLE_UUID_DEVICE_VAQSO_SERVICE       {{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x00, 0x00, 0x40, 0x6E}} /**< Used vendor specific UUID. */
//static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_DEVICE_VAQSO_SERVICE, BLE_UUID_TYPE_VENDOR_BEGIN}};
static ble_uuid128_t  m_vaqso_base_uuid = BLE_UUID_DEVICE_VAQSO_SERVICE;

#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */
static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
{
    {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}
};

static SemaphoreHandle_t xSemaphore;


static uint8_t b_ble_stop = 1;
static uint8_t b_ble_init = 0;
static uint8_t b_ble_adv_start = 0;


#define BLE_RX_RING_FIFO_SIZE	512
static app_fifo_t ble_rx_ring_fifo;
static uint8_t ble_rx_buffer[BLE_RX_RING_FIFO_SIZE];


uint32_t BLE_GetRxFifoLen(void)
{
    uint32_t len = BLE_RX_RING_FIFO_SIZE;
    return len;
}

app_fifo_t* BLE_GetRxFifoPtr(void)
{
    return &ble_rx_ring_fifo;
}


/**@brief Handler for shutdown preparation.
 *
 * @details During shutdown procedures, this function will be called at a 1 second interval
 *          untill the function returns true. When the function returns true, it means that the
 *          app is ready to reset to DFU mode.
 *
 * @param[in]   event   Power manager event.
 *
 * @retval  True if shutdown is allowed by this power manager handler, otherwise false.
 */
static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            NRF_LOG_INFO("Power management wants to reset to DFU mode.");
            // YOUR_JOB: Get ready to reset into DFU mode
            //
            // If you aren't finished with any ongoing tasks, return "false" to
            // signal to the system that reset is impossible at this stage.
            //
            // Here is an example using a variable to delay resetting the device.
            //
            // if (!m_ready_for_reset)
            // {
            //      return false;
            // }
            // else
            //{
            //
            //    // Device ready to enter
            //    uint32_t err_code;
            //    err_code = sd_softdevice_disable();
            //    APP_ERROR_CHECK(err_code);
            //    err_code = app_timer_stop_all();
            //    APP_ERROR_CHECK(err_code);
            //}
            
            
            break;

        default:
            // YOUR_JOB: Implement any of the other events available from the power management module:
            //      -NRF_PWR_MGMT_EVT_PREPARE_SYSOFF
            //      -NRF_PWR_MGMT_EVT_PREPARE_WAKEUP
            //      -NRF_PWR_MGMT_EVT_PREPARE_RESET
            return true;
    }

    NRF_LOG_INFO("Power management allowed to reset to DFU mode.");
    return true;
}

//lint -esym(528, m_app_shutdown_handler)
/**@brief Register application shutdown handler with priority 0.
 */
NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);


static void buttonless_dfu_sdh_state_observer(nrf_sdh_state_evt_t state, void * p_context)
{
    if (state == NRF_SDH_EVT_STATE_DISABLED)
    {
        // Softdevice was disabled before going into reset. Inform bootloader to skip CRC on next boot.
        nrf_power_gpregret2_set(BOOTLOADER_DFU_SKIP_CRC);

        //Go to system off.
        nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
    }
}

/* nrf_sdh state observer. */
NRF_SDH_STATE_OBSERVER(m_buttonless_dfu_state_obs, 0) =
{
    .handler = buttonless_dfu_sdh_state_observer,
};


static void advertising_config_get(ble_adv_modes_config_t * p_config)
{
    memset(p_config, 0, sizeof(ble_adv_modes_config_t));

    p_config->ble_adv_fast_enabled  = true;
    p_config->ble_adv_fast_interval = APP_ADV_INTERVAL;
    p_config->ble_adv_fast_timeout  = APP_ADV_DURATION;
}


static void disconnect(uint16_t conn_handle, void * p_context)
{
    UNUSED_PARAMETER(p_context);

    ret_code_t err_code = sd_ble_gap_disconnect(conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_WARNING("Failed to disconnect connection. Connection handle: %d Error: %d", conn_handle, err_code);
    }
    else
    {
        NRF_LOG_DEBUG("Disconnected connection handle %d", conn_handle);
    }
}



// YOUR_JOB: Update this code if you want to do anything given a DFU event (optional).
/**@brief Function for handling dfu events from the Buttonless Secure DFU service
 *
 * @param[in]   event   Event from the Buttonless Secure DFU service.
 */
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
    switch (event)
    {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
        {
            NRF_LOG_INFO("Device is preparing to enter bootloader mode.");

            // Prevent device from advertising on disconnect.
            ble_adv_modes_config_t config;
            advertising_config_get(&config);
            config.ble_adv_on_disconnect_disabled = true;
            ble_advertising_modes_config_set(&m_advertising, &config);

            // Disconnect all other bonded devices that currently are connected.
            // This is required to receive a service changed indication
            // on bootup after a successful (or aborted) Device Firmware Update.
            uint32_t conn_count = ble_conn_state_for_each_connected(disconnect, NULL);
            NRF_LOG_INFO("Disconnected %d links.", conn_count);
            break;
        }

        case BLE_DFU_EVT_BOOTLOADER_ENTER:
            // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
            //           by delaying reset by reporting false in app_shutdown_handler
            
            I2C2_Init();
            env.cfg.reboot_status = BOARD_REBOOT_BYTE;
            ENV_Save();
            WS2812B_DeInit();
            UART_DeInit();
            
            SYSTEM_DCDC5V_PowerTurnOff();
            PCA9685_Reset();
            PCA9685_DeInit();
            
            I2C2_DeInit();

      
            NRF_LOG_INFO("Device will enter bootloader mode.");
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
            NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
            NRF_LOG_ERROR("Request to send a response to client failed.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            APP_ERROR_CHECK(false);
            break;

        default:
            NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
            break;
    }
}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    bool delete_bonds = false;
    
    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);
    
    switch (p_evt->evt_id)
    {
        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            advertising_start(&delete_bonds);
            BLE_DMSG("pm_evt_handler perrs delete success.\r\n");
            break;

        default:
            break;
    }
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
//static void gap_params_init(void)
static void gap_params_init(uint8_t* deviceNameS)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    //
    uint32_t bleAddress = NRF_FICR->DEVICEADDR[0];
    #define DEVICE_NAME_MAX_LEN 32
    char deviceName[DEVICE_NAME_MAX_LEN] ={0};
    uint8_t deviceNameLen;
    deviceNameLen = snprintf(deviceName, DEVICE_NAME_MAX_LEN, "%s", deviceNameS);
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) deviceName,
                                          deviceNameLen);
                                          
//    err_code = sd_ble_gap_device_name_set(&sec_mode,
//                                          (const uint8_t *)DEVICE_NAME,
//                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    /* YOUR_JOB: Use an appearance value matching the application's use case.
       err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
       APP_ERROR_CHECK(err_code); */

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);                     
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the YYY Service events.
 * YOUR_JOB implement a service handler function depending on the event the service you are using can generate
 *
 * @details This function will be called for all YY Service events which are passed to
 *          the application.
 *
 * @param[in]   p_yy_service   YY Service structure.
 * @param[in]   p_evt          Event received from the YY Service.
 *
 *
   static void on_yys_evt(ble_yy_service_t     * p_yy_service,
                          ble_yy_service_evt_t * p_evt)
   {
    switch (p_evt->evt_type)
    {
        case BLE_YY_NAME_EVT_WRITE:
            APPL_LOG("[APPL]: charact written with value %s. ", p_evt->params.char_xx.value.p_str);
            break;

        default:
            // No implementation needed.
            break;
    }
   }*/
/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t * p_evt)
{
    //char buf[20];
    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        const uint16_t rx_len = p_evt->params.rx_data.length;
       
        uint16_t read_count = 0;
    
        while (read_count < rx_len)
        {
            app_fifo_put(&ble_rx_ring_fifo, p_evt->params.rx_data.p_data[read_count++]);
        }

        return;
    }

}
/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t                  err_code;
    nrf_ble_qwr_init_t        qwr_init  = {0};
    ble_dfu_buttonless_init_t dfus_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);
    
    //
    // Initialize NUS.
    ble_nus_init_t     nus_init;
    memset(&nus_init, 0, sizeof(nus_init));
    
    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
       
    //
    // Initialize DFU.
    dfus_init.evt_handler = ble_dfu_evt_handler;

    err_code = ble_dfu_buttonless_init(&dfus_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

static void power_management_init(void)
{
    uint32_t err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting timers.
 */
static void application_timers_start(void)
{
    /* YOUR_JOB: Start your timers. below is an example of how to start a timer.
       uint32_t err_code;
       err_code = app_timer_start(m_app_timer_id, TIMER_INTERVAL, NULL);
       APP_ERROR_CHECK(err_code); */
    XTIMER_Start();
    
}


///**@brief Function for putting the chip into sleep mode.
// *
// * @note This function will not return.
// */
//static void sleep_mode_enter(void)
//{
//    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);

//    APP_ERROR_CHECK(err_code);

//    // Prepare wakeup buttons.
//    err_code = bsp_btn_ble_sleep_mode_prepare();
//    APP_ERROR_CHECK(err_code);

//    //Disable SoftDevice. It is required to be able to write to GPREGRET2 register (SoftDevice API blocks it).
//    //GPREGRET2 register holds the information about skipping CRC check on next boot.
//    err_code = nrf_sdh_disable_request();
//    APP_ERROR_CHECK(err_code);
//}

uint8_t b_ble_real_stop = 0;

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            //err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            //APP_ERROR_CHECK(err_code);
        
            b_ble_stop	= 0;
			f.BLE_CONNECT_STATUS = BLE_STATUS_FAST_ADV;
   		    BLE_DMSG("<BLE> /on_adv_evt/ BLE_ADV_EVT_FAST.\r\n");
           
            LIGHT_SequenceSet(LIGHT_BLE_ADV_START);
   
            break;

        case BLE_ADV_EVT_IDLE:
            //sleep_mode_enter();
        
            b_ble_stop = 1;
			f.BLE_CONNECT_STATUS = BLE_STATUS_STOP;

        
			BLE_DMSG("<BLE> /on_adv_evt/ BLE_ADV_EVT_IDLE. \r\n");
            
            b_ble_adv_start = 0;
        
            LIGHT_SequenceSet(LIGHT_BLE_BONDS_FAILURE);
        
            break;

        default:
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            
            // LED indication will be changed when advertising starts.
            // Merle20190520
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
        
			//f.BLE_CONNECT_STATUS = BLE_STATUS_DISCONNECT;
		
			BLE_DMSG("<on_ble_evt> BLE GAP EVT DICONNECTED.\r\n");

            //LIGHT_SequenceSet(LIGHT_BLE_BONDS_FAILURE);
 
            break;

        case BLE_GAP_EVT_CONNECTED:
            
//            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
//            APP_ERROR_CHECK(err_code);
//            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
//            APP_ERROR_CHECK(err_code);

            f.BLE_CONNECT_STATUS = BLE_STATUS_CONNECT;
        
            LIGHT_SequenceSet(LIGHT_BLE_BONDS_SUCCESS); 
			
            BLE_DMSG("<on_ble_evt> BLE GAP EVT CONNECTED.\r\n");
            
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);

            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
            break;
        }

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init()
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/** @brief Clear bonding information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    // Initialize timer module.
    uint32_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create timers.

    /* YOUR_JOB: Create any timers to be used by the application.
                 Below is an example of how to create a timer.
                 For every new timer needed, increase the value of the macro APP_TIMER_MAX_TIMERS by
                 one.
       uint32_t err_code;
       err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_handler);
       APP_ERROR_CHECK(err_code); */
    
    
    XTIMER_Create();
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;//BLE_ADVDATA_NO_NAME;//BLE_ADVDATA_SHORT_NAME;//
    init.advdata.include_appearance      = false;//Merle20190610 true;
    init.advdata.flags                   =  BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    
    #define APP_COMPANY_IDENTIFIER         0x072B   // 0xFFFF     /**< Company identifier for Bitkey Inc. as per www.bluetooth.org. */
    
    uint8_t m_device_info[10];
	ble_advdata_manuf_data_t manuf_specific_data;
    m_device_info[0] = 0x40; // 0x40=VAQSO // 0x10=BitLock / 0x20=BitReader / 0x30=ButtonKey.
    
    m_device_info[1] = ((NRF_FICR->DEVICEADDR[0]>> 0) & 0xff);
    m_device_info[2] = ((NRF_FICR->DEVICEADDR[0]>> 8) & 0xff);
    m_device_info[3] = ((NRF_FICR->DEVICEADDR[0]>>16) & 0xff);
    m_device_info[4] = ((NRF_FICR->DEVICEADDR[0]>>24) & 0xff);
    
    m_device_info[5] = ((NRF_FICR->DEVICEADDR[1]>> 0) & 0xff);
    m_device_info[6] = ((NRF_FICR->DEVICEADDR[1]>> 8) & 0xff);
    m_device_info[7] = ((NRF_FICR->DEVICEADDR[1]>>16) & 0xff);
    m_device_info[8] = ((NRF_FICR->DEVICEADDR[1]>>24) & 0xff);
    
    uint8_t device_status = 0x00;
    uint8_t bat_info_bits = (bat.percentage > 850) ? 0x03 : (bat.percentage > 600) ? 0x02 : (bat.percentage > 350) ? 0x01 : 0x00;
    m_device_info[9] = device_status | bat_info_bits;

   
    //
    manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;
    manuf_specific_data.data.p_data = (uint8_t *) m_device_info;
    manuf_specific_data.data.size   = sizeof(m_device_info) / (sizeof(uint8_t));
    init.advdata.p_manuf_specific_data = &manuf_specific_data;
    // 2 + 13 + 4 = 19  (+ 17 =35 ³¬¹ý31¸ö×Ö½Ú
    // 1 + 16, ²»¿ÉÒÔÓÃÔÚÕâÒ»Ïî
    //init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    //init.advdata.uuids_complete.p_uuids  = m_adv_uuids;
    
    // UUID 
    // 1 + 16
    init.srdata.uuids_complete.uuid_cnt =  sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids = m_adv_uuids;
    
    //init.srdata.name_type = BLE_ADVDATA_FULL_NAME;
    //
//    static const uint8_t service_str_len = 4;
//    static uint8_t service_str[service_str_len] ={0x01, 0x02, 0x03, 0x04};
//    ble_advdata_service_data_t service_data;
//    service_data.service_uuid = m_adv_uuids->uuid;
//    service_data.data.p_data = service_str;
//    service_data.data.size = service_str_len;
//    init.srdata.p_service_data_array = &service_data;
//    init.srdata.service_data_count = 1;

    advertising_config_get(&init.config);
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


///**@brief Function for initializing buttons and leds.
// *
// * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
// */
//static void buttons_leds_init(bool * p_erase_bonds)
//{
//    uint32_t err_code;
//    bsp_event_t startup_event;

//    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
//    APP_ERROR_CHECK(err_code);

//    err_code = bsp_btn_ble_init(NULL, &startup_event);
//    APP_ERROR_CHECK(err_code);

//    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
//}



/**@brief   Function for initializing the GATT module.
 * @details The GATT module handles ATT_MTU and Data Length update procedures automatically.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting advertising.
 */
static void advertising_start(void * p_erase_bonds)
{
    bool erase_bonds = *(bool*)p_erase_bonds;
    
    if (erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
        BLE_DMSG("<BLE> delete_bonds... \r\n\r\n");
    }
    else
    {
        uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
        
        BLE_DMSG("<BLE> advertising is started..%s.\r\n", (err_code == NRF_SUCCESS) ? "Successful" : "Failure!");
        
    }
}


///**@brief Function for starting advertising.
// */
//static void advertising_start(void)
//{
//    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);

//    APP_ERROR_CHECK(err_code);
//}



// ַ
uint32_t BLE_SendString(uint8_t const* p_string, uint16_t length)
{
	uint32_t err_code = NRF_ERROR_NULL;
	
	if (b_ble_init && (f.BLE_CONNECT_STATUS == BLE_STATUS_CONNECT))
	{
        err_code = ble_nus_data_send(&m_nus, (uint8_t*)p_string, &length, m_conn_handle);
	}

	return err_code;
}


uint32_t BLE_Send(uint8_t const* p_buf, uint16_t len)
{
	uint32_t err_code = NRF_ERROR_NULL;
	
	if (b_ble_init && (f.BLE_CONNECT_STATUS == BLE_STATUS_CONNECT))
	{
        err_code = ble_nus_data_send(&m_nus, (uint8_t*)p_buf, &len, m_conn_handle);
	}
	return err_code;
}


//static uint8_t packet_ble_send_is_finish = 0;
static uint8_t *packet_ble_send_buf_ptr = NULL;
static uint16_t packet_ble_send_segment_size = 0;
static uint16_t packet_ble_send_segment_remain_len = 0;
static uint16_t packet_ble_send_index = 0;
static uint8_t packet_ble_send_is_remain_state = 0;
static uint8_t packet_ble_send_is_start = 0;

void BLE_PacketSend_ClearStartState(void)
{
    packet_ble_send_is_start = 0;
}

// return 0, mean is send is ok.
uint8_t BLE_PacketSend_GetState(void)
{
	return packet_ble_send_is_start;
}


void BLE_PacketSend_Prepare(uint8_t const* packet, uint16_t len)
{
	packet_ble_send_buf_ptr = (uint8_t*)packet;
	packet_ble_send_segment_size = len / BLE_NUS_MAX_DATA_LEN;
	packet_ble_send_segment_remain_len = len % BLE_NUS_MAX_DATA_LEN;
	packet_ble_send_index = 0;
	if (packet_ble_send_segment_size == 0)
	{
		packet_ble_send_is_remain_state = 1;
	}
	else
	{
		packet_ble_send_is_remain_state = 0;
	}
	
	if (((packet_ble_send_segment_size != 0)|| (packet_ble_send_segment_remain_len != 0)) && (packet_ble_send_buf_ptr != NULL))
	{
		packet_ble_send_is_start = 1;
	}
	BLE_DMSG("<BLE> packet send prepare, len=%d, remain_len=%d, start=%d.\r\n",packet_ble_send_segment_size, packet_ble_send_segment_remain_len, packet_ble_send_is_start);
}



uint32_t BLE_PacketSend_Loop(uint8_t *packet_send_is_finish)
{
	uint32_t err_code;

	if (packet_ble_send_is_start && b_ble_init && (f.BLE_CONNECT_STATUS == BLE_STATUS_CONNECT))
	{
        //bool ble_b_send = true;
        uint16_t ble_nus_send_cnt = packet_ble_send_segment_size + ((packet_ble_send_segment_remain_len > 0) ? 1 : 0);
        
        BLE_DMSG("<BLE> packet_ble_scnt=%d\r\n",  ble_nus_send_cnt);
        uint32_t time_out_max_cnt = 3000; // 3s//20ms
        while(ble_nus_send_cnt && time_out_max_cnt)
        {
            if (packet_ble_send_is_remain_state)
            {
                packet_ble_send_index = packet_ble_send_segment_size;
                err_code = ble_nus_data_send(&m_nus, &packet_ble_send_buf_ptr[packet_ble_send_index * BLE_NUS_MAX_DATA_LEN], &packet_ble_send_segment_remain_len, m_conn_handle);
                if (err_code == NRF_SUCCESS)
                {
                    ble_nus_send_cnt--;
                    
                    BLE_DMSG("<BLE> packet_ble_send_index =%d, Remain len = %d,Finish Send.\r\n", packet_ble_send_segment_size, packet_ble_send_segment_remain_len);
                    packet_ble_send_is_start = 0;
                    //packet_ble_send_is_finish = 1;
                    if (packet_send_is_finish != NULL)
                    {
                       (*packet_send_is_finish) = 1;
                    }
                }
                else
                {
                    delay_ms(1);
                    WDT_Feed();
                    time_out_max_cnt--;
                    
                    //BLE_DMSG("<BLE> packet_ble_send_err111\r\n");
                }
            }
            else
            {
                uint16_t r_send_len = BLE_NUS_MAX_DATA_LEN;
                err_code = ble_nus_data_send(&m_nus, &packet_ble_send_buf_ptr[packet_ble_send_index * BLE_NUS_MAX_DATA_LEN], &r_send_len, m_conn_handle);
                if (err_code == NRF_SUCCESS)
                {
                    ble_nus_send_cnt--;
                    
                    BLE_DMSG("<BLE> packet_ble_send_index =%d, Send len = %d.\r\n", packet_ble_send_index, BLE_NUS_MAX_DATA_LEN);
                    
                    ++packet_ble_send_index;
                    // 0 < 1
                    if (packet_ble_send_index < packet_ble_send_segment_size)
                    {

                    }
                    else
                    {
                        if (packet_ble_send_segment_remain_len)
                        {
                            packet_ble_send_is_remain_state = 1;
                        }
                        else
                        {
                            BLE_DMSG("<BLE> packet_ble_send_index =%d,Finish Send.\r\n", packet_ble_send_index);
                            packet_ble_send_is_start = 0;
                            //packet_ble_send_is_finish = 1;
                            if (packet_send_is_finish != NULL)
                            {
                               (*packet_send_is_finish) = 1;
                            }
                        }
                    }
                }
                else
                {
                    delay_ms(1);
                    WDT_Feed();
                    time_out_max_cnt--;
                    //BLE_DMSG("<BLE> packet_ble_send_err222\r\n");
                }
            }
        }
        BLE_DMSG("<BLE> time_out_cnt=%d\r\n",  time_out_max_cnt);
		
        // timeout.
        if (time_out_max_cnt == 0)
        {
            packet_ble_send_is_start = 0;
            if (packet_send_is_finish != NULL)
            {
               (*packet_send_is_finish) = 1;
            }
        }
	}
    return err_code;
}

uint32_t BLE_PacketSend(uint8_t const* p_packet, uint16_t len, uint8_t *packet_send_is_finish)
{
    if (b_ble_init)
    {
        xSemaphoreTake(xSemaphore, 30000);//portMAX_DELAY
    
        uint32_t err_code;
        BLE_PacketSend_Prepare(p_packet, len);
        err_code = BLE_PacketSend_Loop(packet_send_is_finish);
        
        xSemaphoreGive(xSemaphore);
        return err_code;
    }
    return NRF_ERROR_INTERNAL;
}

// ַ
uint32_t BLE_SendStringSpecial(uint8_t const* p_string, uint16_t length)
{
	uint32_t err_code = NRF_ERROR_NULL;

	if (b_ble_init && (f.BLE_CONNECT_STATUS == BLE_STATUS_CONNECT))
	{
		for (int8_t tries = 10; tries; tries--) 
		{
			err_code = ble_nus_data_send(&m_nus, (uint8_t*)p_string, &length, m_conn_handle);
			if (err_code != NRF_SUCCESS)
			{
				delay_ms(15);
			}
			else
			{				
				BLE_DMSG("<BLE> Send State err_code=%u. tries =%d.\r\n", err_code, tries);//);// BLE_ERROR_NO_TX_PACKETS
				break;
			}
		}
	}
	
	return err_code;
}


uint32_t BLE_SendPkg(uint8_t const* packet, uint16_t len)
{
	uint32_t err_code = NRF_SUCCESS;
	
	uint16_t ble_send = len / BLE_NUS_MAX_DATA_LEN;
	uint16_t ble_remain_send = len % BLE_NUS_MAX_DATA_LEN;
	
	BLE_DMSG("<BLE> Send SendPkg send=%d, remianSend=%d.\r\n", ble_send, ble_remain_send);
	
	for (int16_t i = 0; (i < ble_send) && (err_code == NRF_SUCCESS); i++)
	{
		err_code = BLE_SendStringSpecial(&packet[i * BLE_NUS_MAX_DATA_LEN], BLE_NUS_MAX_DATA_LEN);
	}
	
	if (err_code ==  NRF_SUCCESS)
	{
		if (ble_remain_send)
		{
			err_code = BLE_SendStringSpecial(&packet[ble_send * BLE_NUS_MAX_DATA_LEN], ble_remain_send);
		}
	}
	
	return err_code;
}


uint32_t BLE_Disconnect(void)
{
	uint32_t err_code = NRF_SUCCESS;
    
    BLE_DMSG("<BLE> BLE Disconnect.\r\n");
    if (f.BLE_CONNECT_STATUS == BLE_STATUS_CONNECT)
    {
        BLE_DMSG("<BLE> BLE Disconnect in.\r\n");
        
    	err_code  = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);   
        APP_ERROR_CHECK(err_code);
        if (err_code == NRF_SUCCESS)
        {
            BLE_DMSG("<BLE> BLE Disconnect Ok.\r\n");
        }
        else
        {
            BLE_DMSG("<BLE> BLE Disconnect failure.\r\n");
        }
    }

    return err_code;
}

//  size less than 20 bypes.
void BLE_SetDeviceName(unsigned char* p_ble_name ,unsigned char size)
{
    uint32_t err_code;
    // uint8_t namebuffer[10] = {'a','b','c','d'};
    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    err_code = sd_ble_gap_device_name_set(&sec_mode,(const uint8_t*)p_ble_name, size);
    APP_ERROR_CHECK(err_code);
    advertising_init();
}

uint32_t BLE_ChangeAdvName(uint8_t* p_ble_name)
{
    uint32_t err_code = NRF_SUCCESS;
    
    gap_params_init(p_ble_name);
    
    gatt_init();
    //peer_manager_init();

    //services_init();
    advertising_init(); // Ĺ㲥Ҫ
    
    conn_params_init();
    
    return err_code;

}

//uint32_t BLE_AdvStart(void)
//{	
//    uint32_t err_code = NRF_SUCCESS;
//    
//    BLE_DMSG("<BLE> Adv Start.\r\n");
//    
//    if (b_ble_adv_start == 0)
//    {
//        BLE_DMSG("<BLE> Adv Start in.\r\n");
//        
//        b_ble_adv_start = 1;

//        err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
//        APP_ERROR_CHECK(err_code);
//        
//        if (err_code == NRF_SUCCESS)
//        {
//            BLE_DMSG("<BLE> BLE restart Ok.\r\n");
//        }
//        else
//        {
//            BLE_DMSG("<BLE> BLE restart failure.\r\n");
//        }
//    }

//	return err_code;
//}


uint32_t BLE_AdvStart(void * p_erase_bonds)
{	
    uint32_t err_code = NRF_SUCCESS;
    
    BLE_DMSG("<BLE> Adv Start request.\r\n");
    
    if (b_ble_init && b_ble_adv_start == 0)
    {
        BLE_DMSG("<BLE> Adv Start.\r\n");
        
        b_ble_adv_start = 1;
        
        advertising_init(); // Change the Adv data of device status.
        
        advertising_start(p_erase_bonds);
        
//        bool erase_bonds = *(bool*)p_erase_bonds;
//        if (erase_bonds == true)
//        {
//            delete_bonds();
//            // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
//            BLE_DMSG("<BLE> delete_bonds... \r\n\r\n");
//        }
//        else
//        {
//            err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
//            APP_ERROR_CHECK(err_code);

//            BLE_DMSG("<BLE> advertising is started..%s.\r\n", (err_code == NRF_SUCCESS) ? "ok" : "failure");
//        } 
    }

	return err_code;
}


uint32_t BLE_AdvStop(void)
{
	uint32_t err_code = NRF_SUCCESS;
	
	BLE_DMSG("<BLE> In BLE Adv Stop.\r\n");
     
    if (b_ble_adv_start)
    {
        b_ble_adv_start = 0;
        
        //LIGHT_SequenceSet(LIGHT_DO_NOTHING);

        BLE_DMSG("<BLE> Adv Stop2. ADVhandle=0x%08X\r\n", m_advertising.adv_handle);
        BLE_DMSG("<BLE> Adv Stop2. m_conn_handle=0x%08X\r\n", m_conn_handle);
        
        if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
        {
            BLE_DMSG("<BLE> dISCON11\r\n", m_conn_handle);
            
            // Disconnect from peer.
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
           
            
            uint32_t timeout_cnt = 5000; //5s at lease 350ms
            while ((f.BLE_CONNECT_STATUS == BLE_STATUS_CONNECT) && timeout_cnt)
            {
                delay_ms(1);
                timeout_cnt--;
            }
            BLE_DMSG("<BLE> STOP Timeout= %u \r\n", timeout_cnt);
        }
               
        err_code = sd_ble_gap_adv_stop(m_advertising.adv_handle);
        APP_ERROR_CHECK(err_code);
        
        if (err_code == NRF_SUCCESS)
        {
            b_ble_stop = 1;
            f.BLE_CONNECT_STATUS = BLE_STATUS_STOP;
            BLE_DMSG("<BLE> BLE Stop Ok.\r\n");
            
            //LIGHT_SequenceSet(LIGHT_DO_NOTHING);
        }
        else
        {
            BLE_DMSG("<BLE> BLE Stop failure.\r\n");
        }
        
        BLE_DMSG("<BLE> In BLE Adv Stop In.=%d\r\n", m_advertising.adv_handle);
    }
    
	return err_code;
}


void BLE_AdvInit(void)
{
    advertising_init();
    BLE_DMSG("<BLE> advertising_init.\r\n");
}

bool BLE_IsActive(void)
{
	if (b_ble_stop)
	{
		return false;
	}
	else
	{
		return true;
	}
}


void BLE_DeInit(void)
{
    return;
	
//    if (b_ble_init)
//    {
//        b_ble_init = 0;
//        
//        //softdevice_handler_sd_disable();
//         uint32_t err_code = NRF_SUCCESS;
//        // err_code          = sd_softdevice_disable();
//        
//         //Disable SoftDevice. It is required to be able to write to GPREGRET2 register (SoftDevice API blocks it).
//        //GPREGRET2 register holds the information about skipping CRC check on next boot.
//        err_code = nrf_sdh_disable_request();
//        APP_ERROR_CHECK(err_code);
//        
//        //sd_softdevice_disable();
//		
//		b_ble_stop = 1;
//		f.BLE_CONNECT_STATUS = BLE_STATUS_STOP;
//		
//        //sd_power_dcdc_mode_set(1); // dcdcģʽ
//        
//        BLE_DMSG("<BLE> BLE_DeInit.\r\n");
//    }
}



        
void BLE_Init(bool b_ble_adv_start)
{
	if (b_ble_init == 0)
    {
        BLE_DMSG("<BLE> BLE_Init.\r\n");
        
        static uint8_t b_semaphore_create = 0;
        if (b_semaphore_create == 0)
        {
            xSemaphore = xSemaphoreCreateMutex();
            if( xSemaphore != NULL )
            {
                /* The semaphore was created successfully and
                can be used. */
                b_semaphore_create = 1;
                
                BLE_DMSG("<BLE> xSemaphoreCreateMutex OK.\r\n");
            }
            else
            {
                BLE_DMSG("<BLE> xSemaphoreCreateMutex Failure\r\n");
                delay_ms(10);
                APP_ERROR_CHECK(1);
            }
        }
       
        
        static bool       erase_bonds; // ڶջ
        ret_code_t err_code;
        err_code = app_fifo_init(&ble_rx_ring_fifo, ble_rx_buffer, BLE_RX_RING_FIFO_SIZE);
        APP_ERROR_CHECK(err_code);
        
//        //˳Уwhy
//        ble_stack_init();
//        
//         // Initialize modules.
//        timers_init();
//        
//        peer_manager_init(); 
//        gap_params_init(&env.cfg.ble_adv_name[0]);
//        gatt_init();
//        advertising_init();
//        services_init();
//        conn_params_init();

 //
        ble_stack_init();
        
        // Initialize modules.
        timers_init();
        
        // 
        uint8_t deviceName[20] ={0};
        snprintf((char*)deviceName, 20, "%s%01X%02X", DEVICE_NAME, ((NRF_FICR->DEVICEADDR[0] >> 8) & 0xf), ((NRF_FICR->DEVICEADDR[0] >> 0) & 0xff));
        //
        gap_params_init(&deviceName[0]/*env.cfg.ble_adv_name[0]*/);
 
        gatt_init();
        peer_manager_init(); 
        services_init();
        advertising_init();
        conn_params_init();


        // Start execution.
        application_timers_start();
        erase_bonds = true;//false;
        //advertising_start(erase_bonds);
        
        // Create a FreeRTOS task for the BLE stack.
        // The task will run advertising_start() before entering its loop.
        //nrf_sdh_freertos_init(advertising_start, &erase_bonds);
        //nrf_sdh_freertos_init(NULL, &erase_bonds);
        // nrf_sdh_freertos_init(advertising_start, &erase_bonds);
        
        
        nrf_sdh_freertos_init((nrf_sdh_freertos_task_hook_t)&BLE_AdvStart, &erase_bonds);
//        if (b_ble_adv_start)
//		{
//			err_code = BLE_AdvStart();
//        }

        b_ble_init = 1;
    }
	
}


/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
