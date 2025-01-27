
#ifndef VT100_COMMAND_H
#define VT100_COMMAND_H

#include <stdint.h>
#include "terminal.h"
//https://www.csie.ntu.edu.tw/~r92094/c++/VT100.html
#define VT100_CLEAR_SCREEN   "\x1B[2J"
#define  VT100_CURSOR_HOME   "\x1B[f"
#define VT100_CURSOR_OFF     "\x1B[?25l"
#define VT100_CURSOR_ON      "\x1B[?25h"
#define VT100_ERASE_ENTIRE_LINE "\x1B[2K"


void vt100_set_cursorPos(uint8_t line, uint8_t col);

void vt100_print_frame(uint8_t line,uint8_t colum,const char* text, char a, char b, char tb, size_t width,
color_t col);
void vt100_print_line(uint8_t line,uint8_t colum,char del, char l, size_t width);

void vt100_print_bar(uint32_t line,uint32_t col,int32_t width,const char * pFmt, ...);
void vt100_print(uint32_t line,uint32_t col,const char * pFmt, ...);

void vt100_printfColor(color_t color, char * pFmt, ...);
#endif
