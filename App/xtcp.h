/**
  ******************************************************************************
  * @file    xtcp.h
  * @author  Merle
  * @version V1.0.0
  * @date    9-July-2019
  * @brief   .
  ******************************************************************************
  */
  
#ifndef __XTCP_H
#define __XTCP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "board.h"
#include <stdint.h>
#include <stdbool.h>

#define XTCP_MAX_PAYLOAD_DATA_SIZE	(1024+64)
//	

typedef enum {
    CMD_CORE				= 0x00,
    CMD_AIR_FAN				= 0x10,
    CMD_LIGHT				= 0x20,
    CMD_CARTRIDGE			= 0x30,
    CMD_TEST				= 0x71,
    
    CMD_NOTIFY				= 0x77,	/**< only resp cmd. */
    CMD_ERROR				= 0xE1,	/**< only resp cmd. */
} CMD_TYPE;

typedef enum {
    CMD_CORE_ENTER_DFU_MODE			= 0x01,
    CMD_CORE_GET_FIRMWARE_VERSION	= 0x02,
    CMD_CORE_GET_BATTERY_INFO		= 0x03,
    CMD_CORE_GET_BLE_DEVICE_UDID	= 0x04,
    CMD_CORE_REBOOT					= 0x05,
} CMD_GENERAL_TYPE;

typedef enum {
    CMD_AIR_FAN_SPEED_CONTROL_ALL	= 0x01,
    CMD_AIR_FAN_SPEED_CONTROL_ALONE	= 0x02,
} CMD_AIR_FAN_TYPE;

typedef enum {
    CMD_LIGHT_COLOR_SET_ALONE		 = 0x01,
    CMD_LIGHT_COLOR_FILL_ALL_OR_PART = 0x02,
    CMD_LIGHT_SEQUENCE_SET           = 0x03,
} CMD_LIGHT_TYPE;

typedef enum {
    CMD_CARTRIDGE_GET_STATUS	        = 0x01,
    CMD_CARTRIDGE_INFO_WRITE			= 0x02,
    CMD_CARTRIDGE_INFO_READ			    = 0x03,
    CMD_CARTRIDGE_ENABLING_DATE_WRITE   = 0x04,
    CMD_CARTRIDGE_ENABLING_DATE_READ    = 0x05,
} CMD_CARTRIDGE_TYPE;

typedef enum {
    CMD_TEST_AIR_FAN			            = 0x01,
    CMD_TEST_LIGHT				            = 0x02,
    CMD_TEST_CARTRIDGE_INFO_WRITE	        = 0x03,
    CMD_TEST_CARTRIDGE_INFO_READ		    = 0x04,
    CMD_TEST_CARTRIDGE_ENABLING_DATE_WRITE  = 0x05,
    CMD_TEST_CARTRIDGE_ENABLING_DATE_READ   = 0x06,
} CMD_TEST_TYPE;

typedef enum {
    CMD_NOTIFY_CARTRIDGE_STATUS		= 0x01,
    CMD_NOTIFY_POWER_STATUS	        = 0x02,
    CMD_NOTIFY_BATTERY_STATUS		= 0x03,
} CMD_NOTIFY_TYPE;

typedef enum {
    CMD_ERROR_CALC_DATA_XOR		        = 0x01,  /**< Calculate data xor error. */
    CMD_ERROR_CALC_PARAM_DATA_LEN	    = 0x02,  /**< Calculate parameter data length error. */
    CMD_ERROR_PACKAGE_INVALID		    = 0x03,  /**< The package is invalid. */
    CMD_ERROR_CMD_TYPE_INVALID		    = 0x04,  /**< The command_type is invalid. */
    CMD_ERROR_CMD_INVALID			    = 0x05,  /**< The command is invalid. */
    CMD_ERROR_PARAM_DATA_INVALID	    = 0x06,  /**< The param data in this command is invalid(may be the invalid format or error length). */
    CMD_ERROF_DATA_HAVE_INVALID_0X7D    = 0x07,  /**< The data between 0x7E and 0x7F have the invalid 0x7D, for example: 0x7D 0x04/ 0x7D 0x00 */
} CMD_ERROR_TYPE;


//-----------CMD_NOTIFY_TYPE----PARAM-----------
typedef enum {
    PARAM_NOTIFY_CARTRIDGE_ATTACH	    = 0x01,
    PARAM_NOTIFY_CARTRIDGE_DETACH	    = 0x02,
} PARAM_NOTIFY_CARTRIDGE_STATUS_TYPE;

typedef enum {
    PARAM_NOTIFY_POWER_ON        = 0x01,
    PARAM_NOTIFY_POWER_OFF       = 0x02,
} PARAM_NOTIFY_POWER_STATUS_TYPE;

typedef enum {
    PARAM_NOTIFY_BATTERY_CHARGING               = 0x01,
    PARAM_NOTIFY_BATTERY_CHARGING_COMPLETE      = 0x02,
    PARAM_NOTIFY_BATTERY_CHARGING_IRREGULAR	    = 0x03,
    PARAM_NOTIFY_BATTERY_DISCHARGE              = 0x04,
    PARAM_NOTIFY_BATTERY_POWER_LOW              = 0x05,
} PARAM_NOTIFY_BATTERY_STATUS_TYPE;
//-----------CMD_NOTIFY_TYPE----PARAM-----------

//typedef enum {
//    CMD_NOTIFY_CARTRIDGE_ATTACH		= 0x01,   
//    CMD_NOTIFY_CARTRIDGE_DETACH		= 0x02,    
//    CMD_NOTIFY_POWER_ON				= 0x10,              
//    CMD_NOTIFY_POWER_OFF			= 0x11,         
//    CMD_NOTIFY_CHARGING				= 0x12,      
//    CMD_NOTIFY_CHARGING_COMPLETE	= 0x13, 
//    CMD_NOTIFY_CHARGING_IRREGULAR	= 0x14,  
//    CMD_NOTIFY_DISCHARGE			= 0x15,      
//    CMD_NOTIFY_LOW_BATTERY			= 0x16, 
//} CMD_NOTIFY_TYPE;




#pragma pack(1)

typedef struct xtcp_packet_t {
    uint8_t packet_buf[XTCP_MAX_PAYLOAD_DATA_SIZE];   // the data send.
	uint16_t packet_len;

} xtcp_packet_t;


// AIR_FAN
typedef struct air_fan_param_t {
    uint8_t speed;              /**< ==0~255, stop:0; high speed:255.*/
} air_fan_param_t;

typedef struct pkg_air_fan_set_all_t {
    air_fan_param_t channel[5];    /**<  */
} pkg_air_fan_set_all_t;

typedef struct pkg_air_fan_set_alone_t {
    uint8_t channel;    /**<  */
    uint8_t speed;
} pkg_air_fan_set_alone_t;

// LIGHT
typedef struct light_fill_all_or_part_color_t {
    color_t color;              /**< ==0~255, 255 is the high brightness.*/
    uint8_t first;
    uint8_t count;
} light_fill_all_or_part_color_t;

// CARTRIDGE
typedef struct resp_cartridge_get_status_t {
    uint8_t status;
} resp_cartridge_get_status_t;

typedef struct resp_cartridge_get_status_data_t {
    uint8_t err_code;
    resp_cartridge_get_status_t channel[5];
} resp_cartridge_get_status_data_t;

#pragma pack()


void XTCP_Loop(void);

//uint16_t XTCP_PacketMake(uint8_t cmd_type, uint8_t cmd, uint16_t packet_payload_data_len, uint8_t const* packet_payload_data, uint8_t* resp_data);
uint16_t XTCP_MakePackage(uint8_t cmd_type, uint8_t cmd, uint16_t param_data_len, uint8_t const* p_param_data, uint8_t* p_resp_pkg);

extern xtcp_packet_t packet_send;


#ifdef __cplusplus
}
#endif

#endif /* __XTCP_H */

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
