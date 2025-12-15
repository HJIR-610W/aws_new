

#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "util_safe.h"

#ifndef ULONG_MAX
#define	ULONG_MAX	((unsigned long)(~0L))		/* 0xFFFFFFFF */
#endif

#ifndef LONG_MAX
#define	LONG_MAX	((long)(ULONG_MAX >> 1))	/* 0x7FFFFFFF */
#endif

#ifndef LONG_MIN
#define	LONG_MIN	((long)(~LONG_MAX))		/* 0x80000000 */
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

/* -214748364 ~ 2147483647 */

/**
 * @brief 문자열에서 숫자 추출
 * LONG 사이즈까지만 검출
 */
long atol_safe(const char* s)
{
	long ret = 0;
	long d;
	long min = ~LONG_MAX;
	long max = LONG_MAX;

	int neg = 0;

	if (*s == '-')
	{
		neg = 1;
		s++;
	}

	while(1)
	{
		d = (*s++) - '0';
		if (d > 9)
			break;
		ret *= 10;
		ret += d;

		if(neg)
		{
			if (ret <= min)
			{
				break;
			}
		}
		else
		{
			if (ret >= max)
			{
				break;
			}
		}
	}

	return neg ? ret*(-1) : ret;
}





/* 최대 지수값 511. 이보다 큰 지수는 이미 언더플로우나 오버플로우를 발생시키므로 추가 자릿수를 고려할 필요 없음 */
static int maxExponent = 511;

/* 10의 거듭제곱 테이블. 10^2^i 값을 저장. 10진수 지수를 부동소수점 숫자로 변환하는데 사용 */
static double powersOf10[] = {
	10.,
	100.,
	1.0e4,
	1.0e8,
	1.0e16,
	1.0e32,
	1.0e64,
	1.0e128,
	1.0e256
};

/**
 * @brief 문자열을 double로 안전하게 변환
 * @param string 변환할 문자열 (선택적으로 공백으로 시작 가능)
 *               형식: "-I.FE-X" (I: 가수의 정수부, F: 가수의 소수부, X: 지수)
 *               부호는 "+", "-" 또는 생략 가능. I 또는 F는 생략 가능
 *               소수점은 F가 있을 때만 필요. "E"는 "e"도 가능
 * @param endPtr NULL이 아니면 종료 문자의 주소를 저장
 * @return 변환된 double 값
 */

double strtod_safe(const char* string, char** endPtr)
{
	int sign, expSign = FALSE;
	int c;
	int exp = 0;		/* "EX" 필드에서 읽은 지수 */
	int fracExp = 0;	/* 소수부에서 파생된 지수. 일반적으로 F의 자릿수의 음수값.
				 * 단, I가 매우 길면 I의 마지막 자릿수가 삭제되며,
				 * 이 경우 삭제된 자릿수마다 fracExp가 1씩 증가 */
	int mantSize;		/* 가수의 자릿수 */
	int decPt;		/* 소수점 이전의 가수 자릿수 */
	double fraction, dblExp, * d;
	register const char* p;
	const char* pExp;	/* 문자열에서 지수의 위치를 임시 저장 */

	/* 선행 공백 제거 및 부호 확인 */
	p = string;
	while (isspace(*p)) {
		p += 1;
	}
	if (*p == '-') {
		sign = TRUE;
		p += 1;
	}
	else {
		if (*p == '+') {
			p += 1;
		}
		sign = FALSE;
	}

	/* 가수의 자릿수(소수점 포함)를 세고 소수점 위치를 찾음 */
	decPt = -1;
	for (mantSize = 0; ; mantSize += 1)
	{
		c = *p;
		if (!isdigit(c)) {
			if ((c != '.') || (decPt >= 0)) {
				break;
			}
			decPt = mantSize;
		}
		p += 1;
	}

	/* 가수의 자릿수를 수집. 두 개의 정수를 사용하여 각각 9자리씩 수집 (부동소수점 사용보다 빠름).
	 * 가수가 18자리를 초과하면 나머지는 무시 (값에 영향을 주지 않음) */

	pExp = p;
	p -= mantSize;
	if (decPt < 0) {
		decPt = mantSize;
	}
	else {
		mantSize -= 1;			/* One of the digits was the point. */
	}
	if (mantSize > 18) {
		fracExp = decPt - 18;
		mantSize = 18;
	}
	else {
		fracExp = decPt - mantSize;
	}
	if (mantSize == 0) {
		fraction = 0.0;
		p = string;
		goto done;
	}
	else {
		int frac1, frac2;
		frac1 = 0;
		for (; mantSize > 9; mantSize -= 1)
		{
			c = *p;
			p += 1;
			if (c == '.') {
				c = *p;
				p += 1;
			}
			frac1 = 10 * frac1 + (c - '0');
		}
		frac2 = 0;
		for (; mantSize > 0; mantSize -= 1)
		{
			c = *p;
			p += 1;
			if (c == '.') {
				c = *p;
				p += 1;
			}
			frac2 = 10 * frac2 + (c - '0');
		}
		fraction = (1.0e9 * frac1) + frac2;
	}

	/* 지수 추출 */
	p = pExp;
	if ((*p == 'E') || (*p == 'e')) {
		p += 1;
		if (*p == '-') {
			expSign = TRUE;
			p += 1;
		}
		else {
			if (*p == '+') {
				p += 1;
			}
			expSign = FALSE;
		}
		while (isdigit(*p)) {
			exp = exp * 10 + (*p - '0');
			p += 1;
		}
	}
	if (expSign) {
		exp = fracExp - exp;
	}
	else {
		exp = fracExp + exp;
	}

	/* 지수를 나타내는 부동소수점 숫자 생성.
	 * 지수를 한 비트씩 처리하여 10의 2의 거듭제곱들을 결합.
	 * 그 다음 지수와 분수를 결합 */

	if (exp < 0) {
		expSign = TRUE;
		exp = -exp;
	}
	else {
		expSign = FALSE;
	}
	if (exp > maxExponent) {
		exp = maxExponent;
		errno = ERANGE;
	}
	dblExp = 1.0;
	for (d = powersOf10; exp != 0; exp >>= 1, d += 1) {
		if (exp & 01) {
			dblExp *= *d;
		}
	}
	if (expSign) {
		fraction /= dblExp;
	}
	else {
		fraction *= dblExp;
	}
done:
	if (endPtr != NULL) {
		*endPtr = (char*)p;
	}
	if (sign) {
		return -fraction;
	}
	return fraction;
}





int atoi_safe(const char *s)
{
    char *end;

    return strtol(s,&end,10);
}


/**
 * @brief 경계 검사를 포함한 안전한 memcpy 구현
 * @param dest 대상 버퍼 포인터
 * @param dest_size 대상 버퍼 크기 (바이트)
 * @param src 원본 버퍼 포인터
 * @param count 복사할 바이트 수
 * @return 성공 시 0, 실패 시 -1
 */
errno_safe_t memcpy_safe(void *dest, rsize_safe_t dest_size, const void *src, rsize_safe_t count)
{
  const unsigned char *s;
  unsigned char *d;
  rsize_safe_t i;

  /* 파라미터 검증 */
  if (dest == NULL || src == NULL)
  {
    return -1;
  }

  if (dest_size == 0 || count == 0)
  {
    return -1;
  }

  /* 버퍼 오버플로우 검사 */
  if (count > dest_size)
  {
    return -1;
  }

  /* 버퍼 중복 검사 */
  if (dest == src)
  {
    return 0;
  }

  /* 복사 수행 */
  s = (const unsigned char *)src;
  d = (unsigned char *)dest;

  for (i = 0; i < count; i++)
  {
    d[i] = s[i];
  }

  return 0;
}

/**
 * @brief 최대 길이 검사를 포함한 안전한 strlen 구현
 * @param str null로 종료되는 문자열 포인터
 * @param max_len 검사할 최대 길이
 * @return 문자열 길이 또는 null terminator를 찾지 못하면 max_len
 */
size_t strlen_safe(const char *str, rsize_safe_t max_len)
{
  rsize_safe_t len;

  /* 파라미터 검증 */
  if (str == NULL || max_len == 0)
  {
    return 0;
  }

  /* max_len까지 문자열 길이 찾기 */
  for (len = 0; len < max_len; len++)
  {
    if (str[len] == '\0')
    {
      return len;
    }
  }

  /* max_len 내에서 null terminator를 찾지 못함 */
  return max_len;
}

/**
 * @brief 경계 검사를 포함한 안전한 memset 구현
 * @param dest 대상 버퍼 포인터
 * @param dest_size 대상 버퍼 크기 (바이트)
 * @param ch 설정할 값
 * @param count 설정할 바이트 수
 * @return 성공 시 0, 실패 시 -1
 */
errno_safe_t memset_safe(void *dest, rsize_safe_t dest_size, int ch, rsize_safe_t count)
{
  unsigned char *d;
  unsigned char value;
  rsize_safe_t i;

  /* 파라미터 검증 */
  if (dest == NULL)
  {
    return -1;
  }

  if (dest_size == 0 || count == 0)
  {
    return -1;
  }

  /* 버퍼 오버플로우 검사 */
  if (count > dest_size)
  {
    return -1;
  }

  /* memset 수행 */
  d = (unsigned char *)dest;
  value = (unsigned char)ch;

  for (i = 0; i < count; i++)
  {
    d[i] = value;
  }

  return 0;
}

/**
 * @brief 경계 검사를 포함한 안전한 strcpy 구현
 * @param dest 대상 문자열 버퍼 포인터
 * @param dest_size 대상 버퍼 크기 (바이트)
 * @param src 원본 null로 종료되는 문자열 포인터
 * @return 성공 시 0, 실패 시 -1
 */
errno_safe_t strcpy_safe(char *dest, rsize_safe_t dest_size, const char *src)
{
  char *d;
  const char *s;
  rsize_safe_t i;

  /* 파라미터 검증 */
  if (dest == NULL || src == NULL)
  {
    return -1;
  }

  if (dest_size == 0)
  {
    return -1;
  }

  /* dest와 src 중복 검사 */
  if (dest == src)
  {
    return 0;
  }

  /* 문자열 복사 */
  d = dest;
  s = src;

  for (i = 0; i < dest_size - 1; i++)
  {
    d[i] = s[i];
    if (s[i] == '\0')
    {
      return 0;
    }
  }

  /* null terminator 보장 */
  d[dest_size - 1] = '\0';

  /* 원본 문자열이 너무 긴지 검사 */
  if (s[i] != '\0')
  {
    return -1;
  }

  return 0;
}

/**
 * @brief 경계 검사를 포함한 안전한 strcat 구현
 * @param dest 대상 문자열 버퍼 포인터
 * @param dest_size 대상 버퍼 크기 (바이트)
 * @param src 추가할 null로 종료되는 문자열 포인터
 * @return 성공 시 0, 실패 시 -1
 */
errno_safe_t strcat_safe(char *dest, rsize_safe_t dest_size, const char *src)
{
  char *d;
  const char *s;
  rsize_safe_t dest_len;
  rsize_safe_t i;

  /* 파라미터 검증 */
  if (dest == NULL || src == NULL)
  {
    return -1;
  }

  if (dest_size == 0)
  {
    return -1;
  }

  /* dest의 현재 길이 찾기 */
  dest_len = strlen_safe(dest, dest_size);

  /* dest가 null terminator를 포함하지 않으면 오류 */
  if (dest_len >= dest_size)
  {
    return -1;
  }

  /* src를 추가할 공간이 충분한지 확인 */
  d = dest + dest_len;
  s = src;

  for (i = dest_len; i < dest_size - 1; i++)
  {
    if (*s == '\0')
    {
      *d = '\0';
      return 0;
    }
    *d = *s;
    d++;
    s++;
  }

  /* null terminator 보장 */
  dest[dest_size - 1] = '\0';

  /* src 문자열이 너무 긴지 검사 */
  if (*s != '\0')
  {
    return -1;
  }

  return 0;
}

/**
 * @brief 재진입 가능한 안전한 strtok 구현 (strtok_r과 동일)
 * @param str 토큰화할 문자열 (첫 호출) 또는 NULL (이후 호출)
 * @param delim 구분자 문자열
 * @param saveptr 다음 토큰 위치를 저장할 포인터의 주소
 * @return 다음 토큰의 포인터, 토큰이 없으면 NULL
 */
char *strtok_safe(char *str, const char *delim, char **saveptr)
{
  char *token_start;
  char *token_end;

  /* 파라미터 검증 */
  if (delim == NULL || saveptr == NULL)
  {
    return NULL;
  }

  /* 시작 위치 결정 */
  if (str != NULL)
  {
    token_start = str;
  }
  else
  {
    token_start = *saveptr;
  }

  /* 문자열 끝이면 NULL 반환 */
  if (token_start == NULL || *token_start == '\0')
  {
    *saveptr = NULL;
    return NULL;
  }

  /* 선행 구분자 건너뛰기 */
  while (*token_start != '\0')
  {
    const char *d;
    int is_delim = 0;

    for (d = delim; *d != '\0'; d++)
    {
      if (*token_start == *d)
      {
        is_delim = 1;
        break;
      }
    }

    if (!is_delim)
    {
      break;
    }

    token_start++;
  }

  /* 선행 구분자만 있고 문자열 끝이면 NULL 반환 */
  if (*token_start == '\0')
  {
    *saveptr = NULL;
    return NULL;
  }

  /* 토큰 끝 찾기 */
  token_end = token_start;
  while (*token_end != '\0')
  {
    const char *d;
    int is_delim = 0;

    for (d = delim; *d != '\0'; d++)
    {
      if (*token_end == *d)
      {
        is_delim = 1;
        break;
      }
    }

    if (is_delim)
    {
      *token_end = '\0';
      *saveptr = token_end + 1;
      return token_start;
    }

    token_end++;
  }

  /* 문자열 끝까지 토큰 */
  *saveptr = NULL;
  return token_start;
}

/**
 * @brief 경계 검사를 포함한 안전한 localtime 구현
 * @param timer 변환할 time_t 값 포인터
 * @param result 결과를 저장할 struct tm 버퍼 포인터
 * @return 성공 시 result 포인터, 실패 시 NULL
 */
struct tm *localtime_safe(const time_t *timer, struct tm *result)
{
  struct tm *tmp;

  /* 파라미터 검증 */
  if (timer == NULL || result == NULL)
  {
    return NULL;
  }

  /* localtime 호출 및 사용자 버퍼에 결과 복사 */
  tmp = localtime(timer);
  if (tmp == NULL)
  {
    return NULL;
  }

  /* 결과 버퍼에 복사 */
  memcpy_safe(result, sizeof(struct tm), tmp, sizeof(struct tm));

  return result;
}

