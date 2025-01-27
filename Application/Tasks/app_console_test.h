
#ifndef APP_CONSOLE_TEST_H
#define APP_CONSOLE_TEST_H
#include "cli\fsl_shell.h"
#include "cli\console_scanf.h"


 int32_t io_test(p_shell_context_t ctx, int32_t argc, char** argv);
  int32_t mcu_pin(p_shell_context_t ctx, int32_t argc, char** argv);
int32_t pcb_pin(p_shell_context_t ctx, int32_t argc, char** argv);

#endif
