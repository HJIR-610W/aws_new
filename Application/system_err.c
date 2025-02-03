
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "dev_io.h"
#include "system_err.h"
#include "pcb_define.h"
#include "utile_time.h"


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(const char *file,const int32_t line)
{
 
   debug_printf("%s,%d\r\n",file,line);

//   __disable_irq();
    
//   __asm("BKPT #0"); 

  /* USER CODE END     Error_Handler(__FILE__,__LINE__);_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


#define RST_LOG_MAX 100
//2022-11-11 11:00:00,10,12,test
typedef struct no_init_s
{
  char rstLog[RST_LOG_MAX];
}no_init_t;
__no_init volatile no_init_t noInitData @ 0x20000000; //이 주소에 할당되도록 한다 IAR 전용



void reset_system(uint16_t code,const char * pFmt, ...)
{
    char buff[RST_LOG_MAX];
    uint32_t len=0;
    va_list ap;
    DATE_TIME_BUF tn={1,1,1,1,1,1};
 
    __disable_irq();;//TODO 인터럽트 비활성 코드 삽입
    
    //time_getFromDirect_unsafe(&tn);
    
    snprintf_s(&buff[len],sizeof(buff),"RST,%04d-%02d-%02d %02d:%02d:%02d,%d,",
    tn.Year,tn.Month,tn.Day,tn.Hour,tn.Min,tn.Sec,code);

    len = strnlen_s(buff, sizeof(buff));

    va_start(ap, pFmt);
    vsnprintf_s(&buff[len], sizeof(buff)-len,pFmt, ap);
    va_end(ap);

    strcpy_s((char *)noInitData.rstLog, sizeof(noInitData.rstLog), buff);//리셋 원인 기록

    HAL_NVIC_SystemReset();

}