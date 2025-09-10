

#include <string.h>

#include "pcb_define.h"
#include "system_err.h"

#define PING_CMD_RESET 1
#define PING_CMD_EXCUTE_TELNET 2
void ping_callback(uint8_t *p_payload,uint16_t data_len)
{
  uint8_t *p_data = &p_payload[8];
  uint16_t id;

  memcpy(&id,&p_payload[4],2);

  switch(p_data[0])
  {
    case 255://시작코드 
      if(p_data[1]==PING_CMD_RESET)
        reset_system_delay(2);
      break;
    }
}