/**
  ******************************************************************************
  * @file    mw.c
  * @author  Merle
  * @version V1.0.0
  * @date    17-Octobal-2016
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"

//#define DBG_MW
#ifdef DBG_MW
#define MW_DMSG	    RTT_Printf
#define SNPRINTF    snprintf
#else
#define MW_DMSG(...)
#define SNPRINTF(...)
#endif


//#define YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10  + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))  
#define DATE_YEAR (   (__DATE__ [7] - '0') * 1000   \
                    + (__DATE__ [8] - '0') * 100    \
                    + (__DATE__ [9] - '0') * 10     \
                    + (__DATE__ [10] - '0')) 

#define DATE_MONTH (  __DATE__ [2] == 'n' ? 1   \
                    : __DATE__ [2] == 'b' ? 2   \
                    : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4)   \
                    : __DATE__ [2] == 'y' ? 5   \
                    : __DATE__ [2] == 'n' ? 6   \
                    : __DATE__ [2] == 'l' ? 7   \
                    : __DATE__ [2] == 'g' ? 8   \
                    : __DATE__ [2] == 'p' ? 9   \
                    : __DATE__ [2] == 't' ? 10  \
                    : __DATE__ [2] == 'v' ? 11 : 12)  
  
#define DATE_DAY (((__DATE__ [4] == ' ') ? 0 : (__DATE__ [4] - '0')) * 10  \
                 + (__DATE__ [5] - '0'))
            
            
#define DATE_AS_INT (((M_YEAR - 2000) * 12 + M_MONTH) * 31 + M_DAY)  

#define TIME_HOUR (((__TIME__ [0] == ' ') ? 0 : (__TIME__ [0] - '0')) * 10  \
                  + (__TIME__ [1] - '0'))

#define TIME_MIN (((__TIME__ [3] == ' ') ? 0 : (__TIME__ [3] - '0')) * 10  \
                 + (__TIME__ [4] - '0'))
               
#define TIME_SECOND (((__TIME__ [6] == ' ') ? 0 : (__TIME__ [6] - '0')) * 10  \
                  + (__TIME__ [7] - '0'))
                  

const uint8_t deviceFirmwareVersion[3]  = { 2, 0, 1 };
const uint8_t deviceHardwareVersion[3]  = { 2, 0, 1 };
//const uint8_t deviceFirmwareBuidTime[7] = { (DATE_YEAR & 0xff), ((DATE_YEAR >> 8) & 0xff), DATE_MONTH, DATE_DAY, TIME_HOUR, TIME_MIN, TIME_SECOND };//2019.8.20 15:04:08 
const uint8_t deviceFirmwareBuidTime[7] = { (2019 & 0xff), ((2019 >> 8) & 0xff), 9, 16, 17, 37, 28 };//2019.8.21 9:48:15

/**** golbal param init	***/
global_val_t global	= {0};

flags_t f = {0};
bat_t bat = {0}; //100,0


void MW_ParameterUpdate(void)
{
    global.battery_LastChargeTime =  RTC2_GetTime();
    
    //
    static const char build_date[16] =__DATE__; // "Aug 16 2019"
    static const char build_time[16] =__TIME__; // "17:49:50"
    int year = 0;
    char mon_str[5] = {0};
    int day = 0;
    sscanf(build_date, "%s %d %d",  &mon_str[0], &day, &year); 
    //
    int hour = 0;
    int min = 0;
    int second = 0;
    sscanf(build_time, "%d:%d:%d",  &hour, &min, &second);
    MW_DMSG("<MAIN> Y=%d,M=%s,D=%d, %02d:%02d:%02d.\r\n", year, mon_str, day, hour, min, second);   // Built:Aug 16 2019 17:49:50.
}


		
/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
