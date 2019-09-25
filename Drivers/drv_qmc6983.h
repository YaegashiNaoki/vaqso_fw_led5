/**
  ******************************************************************************
  * @file    drv_qmc6983.h
  * @author  Merle
  * @version V1.0.0
  * @date    27-Marth-2019
  * @brief  qmc6983
  ******************************************************************************
  */

#ifndef __DRV_QMC6983_H
#define __DRV_QMC6983_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>	 

	 
// Register
#define QMC6983_POWER_OFF 				0x00
#define QMC6983_POWER_ON 				0x01
#define QMC6983_RESET 					0x07
	 
#define QMC6983_C_H_RESOLUTION_MODE		0x10	//   1 Lx. 120ms
#define QMC6983_C_H_RESOLUTION_MODE2	0x11	// 0.5 Lx. 120ms
#define QMC6983_C_L_RESOLUTION_MODE		0x13	// 	 4 Lx.  16ms	
#define QMC6983_O_H_RESOLUTION_MODE		0x20	// 	 1 Lx. 120ms,One Time H-Resolution Mode,It is automatically set to Power Down mode after measurement.
#define QMC6983_O_H_RESOLUTION_MODE2	0x21	// 0.5 Lx. 120ms,One Time H-Resolution Mode,It is automatically set to Power Down mode after measurement.
#define QMC6983_O_L_RESOLUTION_MODE		0x23	//   4 Lx.  16ms,One Time H-Resolution Mode,It is automatically set to Power Down mode after measurement.


// Change Measurement time
// ( High bit ) 01000_MT[7,6,5] Change measurement time.
// ( Low bit ) 011_MT[4,3,2,1,0] Change measurement time.
// ¡ù Please refer "adjust measurement result for influence of optical window."
	 
typedef enum
{
    X,
    Y,
    Z
}axis_e;

typedef struct
{
    int16_t axis[3];
}accel_axis_t;
	 
bool QMC6983_Init(void);
void QMC6983_DeInit(void);

int QMC6983_Update(void);

void QMC6983_Disable(void);
void QMC6983_Enable(uint16_t sample_rate);


uint8_t QMC6983_MagGetState(void);
uint8_t QMC6983_IsMagUpdate(void);
uint16_t QMC6983_GetMagSampleRate(void);


#ifdef __cplusplus
}
#endif

#endif /* __DRV_QMC6983_H */

/******************* (C) COPYRIGHT 2017  *****END OF FILE****/
