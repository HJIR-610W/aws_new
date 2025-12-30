
/*
        
| Baudrate | 1바이트             | T3.5      |
| -------- | ------------------- | --------- |
| 1200     | 8.33 ms             | ≈ 29 ms   |
| 2400     | 4.17 ms             | ≈ 14.6 ms |
| 4800     | 2.08 ms             | ≈ 7.3 ms  |
| 9600     | 1.04 ms             | ≈ 3.64 ms |
| 19200    | 0.52 ms             | ≈ 1.82 ms |
| 38400↑   | 고정 1.75 ms (표준 권장값) |    |

모드버스는 프레임 시작 조건 T3.5 
바이트간 T1.5            

19200bps 초과 시
T3.5를 1.75ms 고정값으로 써도 된다고 명시함
*/
#include "modbus_master.h"

#include <string.h>

#include "drv_rs485.h"
#include "drv_rs232.h"


#include "drv_uart_def.h"
#include "modbus.h"
#include "os_user_def.h"
#include "pcb_define.h"
#include "system_err.h"
#include "debug_io.h"

#define MODBUS_RECV_ERR_3BYTE_TIMEOUT_MS -1
#define MODBUS_RECV_ERR_NOT_SUPPORTED -99
#define MODBUS_RECV_ERR_BUFF_SIZE_OVER -3
#define MODBUS_RECV_ERR_DATA_LEN -4
#define MODBUS_RECV_ERR_CRC_ERROR -5


#define RET_SIZE_OVER -1
#define RET_TIMEOUT -2
#define FRAME_485_Q_CNT 1

#define MODBUS_485_RECV_FLUSH(driver) drv_rs485_flush_rx(driver)
#define MODBUS_485_SEND(driver, data, cnt) drv_rs485_send(driver, data, cnt);
#define MODBUS_485_RECV(driver, buff, buffSize, tout) drv_rs485_recv(driver, buff, buffSize, tout)
#define MODBUS_485_RECV_TIMEOUT(driver, buff, buffSize, tout1, tout2) \
  drv_rs485_recv_opt(driver, buff, buffSize, tout1, tout2)

#define MODBUS_232_RECV_FLUSH(driver) drv_uart_flush_rx(driver)
#define MODBUS_232_SEND(driver, data, cnt) drv_uart_send(driver, data, cnt);
#define MODBUS_232_RECV(driver, buff, buffSize, tout) drv_uart_recv(driver, buff, buffSize, tout)
#define MODBUS_232_RECV_TIMEOUT(driver, buff, buffSize, tout1, tout2) \
  drv_uart_recv_opt(driver, buff, buffSize, tout1, tout2)



void *modbus_sem;

typedef struct _send_data
{
  uint8_t data[MODBUS_REG_SIZE*2];
  uint16_t cnt;
} modbus_data_t;



void send_query(modbus_h_t *drv, modbus_t *pmodbus);

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
  
  uint8_t byte_count = pInData[2];  // 바이트 수
  uint8_t u8byte = 3;  // 데이터 시작 위치
  
  if (byte_count > regCnt * 2)  // 안전성 검사
  {
    return;
  }
  
  // 코일 데이터는 비트 단위로 패킹되어 있음
  // 각 바이트는 8개의 코일 상태를 포함
  for (uint8_t i = 0; i < byte_count; i++)
  {
    uint8_t coil_byte = pInData[u8byte + i];
    
    // 각 바이트의 8개 비트를 개별 레지스터에 저장
    for (uint8_t bit = 0; bit < 8; bit++)
    {
      uint16_t coil_index = i * 8 + bit;
      if (coil_index < regCnt)
      {
        regs[coil_index] = (coil_byte & (1 << bit)) ? 1 : 0;
      }
    }
  }
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




int32_t modbus_receive_packet(modbus_h_t *drv, uint8_t *rx_buf, uint16_t buf_size)
{
  uint8_t func_code;
  uint8_t data_len;
  int32_t ret;
  uint16_t total_len;
  uint16_t crc_calc;
  uint16_t crc_recv;

  uint32_t delay;

  //  우선 주소 + 기능 + 최소 1바이트 데이터 수신
  if (drv->modebus_type == eMODBUS_RS232)
  {
    ret = MODBUS_232_RECV(drv->port_num, rx_buf, 3, MODBUS_START_TIMEOUT_MS);
  }
  else
  {
    ret = MODBUS_485_RECV(drv->port_num, rx_buf, 3, MODBUS_START_TIMEOUT_MS);
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
  switch (drv->modebus_type)
  {
    case eMODBUS_RS485:
    {
      uart_config_t ucfg;
      drv_rs485_get(drv->port_num, eUART_GET_CONFIG, &ucfg);
      delay = (uint32_t)(((float)1 / (float)ucfg.baud) * 10 * 3.5 * 1000);  // ms
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
      drv_uart_get(drv->port_num, eUART_GET_CONFIG, &ucfg);
      delay = (uint32_t)(((float)1 / (float)ucfg.baud) * 10 * 3.5 * 1000);  // ms
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
  if (drv->modebus_type == eMODBUS_RS232)
  {
    ret = MODBUS_232_RECV_TIMEOUT(drv->port_num, &rx_buf[3], data_len + 2, delay, delay);
  }
  else
  {
    ret = MODBUS_485_RECV_TIMEOUT(drv->port_num, &rx_buf[3], data_len + 2, delay, delay);
  }


    if (ret != data_len + 2)
      return -4;

    // CRC 확인
    crc_calc = calcCRC(rx_buf, total_len - 2);
    crc_recv = rx_buf[total_len - 2] << 8 | (rx_buf[total_len - 1]);
    if (crc_calc != crc_recv)
    {
      ERROR_PRINT("\r\n");
      for(int i = 0 ; i< total_len; i++)
      {
        ERROR_PRINT("%02X ",rx_buf[i]);
      }
      ERROR_PRINT("\r\n");
      return -5;  // CRC 에러
    }

    return total_len;  // 유효한 패킷 길이 리턴
  }


  eMODBUS_RESULT_t modbus_master_req(modbus_h_t *drv, modbus_t *modbus)
  {
    uint8_t buff[MODBUS_REG_SIZE*2+20];
    int32_t len;
    eMODBUS_RESULT_t result = eMODBUS_FAIL;

    OS_PEND_SEM(modbus_sem,osWaitForever);


    memset(buff, 0, sizeof(buff));
    send_query(drv, modbus);


    len = modbus_receive_packet(drv, buff, sizeof(buff));

    if (len > 0)
    {
      if (parse_recv(buff, len, modbus->regs, modbus->regsCnt) == 0)
      {
        result = eMODBUS_OK;
      }
      else
      {

        result = eMODBUS_PARSE_FAIL;
      }
    }
    else if(len == -1)
    {
      result = eMODBUS_TIMEOUT;
    }
      else if(len == -3)
      {

        result = eMODBUS_RECV_ERR_BUFF_SIZE_OVER;
      }
      else if(len == -4)
      {
        result = eMODBUS_RECV_ERR_DATA_LEN;
      }
      else if(len == -5)
      {
        result =  eMODBUS_RECV_ERR_CRC_ERROR;
      }
      else if(len == -99)
      {
        result =  eMODBUS_RECV_ERR_NOT_SUPPORTED;
      }

    OS_POST_SEM(modbus_sem);
    return result;
  }

/**
 * @brief 특정 주소에 값 쓰기
 * @param drv 모드버스 드라이버
 * @param address
 * @param val
 * @retval
 */
eMODBUS_RESULT_t modbus_write_holding_reg(modbus_h_t *drv, uint16_t address, uint16_t val)
{
  modbus_t modbus;
  uint16_t reg[10];
  eMODBUS_RESULT_t mb_ret;

  modbus.id = drv->id;
  modbus.fc = MB_FC_WRITE_REGISTER;
  modbus.regAdd = address;
  modbus.coilsNo = 1;
  modbus.regs = &reg[0];
  modbus.regsCnt = sizeof(reg) / sizeof(reg[0]);
  modbus.wait_ms = MODBUS_REQ_TIMEOUT_MS;

  modbus.reg[0] = val;  // 시작

  mb_ret = modbus_master_req(drv, &modbus);

  return mb_ret;
}

/**
 * @brief 특정 주소에 단일 코일 쓰기
 * @param drv 모드버스 드라이버
 * @param address 코일 주소
 * @param val 코일 값 (false: OFF, true: ON)
 * @retval RET_OK: 성공, RET_FAIL: 실패
 */
eMODBUS_RESULT_t modbus_write_single_coil(modbus_h_t *drv, uint16_t address, bool val)
{
  modbus_t modbus;
  uint16_t reg[10];
  eMODBUS_RESULT_t mb_ret;

  modbus.id = drv->id;
  modbus.fc = MB_FC_WRITE_COIL;
  modbus.regAdd = address;
  modbus.coilsNo = 1;
  modbus.regs = &reg[0];
  modbus.regsCnt = sizeof(reg) / sizeof(reg[0]);
  modbus.wait_ms = MODBUS_REQ_TIMEOUT_MS;

  modbus.reg[0] = val ? 1 : 0; // 코일 값 수정

  mb_ret = modbus_master_req(drv, &modbus);

  return mb_ret;
}

/**
 * @brief 특정 주소의 단일 코일 읽기
 * @param drv 모드버스 드라이버
 * @param address 코일 주소
 * @param pOutCoil 읽은 코일 값 포인터 (0: OFF, 1: ON)
 * @retval RET_OK: 성공, RET_FAIL: 실패
 */
eMODBUS_RESULT_t modbus_read_single_coil(modbus_h_t *drv, uint16_t address, uint16_t *pOutCoil)
{
  modbus_t modbus;
  uint16_t reg[10];
  eMODBUS_RESULT_t mb_ret;

  if (pOutCoil == NULL)
  {
    return eMODBUS_HANDLE_ERROR;
  }

  memset(reg, 0, sizeof(reg));

  modbus.id = drv->id;
  modbus.fc = MB_FC_READ_COILS;
  modbus.regAdd = address;
  modbus.coilsNo = 1;  // 단일 코일
  modbus.regs = &reg[0];
  modbus.regsCnt = sizeof(reg) / sizeof(reg[0]);
  modbus.wait_ms = MODBUS_REQ_TIMEOUT_MS;

  mb_ret = modbus_master_req(drv, &modbus);

  if (mb_ret == eMODBUS_OK)
  {
    *pOutCoil = modbus.regs[0];  // 첫 번째 코일 값
  }

  return mb_ret;
}

/**
 * @brief 이산 입력(Discrete Inputs) 읽기
 * @param drv 모드버스 드라이버
 * @param address 시작 주소
 * @param pOutInputs 읽은 이산 입력 값들을 저장할 배열 포인터
 * @param inputCnt 읽을 이산 입력 개수
 * @retval RET_OK: 성공, RET_FAIL: 실패
 */
eMODBUS_RESULT_t modbus_read_discrete_inputs(modbus_h_t *drv, uint16_t address, uint16_t *pOutInputs, uint16_t inputCnt)
{
  modbus_t modbus;
  uint16_t reg[MODBUS_REG_SIZE];
  eMODBUS_RESULT_t mb_ret = eMODBUS_FAIL;


  if (pOutInputs == NULL)
  {
    return eMODBUS_HANDLE_ERROR;
  }

  memset(reg, 0, sizeof(reg));
  
  if ((sizeof(reg) / sizeof(reg[0])) < inputCnt)
  {
    return eMODBUS_PARAM_ERROR;
  }

  modbus.id = drv->id;
  modbus.fc = MB_FC_READ_DISCRETE_INPUT;
  modbus.regAdd = address;
  modbus.coilsNo = inputCnt;
  modbus.regs = &reg[0];
  modbus.regsCnt = sizeof(reg) / sizeof(reg[0]);
  modbus.wait_ms = MODBUS_REQ_TIMEOUT_MS;

  mb_ret = modbus_master_req(drv, &modbus);

  if (mb_ret == eMODBUS_OK)
  {
    for (int i = 0; i < inputCnt; i++)
    {
      pOutInputs[i] = modbus.regs[i];
    }
  }

  return mb_ret;
}

eMODBUS_RESULT_t modbus_write_multi_reg(modbus_h_t *drv, uint16_t address, uint16_t *regs,
                               uint16_t regCnt)
{
  modbus_t modbus;
  uint16_t reg[MODBUS_REG_SIZE];
  eMODBUS_RESULT_t mb_ret = eMODBUS_FAIL;

  if ((sizeof(reg) / sizeof(reg[0])) < regCnt)
  {
    return eMODBUS_PARAM_ERROR;
  }

  modbus.id = drv->id;
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

  mb_ret = modbus_master_req(drv, &modbus) ;

  return mb_ret;
}

eMODBUS_RESULT_t modbus_read_hold_reg(modbus_h_t *drv, uint16_t address, uint16_t *pOutRegs,
                              uint16_t regCnt)
{
  modbus_t modbus;
  uint16_t reg[MODBUS_REG_SIZE];
  eMODBUS_RESULT_t mb_ret = eMODBUS_FAIL;


  if ((sizeof(reg) / sizeof(reg[0])) < regCnt)
  {
    return eMODBUS_PARAM_ERROR;
  }



  modbus.id = drv->id;
  modbus.fc = MB_FC_READ_REGISTERS;
  modbus.regAdd = address;
  modbus.coilsNo = regCnt;
  modbus.regs = &reg[0];
  modbus.regsCnt = sizeof(reg) / sizeof(reg[0]);
  modbus.wait_ms = MODBUS_REQ_TIMEOUT_MS;

  mb_ret = modbus_master_req(drv, &modbus);

  if (mb_ret == eMODBUS_OK)
  {
    for (int i = 0; i < regCnt; i++)
    {
      pOutRegs[i] = modbus.regs[i];
    }
  }


  return mb_ret;
}

eMODBUS_RESULT_t modbus_read_input_reg(modbus_h_t *drv, uint16_t address, uint16_t *pOutRegs,
                             uint16_t regCnt)
{
  modbus_t modbus;
  uint16_t reg[MODBUS_REG_SIZE];
  eMODBUS_RESULT_t mb_ret = eMODBUS_FAIL;

  
  memset(reg,0,sizeof(reg));
  if ((sizeof(reg) / sizeof(reg[0])) < regCnt)
  {
    return eMODBUS_PARAM_ERROR;
  }
  
  modbus.id = drv->id;
  modbus.fc = MB_FC_READ_INPUT_REGISTER;
  modbus.regAdd = address;
  modbus.coilsNo = regCnt;
  modbus.regs = &reg[0];
  modbus.regsCnt = sizeof(reg) / sizeof(reg[0]);
  modbus.wait_ms = MODBUS_REQ_TIMEOUT_MS;

  mb_ret = modbus_master_req(drv, &modbus);

  if (mb_ret == eMODBUS_OK)
  {
    for (int i = 0; i < regCnt; i++)
    {
      pOutRegs[i] = modbus.regs[i];
    }
  }


  return mb_ret;
}
void send_query(modbus_h_t *drv, modbus_t *pmodbus)
{
  uint8_t regsno;
  uint8_t bytesno;
  uint16_t cnt = 0;
  uint16_t crc;
  modbus_data_t g_buff;


  g_buff.data[cnt++] = pmodbus->id;  //TODO: 전역변수 선언하고 이문장 실행하면
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

    if (drv->modebus_type == eMODBUS_RS232)
    {
      MODBUS_232_RECV_FLUSH(drv->port_num);
      MODBUS_232_SEND(drv->port_num, g_buff.data, cnt);
    }
    else
    {
      MODBUS_485_RECV_FLUSH(drv->port_num);
      MODBUS_485_SEND(drv->port_num, g_buff.data, cnt);
    }

  }
  g_buff.cnt = cnt;
}



void modbus_init(void)
{
  if (modbus_sem ==NULL)
    OS_CREATE_BINARY_SEM(modbus_sem);
}
#define RET_OK 0
#define RET_TIME_OUT 1
#define RET_OVER 2
#define RET_INVAILD 3
#define RET_UNKNOWN 4
#define RET_UNKNOWN_VAL 5
#define RET_FAIL 6
#define RET_ID_FAIL 7

const char *get_modbus_err_string(eMODBUS_RESULT_t err)
{
  switch (err)
  {
    case eMODBUS_OK:
      return "eMODBUS_OK";
    case eMODBUS_HANDLE_ERROR:
      return "eMODBUS_HANDLE_ERROR";
    case eMODBUS_PARAM_ERROR:
      return "eMODBUS_PARAM_ERROR";
    case eMODBUS_TIMEOUT:
      return "eMODBUS_TIMEOUT";
    case eMODBUS_CRC_ERROR:
      return "eMODBUS_CRC_ERROR";
    case eMODBUS_EXCEPTION:
      return "eMODBUS_EXCEPTION";
    case eMODBUS_FAIL:
      return "eMODBUS_FAIL";
    case eMODBUS_LEN_ZERO:
      return "eMODBUS_LEN_ZERO";
    case eMODBUS_PARSE_FAIL:
      return "eMODBUS_PARSE_FAIL";
    case eMODBUS_RECV_ERR_3BYTE_TIMEOUT:
      return "eMODBUS_RECV_ERR_3BYTE_TIMEOUT";
    case eMODBUS_RECV_ERR_NOT_SUPPORTED:
      return "eMODBUS_RECV_ERR_NOT_SUPPORTED";
    case eMODBUS_RECV_ERR_BUFF_SIZE_OVER:
      return "eMODBUS_RECV_ERR_BUFF_SIZE_OVER";
    case eMODBUS_RECV_ERR_DATA_LEN:
      return "eMODBUS_RECV_ERR_DATA_LEN";
    case eMODBUS_RECV_ERR_CRC_ERROR:
      return "eMODBUS_RECV_ERR_CRC_ERROR";
    default:
      return "UNKNOWN";
  }
}