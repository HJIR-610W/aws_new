
#ifndef CONSOLE_SCANF_H
#define CONSOLE_SCANF_H

#include <stdint.h>


int32_t console_scanf(const char *fmt_ptr, ...);
void console_scanf_init(void);
void console_scanf_exit(void);

#endif
