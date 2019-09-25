#ifndef __DRV_MW_H
#define __DRV_MW_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
	
typedef enum {
	BLE_STATUS_STOP = 0,
	BLE_STATUS_CONNECT = 1,
	BLE_STATUS_DISCONNECT = 2,	
	BLE_STATUS_FAST_ADV  = 3,
} BLE_CONNECT_STATUS_E;


typedef enum {
	SYSTEM_RUN_STATUS_POWER_OFF = 0,
	SYSTEM_RUN_STATUS_NORMAL = 1,
	SYSTEM_RUN_STATUS_STANDBY = 2,
} SYSTEM_RUN_STATUS_E;

    
/////////////////////////////////////////////////////////////////////////
#pragma pack(1)


// 全局参数 
typedef struct global_val_t {
	struct tm tmTime;
    uint32_t battery_LastChargeTime;
} global_val_t;



// 全局 状态标志
typedef struct flags_t {
	uint8_t BLE_CONNECT_STATUS;				    /**< BLE connect status. **/	
	uint8_t SYSTEM_RUN_STATUS;				    /**< System running status. **/
	uint8_t NOTICE_BATTERY_IS_TOO_LOW;  	    /**< Notice battery is to low. **/
} flags_t;


typedef struct bat_t {
    uint8_t chargeState;        // 0:discharge; 1:charging; 2:charging complete; 3:charging error.
	uint16_t percentage;	    // remain bremain battery percentage X10, 0~1000 / 10 = (0~100%).
    uint16_t voltage;           // 367 means -> 3.67V
} bat_t;


//max = 
typedef struct config_t {
	uint8_t env_version;			                // env config version
    uint16_t env_size;				                // env config size 
	uint32_t bootTime;                              // last boot time.
    uint8_t reboot_status;                          // unuseful
	uint16_t lastPowerDown_BatteryPercentage;	    // last powerdown of the beterry percentage.
	uint32_t laseSaveEnvTime;                       // lase powerdown of the time.
    uint8_t ble_adv_name[20];                       // the last char must be '\0' at the end.
} config_t;



// data save in powerdown
typedef struct env_t {
	config_t cfg;
} env_t;
#pragma pack()


void MW_Loop(void);
void MW_ParameterUpdate(void);

extern const uint8_t deviceFirmwareVersion[];
extern const uint8_t deviceHardwareVersion[];
extern const uint8_t deviceBootLoadVersion[];
extern const uint8_t deviceFirmwareBuidTime[];

extern global_val_t global;
extern flags_t f;
extern bat_t bat;
extern env_t env;


#ifdef __cplusplus
}
#endif

#endif /* __DRV_MW_H */

/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
