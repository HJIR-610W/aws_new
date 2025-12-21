/**
 * @file ntle9607.c
 * @brief NTLE9607 모뎀 드라이버 구현
 * @version 1.0.0
 * @date 2025-12-13
 */

#include "ntle9607.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include "cmsis_os2.h"
#include "util_memory.h"
#include "drv_rs232.h"

#include "cellular.h"
#include "dispatcher.h"
#include "system_err.h"
#include "drv_power.h"

#include "util_safe.h"



/* NTLE9607 전용 TCP 데이터 큐 */
static osMessageQueueId_t s_ntle9607_tcp_queue = NULL;
static osMessageQueueId_t s_ntle9607_dtmf_queue = NULL;

/// @brief at 명령어와 응답 목록
const at_comand_t commands_ntle9607[] = { {AT_URC_REBOOT, "^MODE:",eAT_ASYNC},
                                {AT_URC_TCP_DISCONNECTED, "*TCPDISCONNECTED",eAT_ASYNC},
                                {AT_URC_RECV_SMS, "+CMTI",eAT_URC_SMS_RECV},
                                {AT_URC_RECV_TCP, "*TCPRD",eAT_URC_TCP_RECV},
                                {AT_URC_RECV_RING, "+CLIP",eAT_URC_CALL_RECV},
                                {AT_URC_VOICE_END, "*VOICE END",eAT_ASYNC},
                                {AT_URC_RECV_DTMF, "+RXDTMF",eAT_ASYNC},
                                {AT_ASYNC_OFF_VOICE, "AT*VOICE*FLASH=0\r\n",eAT_ASYNC}};


const at_cmd_table_t ntle9607_command_table = {
    .list = commands_ntle9607,
    .count = sizeof(commands_ntle9607) / sizeof(commands_ntle9607[0])
};

#define RET_OK         0
#define RET_TIME_OUT  -1



/*
수신된 문자수신 명령어 에서 전화번호화 문자내용을 추출
msg 문자수신 명령어
sms 문자구조체

*SMS*MTREAD: 2025121310000000,"01012345678","HELLO"

*/
static void parse_sms(char* message, sms_t* p_sms)
{
  char* argument_vector[10] = { NULL };
  char* pointer;
  uint32_t count;
  uint32_t length;

  memset(p_sms, 0x00, sizeof(sms_t));

  count = parse_args(message, argument_vector, 10);

  if (count != 4)
  {
    return;
  }

  pointer = (char*)h_findnum((char*)argument_vector[2]);

  if (pointer)
  {
    strcpy_safe(p_sms->number, sizeof(p_sms->number), pointer);
  }

  pointer = argument_vector[3] + 1;

  if (pointer)
  {
    length = strlen(pointer) - 1;

    if (length < sizeof(p_sms->message))
    {
      length = Convert_HexAscii2uchar(pointer, length, (uint8_t*)p_sms->message);
      p_sms->message[length] = '\0';
    }
  }
}


int32_t ntle9607_init(cellular_if_t* p_if)
{
  const char* command_echo_off = "ATE0V1\r\n";
  const char* command_network_status = "AT*ST*REGSTS\r\n";
  int32_t return_code;

  return_code = 0;

  dispatcher_send_async((const uint8_t*)command_echo_off, strlen(command_echo_off));
  osDelay(1000);
  dispatcher_send_async((const uint8_t*)command_network_status, strlen(command_network_status));
  osDelay(1000);
  /*
  *ST*REGSTS:2,0,0
  2 서비스 가능
  */
  return return_code;
}

int32_t ntle9607_send_sms(cellular_if_t* p_if, char* number_str, char* message_str)
{
  char buffer[200];
  const char* ack_list[] = { "*SMSACK" };
  uint32_t matched_index_val = 0;
  uint8_t response_buffer[200];
  int32_t length;
  int32_t return_code;

  length = 0;
  return_code = 1;

  length = snprintf((char*)buffer, sizeof(buffer), "AT*SMS*MO=%s,\"\",", number_str);

  if ((sizeof(buffer) - length - 2) >= (strlen((char*)message_str) * 2))
  {
    length += Convert_ucharHexAscii((uint8_t*)message_str, strlen(message_str), &buffer[length]);

    buffer[length++] = '\r';
    buffer[length++] = '\n';

    {
      int response_length = dispatcher_send_sync((const uint8_t*)buffer, length, ack_list, 1, &matched_index_val,
                                        (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 200);
        if (response_length > 0) return_code = 0;
        else return_code = 1;
    }
  }

  return return_code;
}



int32_t ntle9607_read_sms(cellular_if_t* p_if, sms_t* p_sms)
{
  const char* command = "AT*SMS*MTREAD=0\r\n";
  const char* delete_command = "AT*SMS*ALLDEL=3\r\n";
  const char* ack_list[] = { "*SMS*MTREAD","+CMS ERROR" };
  uint8_t response_buffer[200];
  uint32_t matched_index_val = 0;
  int32_t return_code;

  return_code = 1;

  {
    /* Use dispatcher_send_sync to send command and wait for a response from the list */
    int response_length = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 2, &matched_index_val,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 1000);

    if (response_length > 0) {
      response_buffer[response_length] = '\0';
      if (matched_index_val == 0) { // Matched "*SMS*MTREAD"
        parse_sms((char*)response_buffer, p_sms);
        dispatcher_send_async((const uint8_t*)delete_command, strlen(delete_command)); // 읽은 메시지는 지운다
        osDelay(1000);
        return_code = 0;
      } else if (matched_index_val == 1) { // Matched "+CMS ERROR"
        return_code = 1;
      } else {
        // This case should not be hit if dispatcher works correctly
        return_code = 1;
      }
    } else {
      /* timeout or error */
      return_code = 1;
    }
  }

  return return_code;
}


int32_t ntle9607_read_num(cellular_if_t* p_if, char* buffer, size_t buffer_size)
{
  const char* command = "AT*SKT*DIAL\r\n";
  const char* ack_list[] = { "*SKT*DIAL:" };           //*SKT*DIAL:01227090440<CR><LF>
  char response_buffer[100];
  uint32_t matched_index_val = 0;
  int32_t length;
  int32_t return_code;

  length = 0;
  return_code = 1;
  buffer[0] = '\0';

  {
    int response_length = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 1, &matched_index_val,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 200);
    if (response_length > 0 && matched_index_val == 0) {
      response_buffer[response_length] = '\0';
      length = strlen(&response_buffer[10]);
      memcpy_safe((uint8_t*)buffer, buffer_size, (uint8_t*)&response_buffer[10], length);
      buffer[length] = '\0';
      return_code = 0;
    }
  }

  return return_code;
}

/*
2025-02-07 10:22:25.384 [COM11] - AT+CSQ<CR><LF>

2025-02-07 10:22:25.400 [COM10] - <CR><LF>
+CSQ: 30,99<CR><LF>
<CR><LF>
OK<CR><LF>
*/
int32_t ntle9607_read_rssi(cellular_if_t* p_if, int16_t* p_rssi_value)
{
  const char* command = "AT+CSQ\r\n";
  const char* ack_list[] = { "+CSQ" };
  char* argument_vector[10];
  char* end_pointer;
  uint8_t response_buffer[100];
  uint32_t matched_index_val = 0;
  int32_t return_code;

  return_code = 1;

  {
    int response_length = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 1, &matched_index_val,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 1000);
    if (response_length > 0 && matched_index_val == 0) {
      response_buffer[response_length] = '\0';
      parse_args((char*)response_buffer, argument_vector, 10);
      *p_rssi_value = (int16_t)strtol(argument_vector[1], &end_pointer, 10);
      return_code = 0;
    }
  }

  return return_code;
}

/*
2025-02-07 10:06:44.721 [COM11] - AT*NET*PPPOP<CR><LF>
2025-02-07 10:06:44.739 [COM10] - <CR><LF>
*PPPOPENED<CR><LF>
<CR><LF>
*NET*PPPOP:1<CR><LF>
<CR><LF>
OK<CR><LF>
<CR><LF>
*PPPOPENED<CR><LF>
*/
int32_t ntle9607_open_ppp(cellular_if_t* p_if)
{
  const char* command = "AT*NET*PPPOP\r\n";
  const char* ack_list[] = { "*NET*PPPOP:" };
  int32_t response_code_val;
  char response_buffer[100];
  uint32_t matched_index_val = 0;
  int32_t return_code;

  response_code_val = 0;
  return_code = 1;

  osDelay(500);
  {
    int response_length = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 1, &matched_index_val,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 10000);
    if (response_length > 0 && matched_index_val == 0) {
      response_buffer[response_length] = '\0';
      sscanf(response_buffer, "*NET*PPPOP:%d", &response_code_val);
      switch (response_code_val) {
      case 1:
        return_code = 0;
        break;
      default:
        return_code = 1;
        break;
      }
    }
  }

  return return_code;
}


/*
2025-02-07 10:05:34.282 [COM11] - AT*NET*PPPCL<CR><LF>
2025-02-07 10:05:34.284 [COM10] - <CR><LF>
*PPPCLOSED<CR><LF>
<CR><LF>
*NET*PPPCL:1<CR><LF>
<CR><LF>
OK<CR><LF>
*/
int32_t ntle9607_close_ppp(cellular_if_t* p_if)
{
  const char* command = "AT*NET*PPPCL\r\n";
  const char* ack_list[] = { "*NET*PPPCL:" };
  int32_t response_code_val;
  char response_buffer[100];
  uint32_t matched_index_val = 0;
  int32_t return_code;

  response_code_val = 0;
  return_code = 1;

  osDelay(500);
  {
    int response_length = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 1, &matched_index_val,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 10000);
    if (response_length > 0 && matched_index_val == 0) {
      response_buffer[response_length] = '\0';
      sscanf(response_buffer, "*NET*PPPCL:%d", &response_code_val);
      switch (response_code_val) {
      case 1:
        return_code = 0;
        break;
      default:
        return_code = 1;
        break;
      }
    }
  }

  return return_code;
}

int32_t ntle9607_open_tcp(cellular_if_t* p_if)
{
  const char* command = "AT*ANET*SOCKOP\r\n";
  const char* ack_list[] = { "*TCPCONNECTED","*TCPCONNECTFAIL" };
  char response_buffer[100];
  uint32_t matched_index_val = 0;
  int32_t return_code;

  return_code = 1;


    int response_length = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 2, &matched_index_val,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 10000);
    if (response_length > 0) {
      response_buffer[response_length] = '\0';
      if (matched_index_val == 0) return_code = 0; // Matched "*TCPCONNECTED"
      else return_code = 1; // Matched "*TCPCONNECTFAIL" or other error
    }

  return return_code;
}

int32_t ntle9607_close_tcp(cellular_if_t* p_if)
{
  const char* command = "AT*ANET*SOCKCL\r\n";
  const char* ack_list[] = { "*TCPDISCONNECTED","*ANET*SOCKCL:0" };
  char response_buffer[100];
  uint32_t matched_index_val = 0;
  int32_t return_code;

  return_code = 1;

  osDelay(500);
  {
    int response_length = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 2, &matched_index_val,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 10000);
    if (response_length > 0) {
      response_buffer[response_length] = '\0';
      if (matched_index_val == 1) return_code = 0; // Matched "*ANET*SOCKCL:0"
    }
  }
  return return_code;
}

int32_t ntle9607_send_tcp(cellular_if_t* p_if, uint8_t* p_data_buffer, size_t data_length)
{
  const char* ack_list[] = { "*ANET*SOCKWR" };
  char command_buffer[512 + 64];
  char response_buffer[100];
  uint32_t matched_index_val = 0;
  int32_t command_length;
  int32_t return_code;
  uint8_t response_code;

  command_length = 0;
  return_code = 1;

  //AT*ANET*SOCKWR=10,1234567890\r\n
  command_length = snprintf((char*)command_buffer, sizeof(command_buffer), "AT*ANET*SOCKWR=%d,", data_length);

  memcpy(&command_buffer[command_length], p_data_buffer, data_length);
  command_length += data_length;

  command_buffer[command_length++] = '\r';
  command_buffer[command_length++] = '\n';

  {
    int response_len_val = dispatcher_send_sync((const uint8_t*)command_buffer, command_length, ack_list, 1, &matched_index_val,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 10000);
    if (response_len_val > 0 && matched_index_val == 0) {
      response_buffer[response_len_val] = '\0';
      response_code = response_buffer[13];
      switch (response_code) {
      case '0': return_code = -1; break;
      case '1': return_code = (int32_t)data_length; break;
      default: return_code = -1; break;
      }
    }
  }

  return return_code;
}

int32_t ntle9607_tcp_recv_handler(cellular_if_t* p_if, uint8_t* p_data_buffer, size_t data_length,size_t buffer_size)
{
  int32_t tcp_length = 0;
  int32_t count_val;
  uint8_t temp_buffer[32];
  uint32_t total_count;

  count_val = data_length - 7;  //*TCPRD=4<CR><LF>에서 숫자의 자리수

  if (count_val < sizeof(temp_buffer))
  {
    memset(temp_buffer, 0x00, sizeof(temp_buffer));
    memcpy(temp_buffer, &p_data_buffer[7], count_val);
    total_count = atoi((char*)temp_buffer);  // 수신 처리해야할 tcp data 길이를 계산
    if (total_count <= buffer_size)
    {
      p_if->uart_io.recv(p_if->uart_handle, (uint8_t*)temp_buffer, 1, 1000);  // '\r' 제거
      tcp_length = p_if->uart_io.recv(p_if->uart_handle, (uint8_t*)p_data_buffer, total_count, 1000);  // 최종 tcp data 버퍼에서 가져옴
    }
  }

  if(tcp_length > 0)
  {
    tcp_data_t tcp_data;
    tcp_data.len = tcp_length;
    memcpy(tcp_data.data, p_data_buffer, tcp_length);
    osMessageQueuePut(s_ntle9607_tcp_queue, &tcp_data, 0, 0);
  }
  return tcp_length;


}

int32_t ntle9607_recv_tcp(cellular_if_t* p_if, uint8_t* buffer, size_t size, uint32_t timeout_ms)
{
  tcp_data_t tcp_data;
  osStatus_t ret;
  size_t copy_len;


  ret = osMessageQueueGet(s_ntle9607_tcp_queue, &tcp_data, NULL, timeout_ms);

  if (ret == osOK) {
    if (tcp_data.len == 0) {

      return 0;
    }
    copy_len = (tcp_data.len < size) ? tcp_data.len : size;
    memcpy(buffer, tcp_data.data, copy_len);

    return copy_len;
  }
  else if (ret == osErrorTimeout) {
    return -1;
  }
  else {
    return -2;
  }
}

int32_t ntle9607_recv_call(cellular_if_t* p_if)
{
  const char* command = "AT*VOICE*ANS\r\n";
  const char* ack_list[] = { "*VOICE CONNECT" };
  char response_buffer[100];
  uint32_t matched_index = 0;
  uint32_t loop_index;
  int32_t return_code;

  return_code = 1;

  for (loop_index = 0; loop_index < 2; loop_index++)
  {
    int response_length = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 200);

    if (response_length > 0 && matched_index == 0)
    {
      return_code = 0;
      break;
    }
    osDelay(1000);
  }

  return return_code;
}

int32_t ntle9607_dial(cellular_if_t* p_if, char* p_number_str, uint32_t timeout_ms_val)
{
  const char* ack_list_array[] = { "+COLP", "OS_DIAL_OFF" };
  char command_buffer[50];
  char response_buffer[100];
  uint32_t matched_index = 0;
  int32_t return_code;

  return_code = 1;

  snprintf(command_buffer, sizeof(command_buffer), "AT*VOICE*ORI=%s\r\n", p_number_str);
  {
    int response_length = dispatcher_send_sync((const uint8_t*)command_buffer, strlen(command_buffer), ack_list_array, 2, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, timeout_ms_val);
    if (response_length > 0) {
      response_buffer[response_length] = '\0';
      if (matched_index == 0) return_code = 0; // Matched "+COLP"
      else if (matched_index == 1) return_code = 1; // Matched "OS_DIAL_OFF"
    }
  }

  return return_code;
}

int32_t ntle9607_vpn_init(cellular_if_t* p_if)
{
  const char* connect_command = "AT*VPN*CONNECT=1\r\n";
  const char* status_command = "AT*VPN*STATUS\r\n";
  const char* disconnect_command = "AT*VPN*CONNECT=0\r\n";
  const char* ack_list[] = { "*VPN*STATUS: Connected" };
  char response_buffer[100];
  uint32_t matched_index = 0;
  uint32_t loop_index_i;
  uint32_t loop_index_j;
  int32_t return_code;

  return_code = 1;
  /*
  전원이 투입되면 vpn 자동연결이 시도되는듯한, 모뎀 전원 리셋후 vpn 상태읽기 하면
  connected 가 되는 경우 존재
  어떤경우에는 아무리 상태확인해도 연결이 안됨,이상태에서 껐다켜고 상태만 확인하면 연결이 되어있음
  그래서 일단 부팅되면 연결이 되었는지 확인하고 안되어있으면 연결 명령어를 시도
  */

  for (loop_index_j = 0; loop_index_j < 5; loop_index_j++)//약 15초 동안 vpn 로그인 상태 확인
  {
    /* vpn 연결 되었는지 확인 */
    {
      int response_length = dispatcher_send_sync((const uint8_t*)status_command, strlen(status_command), ack_list, 1, &matched_index,
                                      (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 200);
      if (response_length > 0 && matched_index == 0) {
        goto LOOP_EXIT;
      }
    }
  }

  dispatcher_send_async((const uint8_t*)disconnect_command, strlen(disconnect_command));
  osDelay(2000);
  dispatcher_send_async((const uint8_t*)connect_command, strlen(connect_command));
  osDelay(2000);

  for (loop_index_j = 0; loop_index_j < 2; loop_index_j++)
  {
    for (loop_index_i = 0; loop_index_i < 5; loop_index_i++)
    {
      /*vpn 연결 되었는지 확인*/
      {
        int response_length = dispatcher_send_sync((const uint8_t*)status_command, strlen(status_command), ack_list, 1, &matched_index,
                                        (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 200);
        if (response_length > 0 && matched_index == 0) {
          goto LOOP_EXIT;
        }
      }

      osDelay(1000);
    }

    if (loop_index_j == 0)// j==0일때 연결이 안되면 다시 연결종료 명령어 전송하고 다시 연결시도
    {
      dispatcher_send_async((const uint8_t*)disconnect_command, strlen(disconnect_command));
      osDelay(5000);
      dispatcher_send_async((const uint8_t*)connect_command, strlen(connect_command));
    }
  }

LOOP_EXIT:
  return return_code;
}

int32_t ntle9607_set_vpn_config(cellular_if_t* p_if, char* p_id_str, char* p_password_str, uint8_t p_ip_address[4], uint16_t port_num)
{
  char command_buffer[100];
  const char* ack_list[] = { "OK" };
  char response_buffer[100];
  uint32_t matched_index = 0;
  int32_t return_code;

  return_code = 1;

  snprintf(command_buffer, sizeof(command_buffer), "AT*VPN*CONFIG=%s,%s,%d.%d.%d.%d,%d\r\n", p_id_str, p_password_str, p_ip_address[0], p_ip_address[1], p_ip_address[2], p_ip_address[3], port_num);
  {
    int response_length = dispatcher_send_sync((const uint8_t*)command_buffer, strlen(command_buffer), ack_list, 1, &matched_index,
                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 1000);
    if (response_length > 0 && matched_index == 0) return_code = 0;
  }
  return return_code;
}

int32_t ntle9607_read_vpn_config(cellular_if_t* p_if, char* p_buffer, size_t buffer_size)
{
  char command_buffer[100];
  const char* ack_list[] = { "*VPN*CONFIG" };
  char response_buffer[100];
  uint32_t matched_index = 0;
  int32_t return_code;

  return_code = 1;

  snprintf(command_buffer, sizeof(command_buffer), "AT*VPN*CONFIG?\r\n");
  {
    int response_length = dispatcher_send_sync((const uint8_t*)command_buffer, strlen(command_buffer), ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 1000);
    if (response_length > 0 && matched_index == 0) {
      response_buffer[response_length] = '\0';
      strcpy_safe(p_buffer, buffer_size, response_buffer);
      return_code = 0;
    }
  }
  return return_code;
}

int32_t ntle9607_at_direct(cellular_if_t* p_if, const char* p_at_command_str, char* p_response_buffer_out, size_t buffer_size_out)
{
  char response_buffer_local[512];
  uint32_t matched_index = 0;
  int32_t return_code;

  return_code = 1;

  {
    // Pass NULL for p_ack_list and 0 for ack_list_count to match any response
    int response_length = dispatcher_send_sync((const uint8_t*)p_at_command_str, strlen(p_at_command_str), NULL, 0, &matched_index,
                                    (uint8_t*)response_buffer_local, sizeof(response_buffer_local) - 1, 1000);
    if (response_length > 0) {
      response_buffer_local[response_length] = '\0';
      strcpy_safe(p_response_buffer_out, buffer_size_out, response_buffer_local);
      return_code = 0;
    }
  }

  return return_code;
}

const char* service_code1_list[] = { "0 No Service",
                            "1 : Limited Service",
                            "2 : Service Available",
                            "3 : Limited Regional Service",
                            "4 : MS is in Power Save or Deep Sleep",
                            "5 : No Service",
                            "6 : Limited Service",
                            "7 : Limited Regional Service",
                            "8 : Power Save" };

const char* service_code2_list[] = { "0 : Error None",
                            "1 : Related USIM",
                            "2 : Cell Restrict",
                            "3 : Access Control",
                            "4 : Out Of Service",
                            "5 : Attach Reject (NW->UE)",
                            "6 : Location Update Reject",
                            "7 : Detach Request",
                            "8 : Active Reject (NW->MS)",
                            "9 : Deactivate Request (NW->MS)",
                            "10 : Tracking Area Update Reject (NW->MS)" };



int32_t ntle9607_check_network_service(cellular_if_t* p_if, char* p_buffer, size_t buffer_size)
{
  const char* command = "AT*ST*REGSTS\r\n";
  const char* ack_list[] = { "*ST*REGSTS:" };

  int32_t code_one, code_two, code_three;
  char response_buffer[100];
  uint32_t matched_index = 0;
  int32_t return_code;

  code_one = 0;
  code_two = 0;
  code_three = 0;
  return_code = 1;

  {
    int response_length = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 200);
    if (response_length > 0 && matched_index == 0) {
      response_buffer[response_length] = '\0';
      sscanf(response_buffer, "*ST*REGSTS:%d,%d,%d", &code_one, &code_two, &code_three);
      snprintf(p_buffer, buffer_size, "%s,%s,%d", service_code1_list[code_one], service_code2_list[code_two], code_three);
      return_code = 0;
    }
  }
  return return_code;
}

int32_t ntle9607_read_ring_number(cellular_if_t* p_if, const char* p_data_buffer, char* p_buffer_out, size_t buffer_size_out)
{
  char* argument_vector[10];
  char* pointer;
  int32_t return_code;

  pointer = NULL;
  return_code = 1;

  parse_args((char*)p_data_buffer, argument_vector, 10);

  pointer = (char*)h_findnum((char*)argument_vector[1]);

  if (pointer)
  {
    strcpy_safe(p_buffer_out, buffer_size_out, pointer);
    return_code = 0;
  }

  return return_code;
}

/**
 * @brief dtmf 코드 추출
 * @retval dtmf 코드
 */
char ntle9607_get_dtmf(cellular_if_t* p_if, uint8_t* p_data_buffer)
{
  char dtmf_code_val;

  // "+RXDTMF: 1"에서 dtmf 코드만 추출
  dtmf_code_val = p_data_buffer[9];

  return dtmf_code_val;
}

void ntle9607_write_ip(cellular_if_t* p_if, uint8_t p_ip_address[4], uint16_t port_num)
{
  const char* ack_list[] = { "*ANET*SOCKPA:" };
  char command_buffer[50];
  int32_t response_code_val;
  char response_buffer[100];
  uint32_t matched_index = 0;


  response_code_val = 0;


  snprintf(command_buffer, sizeof(command_buffer), "AT*ANET*SOCKPA=%d.%d.%d.%d,%d\r\n", p_ip_address[0], p_ip_address[1], p_ip_address[2], p_ip_address[3], port_num);

  osDelay(500);
  {
    int response_length = dispatcher_send_sync((const uint8_t*)command_buffer, strlen(command_buffer), ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 1000);
    if (response_length > 0 && matched_index == 0) {
      response_buffer[response_length] = '\0';
      sscanf(response_buffer, "*ANET*SOCKPA:%d", &response_code_val);

    }
  }
}

void ntle9607_reset_sw(cellular_if_t* p_if)
{
  const char* command = "AT*SET*RESET\r\n";

  dispatcher_send_async((const uint8_t*)command, strlen(command));
}

void ntle9607_reset_hw(cellular_if_t* p_if)
{

  drv_power_off(DRV_POWER_CDMA);
   osDelay(2000);
   drv_power_on(DRV_POWER_CDMA);
}

/**
 * @brief 안정적인 전원 차단을 위해서 아래와 같이 AT CMD 실행이 필요 합니다.
 */
void ntle9607_off_power_safe(cellular_if_t* p_if)
{
  const char* command = "AT+APMODE=0\r\n";

  dispatcher_send_async((const uint8_t*)command, strlen(command));

  osDelay(11000);
}

int32_t ntle9607_recv_uart(cellular_if_t* p_if, uint8_t* buffer, size_t size, uint32_t timeout_ms)
{
  return p_if->uart_io.recv_crlf(p_if->uart_handle, buffer, size, timeout_ms);
}
/**
 * @brief NTLE9607 드라이버 초기화 및 구조체 설정
 * @param p_if cellular_if_t 구조체 포인터
 * @param p_uart_config_val UART 설정 구조체
 * @return 성공 시 0, 실패 시 -1
 */
int32_t ntle9607_open(cellular_if_t *p_if)
{


  if (p_if == NULL ) {
    return -1;
  }

  // 모뎀 이름 설정
  strcpy_safe(p_if->modem_name, sizeof(p_if->modem_name), "NTLE9607");


  // AT 명령어 테이블 설정
  p_if->at_cmd_table = (at_cmd_table_t*)&ntle9607_command_table;

  // API 함수 포인터 설정
  p_if->api.init = ntle9607_init;
  p_if->api.reset_sw = ntle9607_reset_sw;
  p_if->api.reset_hw = ntle9607_reset_hw;
  p_if->api.send_sms = ntle9607_send_sms;
  p_if->api.read_sms = ntle9607_read_sms;
  p_if->api.read_num = ntle9607_read_num;
  p_if->api.read_rssi = ntle9607_read_rssi;
  p_if->api.open_tcp = ntle9607_open_tcp;
  p_if->api.close_tcp = ntle9607_close_tcp;
  p_if->api.open_ppp = ntle9607_open_ppp;
  p_if->api.close_ppp = ntle9607_close_ppp;
  p_if->api.recv_call = ntle9607_recv_call;
  p_if->api.dial = ntle9607_dial;
  p_if->api.vpn_init = ntle9607_vpn_init;
  p_if->api.set_vpn_config = ntle9607_set_vpn_config;
  p_if->api.read_vpn_config = ntle9607_read_vpn_config;
  p_if->api.at_direct = ntle9607_at_direct;
  p_if->api.check_network_service = ntle9607_check_network_service;
  p_if->api.read_ring_number = ntle9607_read_ring_number;
  p_if->api.get_dtmf = ntle9607_get_dtmf;
  p_if->api.write_ip = ntle9607_write_ip;
  p_if->api.off_power_safe = ntle9607_off_power_safe;
  p_if->api.tcp_recv_handler = ntle9607_tcp_recv_handler;
  p_if->api.recv_tcp = ntle9607_recv_tcp;
  p_if->api.send_tcp = ntle9607_send_tcp;
  p_if->api.recv_uart = ntle9607_recv_uart;

  // 메시지 큐 생성
  s_ntle9607_tcp_queue = osMessageQueueNew(1, sizeof(tcp_data_t), NULL);
  if (s_ntle9607_tcp_queue == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"NTLE9607: TCP queue 생성 실패");
    return -1;
  }

  s_ntle9607_dtmf_queue = osMessageQueueNew(10,1, NULL);
  if (s_ntle9607_dtmf_queue == NULL) {
     DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"NTLE9607: DTMF");
    return -1;
  }



   DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"NTLE9607: TCP queue 생성 완료");

  return RET_OK;
}

/**
 * @brief NTLE9607 드라이버 종료 및 리소스 정리
 * @param p_if cellular_if_t 구조체 포인터
 */
void ntle9607_close(cellular_if_t *p_if)
{
  if (p_if == NULL) {
    return;
  }

  // 메시지 큐 삭제 (TCP only)
  if (s_ntle9607_tcp_queue != NULL) {
    osMessageQueueDelete(s_ntle9607_tcp_queue);
    s_ntle9607_tcp_queue = NULL;
  }

   DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"NTLE9607: TCP 메시지 큐 삭제 완료");

  // UART 핸들 정리 (필요 시uart_close 구현)
  p_if->uart_handle = 0;
}


