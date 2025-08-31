


#include <stdint.h>

#include "stm32f4xx_hal.h"

void bsp_delay_init(void) {
    //if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // DWT 타이머 활성화
        DWT->CYCCNT = 0; // 사이클 카운터 초기화
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // 사이클 카운터 활성화
    }
}


void bsp_us_delay(uint32_t us) {
    uint32_t start = DWT->CYCCNT; // 시작 시점의 사이클 카운터 읽기
    uint32_t delayTicks = us * (SystemCoreClock / 1000000); // 지연할 사이클 수 계산 (1us 단위)

    
    
    
    while ((DWT->CYCCNT - start) < delayTicks); // 사이클이 충분히 지날 때까지 대기
}

uint32_t mcu_get_clk(void)
{
 return DWT->CYCCNT;;   
}

uint32_t cal_elapsed_us(uint32_t start)
{
    uint32_t time_us;
    uint32_t cycles;

    cycles = DWT->CYCCNT - start;
    time_us = (uint32_t)(cycles * (1.0 / (SystemCoreClock / 1000000)));  // 168MHz = 1

    return time_us;
}

