
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




#endif

