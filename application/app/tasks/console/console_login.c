
#include "console_login.h"

#include <stdlib.h>
#include <string.h>
#include "dev_io.h"

#include "console_utile.h"
#include "crypto_key.h"
#include "task_logging.h"
void check_login(void)
{
  char buffer[10];
  char key[20];
  char ch;
  uint8_t ret=0;
  uint8_t cnt = 0;
  int32_t password=0;


  
  read_password(key);
  while(1)
  {
  io_printf("\r\n비밀번호를 입력해주세요\r\n");

 while(1)
 {
  if(  io_recv(&ch,1,0xFFFFFFFF))
  {

      buffer[cnt++] = ch;
      
    if(cnt==1 && (ch==0x0D|| ch==0x0A))
    {
      cnt = 0;
      continue;;
    }
    else
    {
      
      if(ch==0x0D || ch == 0x0A)
      {
        buffer[cnt-1]=0;
        cnt = 0;
        break;
      }
      else
      {
            io_put_ch('*');
      }
     
    }
    
    if(cnt==sizeof(buffer))
    {
      cnt = 0;
    }
  }
 }


  if(strncmp(key,buffer,strlen(buffer))==0)
  {
    log_printf(L_INFO,"Login successful");    
    log_printf(L_ERROR,"Login successful");
    break;
  }
  }


  return ;
}