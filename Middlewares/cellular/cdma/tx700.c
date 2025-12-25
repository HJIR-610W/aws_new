/**
 * @file tx700.c
 * @brief TX700 모뎀 드라이버 구현
 * @version 1.0.0
 * @date 2025-12-13
 */

#include "tx700.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include "cmsis_os2.h"
#include "util_memory.h"
#include "drv_rs232.h"
#include "drv_power.h"
#include "cellular.h"
#include "dispatcher.h"
#include "system_err.h"
#include "util_safe.h"


/* TX700 전용 TCP 데이터 큐 */
static osMessageQueueId_t s_tx700_tcp_queue = NULL;

/// @brief AT 명령어와 응답 목록
const at_comand_t commands_tx700[] = {
    {AT_URC_TCP_DISCONNECTED, "$$TELL: 605", eAT_ASYNC},  //$$TELL: 605, TCP : TCP 접속 종료
    {AT_URC_RECV_SMS, "+CMTI", eAT_ASYNC},            //+CMTI: "ME",0
    {AT_URC_RECV_RING, "+CLIP", eAT_ASYNC},           //+CLIP: "01053730725",128,"",0,,0
    {AT_URC_REBOOT, "$$TELL:34", eAT_ASYNC},              //$$TELL:34,Modem Boot Up
    {AT_URC_RECV_TCP, "$$BinRecv", eAT_ASYNC},            //$$BinRecv
    {AT_URC_VOICE_END, "$$TELL: 754, VOICE : NETWORK RELEASE", eAT_ASYNC},     //$$TELL: 754, VOICE : NETWORK RELEASE
    {AT_URC_RECV_DTMF, "$DTMF:", eAT_ASYNC}};

const at_cmd_table_t tx700_command_table = {
    .list = commands_tx700,
    .count = sizeof(commands_tx700) / sizeof(commands_tx700[0])
};

#define RET_OK         0
#define RET_TIME_OUT  -1



/**
 * @brief TCP 데이터 수신 (큐에서 가져오기)
 */
static uint32_t tx700_recv_tcp_data(tcp_data_t *p_tcp_data, uint32_t timeout_ms)
{
  uint32_t return_code = 1;

  if (osMessageQueueGet(s_tx700_tcp_queue, p_tcp_data, NULL, timeout_ms) == osOK) {
    return_code = 0;
  }
  return return_code;
}

/**
 * @brief TCP 데이터 송신 (큐에 넣기)
 */
static void tx700_send_tcp_data(const uint8_t* p_data_buffer, size_t length_val)
{
  tcp_data_t response_data;

  if (length_val < sizeof(response_data.data)) {
    memcpy((uint8_t*)response_data.data, (uint8_t*)p_data_buffer, length_val);
    response_data.data[length_val] = '\0';
    response_data.len = length_val;

    if (osMessageQueuePut(s_tx700_tcp_queue, &response_data, 0, 100) != osOK) {
      printf("TX700: TCP 큐에 메시지 넣기 실패\r\n");
    }
  }
}


#if 0 
/**
 * @brief SMS 메시지에서 전화번호와 내용 추출
 */
static int32_t extract_sms_data(char* p_sms_data, char* p_out_number, int number_buffer_size, char* p_out_message, int message_buffer_size)
{
  const char* first_comma_ptr;
  const char* last_quote_ptr;
  const char* message_start_ptr;
  const char* number_end_ptr;
  const char* number_start_ptr;
  size_t message_length;
  size_t message_copy_length;
  size_t number_length;
  size_t number_copy_length;

  if (p_sms_data == NULL || p_out_number == NULL || p_out_message == NULL || number_buffer_size <= 0 || message_buffer_size <= 0)
  {
    return -1;
  }

  *p_out_number = '\0';
  *p_out_message = '\0';

  first_comma_ptr = strchr(p_sms_data, ',');
  if (first_comma_ptr == NULL) return -1;

  number_start_ptr = strchr(first_comma_ptr, '"');
  if (number_start_ptr == NULL) return -1;
  number_start_ptr++;

  number_end_ptr = strchr(number_start_ptr, '"');
  if (number_end_ptr == NULL) return -1;

  number_length = number_end_ptr - number_start_ptr;
  number_copy_length = (number_length >= number_buffer_size) ? (number_buffer_size - 1) : number_length;

  strncpy(p_out_number, number_start_ptr, number_copy_length);
  p_out_number[number_copy_length] = '\0';

  last_quote_ptr = strrchr(p_sms_data, '"');
  if (last_quote_ptr == NULL) return -1;

  message_start_ptr = strchr(last_quote_ptr, ',');
  if (message_start_ptr == NULL) return -1;
  message_start_ptr++;

  message_length = strlen(message_start_ptr);
  message_copy_length = (message_length >= message_buffer_size) ? (message_buffer_size - 1) : message_length;

  strncpy(p_out_message, message_start_ptr, message_copy_length);
  p_out_message[message_copy_length] = '\0';

  return 0; // 성공
}
#endif

/**
 * @brief 모뎀 초기화
 */
int32_t tx700_init(cellular_if_t* p_if)
{
  const char* command_echo_off = "ATE0V1\r\n";
  const char* command_null_permission = "AT$$TCP_NULLPERMISSION=1\r\n";
  int32_t return_code;

  return_code = 0;

  dispatcher_send_async((const uint8_t*)command_echo_off, strlen(command_echo_off));
  osDelay(100);
  dispatcher_send_async((const uint8_t*)command_null_permission, strlen(command_null_permission));
  osDelay(100);

  return return_code;
}

/**
 * @brief SMS 전송

소켓이 끊기거나 이럴때 문자 전송하면 실패함 (모뎀 문제) 모뎀이 명령어를 무시함

 */
int32_t tx700_send_sms(cellular_if_t* p_if, char* p_number_str, char* p_message_str)
{
  char command_buffer[200];
  const char* ack_sms_list[] = { "\x3E\x20" };
  const char* ack_list[] = { "$$TELL:45" };
  uint32_t matched_index = 0;
  char response_buffer[20];
  int32_t length;




  length = snprintf((char*)command_buffer, sizeof(command_buffer)-2, "AT+CMGS=\"%s\"\r\n", p_number_str);
  
  length = dispatcher_send_sync((const uint8_t*)command_buffer, length, ack_sms_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 5000);

  if(length>0)
  {
    if(matched_index ==0)
    {
        length = snprintf((char*)command_buffer, sizeof(command_buffer)-2, "%s\x1A", p_message_str);
          length = dispatcher_send_sync((const uint8_t*)command_buffer, length, ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 5000);

        if (length > 0)
        {
          return 0;
        }
        else
        {
          return 1;
        }
    }
  }

  
  return 0;


}


/*
+CMGR: "REC READ","01053730725",,"25/12/14,08:44:05+36",123456<CR>

123456이 문자열임, 번호화 메시지 추출하여
p_out_number,p_out_message에 각각 복사
*/
/*
 * SMS 데이터에서 전화번호와 메시지를 추출하는 함수
 *
 * @param p_sms_data: +CMGR 응답 문자열
 * @param p_out_number: 전화번호를 저장할 버퍼
 * @param number_buffer_size: 전화번호 버퍼 크기
 * @param p_out_message: 메시지를 저장할 버퍼
 * @param message_buffer_size: 메시지 버퍼 크기
 * @return: 성공 시 0, 실패 시 -1
 */
int32_t extract_sms_data(char* p_sms_data, char* p_out_number, uint32_t number_buffer_size,
  char* p_out_message, int message_buffer_size)
{
  char number_temp[64] = { 0 };
  char message_temp[512] = { 0 };
  int result = 0;

  /* 입력 파라미터 검증 */
  if (p_sms_data == NULL || p_out_number == NULL || p_out_message == NULL) {
    return -1;
  }

  /* sscanf로 전화번호와 메시지 추출 */
  /* 포맷: +CMGR: "상태","전화번호",,"날짜,시간",메시지 */
  result = sscanf(p_sms_data, "+CMGR: \"%*[^\"]\",\"%63[^\"]\",,\"%*[^\"]\",%511[^\r]",
    number_temp, message_temp);

  if (result != 2) {
    return -1;  /* 파싱 실패 */
  }

  /* 버퍼 크기 검증 후 복사 */
  if (strlen(number_temp) >= number_buffer_size) {
    return -1;  /* 전화번호 버퍼 크기 초과 */
  }

  if (strlen(message_temp) >= message_buffer_size) {
    return -1;  /* 메시지 버퍼 크기 초과 */
  }

  strcpy(p_out_number, number_temp);
  strcpy(p_out_message, message_temp);

  return 0;  /* 성공 */
}
/**
 * @brief SMS 읽기
+CMGR: "REC READ","01053730725",,"25/12/14,08:44:05+36"<CR><LF>
123456<CR><LF>
 */
int32_t tx700_read_sms(cellular_if_t* p_if, sms_t* p_sms)
{
  const char* command = "AT+CMGR=0\r\n";
  const char* delete_command = "AT+CMGD=,4\r\n";
  const char* ack_list[] = { "+CMGR" };
  char response_buffer[310]; // 충분한 크기로 확보
  uint32_t matched_index = 0;
  int32_t results=1;
  int32_t len;





  len = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 200);

  if (len > 0) // 성공 시 반환값은 길이
  {
    results = extract_sms_data(response_buffer, p_sms->number, sizeof(p_sms->number), p_sms->message, sizeof(p_sms->message));

    if (results != 0)
    {
      results = 1;
    }
    else
    {
      dispatcher_send_async((const uint8_t*)delete_command, strlen(delete_command)); // SMS 추출 성공시에만 삭제
      osDelay(2000);
      results = 0;
    }
  }

  return results;
}

/**
 * @brief 전화번호 읽기
 */
int32_t tx700_read_num(cellular_if_t* p_if, char* p_buffer, size_t buffer_size)
{
  const char* command = "AT+CNUM\r\n";
  const char* ack_list[] = { "+CNUM:" };
  char response_buffer[256];
  uint32_t matched_index = 0;
  char number_str[20]; // sscanf용 임시 버퍼
  int32_t return_code;

  return_code = 1;
  p_buffer[0] = '\0';



  return_code = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 200);

  if (return_code > 0 && matched_index == 0) // 성공 시 반환값은 길이, 매치 인덱스 0
  {
    //+CNUM: ,"01220891572",129
    if (sscanf(response_buffer, "+CNUM: ,\"%19[^\"]", number_str) == 1)
    {
      strcpy_safe(p_buffer, buffer_size, number_str);
      return_code = 0;
    }
  }

  return return_code;
}

/**
 * @brief RSSI (신호 강도) 읽기
 */
int32_t tx700_read_rssi(cellular_if_t* p_if, int16_t* p_rssi_value)
{
  const char* command = "AT+CSQ\r\n";
  const char* ack_list[] = { "+CSQ" };
  int32_t data_val;
  char response_buffer[256];
  uint32_t matched_index = 0;
  int32_t return_code;

  data_val = 0;
  return_code = 1;


  return_code = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 200);

  if (return_code > 0 && matched_index == 0) // 성공 시 반환값은 길이, 매치 인덱스 0
  {
    if (sscanf(response_buffer, "+CSQ:%d", &data_val) == 1)
    {
      *p_rssi_value = data_val;
      return_code = 0;
    }
  }

  return return_code;
}

/**
 * @brief PPP 연결 열기
 */
int32_t tx700_open_ppp(cellular_if_t* p_if)
{
  const char* command = "AT$$TCP_PPPOP\r\n";
  const char* ack_list[] = { "$$TELL: 600" };
  char response_buffer[256];
  uint32_t matched_index = 0;
  int32_t return_code;

  return_code = 1;



  return_code = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 1000);

  if (return_code > 0 && matched_index == 0) // 성공 시 반환값은 길이, 매치 인덱스 0
  {
    return 0;
  }

  return return_code;
}

/**
 * @brief PPP 연결 닫기
 */
int32_t tx700_close_ppp(cellular_if_t* p_if)
{
  const char* command = "AT$$TCP_PPPCL\r\n";
  const char* ack_list[] = { "$$TELL: 601" };
  char response_buffer[256];
  uint32_t matched_index = 0;
  int32_t return_code;

  return_code = 1;



  return_code = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 1000);

  if (return_code > 0 && matched_index == 0) // 성공 시 반환값은 길이, 매치 인덱스 0
  {
    return 0;
  }

  return return_code;
}

/**
 * @brief TCP 소켓 열기
 */
int32_t tx700_open_tcp(cellular_if_t* p_if)
{
  const char* command = "AT$$TCP_SCOP=0\r\n";
  const char* ack_list[] = { "$$TELL: 603" }; // 성공/실패 응답
  char response_buffer[256];
  uint32_t matched_index = 0;
  int32_t response_length;





  response_length = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, _countof(ack_list), &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 10000);

  if (response_length > 0) // 성공 시 반환값은 길이
  {
    if (matched_index == 0) return 0; // $$TELL: 603 (성공)
    else return 1; // $$TELL: 602 (실패)
  }
  return 1; // 타임아웃 또는 오류 시
}

/**
 * @brief TCP 소켓 닫기
 */
int32_t tx700_close_tcp(cellular_if_t* p_if)
{
  const char* command = "AT$$TCP_SCCL=0\r\n";
  const char* ack_list[] = { "$$TELL: 605" }; // TCP 접속 종료
  char response_buffer[256];
  uint32_t matched_index = 0;
  int32_t return_code;

  return_code = 1;



  return_code = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 1000);

  if (return_code > 0 && matched_index == 0) // 성공 시 반환값은 길이, 매치 인덱스 0
  {
    return 0;
  }

  return return_code;
}

/**
 * @brief TCP 데이터 전송
 */
int32_t tx700_send_tcp(cellular_if_t* p_if, uint8_t* p_data_buffer, size_t data_length)
{
  const char* ack_list[] = { "$$TCP_SENDDATA:" };
  char command_buffer[512 + 64];
  char response_buffer[256];
  uint32_t matched_index = 0;
  int32_t command_length;
  int32_t return_code;
  int32_t response_code;

  return_code = 1;

  // AT$$TCP_SENDBIN=00<len_high><len_low><data>\r\n 형식
  strcpy(command_buffer, "AT$$TCP_SENDBIN=00"); // 18 바이트 고정 부분 (AT$$TCP_SENDBIN=00<CR><LF>)

  command_buffer[16] = (0xFF & (data_length >> 8)); // 길이 상위 1바이트
  command_buffer[17] = (0xFF & data_length);       // 길이 하위 1바이트

  // 데이터 복사
  memcpy(&command_buffer[18], (void*)p_data_buffer, data_length);

  command_length = 18 + data_length; // 실제 명령 길이

  command_buffer[command_length++] = '\r';
  command_buffer[command_length++] = '\n';


  return_code = dispatcher_send_sync((const uint8_t*)command_buffer, command_length, ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 10000);

  if (return_code > 0 && matched_index == 0) // 성공 시 반환값은 길이, 매치 인덱스 0
  {
    if (sscanf(response_buffer, "$$TCP_SENDDATA:%d", &response_code) == 1)
    {
      if (response_code != 1)
        return -1; // 전송 실패
      else 
        return data_length; // 전송 성공, 전송된 바이트 수 반환
    }
  }

  return -1; // 실패
}



/**
 * @brief TCP 데이터 수신 핸들러 (내부 사용)
 */
int32_t tx700_tcp_recv_handler(cellular_if_t* p_if, uint8_t* p_data_buffer, size_t data_length, size_t buffer_size)
{
  uint16_t len;
  tcp_data_t tcp_data;


  len = (p_data_buffer[10] & 0x03) * 256 + p_data_buffer[11];

  if (len <= sizeof(tcp_data.data))
  {
    memcpy(tcp_data.data, &p_data_buffer[12], len);
    tcp_data.data[len] = '\0';
    tcp_data.len = len;
    if (osMessageQueuePut(s_tx700_tcp_queue, &tcp_data, 0, 100) != osOK) {
      printf("TX700: TCP 큐에 메시지 넣기 실패\r\n");
    }
  }


  return len; // TX700에서는 이 함수가 다른 방식으로 사용될 가능성 높음
}

/**
 * @brief TCP 데이터 수신
 */
int32_t tx700_recv_tcp(cellular_if_t* p_if, uint8_t* p_buffer, size_t buffer_size, uint32_t timeout_ms)
{
  int32_t len = 0;
  tcp_data_t tcp_data;

  if (osMessageQueueGet(s_tx700_tcp_queue, &tcp_data, NULL, timeout_ms) == osOK) {

    if(buffer_size >= tcp_data.len) {
      memcpy(p_buffer, tcp_data.data, tcp_data.len);
      len = tcp_data.len;
    } else {
      memcpy(p_buffer, tcp_data.data, buffer_size);
      len = -1; // 버퍼 오버플로우
    }
  }
  return len;
}
/**
 * @brief 전화 수신
 */
int32_t tx700_recv_call(cellular_if_t* p_if)
{
  const char* command = "ATA\r\n";
  const char* ack_list[] = { "$$TELL: 751, VOICE : CONNECT USER" };
  char response_buffer[100];
  uint32_t matched_index = 0;
  uint32_t loop_index;
  int32_t return_code;

  return_code = 1;

  for (loop_index = 0; loop_index < 2; loop_index++)
  {


    int response_length = dispatcher_send_sync((const uint8_t*)command, strlen(command), ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 200);

    if (response_length > 0 && matched_index == 0) // 성공 시 반환값은 길이, 매치 인덱스 0
    {
      return_code = 0;
      break;
    }
    osDelay(1000);
  }

  return return_code;
}

/**
 * @brief 전화 걸기
 */
int32_t tx700_dial(cellular_if_t* p_if, char* p_number_str, uint32_t timeout_ms_val)
{
  // TX700은 dial 기능이 구현되지 않음 (AT 명령이 다름)
  return -1; // 기능 미구현
}

/**
 * @brief VPN 초기화
 */
int32_t tx700_vpn_init(cellular_if_t* p_if)
{
  // TX700은 VPN 기능이 없음
  return 0; // 기능 없음, 성공으로 처리
}

/**
 * @brief VPN 설정
 */
int32_t tx700_set_vpn_config(cellular_if_t* p_if, char* p_id_str, char* p_password_str, uint8_t p_ip_address[4], uint16_t port_num)
{
  // TX700은 VPN 기능이 없음
  return 0; // 기능 없음, 성공으로 처리
}

/**
 * @brief VPN 설정 읽기
 */
int32_t tx700_read_vpn_config(cellular_if_t* p_if, char* p_buffer, size_t buffer_size)
{
  // TX700은 VPN 기능이 없음
  p_buffer[0] = '\0';
  return 0; // 기능 없음, 성공으로 처리
}

/**
 * @brief AT 명령 직접 실행
 */
int32_t tx700_at_direct(cellular_if_t* p_if, const char* p_at_command_str, char* p_response_buffer_out, size_t buffer_size_out)
{
  char response_buffer_local[512];
  uint32_t matched_index = 0;
  int32_t return_code;

  return_code = 1;



  // dispatcher_send_sync를 사용하여 응답 대기 (ack_list 없음)
  return_code = dispatcher_send_sync((const uint8_t*)p_at_command_str, strlen(p_at_command_str), NULL, 0, &matched_index,
                                    (uint8_t*)response_buffer_local, sizeof(response_buffer_local) - 1, 1000);
  if (return_code > 0) { // 성공 시 반환값은 길이
    response_buffer_local[return_code] = '\0'; // 길이만큼 NULL 종료, 안전하게
    strcpy_safe(p_response_buffer_out, buffer_size_out, response_buffer_local);
    return 0;
  }

  return 1; // 실패
}

/**
 * @brief 네트워크 서비스 상태 확인
 */
int32_t tx700_check_network_service(cellular_if_t* p_if, char* p_buffer, size_t buffer_size)
{
  // TX700은 별도의 AT 명령이 필요할 수 있음. 임시로 항상 서비스 가능으로 처리.
  strcpy_safe(p_buffer, buffer_size, "Service Available");
  return 0;
}

/**
 * @brief Ring 수신 시 전화번호 읽기
 */
int32_t tx700_read_ring_number(cellular_if_t* p_if, const char* p_data_buffer, char* p_buffer_out, size_t buffer_size_out)
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
char tx700_get_dtmf(cellular_if_t* p_if, uint8_t* p_data_buffer)
{
  char dtmf_code_val;

  //$DTMF: 4 에서 dtmf 코드만 추출
  dtmf_code_val = p_data_buffer[7]; // $DTMF: 다음의 숫자

  return dtmf_code_val;
}

/**
 * @brief IP 주소 및 포트 설정
 */
void tx700_write_ip(cellular_if_t* p_if, uint8_t p_ip_address[4], uint16_t port_num)
{
  const char* ack_list[] = { "$$TCP_ADDR:" };
  char command_buffer[512];
  char response_buffer[256];
  uint32_t matched_index = 0;

  snprintf(command_buffer, sizeof(command_buffer), "AT$$TCP_ADDR=0,%d,%d,%d,%d,%d\r\n", p_ip_address[0], p_ip_address[1], p_ip_address[2], p_ip_address[3], port_num);

  int32_t return_code = dispatcher_send_sync((const uint8_t*)command_buffer, strlen(command_buffer), ack_list, 1, &matched_index,
                                    (uint8_t*)response_buffer, sizeof(response_buffer) - 1, 1000);
  osDelay(1000);// 지연 모뎀 안정화 
}

/**
 * @brief 소프트웨어 리셋
 */
void tx700_reset_sw(cellular_if_t* p_if)
{
  const char* command = "AT$$RESET\r\n";

  dispatcher_send_async((const uint8_t*)command, strlen(command));
}

/**
 * @brief 하드웨어 리셋
 */
void tx700_reset_hw(cellular_if_t* p_if)
{
{

  drv_power_off(DRV_POWER_CDMA);
   osDelay(2000);
   drv_power_on(DRV_POWER_CDMA);
}
}

/**
 * @brief 안정적인 전원 차단
 */
void tx700_off_power_safe(cellular_if_t* p_if)
{
  // TX700은 특별한 전원 차단 절차가 없음
}
/*

SMS 읽기

2025-12-14 08:45:37.816 [COM29] - AT+CMGR=0<CR>
<CR><LF>
+CMGR: "REC READ","01053730725",,"25/12/14,08:44:05+36"<CR><LF>
123456<CR><LF>

SMS 읽기기 /r/n으로 분리된채 수신되어 dipatcher로 읽기 어려워서
수신 단에서 아래와같이 합성하여 처리함
따라서 +CMGR: "REC READ","01053730725",,"25/12/14,08:44:05+36",123456<CR><LF>

*/
int32_t tx700_recv_uart(cellular_if_t* p_if, uint8_t* p_buffer, size_t buffer_size, uint32_t timeout_ms)
{
  uint16_t current_count = 0;
  uint8_t received_char;
  uint8_t binary_mode = 0;
  uint8_t first_pass = 1;
  uint8_t first_pass_sms = 1;
  uint16_t data_len = 0;
  int32_t sub_len = 0;
  //uint8_t first_pass_sms_send=1;

  while (1)
  {
    if (p_if->uart_io.recv(p_if->uart_handle, &received_char, 1, 1000) == 1)
    {
      if (current_count >= buffer_size)
      {
        return 0; // 버퍼 오버플로우
      }

      p_buffer[current_count++] = received_char;

      //sms send시 응답대기 코드 별도 처리 
      if(p_buffer[0]==0x3E&&p_buffer[1]==0x20)
      {
        return 2;
      }
      
      if (current_count == 10 && first_pass)
      {
        first_pass = 0;
        if (strncmp((char*)p_buffer, "$$BinRecv:", 10) == 0)
        {
          binary_mode = 1;
        }
      }
      else if(current_count ==12&& binary_mode)
      {
         data_len = (p_buffer[10] & 0x0F) * 256 + p_buffer[11];
      }
      else if(current_count == 6 && first_pass_sms)
      {
        first_pass_sms = 0;
        if (strncmp((char*)p_buffer, "+CMGR:", 6) == 0)
        {

            while (p_if->uart_io.recv(p_if->uart_handle, &received_char, 1, 1000) == 1)
            {
              if (current_count >= buffer_size)
              {
                return 0; // 버퍼 오버플로우
              }
              p_buffer[current_count++] = received_char;
              if (received_char == '\r')
              {
                current_count--;
                p_buffer[current_count++ ] = ',';
                sub_len = p_if->uart_io.recv_crlf(p_if->uart_handle, (char *)&p_buffer[current_count], buffer_size - current_count, 1000); // 메시지 본문 수신
              
                if(sub_len > 0)
                {
                  current_count += sub_len;
                  p_buffer[current_count++] = '\r';
                  p_buffer[current_count++] = 0;
                  return current_count;
                }
  
              }
            }
         
          // 빈 줄 수신, 계속 진행
          current_count = 0; // 카운트 초기화
        }
      }
      else
      {
        if (binary_mode)
        {
          if (current_count >= (12 + data_len))
          {
            return current_count;
          }
        }
        else
        {
          if ((received_char == '\r') || (received_char == '\n'))
          {
            p_buffer[current_count - 1] = 0;
            return (current_count - 1);
          }
        }
      }
    }
  }
}


/**
 * @brief TX700 드라이버 초기화 및 구조체 설정
 * @param p_if cellular_if_t 구조체 포인터
 * @param p_uart_config_val UART 설정 구조체
 * @return 성공 시 0, 실패 시 -1
 */
int32_t tx700_open(cellular_if_t *p_if)
{
  //void *p_uart_handle = NULL;

  if (p_if == NULL ) {
    return -1;
  }

  // 모뎀 이름 설정
  strcpy_safe(p_if->modem_name, sizeof(p_if->modem_name), "TX700");



  // AT 명령어 테이블 설정
  p_if->at_cmd_table = (at_cmd_table_t*)&tx700_command_table;

  // API 함수 포인터 설정
  p_if->api.init = tx700_init;
  p_if->api.send_sms = tx700_send_sms;
  p_if->api.read_sms = tx700_read_sms;
  p_if->api.read_num = tx700_read_num;
  p_if->api.read_rssi = tx700_read_rssi;
  p_if->api.open_tcp = tx700_open_tcp;
  p_if->api.close_tcp = tx700_close_tcp;
  p_if->api.open_ppp = tx700_open_ppp;
  p_if->api.close_ppp = tx700_close_ppp;
  p_if->api.send_tcp = tx700_send_tcp;
  p_if->api.tcp_recv_handler = tx700_tcp_recv_handler;
  p_if->api.recv_call = tx700_recv_call;
  p_if->api.dial = tx700_dial;
  p_if->api.vpn_init = tx700_vpn_init;
  p_if->api.set_vpn_config = tx700_set_vpn_config;
  p_if->api.read_vpn_config = tx700_read_vpn_config;
  p_if->api.at_direct = tx700_at_direct;
  p_if->api.check_network_service = tx700_check_network_service;
  p_if->api.read_ring_number = tx700_read_ring_number;
  p_if->api.get_dtmf = tx700_get_dtmf;
  p_if->api.write_ip = tx700_write_ip;
  p_if->api.reset_sw = tx700_reset_sw;
  p_if->api.reset_hw = tx700_reset_hw;
  p_if->api.off_power_safe = tx700_off_power_safe;
  p_if->api.recv_tcp = tx700_recv_tcp;
  p_if->api.recv_uart = tx700_recv_uart;

  // 메시지 큐 생성 (TCP queue only)
  s_tx700_tcp_queue = osMessageQueueNew(1, sizeof(tcp_data_t), NULL);
  if (s_tx700_tcp_queue == NULL) {
    printf("TX700: 메시지 큐 생성 실패\r\n");
    tx700_close(p_if);
    return -1;
  }



  printf("TX700: TCP 큐 생성 완료\r\n");

  return RET_OK;
}

/**
 * @brief TX700 드라이버 종료 및 리소스 정리
 * @param p_if cellular_if_t 구조체 포인터
 */
void tx700_close(cellular_if_t *p_if)
{
  if (p_if == NULL) {
    return;
  }

  // TCP 메시지 큐 삭제
  if (s_tx700_tcp_queue != NULL) {
    osMessageQueueDelete(s_tx700_tcp_queue);
    s_tx700_tcp_queue = NULL;
  }



}