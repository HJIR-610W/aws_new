#define __STDC_WANT_LIB_EXT1__ 1

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "vt100_command.h"
#include "dev_io.h"
#include "terminal.h"

 void vt100_set_cursorPos(uint8_t line, uint8_t col)
{
     // ESC [ Pl ; Pc H
      debug_printf("\x1B[%d;%dH",line,col);
}


void vt100_print_bar(uint32_t line,uint32_t col,int32_t width,const char * pFmt, ...)
{
    char buff[150];
    va_list ap;  
    int32_t len;
       char temp[3] = { 0,0,0 };
    int tempCnt = 0;

    debug_printf("\x1B[%d;%dH",line,col);

    va_start(ap, pFmt);
    vsnprintf_s((char *)buff, sizeof(buff), (char *)pFmt, ap);
    va_end(ap);
    len = strnlen_s((char *)buff,0xFFFF);
    
     for (int i = 0; i < len; i++)
    {
        if (buff[i] == '\r' || buff[i] == '\n')
        {
            temp[tempCnt++] = buff[i];
            buff[i] = 0;
            if (tempCnt == 2)
            {
                break;
            }
        }
    }

    if (temp[0])
    {
        debug_printf("|%*s|%s", width, buff,temp);
    }
    else
    {
        debug_printf("|%*s|", width, buff);
    }

    return ; 
}


void vt100_print(uint32_t line,uint32_t col,const char * pFmt, ...)
{
    char buff[150];
    va_list ap;  



    debug_printf("\x1B[%d;%dH",line,col);

    va_start(ap, pFmt);
    vsnprintf_s((char *)buff, sizeof(buff), (char *)pFmt, ap);
    va_end(ap);
     strnlen_s((char *)buff,0xFFFF);
    

       debug_printf("%s",  buff);
 

    return ; 
}
void vt100_print_frame(uint8_t line,uint8_t colum,const char* text, char a, char b, char tb, size_t width,
    color_t col)
{


    // Set the chosen color
    terminal_set_color(col);

    vt100_set_cursorPos(line++,colum);
    terminal_print_line(a, tb, width);
        vt100_set_cursorPos(line++,colum);
    terminal_print_centered(text, b, width);
        vt100_set_cursorPos(line,colum);
    terminal_print_line(a, tb, width);

    // Reset color
    terminal_reset_color();
}

void vt100_print_line(uint8_t line,uint8_t colum,char del, char l, size_t width)
{
    char linebuff[200];
    (void)memset_s(linebuff,sizeof(linebuff), l, width);
      
      vt100_set_cursorPos(line,colum);
    debug_printf("%c%.*s%c\r\n", del, width, linebuff, del);
}
