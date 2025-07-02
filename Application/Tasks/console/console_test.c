#include "console_define.h"

#include "test_hart.h"
#include "test_sram.h"
#include "test_sdi12.h"
#include "test_uart.h"
#include "test_rs485.h"
#include "IO\dev_io.h"
#include "console_utile.h"
#include "test_rain.h"
#include "console_test.h"
#include "test_count.h"
#include "test_dinOut.h"
#include "test_power.h"

#include "test_rtc.h"
#include "test_eth.h"
#include "test_filesystem.h"
#include "aws_menu_cali.h"
#include "test_adc.h"
#include "test_flash.h"
#include "test_lcd.h"

int run_test_root()
{
  int choice, status;
  while (1)
  {
    io_printf("+---------------------------------------+\r\n");
    io_printf("|             TEST 메뉴                 |\r\n");
    io_printf("+---------------------------------------+\r\n");
    io_printf("|  1. HART                              |\r\n");
    io_printf("|  2. SRAM                              |\r\n");
    io_printf("|  3. SDI-12                            |\r\n");
    io_printf("|  4. RS232                             |\r\n");
    io_printf("|  5. RS485                             |\r\n");
    io_printf("|  6. RAIN                              |\r\n");
    io_printf("|  7. ADC                               |\r\n");
    io_printf("|  8. COUNT                             |\r\n");
    io_printf("|  9. 디지털 입력(I0~I5)                |\r\n");
    io_printf("| 10. 디지털 출력(D0~D5)                |\r\n");
    io_printf("| 11. 전원(24V,CDMA,모듈,히터,우량D)    |\r\n");
    io_printf("| 12. 모드버스(구현 예정)               |\r\n");
    io_printf("| 13. 이더넷                            |\r\n");
    io_printf("| 14. 시간                              |\r\n");
    io_printf("| 15. 파일시스템                        |\r\n");
    io_printf("| 16. ADC선형성                         |\r\n");
    io_printf("| 17. FLASH 메모리                      |\r\n");
    io_printf("| 18. CLCD                              |\r\n");
    io_printf("|     CTRL+C 이전,CTRL+Q 종료           |\r\n");
    io_printf("+---------------------------------------+\r\n");

    status = input_decimal_prompt("선택", &choice, 1, 18);
    if (status == MENU_ABORT || status == MENU_BACK)
      return status;
    if (status != MENU_OK)
      continue;

    switch (choice)
    {
      case 1:
      test_hart();
      break;
      case 2:
      test_sram();
      break;
      case 3:
      test_sdi12();
      break;
      case 4:
      test_uart();
      break;
      case 5:
      test_rs485();
      break;
      case 6:
      test_rain();
      break;
      case 7:
      aws_menu_calibration();
      break;
      case 8:
      test_freq();
      break;
      case 9:
      test_di();
      break;
      case 10:
      test_do();
      break;
      case 11:
      test_power_signal();
      break;
      case 13:
      test_eth();
      break;
       case 14:
        test_rtc();
        break;
        case 15:
          test_filesystem();
          break;
    case 16:
      test_adc();
      break;
      case 17:
      test_flash();
      break;
      case 18:
      test_lcd();
      break;
           default : break;
    }
  }
}