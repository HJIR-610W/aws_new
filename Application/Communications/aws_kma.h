
#ifndef AWS_KMA_H

#define AWS_KMA_H

#include <stdint.h>


typedef struct {
  uint16_t header;
  uint8_t protocol_year;
  uint8_t protocol_month;
  uint8_t protocol_day;
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint16_t password;
  uint16_t id;
  char cmd[10];
  uint16_t crc;
  uint16_t end;
} kma_req_t;
//수신된 명령어 처리하기 위해 적용


typedef enum KMA_REQ_e
{
  eAI,
  eAB,
  eAQ,
  eAV,
  eAR,
  eAO,
  eAD,
  eAT,
  eAW,
  eAC,
  eAP//국립공원 기존에 있길래 추가,기상청 문서에는 없음
}eKMA_REQ_t;

typedef struct 
{
  eKMA_REQ_t cmd;
  const char *cmdName;
}kma_cmd_t;


extern const kma_cmd_t kma_cmd[];


uint16_t coutntof_kma_cmd(void);

#endif
