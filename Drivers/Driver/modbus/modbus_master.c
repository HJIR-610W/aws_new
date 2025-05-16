

#include "modbus_master.h"

#include "driver_uart.h"
#include "driver_485.h"
#include "driver_485_def.h"
#include "driver_uart_def.h"
#include "modbus.h"
#include "modbus_master_def.h"
#include "os_user_def.h"
#include "pcb_define.h"

#define RET_SIZE_OVER -1
#define RET_TIMEOUT -2
#define FRAME_485_Q_CNT 1

#define MODBUS_RECV_FLUSH(driver) driver_rs485_flush_rx(driver)
#define MODBUS_SEND(driver, data, cnt) driver_rs485_send(driver, data, cnt);
#define MODBUS_RECV(driver, buff, buffSize, tout) driver_rs485_recv(driver, buff, buffSize, tout)
#define MODBUS_RECV_TIMEOUT(driver, buff, buffSize, tout1, tout2) \
  driver_rs485_recv_opt(driver, buff, buffSize, tout1, tout2)

#define MODBUS_232_RECV_FLUSH(driver) driver_uart_flush_rx(driver)
#define MODBUS_232_SEND(driver, data, cnt) driver_uart_send(driver, data, cnt);
#define MODBUS_232_RECV(driver, buff, buffSize, tout) driver_uart_recv(driver, buff, buffSize, tout)
#define MODBUS_232_RECV_TIMEOUT(driver, buff, buffSize, tout1, tout2) \
  driver_uart_recv_opt(driver, buff, buffSize, tout1, tout2)

typedef struct modbus_cfg_s
{
  eMODBUS_TYPE_t modbusType;
  driver_t *bus_io;
  void *sem;
} modbus_cfg_t;
int32_t g_modbusLastErr;

driver_t modbus_m_drv[MODBUS_RTU_MAX];
modbus_cfg_t modbus_m_cfg[MODBUS_RTU_MAX];

typedef struct modbus_regs_s
{
  uint16_t id;
  regs_t read_coils;
  regs_t discrete_input;
  regs_t input_register;
  regs_t read_register;
} modbus_regs_t;

typedef struct _send_data
{
  uint8_t data[256];
  uint16_t cnt;
} modbus_data_t;
modbus_data_t g_buff;

void send_query(driver_t *drv, modbus_t *pmodbus);

uint16_t word(uint8_t H, uint8_t L)
{
  bytesFields W;
  W.u8[0] = L;
  W.u8[1] = H;

  return W.u16[0];
}

uint16_t calcCRC(uint8_t *Buffer, uint32_t length)
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

  return temp;
}

void get_FC1(uint8_t *pInData, uint16_t dataLen, uint16_t *regs, uint16_t regCnt)
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

void get_FC3(uint8_t *pInData, uint16_t dataLen, uint16_t *regs, uint16_t regCnt)
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

int32_t parse_recv(uint8_t *pInData, uint16_t dataLen, uint16_t *pOutRegs, uint16_t regCnt)
{
  int32_t err = 1;
  uint16_t crc;
  uint16_t crc_received;

  if (dataLen < 2)
  {
    return 1;
  }
  crc = calcCRC(pInData, dataLen - 2);

  crc_received = word(pInData[dataLen - 2], pInData[dataLen - 1]);

  if (crc != crc_received)
  {
    return 1;
  }

  switch (pInData[eFUNC])
  {
    case MB_FC_READ_COILS:
    case MB_FC_READ_DISCRETE_INPUT:
      // call get_FC1 to transfer the incoming message to u16regs buffer
      get_FC1(pInData, dataLen, pOutRegs, regCnt);
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

#define MODBUS_START_TIMEOUT_MS 50  // 슬레이브가 늦게 줄수 도 있는걸 고려
#define MODBUS_DATA_TIMEOUT_MS 20   // 선점에 의한 지연 고려

int32_t modbus_receive_packet(driver_t *drv, uint8_t *rx_buf, uint16_t buf_size)
{
  uint8_t func_code;
  uint8_t data_len;
  int32_t ret;
  uint16_t total_len;
  uint16_t crc_calc;
  uint16_t crc_recv;
  modbus_cfg_t *cfg = drv->cfg;
  uint32_t delay;

  //  우선 주소 + 기능 + 최소 1바이트 데이터 수신
  if (cfg->modbusType == eMODBUS_RS232)
  {
    ret = MODBUS_232_RECV(cfg->bus_io, rx_buf, 3, MODBUS_START_TIMEOUT_MS);
  }
  else
  {
    ret = MODBUS_RECV(cfg->bus_io, rx_buf, 3, MODBUS_START_TIMEOUT_MS);
  }

  if (ret != 3)
  {
    return -1;
  }

  func_code = rx_buf[1];
  // 기능 코드에 따라 예상되는 데이터 길이 결정
  // 여기선 Read Holding Registers (0x03) 응답을 예시로 사용
  switch (func_code)
  {
    case 0x01:               // Read Coils
    case 0x02:               // Read Discrete Inputs
    case 0x03:               // Read Holding Registers
    case 0x04:               // Read Input Registers
    case 0x2B:               // Encapsulated Interface (추가 처리 필요할 수도 있음)
      data_len = rx_buf[2];  // Byte count
      break;
    case 0x05:       // Write Single Coil
    case 0x06:       // Write Single Register
    case 0x0F:       // Write Multiple Coils
    case 0x10:       // Write Multiple Registers
      data_len = 6;  // 고정 응답 길이 (주소 + 기능 제외)
      break;
    case 0x08:       // Diagnostic
      data_len = 4;  // 일반적인 응답
      break;
    default:
      // 예외 응답인지 확인
      if (func_code & 0x80)
      {
        data_len = 1;  // ExceptionCode 1바이트
      }
      else
      {
        return -99;  // 미지원 기능 코드
      }
      break;
  }

  //  전체 패킷 길이 계산
  total_len = 3 + data_len + 2;  // 헤더(3) + 데이터 + CRC(2)
  if (total_len > buf_size)
    return -3;

  // baud에 맞춰 1바이트 시간안에 데이터 안오는지 판단
  switch (cfg->modbusType)
  {
    case eMODBUS_RS485:
    {
      uart_config_t ucfg;
      driver_rs485_get(cfg->bus_io, UART_GET_CONFIG, &ucfg);
      delay = ((float)1 / (float)ucfg.baud) * 10 * 3.5 * 1000;  // ms
      delay = delay * 2;
      if (delay == 0)
      {
        delay = 10;  // 10ms
      }
    }
    break;
    case eMODBUS_RS232:
    {
      uart_config_t ucfg;
      driver_uart_get(cfg->bus_io, UART_GET_CONFIG, &ucfg);
      delay = ((float)1 / (float)ucfg.baud) * 10 * 3.5 * 1000;  // ms
      delay = delay * 2;
      if (delay == 0)
      {
        delay = 10;  // 10ms
      }
    }
        default:
      break;
  }
  //  나머지 데이터 수신
  if (cfg->modbusType == eMODBUS_RS232)
  {
    ret = MODBUS_232_RECV_TIMEOUT(cfg->bus_io, &rx_buf[3], data_len + 2, delay, delay);
  }
  else
  {
    ret = MODBUS_RECV_TIMEOUT(cfg->bus_io, &rx_buf[3], data_len + 2, delay, delay);
  }


    if (ret != data_len + 2)
      return -4;

    // CRC 확인
    crc_calc = calcCRC(rx_buf, total_len - 2);
    crc_recv = rx_buf[total_len - 2] << 8 | (rx_buf[total_len - 1]);
    if (crc_calc != crc_recv)
      return -5;  // CRC 에러

    return total_len;  // 유효한 패킷 길이 리턴
  }

int32_t modbus_master_req(driver_t *drv, modbus_t *modbus)
{
  uint8_t buff[300];
  int32_t len;
  modbus_cfg_t *cfg = drv->cfg;

  send_query(drv, modbus);

  len = modbus_receive_packet(drv, buff, sizeof(buff));

  if (len > 0)
  {
    if (parse_recv(buff, len, modbus->regs, modbus->regsCnt) == 0)
    {
      return 0;
    }
  }

  return 1;
}

/**
 * @brief 특정 주소에 값 쓰기
 * @param slave_id
 * @param address
 * @param val
 * @retval
 */
int32_t modbus_write_single_reg(driver_t *drv, uint8_t slave_id, uint16_t address, uint16_t val)
{
  modbus_t modbus;
  uint16_t reg[10];
  int32_t err = RET_FAIL;

  modbus.id = slave_id;
  modbus.fc = MB_FC_WRITE_REGISTER;
  modbus.regAdd = address;
  modbus.coilsNo = 1;
  modbus.regs = &reg[0];
  modbus.regsCnt = sizeof(reg) / sizeof(reg[0]);
  modbus.wait_ms = MODBUS_REQ_TIMEOUT_MS;

  modbus.reg[0] = val;  // 시작

  if (modbus_master_req(drv, &modbus) == RET_OK)
  {
    err = RET_OK;
  }

  return err;
}

int32_t modbus_write_multi_reg(driver_t *drv, uint8_t slave_id, uint16_t address, uint16_t *regs,
                               uint16_t regCnt)
{
  modbus_t modbus;
  uint16_t reg[160];
  int32_t err = RET_FAIL;

#if FREE_RTOS_USE
  PEND_SEM(drv->sem, osWaitForever);
#endif
  if ((sizeof(reg) / sizeof(reg[0])) < regCnt)
  {
#if FREE_RTOS_USE
    POST_SEM(drv->sem);
#endif
    return err;
  }

  modbus.id = slave_id;
  modbus.fc = MB_FC_WRITE_MULTIPLE_REGISTERS;
  modbus.regAdd = address;
  modbus.coilsNo = regCnt;
  modbus.regs = &reg[0];
  modbus.regsCnt = sizeof(reg) / sizeof(reg[0]);
  modbus.wait_ms = MODBUS_REQ_TIMEOUT_MS;

  for (int i = 0; i < regCnt; i++)
  {
    modbus.reg[i] = regs[i];
  }

  if (modbus_master_req(drv, &modbus) == RET_OK)
  {
    err = RET_OK;
  }

#if FREE_RTOS_USE
  POST_SEM(drv->sem);
#endif
  return err;
}

int32_t modbus_read_multi_reg(driver_t *drv, uint8_t slave_id, uint16_t address, uint16_t *pOutRegs,
                              uint16_t regCnt)
{
  modbus_t modbus;
  uint16_t reg[160];
  int32_t ret = RET_FAIL;

  if ((sizeof(reg) / sizeof(reg[0])) < regCnt)
  {
    return ret;
  }

  modbus.id = slave_id;
  modbus.fc = MB_FC_READ_REGISTERS;
  modbus.regAdd = address;
  modbus.coilsNo = regCnt;
  modbus.regs = &reg[0];
  modbus.regsCnt = sizeof(reg) / sizeof(reg[0]);
  modbus.wait_ms = MODBUS_REQ_TIMEOUT_MS;

  ret = modbus_master_req(drv, &modbus);
  g_modbusLastErr = ret;  // 디버깅을 위해 모드버스 마지막 결과값을 저장
  if (ret == RET_OK)
  {
    for (int i = 0; i < regCnt; i++)
    {
      pOutRegs[i] = modbus.regs[i];
    }
  }
  else
  {
    ret = RET_FAIL;
  }

  return ret;
}

void send_query(driver_t *drv, modbus_t *pmodbus)
{
  uint8_t regsno;
  uint8_t bytesno;
  uint16_t cnt = 0;
  uint16_t crc;

  g_buff.data[cnt++] = pmodbus->id;  // 전역변수 선언하고 이문장 실행하면
                                     // 이더넷이 이상함 이해불가..흠.
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
      regsno = pmodbus->coilsNo / 16;
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
  {
    modbus_cfg_t *cfg = drv->cfg;
    if (cfg->modbusType == eMODBUS_RS232)
    {
      MODBUS_232_RECV_FLUSH(cfg->bus_io);
      MODBUS_232_SEND(cfg->bus_io, g_buff.data, cnt);
    }
    else
    {
      MODBUS_RECV_FLUSH(cfg->bus_io);
      MODBUS_SEND(cfg->bus_io, g_buff.data, cnt);
    }

  }
  g_buff.cnt = cnt;
}

modbus_master_api_t modbus_master_api = {.read_multi_reg = modbus_read_multi_reg,
                                         .write_multi_reg = modbus_write_multi_reg,
                                         .write_single_reg = modbus_write_single_reg};

driver_t *modbus_master_open(int32_t num, void *opt)
{
  driver_t *driver;
  modbus_init_t *modbus_init = opt;
  uart_config_t uart_config;

 
  if (modbus_m_drv[num].opened)
  {
    return &modbus_m_drv[num];
  }

  uart_config.baud = modbus_init->baud;
  uart_config.parityIdx = modbus_init->parityIdx;
  uart_config.dataLen = 8;
  uart_config.stop_bit = modbus_init->stop;

  modbus_m_drv[num].opened = true;

  switch(num)
  {
    case MODBUS_RTU_OVER_485:
      modbus_m_cfg[num].modbusType = eMODBUS_RS485;
      modbus_m_cfg[num].bus_io = driver_rs485_open(modbus_init->port_num, &uart_config);

      modbus_m_drv[num].cfg = &modbus_m_cfg[num];
      modbus_m_drv[num].api = &modbus_master_api;
      modbus_m_drv[num].name = "MODBUS_RS485";
      break;
    case MODBUS_RTU_OVER_232:
      modbus_m_cfg[num].modbusType = eMODBUS_RS232;
      modbus_m_cfg[num].bus_io = driver_uart_open(modbus_init->port_num, &uart_config);

      modbus_m_drv[num].cfg = &modbus_m_cfg[num];
      modbus_m_drv[num].api = &modbus_master_api;
      modbus_m_drv[num].name = "MODBUS_RS232";
      break;

  }
#if FREE_RTOS_USE
  CREATE_BINARY_SEM(modbus_m_cfg[num].sem);
#endif

  return &modbus_m_drv[num];
}
