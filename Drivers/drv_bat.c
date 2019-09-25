/**
  ******************************************************************************
  * @file    drv_bat.c
  * @author  
  * @version V1.0.0
  * @date    20-January-2017
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"


//#define DBG_BAT
#ifdef DBG_BAT
#define BAT_DMSG	RTT_Printf
#define SNPRINTF    snprintf
#else
#define BAT_DMSG(...)
#define SNPRINTF(...)
#endif


#define BAT_ADC_PIN		BOARD_BAT_ADC
#define BAT_CHRAGE_PIN 		BOARD_BAT_CHARGE
#define BAT_STANBY_PIN 		BOARD_BAT_STANBY

static uint8_t b_bat_io_ctl_init = 0;

static uint8_t usb_cable_in_status = 0;

uint8_t BAT_GetUSBConnectStatus(void)
{
    uint8_t status = usb_cable_in_status;
    return status;
}

BAT_CHARGE_STATUS_E BAT_GetChargeStatus(void)
{
	BAT_CHARGE_STATUS_E chargeStatus;
	
    
	__IO uint8_t io_charge_val = nrf_gpio_pin_read(BAT_CHRAGE_PIN);
	__IO uint8_t io_stanby_val = nrf_gpio_pin_read(BAT_STANBY_PIN);

    if (io_stanby_val)
    {
        if (io_charge_val == 0)
        {
            chargeStatus = BAT_CHARGE_STATUS_CHARGE;
        }
        else
        {
            chargeStatus = BAT_CHARGE_STATUS_COMPLETE;
        }
        
        usb_cable_in_status = 1;
    }
    else
    {
        chargeStatus = BAT_CHARGE_STATUS_DISCHARGE;
        
        usb_cable_in_status = 0;
    }
//	if (io_charge_val && io_stanby_val)					// io_charge_val == 1 &  io_stanby_val == 1, Undervoltage or Over temperature.
//	{
//		chargeStatus = BAT_CHARGE_STATUS_ERROR;
//	}
//	else if (io_charge_val && (io_stanby_val == 0))		// io_charge_val == 1 &  io_stanby_val == 0, Finished charging.
//	{
//		chargeStatus = BAT_CHARGE_STATUS_COMPLETE;
//	}
//	else if ((io_charge_val == 0) && io_stanby_val)		// io_charge_val == 0 &  io_stanby_val == 1, Charging.
//	{
//		chargeStatus = BAT_CHARGE_STATUS_CHARGE;
//	}
//	else												// io_charge_val == 0 &  io_stanby_val == 0, No in charge.
//	{
//		chargeStatus = BAT_CHARGE_STATUS_DISCHARGE;
//	}
	return chargeStatus;
}






#define BOARD_BAT_CAPACITY		(200U)	// 600电池的容量mA.h



//200mA.h   ->max = 150 *7.95 = 1192mA.h
#define BAT_CAPACITY	(BOARD_BAT_CAPACITY * 8 / 10)	
//4294967296 / 5400000 = 795
//150mA.h   ->max = 150 *7.95 = 1192mA.h

// every percentage of battery can use BAT_US_SEC time.
// 百分之一的电的容量为BAT_UA_SEC uA.sec
#define BAT_UA_SEC	((uint64_t)BAT_CAPACITY * 1000 * 3600 / 100)
				
#define BAT_CHARGE_TO_DISCHAEGE_UPDATE_PERCENTAGE_TIME	(60 * 8)		// //60S = (60 * (8 * 125ms))
				
// voltage->	4142.2,4011.2,3912.9,3831.0,3774.9,3734.4,3688.4,3605.3
// 		adc->   100,	 86,  72,  	  58, 	43,  29, 	  15, 	 1

//V   = 416.17 410.00
//ADC = 3700   3645	

//ADC_SAMPLE_FULL_REF = 0.11352887834821428571428571428571
//3.3v 2907
//
////-------Battery in discharge.
//static const uint16_t dischargeAdvValueTable[11] = {           //3.0V // 2.8v  //3.3v
//	3585, 3498, 3410, 3349, 3297, 3266, 3236, 3209, 3192, 3074, 2643  /*2467*/  /*2907*/
//};

//static const uint8_t dischargePercentageTable[11] = {
//	100,  90,   80,   70,   60,   50,   40,   30,   20,   10,   1
//};
//-----------------------------------------------------------------------------------
//4.1, 4.0, 3.9, 3.83, 3.77, 3.735, 3.70, 3.67, 3.65, 3.615, 3.32


//-------Battery in discharge.
static const uint16_t dischargeAdvValueTable[9] = {           //3.0V // 2.8v  //3.3v
	3585, 3461, 3364, 3276, 3206, 3135, 2994, 2906, 2642//, 3074, 2643  /*2467*/  /*2907*/
};

static const uint8_t dischargePercentageTable[9] = {
	100,  82,   64,   46,   28,   10,   6,   3,     1
};
////ADC_SAMPLE_FULL_REF = 0.11352887834821428571428571428571
//20190916                       40mA 15mA 5mA  
//4.1, 3.93    3.82  3.72   3.64  3.56  3.4   3.3 3.0

//100  82      64     46     28   10    6     3   1

//-------Battery in charge.
//-----------------------------------------------------------------------------------
// every 5min. then current of charge.


// 10x.
// 0.1% ~ 10%, 10% ~ 20%,
// 20190911 before.
//static const uint16_t chargingPercentageTable[15] = {
//	1,   100, 200, 300, 400, 500, 600, 650, 700, 750, 800, 850, 900, 950, 1000
//};
//static const uint16_t chargingPercentageTable[15] = {
//	1,   100, 200, 300, 400, 500, 600, 650, 700, 750, 800, 850, 900, 950, 1000
//};

//static const uint16_t chargingCurrentTable[15] = {       
//    100, 95,  90,  85,  80,  70,  60,  50,  40,  30,  20,  10,  7,   50,   10
//};

static const uint16_t chargingPercentageTable[15] = {
	1,   100, 200, 300, 400, 500, 600, 650, 700, 750, 800, 850, 900, 950, 1000
};

static const uint16_t chargingCurrentTable[15] = {       
    230, 220,  210,  200,  200,  200,  200,  200,  200,  200,  200,  180,  150,  100,  50
};
//350, 320, 300, 280, 260, 240, 230, 200, 195, 190, 180, 175, 135, 100, 80 

//-----------------------------------------------------------------------------------
// uA.s
static uint32_t bat_calc_charging_current(uint16_t charging_percentage)
{
	uint16_t the_charging_percentage = charging_percentage;
	uint8_t length = ARRAY_LEN(chargingPercentageTable);
    uint8_t i  = 0;
    
    if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
	{
        return 1000; //100uA
    }
    
    for (i = 0; i < length; ++i)
	{
        if(the_charging_percentage <= chargingPercentageTable[i])
		{
            break;
        }
    }
    
    if (i >= length)
	{
        return chargingCurrentTable[length - 1]; //1000; //1000uA
    }

	float current;
    if (i == 0)
	{
        current = chargingCurrentTable[0];//400.0f;// 0% * 10x
    }
	else
	{
		current = (float)chargingCurrentTable[i-1] + (float)(the_charging_percentage - chargingPercentageTable[i-1])
                  *(float)((float)(chargingCurrentTable[i] - chargingCurrentTable[i-1]) 
                   / (float)(chargingPercentageTable[i] - chargingPercentageTable[i-1]));
	}
    
	uint32_t charge_current = (uint32_t)((float)current * 1000.0f);
    
	return charge_current;
}

/*
 1. 拔出充电器，应该等待2min后在显示电池的电压，（待电池电压稳定）
*/

static uint8_t b_get_base_of_charging_percentage = 0;
static uint16_t base_of_charging_percentage = 0;
static uint32_t sum_of_charging_current = 0;

void ADC_GetBaseChargePercentageEnable(void)
{
	b_get_base_of_charging_percentage = 1;
}

void ADC_GetBaseCharagePercentageDisable(void)
{
	b_get_base_of_charging_percentage = 0;
}

uint8_t ADC_GetBaseChargePercentageStatus(void)
{
	return b_get_base_of_charging_percentage;
}


//real percentage = returnValue / 10.
//static uint16_t calc_percentage(uint16_t adc)
static uint16_t bat_calc_discharge_percentage(uint16_t discharge_adc_value)
{
	uint16_t the_discharge_adc_value = discharge_adc_value;
	uint8_t length = ARRAY_LEN(dischargeAdvValueTable);
    uint8_t i  = 0;
    
    if (bat.chargeState == BAT_CHARGE_STATUS_COMPLETE)
	{
        return 1000;   // 100% * 10x
    }
    
    for (i = 0; i< length; ++i)
	{
        if (the_discharge_adc_value >= dischargeAdvValueTable[i])
		{
            break;
        }
    }
    
    if (i >= length)
	{
        return 0;
    }

    if (i == 0)
	{
        return 1000; // 100% * 10x
    }
	
	uint16_t percentage;
	percentage = (float)(the_discharge_adc_value - dischargeAdvValueTable[i]) * 10.0f
                / (float)((dischargeAdvValueTable[i-1] - dischargeAdvValueTable[i]) / (float)(dischargePercentageTable[i-1] - dischargePercentageTable[i]))
                + (float)dischargePercentageTable[i] * 10.0f;
	return percentage;
}



static uint16_t battery_of_charging_percentage = 1000;
static uint16_t battery_of_discharge_percentage = 0;
static uint32_t getBaseChargingPercentageLoopTime;
//static uint16_t battery_percentage_prevoius = 1000;

//static uint32_t battery_LastChargeTime = 0;
//static uint8_t b_charge_status = 0;

#define ADC_SAMPLE_RES1		300//100//300//300

#define ADC_SAMPLE_RES2		560//100//560//560

// Merle20190521 有误差，经验值
// NRF_SAADC_REFERENCE_INTERNAL VNRF_SAADC_GAIN1_5 -> 0.6 / (1 / 5) = 3.0V(Full range 4095) 
//ADC_SAMPLE_FULL_REF = 0.11352887834821428571428571428571
#define ADC_SAMPLE_FULL_REF	((302.8f * (float)(ADC_SAMPLE_RES1 + ADC_SAMPLE_RES2) / (float)ADC_SAMPLE_RES2) / 4096.0f)
    
//#define ADC_SAMPLE_FULL_REF	((300.0f * (float)(ADC_SAMPLE_RES1 + ADC_SAMPLE_RES2) / (float)ADC_SAMPLE_RES2) / 4096.0f)
// 300 xx = 0.11247907366071428571428571428571
//0.1135
// 305 xx = 0.11435372488839285714285714285714
//3062 * xx//3.44V
//2942
//2,894.5281175860509979126167321215
// //4.1, 4.0, 3.9, 3.83, 3.77, 3.735, 3.70, 3.67, 3.65, 3.615, 3.32

//float tabf[12] = {410, 400, 390, 383, 377, 373.5, 370, 367, 365, 361.5, 351.5, 332};// 计算
//3585,3498,3410,3349,3297,3266,3236,3209,3192,3161,3074,2903 


//bool BAT_WakeUpBatteryCheck(bool update_bat)
//{
//   bool ret = false;
//    
//   BAT_IO_Ctl_Init();
//   ADC_Init();
//   // 
//   ADC_DataUpdateOnce();
//   delay_us(50);
//   ADC_DeInit(); 
//   //
//   bat.chargeState = BAT_GetChargeStatus();
//   //BAT_IO_Ctl_DeInit();
//    
//   if (bat.chargeState == BAT_CHARGE_STATUS_DISCHARGE)
//   {
//       int32_t m_adc_value;
//       ret = ADC_GetValue(&m_adc_value);
//	
//       // 数据有效
//       if (ret)
//       {
//			uint8_t batteryVoltage = (uint8_t)((float)m_adc_value * ADC_SAMPLE_FULL_REF)
//			uint16_t bat_percent = calc_discharge_percentage(m_adc_value);
//			if (update_bat)
//            {
//                bat.percentage = bat_percent;
//            }
//		   //

//			BAT_DMSG("<BAT> WakeUp With Battery Check ADC =%u,percent=%d,Voltage=%d\r\n", 
//                        m_adc_value, 
//                        bat_percent, 
//                        batteryVoltage + 200);
//                        
//			// 当电量大于5%才允许正常开机
//			if (bat_percent > 50)// 0.9% // 50
//			{
//				ret  = true;
//			}
//			else
//			{
//				ret = false;
//			   //
//			}
//       }
//       else
//       {
//            ret = true;
//       }
//   }
//   else
//   {
//        ret = true;
//   }
//   
//   return ret;
//}


///*
//The battery is too low charge for a while before power on. ??
//电池电量太低,在通电之前无法再支撑。
//*/
static uint8_t bat_low_for_gui_show_cnt = 0;
static uint8_t b_bat_low_check_first_in = 0;
static uint32_t batLowCheckDeadline = 0;
static uint32_t batLowCheckLoopTime;			// 电池电量过低检查

//void BAT_PowerLowCheckParameterReset(void)
//{
//	bat_low_for_gui_show_cnt = 0;
//	b_bat_low_check_first_in = 0;
//	batLowCheckDeadline = 0;
//	batLowCheckLoopTime = 0;
//}

// 如果电池电压为0时，弹出提示框，然后在10分钟后保存参数后关机
void BAT_PowerIsToLowCheck(void)
{
    ///return;
   
    int64_t dtDeadline = (int64_t)RTC2_Get125MsCounter() - batLowCheckDeadline;	
	if (dtDeadline < 0)
	{
		if (dtDeadline < (-9))
		{
			batLowCheckDeadline = 0;
		}
        return;
	}
	else
	{
		batLowCheckDeadline = RTC2_Get125MsCounter() + 8; // 1000ms = 1Sec检查一次
        
        // 电池电量小于20%并且不是充电状态下
        if ((bat.percentage < 200) && (bat.chargeState == BAT_CHARGE_STATUS_DISCHARGE))//if (bat.percent == 0)
        {
            if (b_bat_low_check_first_in == 0)//if (bat.percent == 0)
            {
                b_bat_low_check_first_in = 1;
                
                BAT_DMSG("<BAT> battery too low Check FirstIn.\r\n");
                
                batLowCheckLoopTime = RTC2_Get125MsCounter() ; //+ 3000; //获取当前的时间基准
            }
            

            // 再过10分钟后不充电，就保存数据后强制关机
			dtDeadline = (int64_t)RTC2_Get125MsCounter() - batLowCheckLoopTime;
			if (dtDeadline < 0)
			{
				if (dtDeadline < (-1441))
				{
					batLowCheckLoopTime = 0;
				}
                return; 
            }
            else
            {
                batLowCheckLoopTime = RTC2_Get125MsCounter() + 1440;// 180 * 8 * (125ms)=1440*(125ms) 6000; //3分钟18000  60S
                
                // 弹出框提示 电量过低请充电
                f.NOTICE_BATTERY_IS_TOO_LOW = 1;
                
                BAT_DMSG("<BAT> battery is too low, gui show notice 1. batPercent=%d.\r\n", bat.percentage);
            }
            
            // < 5% = 50
            if (bat.percentage < 50) 
            { 
                bat_low_for_gui_show_cnt++;
                
                if (bat_low_for_gui_show_cnt > 2)
                {
                    bat_low_for_gui_show_cnt = 0;
                    
                    //systemEvent.b.shutdown = 1;

                    
                    b_bat_low_check_first_in = 0;
                }
                BAT_DMSG("\r\n<BAT> battery is too low, auto shutdown 111. cnt=%d\r\n", bat_low_for_gui_show_cnt);
            }
            else
            {
                 bat_low_for_gui_show_cnt = 0;
            }
      
            
//            // 弹出框提示 电量过低请充电
//            f.NOTICE_BATTERY_IS_TOO_LOW = 1;

            
            //BAT_DMEG("<BAT> battery is too low, gui show notice 2.\r\n");
        }
        else
        {
            if (b_bat_low_check_first_in)
            {
                f.NOTICE_BATTERY_IS_TOO_LOW = 0;
            }
            b_bat_low_check_first_in = 0;
            bat_low_for_gui_show_cnt = 0;
        }
    }
}

// 2Sec, 5Sec 60Sec.		
void BAT_DataCompute(void)
{
	int32_t m_adc_value;
	bool ret = ADC_GetValue(&m_adc_value);
	// 
	if (ret)
	{ 
        volatile uint8_t batteryChargeState = BAT_GetChargeStatus();
        
        // step01.获取电池的电压
		uint16_t batteryVoltage = (uint16_t)((float)m_adc_value * ADC_SAMPLE_FULL_REF); //
        bat.voltage = batteryVoltage;
        
        char pBuf[128];
        SNPRINTF(pBuf, 128, "<BAT_DataCompute> bat_status=%d, adc_value=%d, Vot=%7.3lf, bat.percentage=%d. \r\n",
                                            batteryChargeState,                                    
                                            m_adc_value,        
                                           (float)m_adc_value * ADC_SAMPLE_FULL_REF, 
                                           bat.percentage);
        BAT_DMSG("%s", pBuf);
                 
                                           
		uint8_t percentage_reference_mode = 0;

        // step02.
                                           
		// 放电模式,  
        // 电池电量显示的百分百，要避免这样逆转跳动显示(85% -> 86%  -> 85%), 只能这样的形式显示( 85% -> 85%  -> 84%);
		if ((batteryChargeState == BAT_CHARGE_STATUS_DISCHARGE) || (batteryChargeState == BAT_CHARGE_STATUS_ERROR))
		{
            bat.chargeState = batteryChargeState;
            
			global.battery_LastChargeTime = RTC2_GetTime();
            
			// 待拔出充电器60sec后，等待电池电压稳定，才能取充电前电池电压的参考百分比数值.
			int64_t dtDeadline = (int64_t)RTC2_Get125MsCounter() - getBaseChargingPercentageLoopTime;
			if ((b_get_base_of_charging_percentage) && (dtDeadline < 0))
			{
				if (dtDeadline < (-BAT_CHARGE_TO_DISCHAEGE_UPDATE_PERCENTAGE_TIME))
				{
					getBaseChargingPercentageLoopTime = 0;
				}

				// 如果取一个电池放电的电压值，与开机读取flash上次关机的电压值 做比较,
				// 如果这次读取的电压值小于在flash读取的关机电压值，去掉等待计算时.
                
                // 计算放电时候电池的百分比
				uint16_t check_bat_percent = bat_calc_discharge_percentage(m_adc_value);

				 // 电池掉电的电压小于10时需要马上更新
                // 放电百分比小于 参考充电百分比，马上更新
				if ((check_bat_percent < base_of_charging_percentage) || base_of_charging_percentage < 100)	//base_of_charging_percentage = 0;
				{
					b_get_base_of_charging_percentage = 0; // 获取充电的基准值
					battery_of_charging_percentage = 1000; // 最小值限定复位 
				}
			}
			else
			{
                //使能读取充电前的参考电压值，用来计算百分比.
				b_get_base_of_charging_percentage = 0;
			}

            // 更新充电前的基准值
			if (b_get_base_of_charging_percentage == 0)
			{
				uint8_t b_check_ok = 0;
				uint16_t bat_percent;
				static uint16_t bat_discharge_percent_previous = 0;
				static uint8_t b_first_in = 1;
				
                // 获取放电值
				bat_percent = bat_calc_discharge_percentage(m_adc_value);

				// deal with the state in the time between the discharge and charge.
				if (abs(bat_percent - bat_discharge_percent_previous) < 50) // 判断两次抖动数值小于 5%
				{
					b_check_ok = 1;
				}
				else
				{
                    ;
				}
                
                //
				if (b_first_in)
				{
					bat_discharge_percent_previous = bat_percent;
				}
			
				if (b_check_ok)
				{
					int16_t dt_batPercent =  abs(bat_percent - battery_of_charging_percentage);
					// 修正， 当读取的百分比比最低的百分比高2%，更新最低参考值
					if (dt_batPercent > 30)
					{
						battery_of_charging_percentage = 1000;	//重新设置参考
					}
				
					// 当前电池电量 小于参考的最小电量时
					if (bat_percent <= battery_of_charging_percentage)
					{
                        //这时候才更新放电电量的值。
						// 读取非充电状态的电压值
						battery_of_discharge_percentage = bat_discharge_percent_previous;			//bat.percent = bat_percent / 10;					
						// 超过20分钟才允许新的电量值
						base_of_charging_percentage = bat_discharge_percent_previous;
					}
                    
					sum_of_charging_current = 0;
					percentage_reference_mode = 1;
					b_check_ok = 0;
				}

				bat_discharge_percent_previous = bat_percent;
			}
		}
        
        // step03.
        // 充电模式
		else
		{
            percentage_reference_mode = 2;
			
            b_get_base_of_charging_percentage = 1;	//禁止 读取充电前的参考电压值，用来计算百分比.
			getBaseChargingPercentageLoopTime = RTC2_Get125MsCounter() + BAT_CHARGE_TO_DISCHAEGE_UPDATE_PERCENTAGE_TIME;//60*100;	// 60 * 1s后才能 允许读取电池充电前的参考电压百分比

			// 正在充电
			if (batteryChargeState == BAT_CHARGE_STATUS_CHARGE)
			{
				// percentage == 10x.
				volatile uint16_t calc_bat_charging_percentage = 0;
                // 计算模拟充电百分比
				calc_bat_charging_percentage = (float)base_of_charging_percentage +  ((float)sum_of_charging_current / (float)BAT_UA_SEC) * 10.0f;
				
				volatile uint32_t calc_bat_charging_current = 0;
				
				//应该是永远小于100%，等待 tp4057的IO标志充满才显示100%
				if (calc_bat_charging_percentage < 1000)
				{
					// 用放电算法求电池百分比，如果计算出的放电百分比 比 充电百分比的低，就用放电百分比的百分比作为当前的电量百分比

					volatile uint16_t dischrage_calc_ref_percentage = bat_calc_discharge_percentage(m_adc_value);
					
					BAT_DMSG("<BAT> dischrage_calc_ref_percentage=%d, calc_bat_charging_percentage=%d\r\n",
                                    dischrage_calc_ref_percentage, 
                                    calc_bat_charging_percentage);//
                    
					if (dischrage_calc_ref_percentage < calc_bat_charging_percentage)
					{
						calc_bat_charging_percentage = dischrage_calc_ref_percentage;	// 充电时。当前的电量百分比
						
						// 充电参考重新取样，
                        // reset param.
						base_of_charging_percentage = dischrage_calc_ref_percentage;
						sum_of_charging_current = 0;
						BAT_DMSG("<BAT> base_of_charging_percentage = %d\r\n",base_of_charging_percentage);//
					}
                    else
                    {
                        // abs > 20%, reset param.
                        // 1000 - 200 = 800
                        if ((dischrage_calc_ref_percentage - calc_bat_charging_percentage) > 200)
                        {
                            calc_bat_charging_percentage = dischrage_calc_ref_percentage - 200;
                            
                            base_of_charging_percentage = dischrage_calc_ref_percentage - 200;
                            sum_of_charging_current = 0;
                        }
                    }
					
					// 当前电量百分比
					battery_of_charging_percentage = calc_bat_charging_percentage; 
                    // 计算充电电流
					calc_bat_charging_current = bat_calc_charging_current(calc_bat_charging_percentage);
					
					
					int64_t chargeTimeLost = 0;
					
					chargeTimeLost  = (int64_t)RTC2_GetTime() - global.battery_LastChargeTime;
					//
                    global.battery_LastChargeTime = RTC2_GetTime();
                    
					if (chargeTimeLost < 1)
					{
						chargeTimeLost = 1;
					}
					else if (chargeTimeLost > 600)		//这个按照需求改动 10min
					{
						chargeTimeLost = 600;
					}
					
					
                    uint16_t chargeTimeCnt = chargeTimeLost;
                    
					for (uint16_t t = 0; t < chargeTimeCnt; t++)
					{
						sum_of_charging_current += calc_bat_charging_current; // 充电1秒钟用了多少电流
					}

					BAT_DMSG("<BAT> chargeTimeLost = %d, charge_current=%u, sum_of_charging_current= %u.\r\n",chargeTimeCnt, calc_bat_charging_current, sum_of_charging_current);//

					bat.chargeState = batteryChargeState;
				}
				else
				{
                    BAT_DMSG("<BAT> ppp = %d, ppp2=%d.\r\n", calc_bat_charging_percentage, batteryVoltage);
                    if ((calc_bat_charging_percentage >= 1000) && (batteryVoltage > 410))
                    {
                        battery_of_charging_percentage = 1000;
                        base_of_charging_percentage = 1000;
                        sum_of_charging_current = 0;
                        bat.chargeState = BAT_CHARGE_STATUS_COMPLETE; // indicate the battery charge state be charging complete.
                    }
                    else
                    {
                        battery_of_charging_percentage = 999;
                        base_of_charging_percentage = 999;
                        sum_of_charging_current = 0;
                        bat.chargeState = batteryChargeState;
                    }
				}
			}
			else if (batteryChargeState == BAT_CHARGE_STATUS_COMPLETE)
			{
				bat.chargeState = batteryChargeState;
                
				battery_of_charging_percentage = 1000;//bat.percent = 100;
				// 
				base_of_charging_percentage = 999;//990;// Bug 更新？？
                sum_of_charging_current = 0;
			}
			else
			{
			
			}
			
			//battery_percentage_prevoius = battery_of_charging_percentage;
		}
		
        
        // step04.
        // discharge
		if (percentage_reference_mode == 1)			
		{
			// 充电时的电量大于放电时的电量
            //  充电时的电量 == 放电时的电量
			if (battery_of_charging_percentage >= battery_of_discharge_percentage)
			{
				battery_of_charging_percentage = battery_of_discharge_percentage;// 电量=放电的电量
			}

			bat.percentage = battery_of_discharge_percentage;
			
			// 修正
			env.cfg.lastPowerDown_BatteryPercentage = bat.percentage;
		}
        // charging
		else if (percentage_reference_mode == 2)
		{
			bat.percentage = battery_of_charging_percentage;
			env.cfg.lastPowerDown_BatteryPercentage = bat.percentage;
            
		}	
		else
		{
			//bat.percent = bat.percent;
			env.cfg.lastPowerDown_BatteryPercentage = bat.percentage;// * 10;
		}

        // step05.
        // 电池电量过低检测
        //BAT_PowerIsToLowCheck();
	}
	else
	{
        // maybe something error happen.
		BAT_DeInit();
		BAT_Init();
	}
}


static uint8_t b_resetRefrence = 0;
void BAT_ResetRefrence(void)
{
	if (b_resetRefrence == 0)
	{
		b_resetRefrence = 1;
		
        // 参考百分比 60s后才能 允许读取电池充电前的参考电压百分比		
		base_of_charging_percentage = env.cfg.lastPowerDown_BatteryPercentage;
		
		//bat.percent = env.cfg.lastPowerDown_BatteryPercentage;// / 10; // merle20190522
        //base_of_charging_percentage = bat.percentage;
	}
}


void BAT_UpdatePercentageWithDelay(void)
{
	// 禁止 读取充电前的参考电压值，用来计算百分比.
	b_get_base_of_charging_percentage = 1;
	
	// 60s后才能 允许读取电池充电前的参考电压百分比		
	getBaseChargingPercentageLoopTime = RTC2_Get125MsCounter() + BAT_CHARGE_TO_DISCHAEGE_UPDATE_PERCENTAGE_TIME;
}

void BAT_IO_Ctl_Init(void)
{
	if (b_bat_io_ctl_init == 0)
	{
		b_bat_io_ctl_init = 1;
		
		nrf_gpio_cfg_input(BAT_ADC_PIN, NRF_GPIO_PIN_NOPULL); //NRF_GPIO_PIN_PULLUP
		nrf_gpio_cfg_input(BAT_CHRAGE_PIN, NRF_GPIO_PIN_NOPULL);
		nrf_gpio_cfg_input(BAT_STANBY_PIN, NRF_GPIO_PIN_NOPULL);
		
		BAT_DMSG("<BAT> Ctl IO Init.\r\n");
	}
}

void BAT_IO_Ctl_DeInit(void)
{
	if (b_bat_io_ctl_init)
	{
		b_bat_io_ctl_init = 0;
		
		nrf_gpio_cfg_default(BAT_ADC_PIN);
		nrf_gpio_cfg_default(BAT_CHRAGE_PIN);
		nrf_gpio_cfg_default(BAT_STANBY_PIN);
		
		BAT_DMSG("<BAT> Ctl IO DeInit.\r\n");
	}
}

static uint8_t b_bat_init = 0;

void BAT_DeInit(void)
{
	if (b_bat_init)
	{
		b_bat_init = 0;	
		
		ADC_DeInit();
		//BAT_IO_Ctl_DeInit();
		//b_resetRefrence = 0;
		BAT_DMSG("<BAT> DeInit.\r\n");
	}
}

void BAT_Init(void)
{	
	if (b_bat_init == 0)
	{
		b_bat_init = 1;
		
		BAT_IO_Ctl_Init();
		ADC_Init();
		//BAT_ResetRefrence();	//20170602修改，在充电的时候，或者开机的时候才用
		
		global.battery_LastChargeTime = RTC2_GetTime();
		
		BAT_DMSG("<BAT> Init.\r\n");
	}
}

void BAT_UpdateBatInfo(void)
{
    BAT_IO_Ctl_Init();
    ADC_Init();
    ADC_DataUpdateOnce();
    bat.chargeState = BAT_GetChargeStatus();
    delay_ms(2);

    ADC_DeInit();
    BAT_DataCompute();
}

// 充电的时候5秒钟一次， 
int BAT_Update(void)
{
    BAT_IO_Ctl_Init();// 不用再初始化
    //bat.chargeState = BAT_GetChargeStatus();
    ADC_Init();
    ADC_DataUpdateOnce(); //10mS
    //app_timer_start(m_bat_trig_adc_stop_timer, APP_TIMER_TICKS(2), NULL);
    delay_us(50);
    ADC_DeInit();
	//bat.chargeState = BAT_GetChargeStatus();
	BAT_DataCompute();
}

void BAT_LevelUpdate(void)
{
    BAT_IO_Ctl_Init();// 不用再初始化
    //bat.chargeState = BAT_GetChargeStatus();
    ADC_Init();
    ADC_DataUpdateOnce(); //10mS
    //app_timer_start(m_bat_trig_adc_stop_timer, APP_TIMER_TICKS(2), NULL);
    vTaskDelay(1);//delay_us(50);
    ADC_DeInit();
    //bat.chargeState = BAT_GetChargeStatus();
    BAT_DataCompute();
   
}

/******************* (C) COPYRIGHT 2017 - 2019 All Rights Reserved *****END OF FILE****/
