
#include "stm32f4xx_hal.h"

// SWO 출력 함수
void SWO_PrintChar(char c) {
    if (ITM->TCR & ITM_TCR_ITMENA_Msk) {        // ITM 활성화 확인
        if (ITM->TER & (1UL << 0)) {            // 포트 0 활성화 확인
            while (ITM->PORT[0].u32 == 0);      // 포트가 비어있는지 확인
            ITM->PORT[0].u8 = c;                // 데이터 전송
        }
    }
}

// 문자열 출력 예제
void swo_puts(const char *str)
{
    while (*str) {
    if (ITM->TCR & ITM_TCR_ITMENA_Msk) {        // ITM 활성화 확인
        if (ITM->TER & (1UL << 0)) {            // 포트 0 활성화 확인
            while (ITM->PORT[0].u32 == 0);      // 포트가 비어있는지 확인
            ITM->PORT[0].u8 = *str++;                // 데이터 전송
        }
    }
    }
}
