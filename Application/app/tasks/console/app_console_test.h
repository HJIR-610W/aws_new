
#ifndef APP_CONSOLE_TEST_H
#define APP_CONSOLE_TEST_H
#include "cli\fsl_shell.h"
#include "cli\console_scanf.h"



  int32_t mcu_pin(p_shell_context_t ctx, int32_t argc, char** argv);
  int32_t pcb_pin(void);

      int32_t print_di(p_shell_context_t ctx, int32_t argc, char** argv);

  int32_t test_pcb(p_shell_context_t ctx, int32_t argc, char** argv);
#endif
