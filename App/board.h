/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOARD_H
#define __BOARD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
	 
#define __USE_C99_MATH
	 
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>


#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf_dfu_ble_svci_bond_sharing.h"
#include "nrf_svci_async_function.h"
#include "nrf_svci_async_handler.h"

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "bsp_btn_ble.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_state.h"
#include "ble_dfu.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "fds.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_clock.h"
#include "nrf_power.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_bootloader_info.h"

#include "ble_nus.h"
#include "nrf_delay.h"
#include "board.h"



	 
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
	 
// add
#include "ble_srv_common.h"
//#include "boards.h"

#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
//#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_nus.h"
//#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp.h"
#include "bsp_btn_ble.h"

#include "boards.h"
#include "app_util_platform.h"
#include "app_error.h"
//#include "fstorage.h"
// add
#include "fds.h"
#include "peer_manager.h"
//#include "sensorsim.h"
#include "ble_conn_state.h"
#include "ble_dfu.h"


#include "nrf_wdt.h"
#include "nrf_drv_wdt.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_pwm.h"
#include "nrf_saadc.h"
#include "nrf_drv_saadc.h"
//#include "app_timer.h"
#include "nrf_drv_timer.h"
#include "app_fifo.h"
//#include "app_uart.h"
#include "nrf_drv_uart.h"
#include "nrf_drv_twi.h"
#include "nrf_drv_spi.h"
#include "nrf_spi.h"
#include "nrf_drv_clock.h"
#include "nrf_clock.h"
#include "nrf_drv_rtc.h"
#include "nrf_rtc.h"
#include "nrf_drv_pdm.h"
#include "nrf_pdm.h"
#include "sdk_macros.h"
#include "nrf_drv_rng.h"
#include "nrf_drv_power.h"
#include "nrf_drv_i2s.h"
#include "nrf_i2s.h"
//

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#include "boards.h"
#include "app_error.h"
#include <stdint.h>
#include <stdbool.h>
#include "nrfx_spim.h"

#include "nrf_sdh_freertos.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

//#include "nrf_adc.h"
//#include "nrf_drv_adc.h"
#include "nrf_delay.h"
//#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "ble_dfu.h"




#define ARRAY_LEN(a)            (sizeof(a)/sizeof(a[0]))          /* calculate array length */
	

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
//#define abs(x) ((x) > 0 ? (x) : -(x))


#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
#define ct_assert(e) enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }


#define RTT_Printf(...)    (SEGGER_RTT_printf(0, ## __VA_ARGS__))


#if 1
#define DBG_MAIN
#define DBG_THREAD
#define DBG_XTCP
#define DBG_SYSTEM
#define DBG_ENV
#define DBG_ADC
//#define DBG_BAT
#define DBG_I2C
#define DBG_BLE
#define DBG_MW
#define DBG_UART
#define DBG_WDT
#define DBG_RTC2
//#define DBG_KEY
#define DBG_TIMER
#define DBG_PCA9685
#define DBG_TCA9548A
#endif


/* 
 *   GPIO Define 
 */
#if 1
#define BOARD_MIX_POWER_CTL		    (9U)
#define BOARD_DCDC_5V_POWER_CTL     (8U)
#define BOARD_LED_POWER_CTL         (4U)

#define BOARD_KEY                   (10U)


#define	BOARD_BAT_ADC_INPUT_AIN		(NRF_SAADC_INPUT_AIN4)
#define BOARD_BAT_ADC			    (28U)
#define BOARD_BAT_CHARGE		    (18U)
#define BOARD_BAT_STANBY		    (20U)

#define BOARD_I2C_RESET			    (21U)

#define BOARD_I2C2_SCL	 		    (3U)
#define BOARD_I2C2_SDA   		    (2U)

#define BOARD_UART_RX		 	    (5U)
#define BOARD_UART_TX		 	    (7U)

#define BOARD_I2S_SDOUT_PIN		    (6U)
#define BOARD_I2S_SCK_PIN		    (11U)

#define BOARD_REBOOT_BYTE           (0xeB)


#else

#define BOARD_MIX_POWER_CTL		    (9U)

#define BOARD_KEY                   (10U)

#define	BOARD_BAT_ADC_INPUT_AIN		(NRF_SAADC_INPUT_AIN4)
#define BOARD_BAT_ADC			    (28U)
#define BOARD_BAT_CHARGE		    (18U)
#define BOARD_BAT_STANBY		    (20U)

#define BOARD_I2C_RESET			    (21U)

#define BOARD_I2C2_SCL	 		    (2U)
#define BOARD_I2C2_SDA   		    (3U)

#define BOARD_UART_RX		 	    (6U)
#define BOARD_UART_TX		 	    (5U)

#define BOARD_I2S_SDOUT_PIN		    (4U)
#define BOARD_I2S_SCK_PIN		    (11U)

#define BOARD_REBOOT_BYTE           (0xeB)

#endif

#include "SEGGER_RTT.h"
#include "math_algo.h"
#include "color.h"
#include "env.h"
#include "mw.h"

#include "drv_i2c.h"
#include "drv_adc.h"
#include "drv_rtc.h"
#include "drv_wdt.h"
#include "drv_bat.h"

#include "xTimer.h"

#include "drv_ble.h"
#include "drv_timer.h"
#include "drv_key.h"
#include "drv_uart.h"
////
#include "system.h"
#include "drv_pca9685.h"
#include "drv_tca9548a.h"
#include "drv_eeprom.h"

#include "drv_fan.h"
#include "drv_ws2812b.h"
#include "light.h"
#include "drv_cartridge.h"

#include "xtcp.h"
#include "thread.h"


void delay_us(uint32_t us);
void delay_ms(uint32_t ms);


#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H */

/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
