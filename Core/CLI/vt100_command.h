
#ifndef VT100_COMMAND_H
#define VT100_COMMAND_H

#include <stdint.h>

//https://www.csie.ntu.edu.tw/~r92094/c++/VT100.html
#define VT100_CLEAR_SCREEN   "\x1B[2J"
#define  VT100_CURSOR_HOME   "\x1B[f"
#define VT100_CURSOR_OFF     "\x1B[?25l"
#define VT100_CURSOR_ON      "\x1B[?25h"



 void vt100_set_cursorPos(int (*printf)(const char*, ...),uint8_t line, uint8_t col);
#endif
