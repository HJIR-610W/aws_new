#ifndef CONSOLE_UTILE_H

#define CONSOLE_UTILE_H

#include <stdint.h>
#include <stdbool.h>

#include "console_define.h"

#include "menu_handler.h"
#define STRING_INPUT_ERR "입력이 잘못되었습니다"

typedef int32_t (*menu_func)(void);


#define EXIT_PROGRAM -3
#define EXIT_BACK -1

#define ITEM_LIST(cnt, list) cnt >= _countof(list) ? g_unknown : (char *)list[cnt]

uint8_t view_recv_key(uint32_t timeout_ms) ;


int view_input_decimal(const char* title, int* value, int min_val, int max_val);
int view_input_float(const char* title, float min, float max, float* value);
int32_t view_input_active(const char *title,int32_t *choice);
int32_t view_input_combobox(const char *title, const char *item_list[], int32_t item_count, int *choice);
int view_confirm_continue(const char *,int32_t* ok);

void make_comList(char *out, uint16_t outsize);



int print_menu(int width, const char* title, const char** menu_list, int cnt);

int32_t select_index_from_table(const char* list[], int32_t (*func)(), uint16_t listCnt, bool number,
                             int32_t* choice);



int check_pass(const char* title, char* password_str, int* ok);
int32_t shell_scanf_s(const char* fmt, ...);


int print_combobox(int width, const char* title, const char** menu_list, int cnt,int selected_index);

void console_printf(int row, int col, const char* format, ...);

menu_status_t console_input_combobox(const char *title, const char *item_list[], int32_t item_count, int *choice);
#endif