#include "console_define.h"

#include "console_utile.h"
#include "debug_io.h"
#include "aws_menu_cali.h"

#include "test_hart.h"
#include "test_sram.h"
#include "test_sdi12.h"
#include "test_uart.h"
#include "test_rs485.h"
#include "test_rain.h"
#include "console_test.h"
#include "test_count.h"
#include "test_dinOut.h"
#include "test_power.h"
#include "test_rtc.h"
#include "test_eth.h"
#include "test_filesystem.h"
#include "test_adc.h"
#include "test_flash.h"
#include "test_lcd.h"
#include "test_modbus.h"
#include "test_key.h"

#define MENU_HART                   1
#define MENU_SRAM                   2
#define MENU_SDI12                  3
#define MENU_RS232                  4
#define MENU_RS485                  5
#define MENU_RAIN                   6
#define MENU_ADC                    7
#define MENU_COUNT                  8
#define MENU_DIGITAL_INPUT          9     // I0~I5
#define MENU_DIGITAL_OUTPUT         10    // D0~D5
#define MENU_POWER                  11    // 24V, CDMA, Module, Heater, Rain sensor
#define MENU_MODBUS                 12
#define MENU_ETHERNET               13
#define MENU_TIME                   14
#define MENU_FILESYSTEM             15
#define MENU_ADC_LINEARITY          16
#define MENU_FLASH_MEMORY           17
#define MENU_CLCD                   18
#define MENU_KEY                    19



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
    debug_printf("| 12. 모드버스                          |\r\n");
    debug_printf("| 13. 이더넷                            |\r\n");
    debug_printf("| 14. 시간                              |\r\n");
    debug_printf("| 15. 파일시스템                        |\r\n");
    debug_printf("| 16. ADC선형성                         |\r\n");
    debug_printf("| 17. FLASH 메모리                      |\r\n");
    debug_printf("| 18. CLCD                              |\r\n");
    debug_printf("| 19. KEY                               |\r\n");
    debug_printf("|     CTRL+C 이전,CTRL+Q 종료           |\r\n");
    debug_printf("+---------------------------------------+\r\n");

    status = view_input_decimal("선택", &choice, 1, 19);
    if (status == MENU_ABORT || status == MENU_BACK)
      return status;
    if (status != MENU_OK)
      continue;

    switch (choice)
    {
      case MENU_HART:
      test_hart();
      break;
      case MENU_SRAM:
      test_sram();
      break;
      case MENU_SDI12:
      test_sdi12();
      break;
      case MENU_RS232:
      test_uart();
      break;
      case MENU_RS485:
      test_rs485();
      break;
      case MENU_RAIN:
      test_rain();
      break;
      case MENU_ADC:
      aws_menu_calibration();
      break;
      case MENU_COUNT:
      test_freq();
      break;
      case MENU_DIGITAL_INPUT:
      test_di();
      break;
      case MENU_DIGITAL_OUTPUT:
      test_do();
      break;
      case MENU_POWER:
      test_power_signal();
      break;
    case MENU_MODBUS:
      test_modbus();
      break;
      case MENU_ETHERNET:
      test_eth();
      break;
    case MENU_TIME:
      test_rtc();
      break;
    case MENU_FILESYSTEM:
      test_filesystem();
      break;
    case MENU_ADC_LINEARITY:
      test_adc();
      break;
    case MENU_FLASH_MEMORY:
      test_flash();
      break;
    case MENU_CLCD:
      test_lcd();
      break;
    case MENU_KEY:
       test_key();
       break;
    default : 
    break;
    }
  }
}