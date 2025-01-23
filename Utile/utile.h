



#ifndef UTILE_H
#define UTILE_H


#define _countof(_Array)     sizeof(_Array) / sizeof(_Array[0])            

#define TOSTRING(x) #x
#define OFFSET_OF_STRUCT(s,m) ((size_t)&(((s*)0)->m))
#define OFFSET_S(start,stop) ((unsigned int)stop - (unsigned int)start) //두 메모리사이 크기

int getPinNumber(uint16_t pin);
void hex_to_binary_string(uint16_t hex_value, char *binary_str, int bit_length);

uint16_t swap_uint16(uint16_t value);

uint16_t  GetWord(uint8_t* lpBuff);
void    SetWord(uint8_t *lpBuff, uint16_t shVal);		// Big Endiand으로 취함
#endif
