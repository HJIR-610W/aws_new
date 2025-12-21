
#ifndef DIVAS_PROTOCOL_DIVAS_H_
#define DIVAS_PROTOCOL_DIVAS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define DIVAS_FRAME_OFFSET(field) ((size_t)&(((divas_frame_t *)0)->field))

#define DIVAS_FRAME_CMD_OFFSET  11
#define DIVAS_FRAME_DATA_OFFSET 12
#define DIVAS_FRAME_OVERHEAD 14 //STX(1) 길이(2) 시퀀스(1) 년월일시분초(7) 명령어(1) SUM(1) ETX(1)

//NAK 에 따른 에러 코드
#define DIVAS_RES_FSIZE_ERROR 32       // 처음에 보낸 TOTAL 사이즈와 패킷마다 보낸 사이즈가 다른경우
#define DIVAS_RES_FILE_WRITE_ERROR 39  // 파일 쓰기 오류
#define DIVAS_RES_OVERFLOW_ERROR 40  // 버퍼 오버플로우
#define DIVAS_RES_CMD_ERR 0x80


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
#define DIVAS_ASCII_ACK 0x06
#define DIVAS_ASCII_NAK 0x15



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


uint16_t make_divas_frame(uint8_t cmd, uint8_t *rx_frame, const uint8_t *p_in_data, size_t data_length,
                         uint8_t *p_out_data, size_t out_size);

                         
bool is_divas_frame(uint8_t *p_in_data, size_t data_length);
#endif
