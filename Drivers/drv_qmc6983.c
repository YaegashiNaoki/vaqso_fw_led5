/**
  ******************************************************************************
  * @file    drv_QMC6983.c
  * @author  Merle
  * @version V1.0.0
  * @date    02-February-2017
  * @brief   qmc6983.
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"

 
#define DBG_QMC6983
#ifdef DBG_QMC6983
#define QMC6983_DMSG	RTT_Printf
#define SNPRINTF    snprintf
#else
#define QMC6983_DMSG(...)
#define SNPRINTF(...)
#endif

#include "drv_QMC6983.h"
#include "drv_i2c.h"
#include "ICAL.h"

#ifdef BOARD_I2C2_USE_HW
#define QMC6983_SLAVE_ADDR0		(0x2C)//0x2C
#define QMC6983_SLAVE_ADDR1		(0x2D)//0x2C
#else
#define QMC6983_SLAVE_ADDR0		(0x58)//0x58 
#define QMC6983_SLAVE_ADDR1		(0x59)//0x58 
#endif 


static  uint8_t qmc6983_slave_address = QMC6983_SLAVE_ADDR0;
accel_axis_t magData;

static  int xy_sensitivity=0;
static  int z_sensitivity=0;

static unsigned char  chip_id = 0;
static float OTP_Kx = 0;
static float OTP_Ky = 0;
static float mag[3] = {0};
float rad2deg = 180 / 3.1415926;

float headsumX = 0;
//float fhead[MAG_FILTERNUM] = { 0 };
float Xmax,Ymax,Xmin,Ymin,Xsens,Ysens;
#define QMC_ABS(X) ((X) < 0 ? (-1 * (X)) : (X))
#define EARTH_M  50
#define MAG_FILTERNUM      8


#define PI (3.1415926)
//float headsumX = 0;
float fhead[MAG_FILTERNUM] = { 0 };
float Xmax,Ymax,Xmin,Ymin,Xsens,Ysens;
float magX,magY,magZ;
_QMC7983 qmcX983;

volatile static uint16_t mag_sample_rate = 10;



/* Magnetometer registers mapping */

#define QMCX983_SETRESET_FREQ_FAST  1
#define RWBUF_SIZE      16
/* Magnetometer registers */
#define CTL_REG_ONE	0x09  /* Contrl register one */
#define CTL_REG_TWO	0x0a  /* Contrl register two */

/* Output register start address*/
#define OUT_X_REG		0x00

/*Status registers */
#define STA_REG_ONE    0x06
#define STA_REG_TWO    0x0c

/* Temperature registers */
#define TEMP_H_REG 		0x08
#define TEMP_L_REG 		0x07

/*different from qmc6983,the ratio register*/
#define RATIO_REG		0x0b

 
/************************************************/
/* 	Magnetometer section defines	 	*/
/************************************************/

/* Magnetic Sensor Operating Mode */
#define QMCX983_STANDBY_MODE	0x00
#define QMCX983_CC_MODE			0x01
#define QMCX983_SELFTEST_MODE	0x02
#define QMCX983_RESERVE_MODE	0x03


/* Magnetometer output data rate  */
#define QMCX983_ODR_10		0x00	/* 0.75Hz output data rate */
#define QMCX983_ODR_50		0x01	/* 1.5Hz output data rate */
#define QMCX983_ODR_100		0x02	/* 3Hz output data rate */
#define QMCX983_ODR7_200	0x03	/* 7.5Hz output data rate */


/* Magnetometer full scale  */
#define QMCX983_RNG_2G		0x00
#define QMCX983_RNG_8G		0x01
#define QMCX983_RNG_12G		0x02
#define QMCX983_RNG_20G		0x03

#define RNG_2G		2
#define RNG_8G		8
#define RNG_12G		12
#define RNG_20G		20

/*data output register*/
#define OUT_X_M		0x01
#define OUT_X_L		0x00
#define OUT_Z_M		0x05
#define OUT_Z_L		0x04
#define OUT_Y_M		0x03
#define OUT_Y_L		0x02

#define SET_RATIO_REG   0x0b

/*data output rate HZ*/
#define DATA_OUTPUT_RATE_10HZ 	0x00
#define DATA_OUTPUT_RATE_50HZ 	0x01
#define DATA_OUTPUT_RATE_100HZ 	0x02
#define DATA_OUTPUT_RATE_200HZ 	0x03

/*oversample Ratio */
#define OVERSAMPLE_RATIO_512 	0x00
#define OVERSAMPLE_RATIO_256 	0x01
#define OVERSAMPLE_RATIO_128 	0x02
#define OVERSAMPLE_RATIO_64 	0x03


#define SAMPLE_AVERAGE_8		(0x3 << 5)
#define OUTPUT_RATE_75		(0x6 << 2)
#define MEASURE_NORMAL		0
#define MEASURE_SELFTEST		0x1
#define GAIN_DEFAULT		  (3 << 5)


// conversion of magnetic data (for bmm050) to uT units
// conversion of magnetic data to uT units
// 32768 = 1Guass = 100 uT
// 100 / 32768 = 25 / 8096
// 65536 = 360Degree
// 360 / 65536 = 45 / 8192
#define CONVERT_M			6
#define CONVERT_M_DIV		100			// 6/100 = CONVERT_M
#define CONVERT_O			1
#define CONVERT_O_DIV		100			// 1/64 = CONVERT_O
#define CONVERT_Q16			1
#define CONVERT_Q16_DIV		65536		// 1/64 = CONVERT_Gyro
	 

//I2C地址定义
#define GEOMAGNETIC_SENSOR_SCK                    0xff
#define GEOMAGNETIC_SENSOR_SDA                    0xff

//Debug定义
#define QMC7983_DEBUG 1

#define  D2R         0.017453293f

//timer configuration
	#define GPT_TIMER_CALIBRATIOB   2
	#define GPT_TIMER_NORMAL       10
	#define SW_TIMER_POWER_ON       2

 typedef struct
{
	float InitFlag;
    float AccelBasic[3];//,GyroBasic[3];
	float MagFactorX,MagFactorY,MagFactorZ;
	float MagAngleX,MagAngleY,MagAngleZ;
	float MagBasicX,MagBasicY,MagBasicZ;
}SystemParam_t;


/**
* @brief        Write bytes to ic from iic transmission api.
*               ..
* @param[int]   addr: The pointer of the data write device register address.
* @param[int]   reg: The pointer of the data write register address.
* @param[int]   data: The pointer of data to be write.
* @param[int]   len: The length of the data to be write.
* @retval       An return enumeration value:
* - true: write bytes is successful
* - false: write bytes is fault.
*/
static bool QMC6983_SendData(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len)
{
	return I2C2_WriteBuffer(addr, reg, len, data);
}


/**
* @brief        Read bytes to ic from iic api transmission api.
*               ..
* @param[int]   addr: The pointer of the data read device register address.
* @param[int]   reg: The pointer of the data read register address.
* @param[out]   data: The pointer of data to be read.
* @param[int]   len: The length of the data to be read.
* @retval       An return enumeration value:
* - true: read bytes is successful
* - false: read bytes is fault.
*/
static bool QMC6983_ReceiveData(uint8_t addr, uint8_t *data, uint8_t len)
{
	return I2C2_Read(addr, NULL, len, data);
}


/**
  * @brief        Write bytes from cstxxx ctp ic.
  *               ..
  * @param[int]   data: The pointer of data to be write.
  * @param[int]   len: The length of the data to be read.
  * @retval       An return enumeration value:
  * - true: read bytes is successful
  * - false: read bytes is fault.
  */
static bool QMC6983_Write(uint8_t *data, uint8_t len)
{
	volatile bool ret; 
	uint8_t reg_addr;
	
	reg_addr = data[0];
	data++;

	ret = QMC6983_SendData(qmc6983_slave_address, reg_addr, data, len - 1);
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
static bool QMC6983_Read( uint16_t len, uint8_t *value) 
{
	volatile bool ret;
	ret = QMC6983_ReceiveData(qmc6983_slave_address, value, len);
	return ret;
}


static bool QMC6983_ReadInReg(uint8_t addr, uint8_t *data, uint8_t len)
{
	volatile bool ret;
	uint8_t buf[2];
    ret = I2C2_WriteBuffer(qmc6983_slave_address, addr, 0, buf);
    ret = I2C2_Read(qmc6983_slave_address, NULL, len, data);
	return ret;
}

static bool QMC6983_PowerOn(void)
{
	uint8_t temp[1];
	temp[0] = QMC6983_POWER_ON;
	return  QMC6983_Write(temp, 1);
}

static bool QMC6983_PowerOff(void)
{
	uint8_t temp[1];
	temp[0] = QMC6983_POWER_OFF;
	return QMC6983_Write(temp, 1);
}

static bool QMC6983_SetMode(uint8_t mode)
{
	uint8_t temp[1];
	temp[0] = mode;
	return QMC6983_Write(temp, 1);
}

static bool QMC6983_GetData(void)
{
	bool ret;
	uint8_t buf[2];

	ret = QMC6983_Read(2, buf);
	uint32_t data = (uint32_t)((buf[0] << 8) | buf[1]) * 10 / 12;

	return ret;
}



/* Set the Gain range */
int qmcX983_set_range(int range)
{
	int err = 0;
	uint8_t data[2];
	int ran ;
	
	switch (range) 
	{
		case QMCX983_RNG_2G:
			ran = RNG_2G;
			break;
		case QMCX983_RNG_8G:
			ran = RNG_8G;
			break;
		case QMCX983_RNG_12G:
			ran = RNG_12G;
			break;
		case QMCX983_RNG_20G:
			ran = RNG_20G;
			break;
		default:
			return -1;
	}

	xy_sensitivity = 20000/ran;
	z_sensitivity = 20000/ran;

    uint8_t ack = 0;
    uint8_t rw_buf[16];
    
   // err =qmc_ConReadBytes(data,CTL_REG_ONE,1);
    ack = QMC6983_ReadInReg(CTL_REG_ONE, data, 1); // 0x09
    
	data[0] &= 0xcf;
	data[0] |= (range << 4);

	//err= qmc_WriteBytes(CTL_REG_ONE,&data[0],1);
	
    rw_buf[0] = CTL_REG_ONE;
    rw_buf[1] = data[0];
    ack = QMC6983_Write(rw_buf, 2);
    
    
	return err;
}

/* Set the sensor mode */

int qmcX983_set_ratio(int8_t ratio)
{
	int err = 0;

	//err= qmc_WriteBytes(0x0b,&ratio,1);
	uint8_t ack = 0;
    uint8_t rw_buf[16];
    rw_buf[0] = 0x0b;
    rw_buf[1] = ratio;
    ack = QMC6983_Write(rw_buf, 2);
    
	return err;
}

// oversample512, 8G 200Hz, continue
static void qmcX983_start_measure(void)
{
	uint8_t temp_data;
	int err;
	temp_data = 0x1d;
	//err= qmc_WriteBytes(CTL_REG_ONE,&temp_data,1);
    
    uint8_t ack = 0;
    uint8_t rw_buf[16];
    rw_buf[0] = CTL_REG_ONE;
    rw_buf[1] = temp_data;
    ack = QMC6983_Write(rw_buf, 2);
}


static void qmcX983_stop_measure(void)
{
	unsigned char temp_data;
	int err;
	temp_data = 0x1c;
    
    uint8_t ack = 0;
    uint8_t rw_buf[16];
    rw_buf[0] = CTL_REG_ONE;
    rw_buf[1] = temp_data;
    ack = QMC6983_Write(rw_buf, 2);
	//err= qmc_WriteBytes(CTL_REG_ONE,&temp_data,1);
}

static int qmcX983_enable(void)
{
	unsigned char temp_data;
	int err;
	
	temp_data = 0x1;
    
    uint8_t ack = 0;
    uint8_t rw_buf[16];
    rw_buf[0] = 0x21;
    rw_buf[1] = temp_data;
    ack = QMC6983_Write(rw_buf, 2);
	//err= qmc_WriteBytes(0x21,&temp_data,1);

	temp_data = 0x40;
    rw_buf[0] = 0x20;
    rw_buf[1] = temp_data;
    ack = QMC6983_Write(rw_buf, 2);
	//err= qmc_WriteBytes(0x20,&temp_data,1);
	
	
	if(chip_id == 2 || chip_id == 4)
	{
		temp_data = 0x80;
        rw_buf[0] = 0x1b;
        rw_buf[1] = temp_data;
        ack = QMC6983_Write(rw_buf, 2);
		//err= qmc_WriteBytes(0x1b,&temp_data,1);
	}
	
	qmcX983_set_range(QMCX983_RNG_8G);
	qmcX983_set_ratio(QMCX983_SETRESET_FREQ_FAST);				//the ratio must not be 0, different with qmc5983

	qmcX983_start_measure();
	//F_Delay_mS(30);
	delay_ms(30);
    
	return 0;
}

static int qmcX983_disable(void)
{
	qmcX983_stop_measure();
	return 0;
}

static bool QMC6983_Read_OTP(void)
{
    bool ret = false;
	uint8_t data[2];
	uint8_t value[2]={0};
	uint8_t temp_data;
    
    uint8_t rw_buf[16] = {0};
	float otp = 0;

	temp_data = 0x0a;
    
    rw_buf[0] = 0x2e;
    rw_buf[1] = 0x0a;
    ret = QMC6983_Write(rw_buf, 2);
    delay_ms(10);

    ret = QMC6983_ReadInReg(0x2f, data, 1);
	value[0] = data[0];

	if(((value[0]&0x3f) >> 5) == 1)
		otp = (value[0]&0x1f)-32;
	else	
		otp = value[0]&0x1f;
	OTP_Kx = 0.02f *otp;

	temp_data = 0x0d;
   
    rw_buf[0] = 0x2e;
    rw_buf[1] = 0x0d;
    ret = QMC6983_Write(rw_buf, 2);
    delay_ms(10);
    
    ret = QMC6983_ReadInReg(0x2f, data, 1);
	value[0] = data[0];

	temp_data = 0x0f;
    
    
    rw_buf[0] = 0x2e;
    rw_buf[1] = 0x0f;
    ret = QMC6983_Write(rw_buf, 2);
    delay_ms(10);
	
    ret = QMC6983_ReadInReg(0x2f, data, 1);
	value[1] = data[0];

	if((value[0] >> 7) == 1)
		otp = (((value[0]&0x70) >> 4)*4 + (value[1] >> 6))-32;
	else
		otp = (((value[0]&0x70) >> 4)*4 + (value[1] >> 6));
	OTP_Ky = 0.02f * otp ;
	
	return ret;
}


bool QMC6983_MagRead(float *magX, float *magY,  float *magZ)
{
    bool ret = false;
	unsigned char mag_data[6];
	unsigned char databuf[6];
	int16_t hw_d[3] = {0};
	float  data[3] = {0};

	int t1 = 0;
	unsigned char rdy = 0;

    //10Hz,
	/* Check status register for data availability */
	while(!(rdy & 0x07) && (t1 < 3))
	{
		//res=qmc_ConReadBytes(databuf,STA_REG_ONE,1);
        ret = QMC6983_ReadInReg(STA_REG_ONE, databuf, 1);
        
		rdy=databuf[0];
		t1 ++;
	}
    
	//res=qmc_ConReadBytes(databuf,OUT_X_L,6);
    ret = QMC6983_ReadInReg(OUT_X_L, databuf, 6);

   // QMC6983_DMSG("magX->%d,%d,magY->%d,%d,magZ->%d,%d \r\n",databuf[1],databuf[0],databuf[3],databuf[2],databuf[5],databuf[4]);

	memcpy(mag_data,databuf,sizeof(databuf));

	hw_d[0] = (int16_t) (((mag_data[1]) << 8) | mag_data[0]);
	hw_d[1] = (int16_t) (((mag_data[3]) << 8) | mag_data[2]);
	hw_d[2] = (int16_t) (((mag_data[5]) << 8) | mag_data[4]);

    if ((xy_sensitivity != 0) && (z_sensitivity !=0))
    {
        data[0] = hw_d[0] * 1000.f / xy_sensitivity;// uT  单位
        data[1] = hw_d[1] * 1000.f / xy_sensitivity;
        data[2] = hw_d[2] * 1000.f / z_sensitivity;
    }

	*magX = -data[0];
	*magY = -data[1];
	*magZ = data[2] - data[0]*OTP_Kx -data[1]*OTP_Ky;

	mag[0] = *magX;
	mag[1] = *magY;
	mag[2] = *magZ;

//    char bufff[128] = {0};
//    snprintf(bufff,"Read1 magX->%0.2f,magY->%0.2f,magZ->%0.2f \r\n",*magX,*magY,*magZ);
//		QMC6983_DMSG("%s",bufff);

	return ret;
}

static bool QMC6983_Detect(void)
{
    bool ret = false;
    uint8_t databuf[6] = {0};
    uint8_t rw_buf[6] = {0};

    ret = QMC6983_ReadInReg(0x0d, databuf, 1);
    if (!ret)
	{ 
        QMC6983_DMSG("QMC7983  init fail 1");
		return false;
	}
    
    ret = QMC6983_Read_OTP();
    
    if(0x31 == databuf[0])//6983 //if(0x32 == databuf[0])//7983
	{
        rw_buf[0] = 0x2e;
        rw_buf[1] = 0x01;
        ret = QMC6983_Write(rw_buf, 2);
		
		if(ret == false)
		{
            QMC6983_DMSG("QMC7983  init fail 2");
			return ret;			
		}

		ret = QMC6983_ReadInReg(0x2f, databuf, 1);
        
		if(ret == false)
		{
            QMC6983_DMSG("QMC7983  init fail 3");
			return ret;
		}	
		
		if(((databuf[0]&0x04 )>> 2))
		{
			chip_id = 2;
		}
		else
		{
            rw_buf[0] = 0x2e;
            rw_buf[1] = 0x0f;
            ret = QMC6983_Write(rw_buf, 2);
			if(ret == false)
			{
                QMC6983_DMSG("QMC7983  init fail 4");
				return ret;			
			}

			ret = QMC6983_ReadInReg(0x2f, databuf, 1);
			if(ret == false)
			{
                QMC6983_DMSG("QMC7983  init fail 5");
				return ret;
			}
			
			if(0x02 == ((databuf[0]&0x3c)>>2))
			{
				chip_id = 3;
			}
			
			if(0x03 == ((databuf[0]&0x3c)>>2))
			{
				chip_id = 4;
			}
		}
  	 }
	 
    return true;
}








void QMC6983_SensorAdcRead(int16_t *x1, int16_t *y1, int16_t *z1)
{
	int res;
	unsigned char mag_data[6];
	unsigned char databuf[6];
	int16_t hw_d[3] = {0};
	float  data[3] = {0};

	int t1 = 0;
	unsigned char rdy = 0;
    uint8_t ack = 0;
	
	/* Check status register for data availability */
	while(!(rdy & 0x07) && (t1 < 3))
	{
        ack = QMC6983_ReadInReg(STA_REG_ONE, databuf, 1);
		rdy=databuf[0];
		t1 ++;
	}

    ack = QMC6983_ReadInReg(OUT_X_L, databuf, 6);
	
	memcpy(mag_data,databuf,sizeof(databuf));

	hw_d[0] = (int16_t) (((mag_data[1]) << 8) | mag_data[0]);
	hw_d[1] = (int16_t) (((mag_data[3]) << 8) | mag_data[2]);
	hw_d[2] = (int16_t) (((mag_data[5]) << 8) | mag_data[4]);

	data[0]= hw_d[0] * 100.f /xy_sensitivity;// uT  单位
	data[1]= hw_d[1] * 100.f /xy_sensitivity;
	data[2] = hw_d[2] * 100.f / z_sensitivity;

	*x1 = (int16_t)data[0];
	*y1 =(int16_t) data[1];
	*z1 = (int16_t)(data[2] - data[0]*OTP_Kx -data[1]*OTP_Ky);	
    
    //QMC6983_DMSG("read1 magX -> %d,%d,magY -> %d,%d\n",databuf[0],databuf[1],databuf[2],databuf[3]);
	
}


static uint8_t b_QMC6983_init = 0;

static uint8_t b_qmc6983_enable = 0;



static uint8_t b_qmc6983_mag_update = 0;

int QMC6983_Update(void)
{
    if (b_qmc6983_enable)
	{ 
        QMC6983_MagRead(&magX, &magY, &magZ);
        
        b_qmc6983_mag_update = 1;
        //
        magData.axis[X] = lrintf(magX);
        magData.axis[Y] = lrintf(magY);
        magData.axis[Z] = lrintf(magZ);
        //
        uint32_t accMagSum = magData.axis[X]*magData.axis[X] + magData.axis[Y]*magData.axis[Y] +magData.axis[Z]*magData.axis[Z];
        
        uint32_t accMagSqrt = lrintf(sqrt(accMagSum));
        
        QMC6983_DMSG( "mag:%d,%d,%d-->%u.\r\n", magData.axis[0], magData.axis[1], magData.axis[2], accMagSqrt);
        
    }
}
void QMC6983_SlectChannnel(uint8_t ch)
{
    const uint8_t qmc6983_slave_addr[2] = {QMC6983_SLAVE_ADDR0, QMC6983_SLAVE_ADDR1};
    
    qmc6983_slave_address = qmc6983_slave_addr[ch];
}
int QMC6983_Update1(uint8_t ch)
{
    //if (b_qmc6983_enable)
    QMC6983_SlectChannnel(ch);
	{ 
        QMC6983_MagRead(&magX, &magY, &magZ);
        
        b_qmc6983_mag_update = 1;
        //
        magData.axis[X] = lrintf(magX);
        magData.axis[Y] = lrintf(magY);
        magData.axis[Z] = lrintf(magZ);
        //
        uint32_t accMagSum = magData.axis[X]*magData.axis[X] + magData.axis[Y]*magData.axis[Y] +magData.axis[Z]*magData.axis[Z];
        
        uint32_t accMagSqrt = lrintf(sqrt(accMagSum));
        
        QMC6983_DMSG( "mag%d:%d,%d,%d-->%u.\r\n", ch, magData.axis[0], magData.axis[1], magData.axis[2], accMagSqrt);
        
    }
}


uint8_t QMC6983_IsMagUpdate(void)
{
    const uint8_t ret = b_qmc6983_mag_update;
    b_qmc6983_mag_update = 0;
    return ret;
}

uint16_t QMC6983_GetMagSampleRate(void)
{
    const uint16_t tmp = mag_sample_rate;
    return tmp;
}

uint8_t QMC6983_MagGetState(void)
{
    return b_qmc6983_enable;
}

void QMC6983_Disable(void)
{

    b_qmc6983_enable = 0;
    qmcX983_disable();
}


void QMC6983_Enable(uint16_t sample_rate)
{
    mag_sample_rate = sample_rate;
    
    b_qmc6983_enable = 1;
    qmcX983_enable();
}

void QMC6983_DeInit(void)
{
	if (b_QMC6983_init)
	{
		b_QMC6983_init = 0;
		
        qmcX983_disable();
		
		QMC6983_DMSG("<QMC6983> DeInit.In.\r\n");
	}
	QMC6983_DMSG("<QMC6983> DeInit.Out.\r\n");
}

bool QMC6983_Init(void)
{ 	
	bool ret = false;
    
	//if (b_QMC6983_init == 0)
	{
		I2C2_Init();
		
		if (QMC6983_Detect())
		{
			b_QMC6983_init = 1;
			QMC6983_DMSG("<QMC6983> Detect OK.\r\n");
            
            qmcX983_enable();
		}
		else
		{
			QMC6983_DMSG("<QMC6983> Detect Failure.\r\n");
		}
	}
	return ret;
}  





/******************* (C) COPYRIGHT 2017  *****END OF FILE****/
