
#include "cmsis_os2.h"
#include "config.h"

#include "driver_485.h"

#include "task_modbus.h"

#define RET_SIZE_OVER -1
#define RET_TIMEOUT   -2
#define FRAME_485_Q_CNT 1

#define lowByte(w) ((w) & 0xff)
#define highByte(w) ((w) >> 8)
#define QUERY_CNT 1

#define CMD_BYPASS      0
#define CMD_CHNAGE_BAUD 1
#define MODBUS_REQ_TIMEOUT_MS 3000
typedef struct _send_data
{
    uint8_t data[256];
    uint16_t cnt;
}modbus_data_t;

const osThreadAttr_t modbusTask_attributes = {
  .name = "modbusTask",
  .stack_size = 1024,
  .priority = (osPriority_t) osPriorityNormal1,
};

static osSemaphoreId_t g_modbusQuerySem = NULL;
osMessageQueueId_t g_queryMailId;
driver_t *modbus_driver;


 
uint16_t calcCRC(uint8_t* Buffer, uint32_t length)
{
    unsigned int temp, temp2, flag;
    temp = 0xFFFF;
    for (uint32_t i = 0; i < length; i++)
    {
        temp = temp ^ Buffer[i];
        for (unsigned char j = 1; j <= 8; j++)
        {
            flag = temp & 0x0001;
            temp >>= 1;
            if (flag)
                temp ^= 0xA001;
        }
    }
    // Reverse byte order.
    temp2 = temp >> 8;
    temp = (temp << 8) | temp2;
    temp &= 0xFFFF;
    // the returned value is already swapped
    // crcLo byte is first & crcHi byte is last
    return temp;

}



bool modbus_is_query(modbus_t * pOutModbus, uint32_t timeOutMs)
{
    osEvent event;
    uint32_t err = 1;
    bool ret=false;

    if(osMessageQueueGet(g_queryMailId, pOutModbus, NULL, timeOutMs) == osOK)
    {
      ret = true;
    }
    
    return ret;
}




//모드버스 통해 자료 요청
void modbus_query(modbus_t *telegram)
{

    telegram->currentTask = (uint32_t*)osThreadGetId();
    if(osMessageQueuePut(g_queryMailId, telegram, 0, osWaitForever) != osOK)
    {
 
    }

}


/**
 * @brief 모드버스 장치에 데이터 요청
 * @param
 * @retval 0 정상 1이상이면 에러
*/
eRET_t modbus_req(modbus_t *modbus)
{

	eRET_t err = eRET_UNKNOWN;
	uint32_t flag;

   (void)osSemaphoreAcquire(g_modbusQuerySem, osWaitForever);
  	modbus_query(modbus);


  flag =   osThreadFlagsWait(REQ_OK | REQ_TIMEOUT | REQ_BUFF_ERR|REQ_INVALID,osFlagsWaitAny,osWaitForever);

    if(flag & REQ_OK)
    {
      err =  eRET_OK;// 정상
    }
    else if(flag &REQ_TIMEOUT)
    {
      err = eRET_TIME_OUT;
    }
    else if(flag &REQ_BUFF_ERR)
    {
      err = eRET_OVER;
    }
    else if(flag &REQ_INVALID)
    {
      err = eRET_INVAILD;
    }
    else
    {
      err = eRET_UNKNOWN_VAL;
    }




    (void)osSemaphoreRelease(g_modbusQuerySem);
    
	return err;
}

/**
 * @brief 특정 주소에 값 쓰기
 * @param slave_id
 * @param address
 * @param val
 * @retval  
*/
eRET_t modbus_write_single_reg(uint8_t slave_id, uint16_t address, uint16_t val)
{
	modbus_t modbus;
	uint16_t reg[10];
	eRET_t err = eRET_FAIL;

	modbus.id = slave_id;
	modbus.fc = MB_FC_WRITE_REGISTER;
	modbus.regAdd  = address;
	modbus.coilsNo = 1;
	modbus.regs    = &reg[0];
	modbus.regsCnt = sizeof(reg) / sizeof(reg[0]);
	modbus.wait_ms = MODBUS_REQ_TIMEOUT_MS;

	modbus.reg[0] = val;//시작

	if(modbus_req(&modbus) == eRET_OK)
	{
        err = eRET_OK;
	}

    return err;
}

eRET_t modbus_write_multi_reg(uint8_t slave_id, uint16_t address, uint16_t *regs,uint16_t regCnt)
{
	modbus_t modbus;
	uint16_t reg[160];
	eRET_t err = eRET_FAIL;

    if((sizeof(reg)/sizeof(reg[0])) < regCnt)
    {
        return err;
    }

	modbus.id = slave_id;
	modbus.fc = MB_FC_WRITE_MULTIPLE_REGISTERS;
	modbus.regAdd  = address;
	modbus.coilsNo = regCnt;
	modbus.regs    = &reg[0];
	modbus.regsCnt = sizeof(reg) / sizeof(reg[0]);
	modbus.wait_ms = MODBUS_REQ_TIMEOUT_MS;

    for(int i =  0 ; i < regCnt ; i++)
    {
	    modbus.reg[i] = regs[i];
    }

	if (modbus_req(&modbus) == eRET_OK)
	{
         err = eRET_OK;
	}

    return err;
}
eRET_t modbus_read_multi_reg(uint8_t slave_id, uint16_t address, uint16_t *pOutRegs,uint16_t regCnt)
{
	modbus_t modbus;
	uint16_t reg[160];
	eRET_t ret = eRET_FAIL;

    if((sizeof(reg)/sizeof(reg[0])) < regCnt)
    {
        return ret;
    }

	modbus.id      = slave_id;
	modbus.fc      = MB_FC_READ_REGISTERS;
	modbus.regAdd  = address;
	modbus.coilsNo = regCnt;
	modbus.regs    = &reg[0];
	modbus.regsCnt = sizeof(reg) / sizeof(reg[0]);
	modbus.wait_ms = MODBUS_REQ_TIMEOUT_MS;

    ret = modbus_req(&modbus);
    g_modbusLastErr = ret;//디버깅을 위해 모드버스 마지막 결과값을 저장 
	if(ret == eRET_OK)
	{
        for(int i =  0 ; i < regCnt ; i++)
        {
            pOutRegs[i] = modbus.regs[i];
        }
	}
    else
    {
        ret = eRET_FAIL;
    }

    return ret;
}

void send_query(modbus_t* pmodbus)
{
    uint8_t regsno;
    uint8_t bytesno;
    uint16_t cnt = 0;
    uint16_t crc;
modbus_data_t g_buff;// 이상:전역변수로 선안하면 이더넷 불안정함 원인파악 추후

    g_buff.data[cnt++] = pmodbus->id;// 전역변수 선언하고 이문장 실행하면 이더넷이 이상함 이해불가..흠.
    g_buff.data[cnt++] = pmodbus->fc;
    g_buff.data[cnt++] = highByte(pmodbus->regAdd);
    g_buff.data[cnt++] = lowByte(pmodbus->regAdd);


    switch (pmodbus->fc)
    {
        case MB_FC_READ_COILS:
        case MB_FC_READ_DISCRETE_INPUT:
        case MB_FC_READ_REGISTERS:
        case MB_FC_READ_INPUT_REGISTER:
            g_buff.data[cnt++] = highByte(pmodbus->coilsNo);
            g_buff.data[cnt++] = lowByte(pmodbus->coilsNo);
            break;
        case MB_FC_WRITE_COIL:
            g_buff.data[cnt++] = ((pmodbus->reg[0] > 0) ? 0xff : 0);
            g_buff.data[cnt++] = 0;
            break;
        case MB_FC_WRITE_REGISTER:
            g_buff.data[cnt++] = highByte(pmodbus->reg[0]);
            g_buff.data[cnt++] = lowByte(pmodbus->reg[0]);
            break;
        case MB_FC_WRITE_MULTIPLE_COILS:
            regsno  = pmodbus->coilsNo / 16;
            bytesno = regsno * 2;
            if ((pmodbus->coilsNo % 16) != 0)
            {
                bytesno++;
                regsno++;
            }

            g_buff.data[cnt++] = highByte(pmodbus->coilsNo);
            g_buff.data[cnt++] = lowByte(pmodbus->coilsNo);
            g_buff.data[cnt++] = bytesno;
            

            for (uint16_t i = 0; i < bytesno; i++)
            {
                if (i % 2)
                {
                    g_buff.data[cnt++] = lowByte(pmodbus->reg[i / 2]);
                }
                else
                {
                    g_buff.data[cnt++] = highByte(pmodbus->reg[i / 2]);

                }

            }
            break;
        case MB_FC_WRITE_MULTIPLE_REGISTERS:
            g_buff.data[cnt++] = highByte(pmodbus->coilsNo);
            g_buff.data[cnt++] = lowByte(pmodbus->coilsNo);
            g_buff.data[cnt++] = (uint8_t)(pmodbus->coilsNo * 2);


            for (uint16_t i = 0; i < pmodbus->coilsNo; i++)
            {
                g_buff.data[cnt++] = highByte(pmodbus->reg[i]);
                g_buff.data[cnt++] = lowByte(pmodbus->reg[i]);
            }
            break;

    }




    crc = calcCRC(g_buff.data, cnt);

    g_buff.data[cnt++] = crc >> 8;
    g_buff.data[cnt++] = (crc & 0x00FF);

    driver_rs485_send(modbus_driver, g_buff.data, cnt);
   
    g_buff.cnt = cnt;

}

uint16_t word(uint8_t H, uint8_t L)
{
    bytesFields W;
    W.u8[0] = L;
    W.u8[1] = H;

    return W.u16[0];
}
/**
 * This method processes functions 1 & 2 (for master)
 * This method puts the slave answer into master data buffer
 *
 * @ingroup register
 */
void get_FC1(uint8_t *pInData,uint16_t dataLen, uint16_t *regs,uint16_t regCnt)
{

    if (regs == NULL)
    {
        return;
    }
#if 0 
    uint8_t u8byte, i;
    u8byte = 3;
    for (i = 0; i < pInData[2]; i++) {

        if (i % 2)
        {
            regs[i / 2] = word(pInData[i + u8byte], lowByte(modH->u16regs[i / 2]));
        }
        else
        {

            regs[i / 2] = word(highByte(modH->u16regs[i / 2]), modH->u8Buffer[i + u8byte]);
        }

    }
#endif
}
void get_FC3(uint8_t* pInData, uint16_t dataLen, uint16_t* regs, uint16_t regCnt)
{
    uint8_t u8byte, i;
    u8byte = 3;


    if (regs == NULL)
    {
        return;
    }

    if ((pInData[2] / 2) > regCnt)
    {
        return;
    }

    for (i = 0; i < pInData[2] / 2; i++)
    {
        regs[i] = word(pInData[u8byte], pInData[u8byte + 1]);
        u8byte += 2;
    }
}
int32_t parse_recv(uint8_t* pInData, uint16_t dataLen,uint16_t *pOutRegs,uint16_t regCnt)
{

    int32_t err = 1;
    uint16_t crc;
    uint16_t crc_received;

    if(dataLen <2)
    {
      return 1;
    }
    crc = calcCRC(pInData, dataLen-2);


    crc_received = word(pInData[dataLen - 2], pInData[dataLen - 1]);

    if (crc != crc_received)
    {
        return 1;
    }


    switch (pInData[eFUNC])
    {
    case MB_FC_READ_COILS:
    case MB_FC_READ_DISCRETE_INPUT:
        //call get_FC1 to transfer the incoming message to u16regs buffer
        get_FC1(pInData,dataLen,pOutRegs,regCnt);
        break;
    case MB_FC_READ_INPUT_REGISTER:
    case MB_FC_READ_REGISTERS:
        // call get_FC3 to transfer the incoming message to u16regs buffer
        get_FC3(pInData, dataLen, pOutRegs, regCnt);
        break;
    case MB_FC_WRITE_COIL:
    case MB_FC_WRITE_REGISTER:
    case MB_FC_WRITE_MULTIPLE_COILS:
    case MB_FC_WRITE_MULTIPLE_REGISTERS:
        // nothing to do
        break;
    default:
        break;
    }
    
        err = 0;
    return err;
}

void modbusTask(void *arg)
{
    uint8_t buff[512];
    int32_t len;
    modbus_t modbus;



 

    while (1)
    {
        if(modbus_is_query(&modbus, osWaitForever))
        {
            send_query(&modbus);

            len = driver_rs485_recv(modbus_driver, buff, sizeof(buff),modbus.wait_ms);

            switch(len)
            {
                case RET_SIZE_OVER:
                 osThreadFlagsSet(modbus.currentTask, REQ_BUFF_ERR);
                    break;
                case RET_TIMEOUT:
                  osThreadFlagsSet(modbus.currentTask, REQ_TIMEOUT);
                    break;
                default:
                    if (parse_recv(buff, len, modbus.regs, modbus.regsCnt) == 0)
                    {
                      osThreadFlagsSet(modbus.currentTask, REQ_OK);
                    }
                    else
                    {
                      osThreadFlagsSet(modbus.currentTask, REQ_INVALID);
                    }
                    break;
            }
        }
    }
}
//  osThreadFlagsWait(0x00000001,osFlagsWaitAny,osWaitForever);
void modbusTask_init(void)
{
  uart_config_t uart_config={.dataLen=UART_DATA_LEN_8,.stop_bit=0};

  uart_config.baud = 19200;
  uart_config.parityIdx = 0;
  uart_config.stop_bit = 0;
  //modbus_driver = driver_rs485_open(RS485_A,&uart_config);


  g_modbusQuerySem = osSemaphoreNew(1, 1, NULL); 

  g_queryMailId = osMessageQueueNew(2, sizeof(modbus_t), NULL);


  //osThreadNew(modbusTask, NULL, &modbusTask_attributes);

}