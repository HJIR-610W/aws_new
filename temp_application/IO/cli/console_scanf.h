
#ifndef CONSOLE_SCANF_H

#define  CONSOLE_SCANF_H


#include <stdint.h>

#include "fsl_shell.h"
#include "fsl_debug_console.h"


int32_t console_scanf(const char *fmt_ptr, ...);
void console_scanf_init( p_shell_context_t context);
void console_scanf_exit(void);

#endif
