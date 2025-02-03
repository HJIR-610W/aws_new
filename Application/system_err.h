
#ifndef SYSTEM_ERR_H
#define SYSTEM_ERR_H

#include <stdint.h>

void Error_Handler(const char *file,int32_t line);
void reset_system(uint16_t code,const char * pFmt, ...);
#endif
