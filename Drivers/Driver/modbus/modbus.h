
#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>

typedef enum _mb_message
{
  eID = 0,   //!< ID field
  eFUNC,     //!< Function code position
  eADD_HI,   //!< Address high byte
  eADD_LO,   //!< Address low byte
  eNB_HI,    //!< Number of coils or registers high byte
  eNB_LO,    //!< Number of coils or registers low byte
  eBYTE_CNT  //!< byte counter
} eMB_MESSAGE_t;

#define RET_OK 0
#define RET_TIME_OUT 1
#define RET_OVER 2
#define RET_INVAILD 3
#define RET_UNKNOWN 4
#define RET_UNKNOWN_VAL 5
#define RET_FAIL 6
#define RET_ID_FAIL 7

typedef union
{
  uint8_t u8[4];
  uint16_t u16[2];
  uint32_t u32;

} bytesFields;

typedef enum MB_FC
{
  MB_FC_READ_COILS = 1,               /* read coils or digital outputs */
  MB_FC_READ_DISCRETE_INPUT = 2,      /* read digital inputs */
  MB_FC_READ_REGISTERS = 3,           /*  read registers or analog outputs */
  MB_FC_READ_INPUT_REGISTER = 4,      /* read analog inputs */
  MB_FC_WRITE_COIL = 5,               /* write single coil or output */
  MB_FC_WRITE_REGISTER = 6,           /* write single register */
  MB_FC_WRITE_MULTIPLE_COILS = 15,    /*  write multiple coils or outputs */
  MB_FC_WRITE_MULTIPLE_REGISTERS = 16 /* write multiple registers */
} mb_functioncode_t;
typedef struct
{
  uint8_t id;            /*!< Slave address between 1 and 247. 0 means broadcast */
  mb_functioncode_t fc;  /*!< Function code: 1, 2, 3, 4, 5, 6, 15 or 16 */
  uint16_t regAdd;       /*!< Address of the first register to access at slave/s */
  uint16_t coilsNo;      /*!< Number of coils or registers to access */
  uint16_t reg[256];     /*!< Pointer to memory image in master */
  uint32_t *currentTask; /*!< Pointer to the task that will receive
                            notifications from Modbus */
  uint16_t *regs;
  uint16_t regsCnt;
  uint32_t status;
  uint32_t wait_ms;  // rs485 최소 대기 시간
} modbus_t;

typedef struct regs_s
{
  uint8_t *reg;
  uint16_t base;
  uint16_t max;
} regs_t;

#define lowByte(w) ((w) & 0xff)
#define highByte(w) ((w) >> 8)
#define QUERY_CNT 1

#define CMD_BYPASS 0
#define CMD_CHNAGE_BAUD 1
#define MODBUS_REQ_TIMEOUT_MS 3000

uint16_t ModRTU_CRC(uint8_t *buf, int len);

#ifndef MODBUS_INIT_T
#define MODBUS_INIT_T
#include "driver_485_def.h"
typedef rs485_init_t modbus_init_t;
#endif

typedef enum modbus_type_e
{
  eMODBUS_RS485
} eMODBUS_TYPE_t;
#endif