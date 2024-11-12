



#ifndef UTILE_H
#define UTILE_H


#define _countof(_Array)     sizeof(_Array) / sizeof(_Array[0])            



int getPinNumber(uint16_t pin);
void hex_to_binary_string(uint16_t hex_value, char *binary_str, int bit_length);
#endif
