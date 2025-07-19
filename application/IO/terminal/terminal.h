
#ifndef TERMINAL_H

#define TERMINAL_H
#include <stdlib.h>

typedef enum {
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37
}color_t;




void terminal_set_color(color_t c);
void terminal_print_line(char del, char l, size_t width);
void terminal_print_centered(const char* text, char border, size_t width);
void terminal_reset_color(void);
void terminal_print_frame(const char* text, char a, char b, char tb, size_t width, color_t col);
void terminal_print_centered_selected(const char* text, char border, size_t width, color_t col);
#endif

