
#include "console_login.h"

#include <stdlib.h>
#include <string.h>
#include "debug_io.h"
#include "cmsis_os.h"
#include "console_utile.h"
#include "crypto_key.h"
#include "task_logging.h"


bool check_login(const char *login_key)
{
  char buffer[10];
  uint8_t ch;
  uint8_t ret=0;
  uint8_t cnt = 0;
  int32_t password=0;
  uint8_t retry_count=0;


      
  while(1)
  {
      debug_printf("\r\n비밀번호를 입력해주세요\r\n");
    while(1)
    {
      if(debug_recv(&ch,1,0xFFFFFFFF)>0)
      {
        buffer[cnt++] = ch;
      
        if(cnt==1 && (ch==0x0D|| ch==0x0A)){
  debug_printf("\r\n비밀번호를 입력해주세요\r\n");
          cnt = 0;
          continue;;
        }
        else{
          if(ch==0x0D || ch == 0x0A)
          {
            buffer[cnt-1]=0;
            cnt = 0;
            break;
          }
          else{
            debug_put_ch('*');
          }
        }
    
        if(cnt==sizeof(buffer))
        {
          cnt = 0;
        }
      }
    }

    retry_count++;
    if(strncmp(login_key,buffer,strlen(login_key))==0 )
    {
      log_write(L_INFO,"Login successful");    
      log_write(L_ERROR,"Login successful");
      break;
    }
    if(retry_count>5)
    {
      debug_printf("비밀번호를 확인해주세요\r\n");
      return false;
    }
  }


  return true;
}