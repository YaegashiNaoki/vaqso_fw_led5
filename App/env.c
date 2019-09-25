/**
  ******************************************************************************
  * @file    env.c
  * @author  Merle
  * @version V1.0.0
  * @date    22-Feburary-2017
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"



//#define DBG_ENV
#ifdef DBG_ENV
#define ENV_DMSG	RTT_Printf
#else
#define ENV_DMSG(...)
#endif


//#define ASSERT_CONCAT_(a, b) a##b
//#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
//#define ct_assert(e) enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }



#define ENV_CONF_VERSION    (17)
#define STORE_PAGE_SIZE     (255)
#define BLE_DEFAULT_ADV_NAME    {"VAQSO_VR_V5"}
static const uint8_t ble_default_adv_name[20] = BLE_DEFAULT_ADV_NAME;

env_t env = {
    .cfg.env_version = ENV_CONF_VERSION,
    .cfg.env_size = sizeof(config_t),
    .cfg.lastPowerDown_BatteryPercentage = 1000,
    .cfg.ble_adv_name = BLE_DEFAULT_ADV_NAME,
};


void ENV_Save(void)
{
    bool ret;
    
    ENV_DMSG("<ENV_Save> ENV_SAVE in \r\n");

    env.cfg.lastPowerDown_BatteryPercentage = bat.percentage;

    env.cfg.laseSaveEnvTime = RTC2_GetTime();

    time_t aTime = env.cfg.laseSaveEnvTime;

    struct tm tmTime = *localtime(&aTime);

    ENV_DMSG("<ENV_Save> DayTime %04d-%02d-%02d, %02d:%02d:%02d. \r\n", 
		tmTime.tm_year + 1900, 
		tmTime.tm_mon +1,
		tmTime.tm_mday,
		tmTime.tm_hour,
		tmTime.tm_min,
		tmTime.tm_sec
		);


    uint32_t write_addr = 0;
    ret = EEPROM_WriteBytesWithMagic(0, write_addr, (uint8_t *)&env, sizeof(env_t));

    ENV_DMSG("<ENV> Write ret=%u.cfg.batPercentage=%d, .RTC=%u.\r\n", 
            ret, 
            env.cfg.lastPowerDown_BatteryPercentage, 
            env.cfg.laseSaveEnvTime);
}


void ENV_ActiveConfig(void)
{   
	ENV_DMSG("<ENV> ActiveConfig .. bat.percent=%d.env.cfg.lastPowerDown_BatteryPercentage=%d\r\n",  bat.percentage,env.cfg.lastPowerDown_BatteryPercentage);
	
	bat.percentage = env.cfg.lastPowerDown_BatteryPercentage;
}


void ENV_Reset(void)
{
	uint16_t batPercentage = env.cfg.lastPowerDown_BatteryPercentage;

    
    uint8_t default_ble_adv_name[20];// = BLE_DEFAULT_ADV_NAME;
    memcpy(default_ble_adv_name, ble_default_adv_name,  20);
   
	////////////////////////
    
	// Reset params.
	memset(&env, 0, sizeof(env_t));
    

    ///////////////////////////////////////////////

    memcpy(env.cfg.ble_adv_name, default_ble_adv_name, 20);


	env.cfg.lastPowerDown_BatteryPercentage = batPercentage;
	ENV_ActiveConfig();


	env.cfg.env_version = ENV_CONF_VERSION;
	env.cfg.env_size = sizeof(config_t);
	env.cfg.lastPowerDown_BatteryPercentage = batPercentage;

	
//	struct tm t;
//	t.tm_year =  2019 - 1900;
//	t.tm_mon = 5 - 1;
//	t.tm_mday = 1;
//	t.tm_hour = 12;
//	t.tm_min = 0;
//	t.tm_sec = 0;
//	env.cfg.laseSaveEnvTime = mktime(&t);
    
	//BAT_PowerLowCheckParameterReset();

	//
	
	ENV_DMSG("ENV_Reset...1111.....RTC=%u\r\n",RTC2_GetTime());
	ENV_DMSG("<WRITE> ENV_ActiveConfig .. bat.percentage=%d.\r\n",  bat.percentage);
}

void ENV_Load(void)
{
	bool ret;
    
    ret = EEPROM_Init();
    
	uint16_t read_count = 0;
	uint32_t read_addr = 0;
	
	uint16_t normal_len = sizeof(env_t);

    ret = EEPROM_ReadBytesWithMagic(0, read_addr, (uint8_t *)&env, sizeof(env_t), &read_count);
    if (ret)
    {
        if (normal_len != read_count)
        {
            ENV_DMSG("<ENV> Load failure, env_size =%d, read_szie=%d \r\n", 
                        normal_len, 
                        read_count);
			ENV_Reset();
        }
        else
        {     
            BAT_ResetRefrence();
            
            ENV_DMSG("<ENV> Load ok, addr=0x%08X, cfg.batPercentage=%d,normal_len=%d,read_count=%d \r\n", 
                read_addr, 
                env.cfg.lastPowerDown_BatteryPercentage,
                normal_len, 
                read_count);
            ENV_DMSG("<ENV> bootTime = %u\r\n", env.cfg.bootTime);
            
            if (env.cfg.ble_adv_name[0] == 0)
            {
                memcpy(env.cfg.ble_adv_name, ble_default_adv_name, sizeof(ble_default_adv_name));
                ENV_DMSG("<ENV> ble name error.\r\n");
            }
        }
    }
	else
	{
        ENV_DMSG("<ENV> Load failure.ret=%d. read_count=%d.\r\n", ret, read_count);
		ENV_ActiveConfig();
        
        //ENV_Save();
	}
}


//env_t * ENV_Get(void)
//{
//	return &env;
//}


void ENV_Init(void)
{
	 // make sure (at compile time) that config struct doesn't overflow allocated flash pages
    ct_assert(sizeof(env) < (STORE_PAGE_SIZE - 2 * 4));
	
	ENV_Load();
}


/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
