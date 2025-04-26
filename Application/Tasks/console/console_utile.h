#ifndef CONSOLE_UTILE_H

#define CONSOLE_UTILE_H

#define ITEM_LIST(cnt, list) cnt >= _countof(list) ? g_unknown : (char *)list[cnt]


char recv_key(void);
void make_comList(char *out, uint16_t outsize);
extern const char *g_unknown;
#endif