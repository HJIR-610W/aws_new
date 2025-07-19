
#ifndef VT100_COMMAND_H
#define VT100_COMMAND_H

#include <stdint.h>

#include "terminal.h"
#include "util_escape_sequence.h"





void vt100_set_cursorPos(uint8_t line, uint8_t col);

void vt100_print_frame(uint8_t line,uint8_t colum,const char* text, char a, char b, char tb, size_t width,
color_t col);
void vt100_print_line(uint8_t line,uint8_t colum,char del, char l, size_t width);

void vt100_print_bar(uint32_t line,uint32_t col,int32_t width,const char * pFmt, ...);
void vt100_print(uint32_t line,uint32_t col,const char * pFmt, ...);

void vt100_printfColor(color_t color, char * pFmt, ...);

void vt100_print_frame_selected(uint8_t line, uint8_t colum, const char *text, char a, char b, char tb,
  size_t width, color_t col,uint8_t selected);
  
#endif

