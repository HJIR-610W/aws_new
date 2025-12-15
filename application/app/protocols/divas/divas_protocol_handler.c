#include "divas_protocol_handler.h"

#include <string.h>

#include "app_file.h"
#include "app_alarm_logging.h"

#include "aws_monitor.h"
#include "config_app.h"
#include "config_nvm.h"
#include "kma_protocol_handler.h"
#include "update_fw.h"
#include "user_heap.h"
#include "util_time.h"
#include "system_err.h"
#include "task_logging.h"
#include "app_logging.h"
#include "app_version.h"
#include "boot_version.h"
typedef struct
{
  uint8_t STX;
  uint16_t LEN;
  uint8_t SEQ;
  uint16_t Year;
  uint8_t Month;
  uint8_t Day;
  uint8_t Hour;
  uint8_t Min;
  uint8_t Sec;
  uint8_t CMD;
  uint8_t DATA[1];  // 가변 길이 데이터
  // 이후 ETX, SUM
} __attribute__((packed)) divas_frame_t;

#define DIVAS_FRAME_OFFSET(field) ((size_t)&(((divas_frame_t *)0)->field))

//디바스 명령어 정의
#define DIVAS_CMD_RD_INDEX    0x01
#define DIVAS_CMD_RD_VERSION  0x15
#define DIVAS_CMD_RD_CFG_OFS  0x29
#define DIVAS_CMD_WR_CFG_OFS  0x2A
#define DIVAS_CMD_FW_DOWNLOAD 0x63
#define DIVAS_CMD_FW_UPDATE   0x64
#define DIVAS_CMD_RD_SYSTEM   0x06
#define DIVAS_CMD_RESET       0x74
#define DIVAS_CMD_RD_SYSLOG   0x07



//응답 프레임 에러 여부
#define ASCII_ACK 0x06
#define ASCII_NAK 0x15

//NAK 에 따른 에러 코드
#define RES_FSIZE_ERROR 32       // 처음에 보낸 TOTAL 사이즈와 패킷마다 보낸 사이즈가 다른경우
#define RES_FILE_WRITE_ERROR 39  // 파일 쓰기 오류
#define RES_OVERFLOW_ERROR 40  // 버퍼 오버플로우

#define RES_CMD_ERR 0x80

#define DIVAS_FRAME_CMD_OFFSET  11
#define DIVAS_FRAME_DATA_OFFSET 12

#define DIVAS_FRAME_OVERHEAD 14 //STX(1) 길이(2) 시퀀스(1) 년월일시분초(7) 명령어(1) SUM(1) ETX(1)

uint32_t g_download_file_size = 0;
uint32_t g_received_bytes;
uint8_t *p_fw_buffer;




uint32_t get_download_file_size(void)
{
  return g_download_file_size;
}

uint32_t get_received_bytes(void)
{
  return g_received_bytes;
}



/**
 * @brief 디바스 프레임 생성
 * @param rx_frame 수신받은 프레임
 * @param p_in_data 전송 데이터(NULL이면 이미 p_out_data에 메모리 활용
 */
uint16_t make_divas_frame(uint8_t cmd, uint8_t *rx_frame, const uint8_t *p_in_data, uint16_t data_length,
                         uint8_t *p_out_data, uint16_t buffer_size)
{
  uint8_t sum = 0;
  uint16_t frameLen;
  uint16_t cnt = 0;
  DATE_TIME_BUF ct=Date_Time;
  uint32_t i;

  frameLen = 12 + 2 + data_length;

  if (buffer_size < frameLen)
  {
    return 0;
  }

  p_out_data[cnt++] = 0x02;                               //[0   ]STX
  memcpy(&p_out_data[cnt], &frameLen, sizeof(frameLen));  //[1..2]LEN
  cnt += sizeof(frameLen);

  p_out_data[cnt++] = rx_frame[3];  //[3   ]SEQ
  memcpy(&p_out_data[cnt], &ct.Year, sizeof(ct.Year)); //[4..5]YEAR

  cnt += sizeof(ct.Year);
  p_out_data[cnt++] = ct.Month;  //[6   ]Month
  p_out_data[cnt++] = ct.Day;    //[7   ]Day
  p_out_data[cnt++] = ct.Hour;   //[8   ]Hour
  p_out_data[cnt++] = ct.Min;    //[9   ]Min
  p_out_data[cnt++] = ct.Sec;    //[10  ]Sec
  p_out_data[cnt++] = cmd;       //[11  ]CMD

  if (data_length)
  {
    if (p_in_data)
    {
      memcpy(&p_out_data[cnt], p_in_data, data_length);  //[12..N]DATA
    }
    cnt += data_length;
  }

  for (i = 1; i < cnt; i++)
  {
    sum += p_out_data[i];  // 체크섬,LEN부터 데이터까지
  }

  p_out_data[cnt++] = 0x03;  //[     ]ETX
  p_out_data[cnt++] = sum;   //[     ]SUM

  return cnt;
}

#define FW_DOWNLOAD_BUFFER_SIZE (1024*1024)

uint16_t divas_fw_download(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint32_t totsize, offset;
  uint16_t length;
  uint8_t rtnstat;
  uint8_t res;

  uint32_t rcvSize;
  FRESULT fret;
  uint16_t len;
  uint16_t cnt = 0;
  uint8_t data[20];

  do
  {
    res = RES_CMD_ERR;
    rtnstat = ASCII_NAK;

    memcpy(&len, &rx_frame[1], sizeof(len));
    memcpy(&totsize, &rx_frame[12], sizeof(totsize));
    memcpy(&offset, &rx_frame[16], sizeof(offset));

    length = len - 8 - 14;  // 8:total(4) + offset(4)

    if (offset == 0) 
    {
      g_download_file_size = totsize;
      g_received_bytes = 0;
      if(p_fw_buffer == NULL)
      {
        p_fw_buffer = user_malloc(FW_DOWNLOAD_BUFFER_SIZE); 
      }
    }


    if (p_fw_buffer)
    {
      if( offset + length > FW_DOWNLOAD_BUFFER_SIZE)
      {
        rtnstat = ASCII_NAK;
        res = RES_OVERFLOW_ERROR;
        break;
      }
      else
      {
       memcpy(&p_fw_buffer[offset], &rx_frame[20], length);
      }
    }

    rcvSize = offset + length;

    g_received_bytes =rcvSize;
    if ((rcvSize == totsize))
    {
      if (p_fw_buffer)
      {
       // delete_file(UPDATE_FW__REMOTE_PATH);
            fret = write_file(UPDATE_FW__REMOTE_PATH, p_fw_buffer, totsize, 0);
        user_free(p_fw_buffer);
        p_fw_buffer = 0;

        if (fret != FR_OK)
        {
          res = RES_FILE_WRITE_ERROR;
          break;
        }
      }
  

    }
    rtnstat = ASCII_ACK;
  } while (0);

  // 리턴상태
  data[cnt++] = rtnstat;

  if (rtnstat == ASCII_ACK)
  {
    memcpy(&data[cnt], &totsize, 4);
    cnt += 4;
    memcpy(&data[cnt], &rcvSize, 4);
    cnt += 4;
  }
  else
  {
    data[cnt++] = res;
  }

  return make_divas_frame(DIVAS_CMD_FW_DOWNLOAD, rx_frame, data, cnt, tx_frame,
                         KMA_TX_BUFFER_SIZE);  // 200 주의 하드코딩
}

uint16_t divas_fw_update(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint8_t data[10];
  uint16_t cnt = 0;
  uint8_t code;

  code = check_firmware(UPDATE_REMOTE);

  if(code)
  {
    data[cnt++] = ASCII_NAK;
    data[cnt++] = code;
  }
  else
  {
    data[cnt++] = ASCII_ACK;
    set_magic_value(MAGIC_UPDATE_FW_REMOTE);
    set_firmware_update();
  }

  return make_divas_frame(DIVAS_CMD_FW_UPDATE, rx_frame, data, cnt, tx_frame, KMA_TX_BUFFER_SIZE);
}

bool is_divas_frame(uint8_t *p_in_data, uint16_t data_length)
{
  uint8_t sum = 0;
  uint16_t len;
  uint16_t i;

  memcpy(&len, &p_in_data[1], sizeof(len));

  if (len != data_length)
  {
    return 0;
  }

  for (i = 1; i < (data_length - 2); i++)
  {
    sum += p_in_data[i];
  }

  if (sum != p_in_data[data_length - 1])
  {
    return false;
  }

  return true;
}


#define INDEX_CONFIG_APP    0
#define INDEX_CONFIG_SENSOR 1
#define INDEX_CONFIG_NVM    2

uint16_t divas_read_config_offset(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint8_t *tx_data = &tx_frame[DIVAS_FRAME_OFFSET(DATA[0])];
  uint8_t * p_config=NULL;
  uint16_t cnt=0;
  uint8_t parameter_error=0;
#pragma pack(push, 1)
  struct read_config_offset_s
  {
    uint8_t config_type;
    uint16_t offset;
    uint16_t length;
  } request;
#pragma pack(pop)

  memcpy(&request, &rx_frame[DIVAS_FRAME_OFFSET(DATA[0])], sizeof(request));

  switch (request.config_type)
  {
    case INDEX_CONFIG_APP:
      p_config = (uint8_t *)get_config_app();
      break;
    case INDEX_CONFIG_SENSOR:
      p_config = (uint8_t *)get_config_sensor();
      break;
    case INDEX_CONFIG_NVM:
      p_config = (uint8_t *)get_config_nvm();
      break;
    default:
      parameter_error = 1;
      break;
  }

  do
  {
    if(parameter_error)
    {
      tx_data[cnt++] = ASCII_NAK;
      tx_data[cnt++] = 1;
      break;
    }
    
    //요청 길이가 전송가능한 버퍼보다 크면 에러 
    if (request.length >= (KMA_TX_BUFFER_SIZE - DIVAS_FRAME_OVERHEAD-1))
    {
      tx_data[cnt++] = ASCII_NAK;
      tx_data[cnt++] = 2;
      break;
    }
    if(p_config)
    {
      tx_data[cnt++] = ASCII_ACK;

      memcpy(&tx_data[cnt], p_config + request.offset, request.length);
      cnt += request.length;
    }

  }while(0);

  return make_divas_frame(DIVAS_CMD_RD_CFG_OFS, rx_frame, NULL, cnt, tx_frame,
                            KMA_TX_BUFFER_SIZE);
}




uint16_t
    divas_write_config_offset(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint8_t *tx_data = &tx_frame[DIVAS_FRAME_OFFSET(DATA[0])];
  uint8_t *rx_data = &rx_frame[DIVAS_FRAME_OFFSET(DATA[0])];
  uint8_t *p_config;
  uint16_t cnt = 0;
  uint8_t parameter_error = 0;
#pragma pack(push, 1)
  struct write_config_offset_s
  {
    uint8_t config_type;
    uint16_t offset;
    uint16_t length;
    //uint8_t data[]; // 데이터 
  } request;
#pragma pack(pop)
uint8_t *p_data;

  memcpy(&request, &rx_data[0], sizeof(request));
  p_data = &rx_data[sizeof(request)];

  switch (request.config_type)
  {
    case INDEX_CONFIG_APP:
      p_config = (uint8_t *)get_config_app();
      memcpy(p_config + request.offset, p_data, request.length);
      save_config_app();
      break;
    case INDEX_CONFIG_SENSOR:
      p_config = (uint8_t *)get_config_sensor();
      memcpy(p_config + request.offset, p_data, request.length);
      save_config_sensor();
      break;
    case INDEX_CONFIG_NVM:
      p_config = (uint8_t *)get_config_nvm();
      memcpy(p_config + request.offset, p_data, request.length);
      save_config_nvm();
      break;
    default:
      parameter_error = 1;
      break;
  }

  do
  {
    if (parameter_error)
    {
      tx_data[cnt++] = ASCII_NAK;
      tx_data[cnt++] = 1;
      break;
    }
    tx_data[cnt++] = ASCII_ACK;
  }while(0);

  return make_divas_frame(DIVAS_CMD_RD_CFG_OFS, rx_frame, NULL, cnt, tx_frame, KMA_TX_BUFFER_SIZE);
}



/// @brief 시스템 모니터링 정보 읽기
/// @param rx_frame 
/// @param tx_frame 
/// @return tx 프레임 길이
uint16_t divas_read_system(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint8_t *tx_data = &tx_frame[DIVAS_FRAME_OFFSET(DATA[0])];
  uint8_t *rx_data = &rx_frame[DIVAS_FRAME_OFFSET(DATA[0])];
  uint8_t *p_system;
  uint16_t cnt = 0;

#pragma pack(push, 1)
  struct read_config_offset_s
  {
    uint16_t offset;
    uint16_t length;
  } request;
#pragma pack(pop)
  aws_monitor_t aws_monitor;//TODO:heap 사용 

  memcpy(&request, &rx_data[0], sizeof(request));

  make_aws_monitor_frame(&aws_monitor);

  p_system = (uint8_t *)&aws_monitor;
  
  do
  {
    // 요청 길이가 전송가능한 버퍼보다 크면 에러
    if (request.length >= (KMA_TX_BUFFER_SIZE - DIVAS_FRAME_OVERHEAD - 1))
    {
      tx_data[cnt++] = ASCII_NAK;
      tx_data[cnt++] = 2;
      break;
    }

    tx_data[cnt++] = ASCII_ACK;
    memcpy(&tx_data[cnt], p_system + request.offset, request.length);
    cnt += request.length;

  } while (0);

  return make_divas_frame(DIVAS_CMD_RD_SYSTEM, rx_frame, NULL, cnt, tx_frame, KMA_TX_BUFFER_SIZE);
}





uint16_t divas_cmd_reset(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint8_t *tx_data = &tx_frame[DIVAS_FRAME_OFFSET(DATA[0])];
  uint16_t cnt = 0;

   tx_data[cnt++] = ASCII_ACK;

   log_printf(L_INFO, "divas cmd reset");
   reset_system_delay(5);
   return make_divas_frame(DIVAS_CMD_RESET, rx_frame, NULL, cnt, tx_frame, KMA_TX_BUFFER_SIZE);
}

#define RD_LOG_TYPE_Q 0
#define RD_LOG_TYPE_TIME 1

uint16_t divas_read_log(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint8_t *tx_data = &tx_frame[DIVAS_FRAME_OFFSET(DATA[0])];
  uint8_t *rx_data = &rx_frame[DIVAS_FRAME_OFFSET(DATA[0])];
  uint16_t cnt = 0;

#pragma pack(push, 1)
  struct read_config_offset_s
  {
    uint16_t q_start;
    uint16_t cnt;
    uint8_t log_type;
  } request;
#pragma pack(pop)
enum {SYSTEM_LOG=0,ALARM_LOG=1};

  int status;
  memcpy(&request, &rx_data[0], sizeof(request));

  do
  {
  
      if (request.cnt * sizeof(system_log_t) >= (KMA_TX_BUFFER_SIZE - DIVAS_FRAME_OVERHEAD - 1))
      {
        cnt = 0;
        tx_data[cnt++] = ASCII_NAK;
        tx_data[cnt++] = (uint8_t)-100;
        break;
      }
      
      tx_data[cnt]= ASCII_ACK;
      cnt++;

    for (int i = 0; i < request.cnt; i++)
    {
      switch(request.log_type)
      {
      case ALARM_LOG://에러
      status = alarm_read_log(request.q_start + i, (system_log_t*)&tx_data[1+i * sizeof(system_log_t)]);
        case SYSTEM_LOG://시스템
        default:
      status = logging_read_log(request.q_start + i, (system_log_t*)&tx_data[1+i * sizeof(system_log_t)]);
      break;
    }

      if (status != 0)
      {
        cnt = 0;
        tx_data[cnt++] = ASCII_NAK;
        tx_data[cnt++] = status;
        break;
      }
      cnt += sizeof(system_log_t);
    }
  }while(0);

  return make_divas_frame(DIVAS_CMD_RD_SYSLOG, rx_frame, NULL, cnt, tx_frame, KMA_TX_BUFFER_SIZE);
}

uint16_t divas_read_version(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint8_t *tx_data = &tx_frame[DIVAS_FRAME_OFFSET(DATA[0])];

  uint16_t cnt = 0;


  SetU32(&tx_data[cnt], get_app_version(NULL, NULL, NULL, NULL));
  cnt +=4;
  SetU32(&tx_data[cnt], get_product_code());
  cnt += 4;
  SetU32(&tx_data[cnt], get_app_alias());
  cnt += 4;
  SetU32(&tx_data[cnt], get_app_area_code());
  cnt += 4;
   SetU32(&tx_data[cnt], get_app_build_timestamp());
  cnt += 4;
  SetU32(&tx_data[cnt], get_boot_version(NULL, NULL, NULL, NULL));
  cnt += 4;
  SetU32(&tx_data[cnt], get_boot_build_timestamp());
  cnt += 4;
  SetU32(&tx_data[cnt], get_boot_pcb_version());
  cnt += 4;
  return make_divas_frame(DIVAS_CMD_RD_VERSION, rx_frame, NULL, cnt, tx_frame, KMA_TX_BUFFER_SIZE);
}

#define CONNET_TYPE_AWS 0x30
uint16_t divas_read_index(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint8_t *tx_data = &tx_frame[DIVAS_FRAME_OFFSET(DATA[0])];
  uint16_t cnt = 0;
  uint16_t id;


  tx_data[cnt++] = CONNET_TYPE_AWS;
  id = get_config_app()->id;
  memcpy(&tx_data[cnt],&id,2);
  
  cnt += 2;
  SetU32(&tx_data[cnt], 0);
  cnt+=4;

  return make_divas_frame(DIVAS_CMD_RD_INDEX, rx_frame, NULL, cnt, tx_frame, KMA_TX_BUFFER_SIZE);
}


uint16_t divas_cmd_handler(uint8_t *rx_frame, uint16_t rx_len, uint8_t *tx_frame)
{
  uint16_t len = 0;

  if (is_divas_frame(rx_frame, rx_len) == false)
  {
    return 0;
  }

  switch (rx_frame[11])
  {
    case DIVAS_CMD_FW_DOWNLOAD:
      len = divas_fw_download(rx_frame, tx_frame);
      break;
    case DIVAS_CMD_FW_UPDATE:
      len = divas_fw_update(rx_frame, tx_frame);
      break;
    case DIVAS_CMD_RD_CFG_OFS:
      len = divas_read_config_offset(rx_frame,tx_frame);
      break;
    case DIVAS_CMD_WR_CFG_OFS:
      len = divas_write_config_offset(rx_frame,tx_frame);
      break;
    case DIVAS_CMD_RD_SYSTEM:
      len = divas_read_system(rx_frame, tx_frame);
      break;
    case DIVAS_CMD_RESET:
      len = divas_cmd_reset(rx_frame, tx_frame);
      break;
    case DIVAS_CMD_RD_SYSLOG:
      len = divas_read_log(rx_frame,tx_frame);
    break;
    case DIVAS_CMD_RD_VERSION:
      len = divas_read_version(rx_frame, tx_frame);
    break;
    case DIVAS_CMD_RD_INDEX:
    //  len = divas_read_index(rx_frame,tx_frame);
      break;

  }

  return len;
}
