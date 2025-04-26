
#ifndef CLI_INPUT_H
#define CLI_INPUT_H

int uart_get_line_with_edit(char *buf, int maxlen);
int cli_scanf_s(const char *fmt, ...);
#endif