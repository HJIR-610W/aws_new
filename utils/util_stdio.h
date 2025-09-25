

#ifndef UTILE_STDIO_H
#define UTILE_STDIO_H


#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
void make_centered(char *buffer, size_t buf_size, const char *text, int width);
int32_t get_formatted_length_v(const char *format, va_list args);
size_t utf8_strlen(const char* s);
char *m_l(char *label, int width);//utf-8한글 자간 일정 
void make_utf8_string(char *buff,int buff_size,int wd,const char *string);//자간 일정
float round_to_n_digits(double value, int n);
bool normalize_bool(uint8_t raw);
#endif