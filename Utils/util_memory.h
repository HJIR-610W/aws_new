



#ifndef UTILE_H
#define UTILE_H

#include <stdbool.h>
#include <stdio.h>

#define BIT_UPDATE(val, cond, bitmask) \
  do                                   \
  {                                    \
    if (cond)                          \
      (val) |= (bitmask);              \
    else                               \
      (val) &= ~(bitmask);             \
  } while (0)
#define _countof(_Array)     sizeof(_Array) / sizeof(_Array[0])            

#define TOSTRING(x) #x
#define OFFSET_OF_STRUCT(s,m) ((size_t)&(((s*)0)->m))
#define OFFSET_S(start,stop) ((unsigned int)stop - (unsigned int)start) //두 메모리사이 크기
#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

#define MAX_ARGV 10

#define CIRCULAR_PUSH(arr, index, value, max) \
  do                                          \
  {                                           \
    arr[index] = value;                       \
    index = (index + 1) % max;                \
  } while (0)

/*사용 예
버퍼가 3개로 3개의 샘플을 평균내는 코드에서
버퍼가 다 차지 전까지는 저장한 샘플수만큼만 평균내고
샘플이 다찬상태에서는 버퍼갯수 만큼 평균을 낼때 사용

index가 max보다 작으면 버퍼가 한번이라도 완전히 찬적이 없으면
버퍼가 max보다 같거나 크면 한번은 완전히 찬 상태가 된다.

max가 20이면 index는 0~19사이인데
20~40사이 값을 모듈러 연산해도 동일한 값이다.
대신 이버퍼가 완전히 찬적이 있는지 판단할수 있게 한다.
*/
#define CIRCULAR_PUSH2(arr, index, value, max) \
  do                                           \
  {                                            \
    int idx = index % max;                     \
    arr[idx] = value;                          \
    index++;                                   \
    if (index >= max * 2)                      \
    {                                          \
      index = max;                             \
    }                                          \
  } while (0);

#define UPDATE_CNT(cnt_ptr, max_val)   \
  do                                   \
  {                                    \
    uint8_t _val = cnt_ptr + 1;        \
    if (_val > (max_val) || _val == 0) \
    {                                  \
      _val = 1;                        \
    }                                  \
    cnt_ptr = _val;                    \
  } while (0)

int getPinNumber(uint16_t pin);
void hex_to_binary_string(uint16_t hex_value, char *binary_str, int bit_length);

uint16_t swap_uint16(uint16_t value);

uint16_t  GetWord(uint8_t* lpBuff);
void    SetWord(uint8_t *lpBuff, uint16_t shVal);		// Big Endiand으로 취함


uint32_t parse_args2(char* str, char* argv[], uint32_t argvCnt);
char * h_findnum(char *buff);
uint32_t parse_args(char* str, char* argv[],uint32_t argvCnt);
bool isDigit(uint8_t d);

void strcpy_safe(char* det, size_t detSize, const char* src);
size_t memcpy_safe(uint8_t* des, size_t desLen, uint8_t* src, size_t len);

uint32_t Convert_HexAscii2uchar(char* src, uint16_t len, uint8_t * dst);


uint32_t Convert_ucharHexAscii(uint8_t* src, uint16_t len, char* dst);

float recursiveAvg(double pre_avg,float adc, int cnt);

uint8_t	 make_sum(uint8_t *lpRcv, uint32_t len);
float round_to(float value, int digits);

    bool equal_float(float x, float y);
bool less_float(float a, float b);
bool bigger_float(float a, float b);
bool bigger_equal_float(float a,float b);
bool less_equal_float(float a, float b);

uint8_t calculate_xor_checksum(const uint8_t* data, uint16_t length);

#endif
