

#ifndef MENU_HANDLER_H

#define MENU_HANDLER_H
#include <stdint.h>

#include "console_define.h"

#define SIGN_ENABLE 1
#define SIGN_DISABLE 0
int32_t print_menu_list(const char* menu_list[], int32_t menu_count, int* choice);
int32_t input_decimal(const char* title, int min, int max, int* val, int sign_use);

#endif