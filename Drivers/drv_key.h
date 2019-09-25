/**
  ******************************************************************************
  * @file    drv_key.h
  * @author  Merle
  * @version V1.0.0
  * @date    21-November-2016
  * @brief   .
  ******************************************************************************
  */
#ifndef __DRV_KEY_H
#define __DRV_KEY_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>



	 
#define NO_KEY				KEY_NONE
	 
#define KEY1_DOWN			KEY_1_DOWN		/* ��*/
#define KEY1_UP				KEY_1_UP
#define KEY1_SINGLE			KEY_1_SINGLE
#define KEY1_DOUBLE			KEY_1_DOUBLE
#define KEY1_THREE			KEY_1_THREE
#define KEY1_LONG			KEY_1_LONG
#define KEY1_LONG_LONG		KEY_1_LONG_LONG

#define KEY2_DOWN			KEY_2_DOWN		/* �� */
#define KEY2_UP				KEY_2_UP
#define KEY2_SINGLE			KEY_2_SINGLE
#define KEY2_DOUBLE			KEY_2_DOUBLE
#define KEY2_THREE			KEY_2_THREE
#define KEY2_LONG			KEY_2_LONG
#define KEY2_LONG_LONG		KEY_2_LONG_LONG

#define	KEY3_DOWN			KEY_3_DOWN		/* �� */
#define KEY3_UP				KEY_3_UP
#define KEY3_SINGLE			KEY_3_SINGLE
#define KEY3_DOUBLE			KEY_3_DOUBLE
#define KEY3_HREE			KEY_3_THREE
#define KEY3_LONG			KEY_3_LONG
#define KEY3_LONG_LONG		KEY_3_LONG_LONG

#define KEY4_DOWN			KEY_4_DOWN		/* �� */
#define KEY4_UP				KEY_4_UP
#define KEY4_SINGLE			KEY_4_SINGLE
#define KEY4_DOUBLE			KEY_4_DOUBLE
#define KEY4_THREE			KEY_4_THREE
#define KEY4_LONG			KEY_4_LONG
#define KEY4_LONG_LONG		KEY_4_LONG_LONG
	 
/*
	�����ֵ����, ���밴���´���ʱÿ�����İ��¡�����ͳ����¼�

	�Ƽ�ʹ��enum, ����#define��ԭ��
	(1) ����������ֵ,�������˳��ʹ���뿴���������
	(2) �������ɰ����Ǳ����ֵ�ظ���
*/
typedef enum
{
	KEY_NONE = 0,			/* 0 ��ʾ�����¼� */

	KEY_1_DOWN,				/* 1������ */
	KEY_1_UP,				/* 1������ */
	KEY_1_SINGLE,			/* 1����� */
	KEY_1_DOUBLE,			/* 1��˫�� */
	KEY_1_THREE,			/* 1������ */
	KEY_1_LONG,				/* 1������ */
	KEY_1_LONG_LONG,		/* 1�������� */
	KEY_1_N1,
	KEY_1_N2,
	
} key_e;


/* ����FIFO�õ����� */
#define KEY_FIFO_SIZE	10
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE];		/* ��ֵ������ */
	uint8_t Read;					/* ��������ָ��1 */
	uint8_t Write;					/* ������дָ�� */
	uint8_t Read2;					/* ��������ָ��2, (������ָ��) */
} keyFIFO_t;	 
	 

uint8_t KEY_Get(void);
uint8_t KEY_Peek(void);
void KEY_FIFO_Flush(void);

void KEY_Init(void);
void KEY_DeInit(void);

uint32_t KEY_Scan(void);	 
void KEY_Update(uint32_t key);

uint8_t KEY_ReadIOState(void);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_KEY_H */

/******************* (C) COPYRIGHT 2016 - 2019 All Rights Reserved *****END OF FILE****/
