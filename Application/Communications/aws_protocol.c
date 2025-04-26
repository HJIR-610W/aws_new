
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "crc16_ccitt.h"
#include "utile.h"
#include "aws_kma3.h"

#define KMA3_REQ_LEN 29  



bool is_awsProtocol(uint8_t *input,uint32_t len)
{
  uint16_t crc;
  uint16_t recv_crc;

  if(input[0]==0xFA && input[1]==0xFB)
  {
    crc = crc16_ccitt_table(&input[2],KMA3_REQ_LEN -6);
    crc = swap_uint16(crc);
    memcpy(&recv_crc,&input[KMA3_REQ_LEN -4],2);

    if(crc == recv_crc)
    {
      return true;
    }
  }
  return false;
}

int32_t aws_cmd(uint8_t *input,uint32_t inputLen,uint8_t *txBuff,uint16_t txSize,uint8_t source)
{
  int32_t len=0;

  if(is_awsProtocol(input,inputLen)==false)
  {
    return 0;
  }

  switch(source)
  {
    case 0://break;
    len = kma2_cmd_handler(input,inputLen,txBuff,txSize,source);
    break;
  }


  return len;
}