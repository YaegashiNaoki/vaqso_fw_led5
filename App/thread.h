#ifndef __THREAD_H
#define __THREAD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "timers.h"

void THREAD_Init(void);

void THREAD_Start(void);
void THREAD_Create(void);



#ifdef __cplusplus
}
#endif

#endif /* __THREAD_H */

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
