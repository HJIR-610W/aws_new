

#ifndef MENU_HANDLER_H

#define MENU_HANDLER_H
#include <stdint.h>

#include "console_define.h"
int32_t print_menu_list(const char* menu_list[], int32_t menu_count, int* choice);
#endif