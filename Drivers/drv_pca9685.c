/**
  ******************************************************************************
  * @file    drv_pca9685.c
  * @author  Merle
  * @version V1.0.0
  * @date    28-June-2019
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"

//#define DBG_PCA9685
#ifdef DBG_PCA9685
#define PCA9685_DMESG		RTT_Printf
#else
#define PCA9685_DMESG(...)
#endif


#define PCA9685_SLAVE_ADDR		(0x40)      // IIC address

static i2c_bus_t i2c_bus = {&I2C2_Init, &I2C2_Rx, &I2C2_Tx};


#define PCA9685_MODE1		0x00
#define PCA9685_MODE2		0x01
//
#define PCA9685_SUBADR1		0x02
#define PCA9685_SUBADR2		0x03
#define PCA9685_SUBADR3		0x04

//
#define PCA9685_ALLCALLADR	0x05
//
#define PCA9685_PRE_SCALE	0xFE	// Writes to PRE_SCALE register are blocked when SLEEP bit is logic 0 (MODE 1).
//
#define PCA9685_TEST_MODE	0xFF	// Reserved. Writes to this register may cause unpredictable results.
//

#define PCA9685_LED0_ON_L	0x06
#define PCA9685_LED0_ON_H	0x07	// LED0_ON_H[4],LED0 full ON
#define PCA9685_LED0_OFF_L	0x08
#define PCA9685_LED0_OFF_H	0x09	// LED0_OFF_H[4],LED0 full OFF

//
#define PCA9685_ALL_LED_ON_L	0xFA
#define PCA9685_ALL_LED_ON_H	0xFB	// ALL_LED_ON_H[4]
#define PCA9685_ALL_LED_OFF_L	0xFC
#define PCA9685_ALL_LED_OFF_H	0xFD	// ALL_LED_OFF_H[4]


static uint8_t b_pca9685_init = 0;


/**
  * @brief        Write bytes from cstxxx ctp ic.
  *               ..
  * @param[int]   data: The pointer of data to be write.
  * @param[int]   len: The length of the data to be read.
  * @retval       An return enumeration value:
  * - true: read bytes is successful
  * - false: read bytes is fault.
  */
static bool PCA9685_WriteBytes(uint8_t reg, uint8_t* p_data, uint8_t len)
{
    bool ret; 
    //ret = PCA9685_SendData(pca9685_slave_address, reg, data, len);
    
    uint8_t buf[1 + len];
	buf[0] = reg;
	for (uint8_t i = 1; i <= len; i++) 
    {
		buf[i] = *p_data++;
	}
    ret = i2c_bus.write(PCA9685_SLAVE_ADDR, buf, 1 + len, false);
    return ret;
}

/**
  * @brief        Read bytes from cstxxx ctp ic.
  *               ..
  * @param[int]   reg_addr: Register Address.
  * @param[out]   value: The pointer of data to be read.
  * @param[int]   len: The length of the data to be read.
  * @retval       An return enumeration value:
  * - true: read bytes is successful
  * - false: read bytes is fault.
  */
static bool PCA9685_ReadBytes(uint8_t reg, uint8_t* p_data, uint16_t len) 
{
    bool ret;
    //ret = PCA9685_ReceiveData(pca9685_slave_address, reg, p_data, len);
    ret = i2c_bus.write(PCA9685_SLAVE_ADDR, &reg, 1, true);
    ret = i2c_bus.read(PCA9685_SLAVE_ADDR, p_data, len);
    return ret;
}


static bool PCA9685_Read8(uint8_t reg, uint8_t* p_data) 
{
    bool ret;
    //ret = PCA9685_ReceiveData(pca9685_slave_address, reg, p_data, 1);
    
    ret = i2c_bus.write(PCA9685_SLAVE_ADDR, &reg, 1, true);
    ret = i2c_bus.read(PCA9685_SLAVE_ADDR, p_data, 1);
    
    return ret;
}

static bool PCA9685_Write8(uint8_t reg, uint8_t data)
{
    volatile bool ret; 
    //ret = PCA9685_SendData(pca9685_slave_address, reg, &data, 1);
    uint8_t buf[2] = {reg, data};
    ret = i2c_bus.write(PCA9685_SLAVE_ADDR, buf, 2, false);
    return ret;
}



bool PCA9685_Reset(void)
{
    bool ret;

    uint8_t buf[1] = {0x06};

    ret = i2c_bus.write(0x00, buf, 1, false);

    return ret;
}

/*
 * The maximum PWM frequency is 1526 Hz if the PRE_SCALE register is set "0x03h".
 * The minimum PWM frequency is 24 Hz if the PRE_SCALE register is set "0xFFh".
 * The PRE_SCALE register can only be set when the SLEEP bit of MODE1 register is set to logic 1.
 */
bool PCA9685_SetPWMFreq(float freq) 
{
    bool ret;

    freq *= 0.915f;     //0.9;  //0.915// Correct for overshoot in the frequency setting (see issue #11).
    double prescaleval = 25000000;
    prescaleval /= 4096;
    prescaleval /= freq;
    prescaleval -= 1;

    uint16_t _prescaleval = floor(prescaleval + 0.5);
    uint8_t prescale = min(0xFF, _prescaleval);
    prescale = max(0x03, prescale);

    uint8_t old_mode;
    ret = PCA9685_Read8(PCA9685_MODE1, &old_mode);
    uint8_t new_mode = (old_mode & 0x7F) | 0x10;    // sleep 

    ret = PCA9685_Write8(PCA9685_MODE1, new_mode);  // go to sleep
    
    ret = PCA9685_Write8(PCA9685_PRE_SCALE, prescale);	// set the prescaler. (Writes to PRE_SCALE register are blocked when SLEEP bit is logic 0 (MODE 1).)
     
    //old_mode &= 0xef;	//Çå³ýsleepÎ»
     
    ret = PCA9685_Write8(PCA9685_MODE1, old_mode);//   
    delay_us(500);

    ret = PCA9685_Write8(PCA9685_MODE1, old_mode | 0xa1);  // 0b 1010 0001,This sets the MODE1 register to turn on auto increment.
							   // This is why the beginTransmission below was not working.
    
    uint8_t current_mode = 0, current_mode2;
    ret = PCA9685_Read8(PCA9685_MODE1, &current_mode);
    ret = PCA9685_Read8(PCA9685_MODE2, &current_mode2);
    

    PCA9685_DMESG("<PCA9685> currentMode.1= 0x%02X, .2=0x%02X.\r\n", current_mode,current_mode2);

    return ret;
}

static uint8_t b_pca9685_power_on = 0;

bool PCA9685_PowerOn(void) 
{
    bool ret = true;
    
    if (b_pca9685_power_on == 0)
    {
        b_pca9685_power_on = 1;
        
        uint8_t old_mode;
        ret = PCA9685_Read8(PCA9685_MODE1, &old_mode);
        uint8_t new_mode = old_mode & 0xef;	   // clear sleep bit.
        new_mode |= 0xa1;
        ret = PCA9685_Write8(PCA9685_MODE1, new_mode);  
        
        
    //    ret = PCA9685_Read8(PCA9685_MODE2, &old_mode);
    //    //new_mode = old_mode & 0xFB;	   // clear OUTDRV bit. 1111 1 OUTDRCV 11
    //    new_mode |= 0x04;
    //    ret = PCA9685_Write8(PCA9685_MODE2, new_mode);  
        
        delay_us(500);
        
        PCA9685_DMESG("<PCA9685> PCA9685_PowerOn.\r\n");
    }


    return ret;
}

bool PCA9685_PowerOff(void) 
{
    bool ret = true;
    
    if (b_pca9685_power_on)
    {
        b_pca9685_power_on = 0;
        uint8_t old_mode;
        ret = PCA9685_Read8(PCA9685_MODE1, &old_mode);
        uint8_t new_mode = (old_mode & 0x7F) | 0x10;    // sleep bit set.
                                                        // go to sleep
        ret = PCA9685_Write8(PCA9685_MODE1, new_mode | 0xa1);  // 0b 1010 0001,This sets the MODE1 register to turn on auto increment.
        
        PCA9685_DMESG("<PCA9685> PCA9685_PowerOff.\r\n");
    }
    return ret;
}


bool PCA9685_SetPWM(uint8_t num, uint16_t on_cnt, uint16_t off_cnt) 
{
    bool ret;
    uint8_t write_reg = PCA9685_LED0_ON_L + 4 * num;
    uint8_t buf[4];
    buf[0] = on_cnt & 0xff;
    buf[1] = (on_cnt >> 8);
    buf[2] = off_cnt & 0xff;
    buf[3] = (off_cnt >> 8);

    ret = PCA9685_WriteBytes(write_reg, buf, 4);

    return ret;
}

bool PCA9685_SetPinOn(uint8_t num) 
{
    bool ret;
    uint8_t buf[4] = {0};
    uint8_t reg = PCA9685_LED0_ON_L + 4 * num;
    
    //PCA9685_ReadBytes(reg, buf, 4);
     // LEDX_OFF_H[4]
    buf[1] |= 0x10;
    
    // LEDX_ON_H[4]
    buf[3] &= ~0x10;

    ret = PCA9685_WriteBytes(reg, buf, 4);

    return ret;
}


bool PCA9685_SetPinOff(uint8_t num) 
{
    bool ret;
    uint8_t buf[4] = {0};
    uint8_t reg = PCA9685_LED0_ON_L + 4 * num;
    
    //PCA9685_ReadBytes(reg, buf, 4);
    
    // LEDX_OFF_H[4]
    buf[1] &= ~0x10;
    
    // LEDX_ON_H[4]
    buf[3] |= 0x10;
   
    ret = PCA9685_WriteBytes(reg, buf, 4);

    return ret;
}


/* Sets pin without having to deal with on/off tick placement and properly handles
 * a zero value as completely off.  Optional invert parameter supports inverting
 * the pulse for sinking to ground.  Val should be a value from 0 to 4095 inclusive.
 */
bool PCA9685_SetPin(uint8_t num, uint16_t val, bool invert)
{
    bool ret;
    // Clamp value between 0 and 4095 inclusive.
    val = min(val, 4095);

    if (invert) 
    {
        if (val == 0) 
        {
            // Special value for signal fully on.
            ret = PCA9685_SetPWM(num, 4096, 0);
        }
        else if (val == 4095) 
        {
            // Special value for signal fully off.
            ret = PCA9685_SetPWM(num, 0, 4096);
        }
        else 
        {
            ret = PCA9685_SetPWM(num, 0, 4095 - val);
        }
    }
    else 
    {
        if (val == 4095) 
        {
            // Special value for signal fully on.
            ret = PCA9685_SetPWM(num, 4096, 0);
        }
        else if (val == 0) 
        {
            // Special value for signal fully off.
            ret = PCA9685_SetPWM(num, 0, 4096);
        }
        else 
        {
            ret = PCA9685_SetPWM(num, 0, val);
        }
    }
    
    return ret;
}

// start_index,end_index = 0~15
bool PCA9685_SetPinValue(uint32_t total_pin_value, uint8_t start_index, uint8_t end_index)
{
    bool ret;
    //uint16_t pin_low16bit = pin & 0x0000ffff;
    //uint16_t pin_high16bit = (pin >> 16) & 0x0000ffff;
    // low 16bit
    for (uint16_t i = start_index; i <= end_index; i ++)
    {
        if (total_pin_value & (1 << i))
        {
            ret = PCA9685_SetPinOn(i);
        }
        else
        {
            ret = PCA9685_SetPinOff(i);
        }
    }
    
    return ret;
}


bool PCA9685_SetAll24LedPinState(uint32_t pin)
{
    bool ret;
    uint16_t pin_low16bit = pin & 0x0000ffff;
    // low 16bit
    for (uint16_t i = 0; i < 16; i ++)
    {
        if (pin_low16bit & (1 << i))
        {
            ret = PCA9685_SetPinOn(i);
        }
        else
        {
            ret = PCA9685_SetPinOff(i);
        }
    }

    return ret;
}


bool PCA9685_SetAllPWM(uint16_t on_cnt, uint16_t off_cnt) 
{
    bool ret;

    uint8_t write_reg = PCA9685_ALL_LED_ON_L;
    uint8_t buf[4];
    buf[0] = on_cnt & 0xff;
    buf[1] = (on_cnt >> 8);
    buf[2] = off_cnt & 0xff;
    buf[3] = (off_cnt >> 8);

    ret = PCA9685_WriteBytes(write_reg, buf, 4);

    return ret;
}


bool PCA9685_SetAllPinOn(void) 
{
    bool ret;
    uint8_t buf[4];
    uint8_t reg = PCA9685_ALL_LED_ON_L;
    
    PCA9685_ReadBytes(reg, buf, 4);
     // LEDX_OFF_H[4]
    buf[1] |= 0x10;
    
    // LEDX_ON_H[4]
    buf[3] &= ~0x10;

    ret = PCA9685_WriteBytes(reg, buf, 4);

    return ret;
}


bool PCA9685_SetAllPinOff(void) 
{
    bool ret;
    uint8_t buf[4];
    uint8_t reg = PCA9685_ALL_LED_ON_L;
    
    PCA9685_ReadBytes(reg, buf, 4);
    
    // LEDX_OFF_H[4]
    buf[1] &= ~0x10;
    
    // LEDX_ON_H[4]
    buf[3] |= 0x10;
   
    ret = PCA9685_WriteBytes(reg, buf, 4);

    return ret;
}


bool PCA9685_SetALLPin(uint16_t val, bool invert)
{
    bool ret;
    
    // Clamp value between 0 and 4095 inclusive.
    val = min(val, 4095);

    if (invert) 
    {
        if (val == 0) 
        {
            // Special value for signal fully on.
            ret = PCA9685_SetAllPWM(4096, 0);
        }
        else if (val == 4095) 
        {
            // Special value for signal fully off.
            ret = PCA9685_SetAllPWM(0, 4096);
        }
        else 
        {
            ret = PCA9685_SetAllPWM(0, 4095 - val);
        }
    }
    else 
    {
        if (val == 4095) 
        {
            // Special value for signal fully on.
            ret = PCA9685_SetAllPWM(4096, 0);
        }
        else if (val == 0) 
        {
            // Special value for signal fully off.
            ret = PCA9685_SetAllPWM(0, 4096);
        }
        else 
        {
            ret = PCA9685_SetAllPWM(0, val);
        }
    }
    
    return ret;
}

void PCA9685_DeInit(void)
{
    if (b_pca9685_init)
    {
        b_pca9685_init = 0;
        
        PCA9685_PowerOff();

        PCA9685_DMESG("<PCA9685> DeInit.\r\n");
    }
}



bool PCA9685_Init(void)
{
    bool ret = true;
    
    if (b_pca9685_init == 0)
    {
        i2c_bus.init();

        ret = PCA9685_Reset();
        
        ret = PCA9685_SetPWMFreq(1500);  //1600, This is the maximum PWM frequency

        ret = PCA9685_SetAllPinOff();
        
        ret = PCA9685_PowerOn(); 
        
        ret = PCA9685_PowerOff(); 
        
        PCA9685_DMESG("<PCA9685> Init. ret=%d.\r\n", ret);
        
        b_pca9685_init = 1;
    }
    
    return ret;
}



/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
