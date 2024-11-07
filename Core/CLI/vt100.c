

#include "vt100_command.h"

 void vt100_set_cursorPos(int (*pprintf)(const char*, ...),uint8_t line, uint8_t col)
{
     // ESC [ Pl ; Pc H
      pprintf("\x1B[%d;%dH",line,col);
}


