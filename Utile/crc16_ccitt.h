

#ifndef CRC16_CCITT_H
#define CRC16_CCITT_H



uint16_t crc16_ccitt_table(uint8_t* data, uint16_t dataLen);
uint16_t Cal_CRC16_xmodem(uint8_t* data, uint16_t dataLen);
#endif