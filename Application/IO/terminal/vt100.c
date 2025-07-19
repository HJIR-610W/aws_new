#define __STDC_WANT_LIB_EXT1__ 1

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "vt100_command.h"
#include "dev_io.h"
#include "terminal.h"

static int get_visual_width(const char* str)
{
  int width = 0;
  int i = 0;
  
  while (str[i] != '\0') 
  {
    unsigned char c = (unsigned char)str[i];
    
    if (c < 0x80) 
    {
      width++;
      i++;
    }
    else if ((c & 0xE0) == 0xC0) 
    {
      width++;
      i += 2;
    }
    else if ((c & 0xF0) == 0xE0) 
    {
      width += 2;
      i += 3;
    }
    else if ((c & 0xF8) == 0xF0) 
    {
      width += 2;
      i += 4;
    }
    else 
    {
      width++;
      i++;
    }
  }
  
  return width;
}

 void vt100_set_cursorPos(uint8_t line, uint8_t col)
{
     // ESC [ Pl ; Pc H
      io_printf("\x1B[%d;%dH",line,col);
}


void vt100_print_bar(uint32_t line,uint32_t col,int32_t width,const char * pFmt, ...)
{
    char buff[150];
    va_list ap;  
    int32_t len;
    int32_t actual_width;
    int visual_width;
    int padding_needed;
    int i;

    io_printf("\x1B[%d;%dH",line,col);

    va_start(ap, pFmt);
    vsnprintf_s((char *)buff, sizeof(buff), (char *)pFmt, ap);
    va_end(ap);
    len = strnlen_s((char *)buff,0xFFFF);
    
    for (i = 0; i < len; i++)
    {
        if (buff[i] == '\r' || buff[i] == '\n')
        {
            buff[i] = '\0';
            break;
        }
    }

    actual_width = (width < 0) ? -width : width;
    visual_width = get_visual_width(buff);
    
    padding_needed = actual_width - visual_width - 2;
    
    if (padding_needed < 0) {
        padding_needed = 0;
    }
    
    io_printf("|%s", buff);
    
    for (i = 0; i < padding_needed; i++) {
        io_printf(" ");
    }
    
    io_printf("|");

    return ; 
}


void vt100_print(uint32_t line,uint32_t col,const char * pFmt, ...)
{
  char buff[150];
  va_list ap;  

  io_printf("\x1B[%d;%dH",line,col);

  va_start(ap, pFmt);
  vsnprintf_s((char *)buff, sizeof(buff), (char *)pFmt, ap);
  va_end(ap);
  strnlen_s((char *)buff,0xFFFF);
  io_printf("%s",  buff);
  return ; 
}

void vt100_printfColor(color_t color, char * pFmt, ...)
{
  char buff[150];
  va_list ap;  

  terminal_set_color(color);

  va_start(ap, pFmt);
  vsnprintf_s((char *)buff, sizeof(buff), (char *)pFmt, ap);
  va_end(ap);
  strnlen_s((char *)buff,0xFFFF);
  io_printf("%s",  buff);

  terminal_reset_color();
  return ; 
}


void vt100_print_frame(uint8_t line,uint8_t colum,const char* text, char a, char b, char tb, size_t width,
    color_t col)
{
    terminal_set_color(col);
    vt100_set_cursorPos(line++,colum);
    terminal_print_line(a, tb, width);
    vt100_set_cursorPos(line++,colum);
    terminal_print_centered(text, b, width);
    vt100_set_cursorPos(line,colum);
    terminal_print_line(a, tb, width);
    terminal_reset_color();
}

void vt100_print_frame_selected(uint8_t line, uint8_t colum, const char *text, char a, char b, char tb,
                       size_t width, color_t col,uint8_t selected)
{
    color_t title_col;
  terminal_set_color(col);
  vt100_set_cursorPos(line++, colum);
  terminal_print_line(a, tb, width);
  vt100_set_cursorPos(line++, colum);
  if(selected)
  {
    if(selected==1)
    {
      title_col = 0;
    }
    else
    {
      title_col = MAGENTA;
    }
    terminal_print_centered_selected(text, b, width, title_col);
  }
  else
  {
  terminal_print_centered(text, b, width);
  }
  vt100_set_cursorPos(line, colum);
  terminal_print_line(a, tb, width);
  terminal_reset_color();
}


void vt100_print_line(uint8_t line,uint8_t colum,char del, char l, size_t width)
{
    char linebuff[200];
    (void)memset_s(linebuff,sizeof(linebuff), l, width);
      
      vt100_set_cursorPos(line,colum);
    io_printf("%c%.*s%c\r\n", del, width, linebuff, del);
}

