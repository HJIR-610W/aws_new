
#include "utile_stdio.h"


int32_t get_formatted_length_v(const char *format, va_list args)
{
    // vsnprintf를 사용하여 길이 계산
    int len = vsnprintf(NULL, 0, format, args);
    return len;
}