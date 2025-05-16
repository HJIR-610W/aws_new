
#ifndef CLI_INPUT_H
#define CLI_INPUT_H

#include <stdint.h>
#define CLI_KEYCODE_CTRL_C -3
#define CLI_KEYCODE_CTRL_Q -17
#define CLI_KEYCODE_ESC -27

int uart_get_line_with_edit(char *buf, int maxlen);
int cli_scanf_s(const char *fmt, ...);

int get_confirm_input(void);
int32_t get_user_confirm(const char *message);
#endif