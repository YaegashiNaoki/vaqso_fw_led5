/**
  ******************************************************************************
  * @file    light.h
  * @author  Merle
  * @version V1.0.0
  * @date    26-July-2019
  * @brief   .
  ******************************************************************************
  */
#ifndef __LIGHT_H
#define __LIGHT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "color.h"

typedef enum {
    LIGHT_OFF                       = 0,
    
    LIGHT_SYSTEM_STARTUP            , //1
    LIGHT_SYSTEM_SHUTDOWN           , //2
    
    LIGHT_BLE_ADV_START             , //3
    LIGHT_BLE_BONDS_SUCCESS         , //4
    LIGHT_BLE_BONDS_FAILURE         ,
    
    LIGHT_BAT_DISCHARGE             , // 6
    LIGHT_BAT_CHARGING              ,
    LIGHI_BAT_CHARGING_COMPLETE     ,
    LIGHT_BAT_CHARGING_IRREGULAR    ,
    LIGHT_BAT_POWER_LOW             ,
    
    LIGHT_CARTRIDGE_ECPIRATION_1      ,
    LIGHT_CARTRIDGE_ECPIRATION_2      ,
    LIGHT_CARTRIDGE_ECPIRATION_3      ,
    LIGHT_CARTRIDGE_ECPIRATION_4      ,
    LIGHT_CARTRIDGE_ECPIRATION_5      ,
    
    LIGHT_CARTRIDGE_ATTACH_1        ,
    LIGHT_CARTRIDGE_ATTACH_2        ,
    LIGHT_CARTRIDGE_ATTACH_3        ,
    LIGHT_CARTRIDGE_ATTACH_4        ,
    LIGHT_CARTRIDGE_ATTACH_5        ,

    LIGHT_CARTRIDGE_DETACH_1        ,
    LIGHT_CARTRIDGE_DETACH_2        ,
    LIGHT_CARTRIDGE_DETACH_3        ,
    LIGHT_CARTRIDGE_DETACH_4        ,
    LIGHT_CARTRIDGE_DETACH_5        ,

    LIGHT_AIR_FAN1_TURN_ON        ,
    LIGHT_AIR_FAN2_TURN_ON        ,
    LIGHT_AIR_FAN3_TURN_ON        ,
    LIGHT_AIR_FAN4_TURN_ON        ,
    LIGHT_AIR_FAN5_TURN_ON        ,
    
    LIGHT_AIR_FAN1_TURN_OFF        ,
    LIGHT_AIR_FAN2_TURN_OFF        ,
    LIGHT_AIR_FAN3_TURN_OFF        ,
    LIGHT_AIR_FAN4_TURN_OFF        ,
    LIGHT_AIR_FAN5_TURN_OFF        ,    
    
    LIGHT_FAN_GIVES_OFF_SMELL      ,
    //
    LIGHT_TURN_ON_WITH_THE_SETTING_COLOR,
    TEST1                        ,
    TEST2                        ,
    TEST3                        ,
    TEST4                        ,
    TEST5                        ,
    TEST6                        ,
    TEST7                        ,
    TEST8                        ,
    TEST9                        ,
    TEST10                        ,
    TEST11                        ,
    TEST12                        ,
    TEST13                        ,
    TEST14                        ,
    LIGHT_DO_NOTHING             = 255,
} light_sequence_type_t;

void LIGHT_TurnOn(color_t const* color, uint16_t first, uint16_t count, uint16_t until_m_scond);

void LIGHT_ShowKnightRiderlights(void);

void LIGHT_ShowLowBattery(void);
void LIGHT_ShowFanGivesOffSmell(void);

void LIGHT_ShowCartridgeDetach(uint8_t ch);
void LIGHT_ShowCartridgeAttach(uint8_t ch);
void LIGHT_ShowCartridgeExpiration(uint8_t ch);

void LIGHT_ShowChargingIrregular(void);
void LIGHT_ShowChargingComplete(void);
void LIGHT_ShowCharging(void);

void LIGHT_ShowBleBondsFailure(void);
void LIGHT_ShowBleBondsSuccess(void);
void LIGHT_ShowBleAdvStart(void);

void LIGHT_ShowSystemShutdown(void);
void LIGHT_ShowSystemStartup(void);


void LIGHT_ShowWhite_Test();
void LIGHT_all_off_Test(void);
void LIGHT_ShowWhite_LED1_Test(void);
void LIGHT_ShowWhite_LED2_Test(void);
void LIGHT_ShowWhite_LED3_Test(void);
void LIGHT_ShowWhite_LED4_Test(void);
void LIGHT_ShowWhite_LED5_Test(void);


void LIGHT_SetColor(color_t const* color);
void LIGHT_TurnOnWithTheSettingColor(void);
    
void LIGHT_Init(void);
void LIGHT_DeInit(void);

uint32_t LIGHT_SequenceInit(void);
uint32_t LIGHT_SequenceFlush(void);
uint32_t LIGHT_SequenceSet(light_sequence_type_t type);
uint8_t LIGHT_SequenceGetLastType(void);

void LIGHT_SequenceScheduler(void);


#ifdef __cplusplus
}
#endif

#endif /* __LIGHT_H */

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
