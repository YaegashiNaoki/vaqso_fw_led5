#ifndef __XTIMER_H
#define __XTIMER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "timers.h"

void XTIMER_Init(void);

void XTIMER_Start(void);
void XTIMER_Create(void);



#ifdef __cplusplus
}
#endif

#endif /* __XTEIMR_H */

/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
