

#ifndef MENU_HANDLER_H

#define MENU_HANDLER_H
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "console_define.h"

#define SIGN_ENABLE 1
#define SIGN_DISABLE 0

typedef void (*string_fmt_callback_t)(char buff[21]);

typedef struct string_fmt_s
{
  char data[21];
  const char* fmt;
} string_fmt_t;

int32_t print_menu_list(const char* menu_list[], int32_t menu_count, int* choice);
int32_t input_decimal(const char* title, int min, int max, int* val);
int input_fmt(string_fmt_t* strfmt, const char* title);
int32_t input_float(const char *title, float min, float max, float *val, const char *fmt);
int32_t input_combobox(const char* title, const char* item_list[], int32_t item_count, int* choice);
int32_t show_popup(const char *title,const char *message);
int32_t convert_key_to_status(int key);

int32_t input_active(const char* title, int32_t* choice);
int32_t show_ok(const char* title, const char* msg);
#endif