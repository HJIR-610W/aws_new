#ifndef CONSOLE_UTILE_H

#define CONSOLE_UTILE_H

#include <stdint.h>
#include <stdbool.h>

#include "console_define.h"


#define STRING_INPUT_ERR "입력이 잘못되었습니다"

typedef int32_t (*menu_func)(void);


#define EXIT_PROGRAM -3
#define EXIT_BACK -1

#define ITEM_LIST(cnt, list) cnt >= _countof(list) ? g_unknown : (char *)list[cnt]

char recv_key(uint32_t timeout_ms) ;
void make_comList(char *out, uint16_t outsize);

int input_decimal_prompt(const char* prompt, int* value, int min_val, int max_val);

int input_float_prompt(const char* prompt, float min, float max, float* value);

    int print_menu(int width, const char* title, char** menu_list, int cnt);
int32_t choice_menu(int width, const char* title, char** menu_list, int cnt,int32_t *choice);
int32_t select_index_from_table(const char* list[], int32_t (*func)(), uint16_t listCnt, bool number,
                             int32_t* choice);



bool wait_break(uint32_t timeoutms);
int32_t choice_enable(uint8_t* enable);
int confirm_continue(const char *,int32_t* ok);
int check_pass(const char* title, char* password_str, int* ok);
int32_t console_scanf_s(const char* fmt, ...);
    extern const char* g_unknown;

extern const char* enableList[2];
#endif