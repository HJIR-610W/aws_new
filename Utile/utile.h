



#ifndef UTILE_H
#define UTILE_H

#include <stdbool.h>
#include <stdio.h>


#define _countof(_Array)     sizeof(_Array) / sizeof(_Array[0])            

#define TOSTRING(x) #x
#define OFFSET_OF_STRUCT(s,m) ((size_t)&(((s*)0)->m))
#define OFFSET_S(start,stop) ((unsigned int)stop - (unsigned int)start) //두 메모리사이 크기

#define MAX_ARGV 10

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
#endif
