#include "console_define.h"

#include "test_hart.h"
#include "test_sram.h"
#include "test_sdi12.h"
#include "test_uart.h"
#include "test_rs485.h"
#include "IO\dev_io.h"
#include "console_utile.h"
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
    debug_printf("|  3. SDI12                             |\r\n");
    debug_printf("|  4. RS232                             |\r\n");
    debug_printf("|  5. RS485                             |\r\n");
    debug_printf("|     CTRL+C 이전,CTRL+Q 종료           |\r\n");
    debug_printf("+---------------------------------------+\r\n");

    status = get_int_input("선택", &choice, 1, 5);
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

           default : break;
    }
  }
}