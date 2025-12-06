
#include <string.h>

#include "cli_key_code.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "debug_io.h"
#include "drv_di.h"
#include "drv_do.h"
#include "drv_rs232.h"
#include "drv_adc.h"
#include "app_adc.h"
#include "app_file.h"

void test_adc(void)
{

  char buff[100];

  uint8_t err;
  int32_t adc_raw;

  dbg_printf("ADC 선형성 테스트\r\n");


  //0~5V까지 1mv 씩 입력받아서 선형성 테스트용 샘플 수집
  for (int i = 0; i < 5000; i++)
  {
    dbg_printf("싱글 채널  0전압 %dmv입력하고 아무키나 입력하세요\r\n",i);

    if (get_key(osWaitForever) == KEY_CODE_CTRL_C)
      break;

    adc_raw = (int32_t)drv_adc_single_raw_read(0,1, &err);
    snprintf(buff, sizeof(buff), "%d,%d\r\n",i,adc_raw);
    dbg_printf("%s",buff);

    append_file("adc.csv",(uint8_t *)buff,strlen(buff));
    }
    dbg_printf("종료\r\n");
}