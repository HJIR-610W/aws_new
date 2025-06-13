

#define __STDC_WANT_LIB_EXT1__ 1
#include "terminal.h"


#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>



extern int32_t io_printf(const char * pFmt, ...);




void terminal_set_color(color_t c)
{
  io_printf("%c[%dm", 27, c);
}


void terminal_print_line(char del, char l, size_t width)
{
    char line[200];
    (void)memset_s(line,sizeof(line), l, width);

    io_printf("%c%.*s%c\r\n", del, width, line, del);
}

void terminal_print_centered(const char* text, char border, size_t width)
{
    // Text size
    size_t b = strlen(text);
    // Left empty space
    size_t a = (width - b) / 2;
    // Right empty space
    size_t c = width - a - b;

    // [b]<empty>[text]<empty>[b]
    io_printf("%c%*.s%s%*.s%c\r\n", border, a, "", text, c, "", border);
}

void terminal_print_centered_selected(const char* text, char border, size_t width, color_t col)
{
  // Text size
  size_t b = strlen(text);
  // Left empty space
  size_t a = (width - b) / 2;
  // Right empty space
  size_t c = width - a - b;

  // [b]<empty>[text]<empty>[b]
  if(col)//colÀÌ ¹àÀº ÆÄ¶û
  {
    io_printf("%c\x1b[94m%*.s%s%*.s\x1b[0m%c\r\n", border, a, "", text, c, "", border);

  }
  else//¹ÝÀü
  {
    io_printf("%c\x1b[7m%*.s%s%*.s\x1b[0m%c\r\n", border, a, "", text, c, "", border);
  }
}

void terminal_reset_color(void)
{
    io_printf("%c[%dm", 27,37);
}



void terminal_print_frame(const char* text, char a, char b, char tb, size_t width,
    color_t col)
{
    // Set the chosen color
    terminal_set_color(col);

    // Print framed text [line; text; line]
    terminal_print_line(a, tb, width);
    terminal_print_centered(text, b, width);
    terminal_print_line(a, tb, width);

    // Reset color
    terminal_reset_color();
}
