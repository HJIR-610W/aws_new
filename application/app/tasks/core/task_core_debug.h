
#ifndef TASK_DEBUG_H
#define TASK_DEBUG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void set_task_id(void *task_id);
void task_printf( const char *pFmt, ...);
void task_hex_dump(const char *title, const uint8_t *data, size_t length);
void set_forced_print(bool set);
const char* get_task_name(void);

#endif