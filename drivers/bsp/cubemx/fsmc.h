
#ifndef __FSMC_H
#define __FSMC_H
#ifdef __cplusplus
 extern "C" {
#endif

#include <string.h>

#include "pcb_define.h"
#include "system_err.h"



void MX_FSMC_Init(void);
void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram);
void HAL_SRAM_MspDeInit(SRAM_HandleTypeDef *hsram);



#ifdef __cplusplus
 }
#endif
#endif

