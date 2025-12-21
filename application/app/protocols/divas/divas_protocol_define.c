

#include "divas_protocol_define.h"
#include "util_safe.h"

#include "util_time.h"


/**
 * @brief 디바스 프레임 생성
 * @param rx_frame 수신받은 프레임
 * @param p_in_data 전송 데이터(NULL이면 이미 p_out_data에 메모리 활용
 */
uint16_t make_divas_frame(uint8_t cmd, uint8_t *rx_frame, const uint8_t *p_in_data, size_t data_length,
                         uint8_t *p_out_data, size_t out_size)
{
  uint8_t sum = 0;
  uint16_t frameLen;
  uint16_t cnt = 0;
  DATE_TIME_BUF ct=Date_Time;
  uint32_t i;

  frameLen = 12 + 2 + data_length;

  if (out_size < frameLen)
  {
    return 0;
  }

  p_out_data[cnt++] = 0x02;                               //[0   ]STX
  memcpy_safe(&p_out_data[cnt],out_size-cnt, &frameLen, sizeof(frameLen));  //[1..2]LEN
  cnt += sizeof(frameLen);

  p_out_data[cnt++] = rx_frame[3];  //[3   ]SEQ
  memcpy_safe(&p_out_data[cnt], out_size-cnt, &ct.Year, sizeof(ct.Year)); //[4..5]YEAR
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
      memcpy_safe(&p_out_data[cnt], out_size-cnt, p_in_data, data_length);  //[12..N]DATA
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

bool is_divas_frame(uint8_t *p_in_data, size_t data_length)
{
  uint8_t sum = 0;
  uint16_t len;
  uint16_t i;

  memcpy_safe(&len,sizeof(len), &p_in_data[1], sizeof(len));

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
