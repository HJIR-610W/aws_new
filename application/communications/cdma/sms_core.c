
#include "sms_core.h"
#include "util_crc16_ccitt.h"
#include "util_memory.h"

size_t make_sms_frame(eSMS_CMD_t cmd, const uint8_t* data, size_t data_len, uint8_t* buffer, size_t buffer_len)
{
  size_t count = 0;
  uint16_t crc16 = 0;
  memset(buffer, 0x00, buffer_len);

  buffer[count++] = 0x02;
  buffer[count++] = 5 + data_len;
  buffer[count++] = (uint8_t)cmd;
  memcpy(&buffer[count], data, data_len);
  count += data_len;
  crc16 = crc16_xmodem(&buffer[1], count - 1);
  memcpy(&buffer[count], &crc16, sizeof(crc16));
  count += sizeof(crc16);
  return count;
}
