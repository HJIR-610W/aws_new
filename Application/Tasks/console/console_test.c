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
#include "console_cali.h"
#include "test_rtc.h"
#include "test_eth.h"
int run_test_root()
{
  int choice, status;
  while (1)
  {
    debug_printf("+---------------------------------------+\r\n");
    debug_printf("|             TEST 메뉴                 |\r\n");
    debug_printf("+---------------------------------------+\r\n");
    debug_printf("|  1. HART                              |\r\n");
    debug_printf("|  2. SRAM                              |\r\n");
    debug_printf("|  3. SDI-12                            |\r\n");
    debug_printf("|  4. RS232                             |\r\n");
    debug_printf("|  5. RS485                             |\r\n");
    debug_printf("|  6. RAIN                              |\r\n");
    debug_printf("|  7. ADC                               |\r\n");
    debug_printf("|  8. COUNT                             |\r\n");
    debug_printf("|  9. 디지털 입력(I0~I5)                |\r\n");
    debug_printf("| 10. 디지털 출력(D0~D5)                |\r\n");
    debug_printf("| 11. 전원(24V,CDMA,모듈,히터,우량D)    |\r\n");
    debug_printf("| 12. 모드버스(구현 예정)               |\r\n");
    debug_printf("| 13. 이더넷                            |\r\n");
    debug_printf("| 14. 시간                              |\r\n");
    debug_printf("|     CTRL+C 이전,CTRL+Q 종료           |\r\n");
    debug_printf("+---------------------------------------+\r\n");

    status = get_int_input("선택", &choice, 1, 15);
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
      run_calibraion_root();
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
      default :
      break;
    }
  }
}