
#ifndef APP_CONSOLE_TEST_H
#define APP_CONSOLE_TEST_H
#include "cli\fsl_shell.h"
#include "cli\console_scanf.h"



  int32_t mcu_pin( int32_t argc, char** argv);
  int32_t pcb_pin(void);

      int32_t print_di( int32_t argc, char** argv);

  int32_t test_pcb( int32_t argc, char** argv);
#endif
