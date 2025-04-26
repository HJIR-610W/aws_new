
#include <stdio.h>
#include "dev_io.h"

const char *g_unknown = "unknown";

char recv_key(uint32_t timeout_ms)
{
  char key;
  char ch=0;

  while(1)
  {
    if(debug_recv(&ch, 1, timeout_ms))
    {
      if(ch==0x1B || ch==0x5B)
      {
        continue;
      }
      break;
    }
    break;
  }
return ch;

}




