#include "divas_protocol_handler.h"
#include "app_file.h"
#include "util_time.h"
#include "kma_protocol_handler.h"
#include "user_heap.h"
#include "update_fw.h"
#include <string.h>

#define DIVAS_CMD_FW_DOWNLOAD 0x63
#define DIVAS_CMD_FW_UPDATE 0x64
#define RES_FSIZE_ERROR 32       // 처음에 보낸 TOTAL 사이즈와 패킷마다 보낸 사이즈가 다른경우
#define RES_FILE_WRITE_ERROR 39  // 파일 쓰기 오류

#define RES_CMD_ERR 0x80
#define ASCII_ACK 0x06
#define ASCII_NAK 0x15

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




uint16_t make_divasFrame(uint8_t cmd, uint8_t seq, const uint8_t *pInData, uint16_t dataLen,
                         uint8_t *pOutBuff, uint16_t buffSize)
{
  uint8_t sum = 0;
  uint16_t frameLen;
  uint16_t cnt = 0;
  DATE_TIME_BUF ct;
  uint32_t i;

  frameLen = 12 + 2 + dataLen;

  if (buffSize < frameLen)
  {
    return 0;
  }

  pOutBuff[cnt++] = 0x02;                               //[0   ]STX
  memcpy(&pOutBuff[cnt], &frameLen, sizeof(frameLen));  //[1..2]LEN
  cnt += sizeof(frameLen);

  pOutBuff[cnt++] = seq;  //[3   ]SEQ
  memcpy(&pOutBuff[cnt], &ct.Year, sizeof(ct.Year));
  ;  //[4..5]YEAR
  cnt += sizeof(ct.Year);
  pOutBuff[cnt++] = ct.Month;  //[6   ]Month
  pOutBuff[cnt++] = ct.Day;    //[7   ]Day
  pOutBuff[cnt++] = ct.Hour;   //[8   ]Hour
  pOutBuff[cnt++] = ct.Min;    //[9   ]Min
  pOutBuff[cnt++] = ct.Sec;    //[10  ]Sec
  pOutBuff[cnt++] = cmd;       //[11  ]CMD

  if (dataLen)
  {
    memcpy(&pOutBuff[cnt], pInData, dataLen);  //[12..N]DATA
    cnt += dataLen;
  }

  for (i = 1; i < cnt; i++)
  {
    sum += pOutBuff[i];  // 체크섬,LEN부터 데이터까지
  }

  pOutBuff[cnt++] = 0x03;  //[     ]ETX
  pOutBuff[cnt++] = sum;   //[     ]SUM

  return cnt;
}

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
        p_fw_buffer = aws_malloc(1024*512);
      }
    }


    if (p_fw_buffer)
    {
      memcpy(&p_fw_buffer[offset], &rx_frame[20], length);
    }

    rcvSize = offset + length;

    g_received_bytes =rcvSize;
    if ((rcvSize == totsize))
    {
      if (p_fw_buffer)
      {
        fret = write_file(UPDATE_FW__REMOTE_PATH, p_fw_buffer, totsize, 0);
        aws_free(p_fw_buffer);

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

  return make_divasFrame(DIVAS_CMD_FW_DOWNLOAD, rx_frame[3], data, cnt, tx_frame,
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

  return make_divasFrame(DIVAS_CMD_FW_UPDATE, rx_frame[3], data, cnt, tx_frame, KMA_TX_BUFFER_SIZE);
}

bool is_divas_frame(uint8_t *pInData, uint16_t dataLen)
{
  uint8_t sum = 0;
  uint16_t len;
  uint16_t i;

  memcpy(&len, &pInData[1], sizeof(len));

  if (len != dataLen)
  {
    return 0;
  }

  for (i = 1; i < (dataLen - 2); i++)
  {
    sum += pInData[i];
  }

  if (sum != pInData[dataLen - 1])
  {
    return false;
  }

  return true;
}


uint16_t divas_cmd_handler(uint8_t *rx_frame, uint16_t rx_len,uint8_t *tx_frame)
{
  uint16_t len = 0;

  if (is_divas_frame(rx_frame, rx_len)==false)
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
  }

  return len;
}
