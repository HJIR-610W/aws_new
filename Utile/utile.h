



#ifndef UTILE_H
#define UTILE_H


#define _countof(_Array)     sizeof(_Array) / sizeof(_Array[0])            

#define TOSTRING(x) #x
#define OFFSET_OF_STRUCT(s,m) ((size_t)&(((s*)0)->m))

int getPinNumber(uint16_t pin);
void hex_to_binary_string(uint16_t hex_value, char *binary_str, int bit_length);

uint16_t swap_uint16(uint16_t value);
#endif
