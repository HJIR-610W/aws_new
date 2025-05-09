#ifndef CONSOLE_UTILE_H

#define CONSOLE_UTILE_H

#include <stdint.h>

#define ITEM_LIST(cnt, list) cnt >= _countof(list) ? g_unknown : (char *)list[cnt]

char recv_key(uint32_t timeout_ms) ;
void make_comList(char *out, uint16_t outsize);

int get_int_input(const char* prompt, int* value, int min_val, int max_val);


extern const char *g_unknown;
#endif