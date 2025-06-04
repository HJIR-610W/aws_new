
#ifndef SYSTEM_ERR_H
#define SYSTEM_ERR_H

#include <stdint.h>
#include <stdbool.h>

void Error_Handler(const char *file,int32_t line);
void reset_system(const char * pFmt, ...);
bool restore_error(char *p_out, int32_t out_size);
void assert_print(uint8_t *file, uint32_t line, char *msg);

#endif
