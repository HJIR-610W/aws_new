
/*
 장비 관리 목적으로 ping 패킷을 조작하여 사용
  
 */
#include <string.h>
#include <stdio.h>


#include "pcb_define.h"
#include "system_err.h"
#include "config_app.h"
#include "task_logging.h"

#define PING_CMD_RESET 1
#define PING_CMD_EXCUTE_TELNET 2
void ping_callback(uint8_t *p_payload,uint16_t data_len)
{
  uint8_t *p_data = &p_payload[8];
  uint16_t id;
  int cmd;
  int station_id;
  char buff[50];
  
  memcpy(&id,&p_payload[4],2);

  // CUSTOM:station_id,cmd,message
  if(sscanf((char *)p_data, "CUSTOM:%d,%d,%49s", &station_id, &cmd, buff)!=3)
  {
    return;
  }

  if(station_id == 65535 || get_config_app()->device_id== station_id)
  {
    
    switch(cmd)
    {
      case PING_CMD_RESET:
        log_printf(L_INFO, "ping reset");
        reset_system_delay(2);
        break;
    }
  }

}