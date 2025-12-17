
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
  char pw_buffer[50]={"│         ____          │"};
  uint8_t ch;
  uint8_t ret=0;
  uint8_t cnt = 0;
  int32_t password=0;
  uint8_t retry_count=0;
  uint32_t start_pos;

  start_pos = (uint32_t)(strstr(pw_buffer,"____") - pw_buffer);;

      
  while(1)
  {
    debug_printf(ES_CURSOR_HOME);
    debug_printf("┌───────────────────────┐\r\n");
    debug_printf("│       비밀번호        │\r\n");
    debug_printf("├───────────────────────┤\r\n");
    debug_printf("%s\r\n",pw_buffer);
    debug_printf("└───────────────────────┘\r\n");

    while(1)
    {
      if(debug_recv(&ch,1,0xFFFFFFFF)>0)
      {
        buffer[cnt++] = ch;
      
        if(cnt==1 && (ch==0x0D|| ch==0x0A)){
          cnt = 0;
          continue;;
        }else{
          if(cnt<=4)
          {
            if(ch==0x0D || ch == 0x0A){
              buffer[cnt-1]=0;
              cnt = 0;
              break;
            }else{
              pw_buffer[start_pos+cnt-1]='*';
            }
          }
      }
    
        if(cnt==sizeof(buffer))
        {
          buffer[cnt-1]=0;
          cnt = 0;
          break;
        }
      }
    }

    retry_count++;
    if(strncmp(login_key,buffer,strlen(login_key))==0 )
    {
      log_printf(L_INFO,"Login successful");    
      log_printf(L_ERROR,"Login successful");
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