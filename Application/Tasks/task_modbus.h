
#ifndef TASK_MODBUS_H
#define TASK_MODBUS_H

#include <stdint.h>

#define REQ_OK       0x00000001
#define REQ_TIMEOUT  0x00000002
#define REQ_BUFF_ERR 0x00000004
#define REQ_INVALID  0x00000008

/**
 * @enum MB_FC
 * @brief
 * Modbus function codes summary.
 * These are the implement function codes either for Master or for Slave.
 *
 * @see also fctsupported
 * @see also modbus_t
 */
typedef enum MB_FC
{
    MB_FC_READ_COILS = 1,	 /*!< FCT=1 -> read coils or digital outputs */
    MB_FC_READ_DISCRETE_INPUT = 2,	 /*!< FCT=2 -> read digital inputs */
    MB_FC_READ_REGISTERS = 3,	 /*!< FCT=3 -> read registers or analog outputs */
    MB_FC_READ_INPUT_REGISTER = 4,	 /*!< FCT=4 -> read analog inputs */
    MB_FC_WRITE_COIL = 5,	 /*!< FCT=5 -> write single coil or output */
    MB_FC_WRITE_REGISTER = 6,	 /*!< FCT=6 -> write single register */
    MB_FC_WRITE_MULTIPLE_COILS = 15, /*!< FCT=15 -> write multiple coils or outputs */
    MB_FC_WRITE_MULTIPLE_REGISTERS = 16	 /*!< FCT=16 -> write multiple registers */
}mb_functioncode_t;




typedef enum _mb_message
{
    eID = 0, //!< ID field
    eFUNC, //!< Function code position
    eADD_HI, //!< Address high byte
    eADD_LO, //!< Address low byte
    eNB_HI, //!< Number of coils or registers high byte
    eNB_LO, //!< Number of coils or registers low byte
    eBYTE_CNT  //!< byte counter
}eMB_MESSAGE_t;

typedef enum {
	eRET_OK = 0,
	eRET_TIME_OUT,
	eRET_OVER,
	eRET_INVAILD,
	eRET_UNKNOWN,
	eRET_UNKNOWN_VAL,
	eRET_FAIL,
    eRET_ID_FAIL
}eRET_t;


typedef struct
{
    uint8_t id;          /*!< Slave address between 1 and 247. 0 means broadcast */
    mb_functioncode_t fc;         /*!< Function code: 1, 2, 3, 4, 5, 6, 15 or 16 */
    uint16_t regAdd;    /*!< Address of the first register to access at slave/s */
    uint16_t coilsNo;   /*!< Number of coils or registers to access */
    uint16_t reg[256];     /*!< Pointer to memory image in master */
    uint32_t* currentTask; /*!< Pointer to the task that will receive notifications from Modbus */
    uint16_t* regs;
    uint16_t regsCnt;
    uint32_t status;
    uint32_t wait_ms;//rs485 최소 대기 시간
}
modbus_t;

typedef union {
    uint8_t  u8[4];
    uint16_t u16[2];
    uint32_t u32;

} bytesFields;




void modbus_query(modbus_t *telegram);
eRET_t modbus_req(modbus_t *modbus);
eRET_t modbus_write_single_reg(uint8_t slave_id, uint16_t address, uint16_t val);
eRET_t modbus_write_multi_reg(uint8_t slave_id, uint16_t address, uint16_t *regs,uint16_t regCnt);
eRET_t modbus_read_multi_reg(uint8_t slave_id, uint16_t address, uint16_t *pOutRegs,uint16_t regCnt);
void modbus_setBaud(uint32_t baud);

extern eRET_t g_modbusLastErr;

void modbusTask_init(void);

#endif