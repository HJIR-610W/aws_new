

#ifndef UTILE_STDIO_H
#define UTILE_STDIO_H


#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

void make_centered(char *buffer, size_t buf_size, const char *text, int width);
int32_t get_formatted_length_v(const char *format, va_list args);
size_t utf8_strlen(const char* s);
#endif