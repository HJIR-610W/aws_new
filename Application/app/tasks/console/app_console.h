
#ifndef APP_CONSOLE_H
#define APP_CONSOLE_H

#include "cli\fsl_shell.h"
#include "cli\console_scanf.h"

 int32_t menu_root(p_shell_context_t context, int32_t argc, char** argv);
 int32_t menu_develop(p_shell_context_t ctx, int32_t argc, char** argv);
#endif