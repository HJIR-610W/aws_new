#ifndef __TASK_LOGGER_H__
#define __TASK_LOGGER_H__

#include "cmsis_os.h"
#include <stdarg.h>

int32_t os_printf(const char *fmt, ...);
int32_t os_vprintf(const char *fmt, va_list ap);
void loggerTask_init(void);
int32_t os_puts(const char *str);
void os_put_ch(uint8_t ch);
void os_debug_send(const uint8_t *data, size_t len);

#endif /* __TASK_LOGGER_H__ */
