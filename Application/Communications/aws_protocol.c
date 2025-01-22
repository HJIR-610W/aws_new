
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "crc16_ccitt.h"
#include "utile.h"
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

int32_t aws_cmd(uint8_t *input,uint32_t len)
{

  if(is_awsProtocol(input,len)==false)
  {
    return 0;
  }




  return 0;
}