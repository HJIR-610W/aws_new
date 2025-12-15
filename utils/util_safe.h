
#ifndef UTIL_SAFE_H
#define UTIL_SAFE_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>


typedef int errno_safe_t;
typedef size_t rsize_safe_t;


/* 안전한 메모리 함수들 */
errno_safe_t memcpy_safe(void *dest, rsize_safe_t dest_size, const void *src, rsize_safe_t count);
size_t strlen_safe(const char *str, rsize_safe_t max_len);
errno_safe_t memset_safe(void *dest, rsize_safe_t dest_size, int ch, rsize_safe_t count);

/* 안전한 문자열 함수들 */
errno_safe_t strcpy_safe(char *dest, rsize_safe_t dest_size, const char *src);
errno_safe_t strcat_safe(char *dest, rsize_safe_t dest_size, const char *src);
char *strtok_safe(char *str, const char *delim, char **saveptr);

/* 안전한 변환 함수들 */
long atol_safe(const char *s);
int atoi_safe(const char *s);
double strtod_safe(const char *string, char **endPtr);


/* 안전한 시간 함수들 */
struct tm *localtime_safe(const time_t *timer, struct tm *result);

#endif