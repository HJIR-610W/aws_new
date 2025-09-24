

#include <string.h>

#include "hjsmartCharger.h"
#include "util_time.h"
#include "util_memory.h"
#include "os_user_def.h"
#include "bsp_rs485.h"
#include "bsp_uart.h"
#include "bsp_rs485.h"
#include "driver_uart_def.h"



typedef struct
{
    uint16_t SolraVolt1;        // nAIN_SV1
    uint16_t SolraCurr1;        // nAIN_SC1
    uint16_t BattVolt1;         // nAIN_BV1
    uint16_t LoadCurr1;         // nAIN_L1C
    uint16_t LoadCurr2;         // nAIN_L2C
    uint16_t LoadCurr3;         // nAIN_L3C
    uint16_t SystemCurr;        // nAIN_SYSC
    uint16_t SolraVolt2;        // nAIN_SV2
    uint16_t SolraCurr2;        // nAIN_SC2
    uint16_t BattVolt2;         // nAIN_BV2
    uint16_t LoadVolt12;        // nAIN_LV12
    uint16_t AdcDummy1;

    // 연산된 데이터
    uint16_t SrcPower1;         // 0.1W step
    uint16_t SrcPower2;         // 0.1W step
    uint16_t BattVoltN;         // 배터리 합산 전압
    uint16_t LoadCurrN;         // 24V 기준으로 환산한 전류
    uint16_t LoadPower;         // BattVoltN * LoadCurrN
    uint16_t AdcDummy2;
    uint16_t BatAvgVolt1;       // 배터리1 평균 전압
    uint16_t BatAvgVolt2;       // 배터리2 평균 전압
                                // eTempSens_t 순서로 배치
    int16_t RoomTemp;       // 25   -> 25`C
    int16_t Humidity;       // 30       -> 30%
    int16_t ChgTemp1;       // 385  -> 38.5`C,  -102 -> -10.2`C
    int16_t ChgTemp2;
    int16_t BattTemp1;
    int16_t BattTemp2;
    // Charger Status
    uint8_t ChgStage1;  // eChgStat_t
    uint8_t ChgStage2;  // eChgStat_t
    uint8_t AcPwStat1;  // 0:AcOk, 1:AcAlarm, 2:AcFault(not used)
    uint8_t AcPwStat2;  // 0:AcOk, 1:AcAlarm, 2:AcFault(not used)
                        // Load Status
    uint8_t LoadStat1;  // 현재 Load 상태  0:off, 1:on
    uint8_t LoadStat2;
    uint8_t LoadStat3;
    uint8_t LoadStat4;
    uint8_t ComPingCnt[4];      // 핑신호가 들어온 횟수
    uint8_t LoadTogCnt[4];      // 로드 토글 횟수
    uint32_t LoadTogTime[4];    // 로드를 토글한 시간
    uint32_t SysResetTime;      // 시스템 파워 온 시간
    uint8_t ExtPortInp;             // B1:ACPW2Mode, B0:ACPW1Mode
    uint8_t ExtPortOut;             // B1:AcAlarm2, B0:AcAlarm1
    uint8_t ErrLedStat;             // V1006 // B4:ERRLED_BIT_ACPW, B3:ERRLED_BIT_FCSHDN, B2:ERRLED_BIT_BTEMP, B1:ERRLED_BIT_CTEMP, B0:ERRLED_BIT_OVCHG
    uint8_t ExtPortDummy2;
    int8_t MstMcuInit;              // TFTMCU 가 초기화한 상태
    uint8_t MstMcuConnect;      // TFTMCU 연결상태
    uint8_t MstPwOffStat;           // SLVMCU LCD ON,OFF상태
    uint8_t MstDummy1;
}SYSTEM_TypeDef;

typedef struct hj_smartcharger_instance_s
{
  int32_t uart_num;
  bool opened;
  void *sem;
} hj_smartcharger_instance_t;

hj_smartcharger_instance_t charger_inst;
SYSTEM_TypeDef chg_system;                        // TODO:heap으로 변경
static uint8_t buff[sizeof(SYSTEM_TypeDef) + 20]; // TODO:heap으로 변경

uint32_t make_charger_frame(uint8_t *pBuff, uint32_t buff_size, uint8_t cmd, uint8_t *pData, uint32_t dataLen)
{
  static uint8_t seq = 0;
  uint8_t sum = 0;
  uint16_t frameLen;
  uint32_t cnt = 0;
  uint32_t i;
  DATE_TIME_BUF curTime=Date_Time;

  frameLen = 12 + 2 + dataLen;


	pBuff[cnt++] = 0x02;
	memcpy(&pBuff[cnt], &frameLen, sizeof(frameLen));
	cnt += sizeof(frameLen);

	pBuff[cnt++] = ++seq;
	pBuff[cnt++] = 0xFF;
	pBuff[cnt++] = curTime.Year%100;
	pBuff[cnt++] = curTime.Month;
	pBuff[cnt++] = curTime.Day;
	pBuff[cnt++] = curTime.Hour;
	pBuff[cnt++] = curTime.Min;
	pBuff[cnt++] = curTime.Sec;
	pBuff[cnt++] = cmd;

  if(pData)
  {
	  memcpy(&pBuff[cnt], pData, dataLen);
  }

  cnt += dataLen;

	for (i = 0; i < (dataLen + 11); i++)
	{
		sum += pBuff[1 + i];
	}

	pBuff[cnt++] = 0x03;
	pBuff[cnt++] = sum;

	return cnt;

}

/**
 * @brief 충전기 프레임 수집
 * 
 */
int32_t recv_charger_frame(int32_t rs232_num,uint8_t *pbuff,int32_t buff_size)
{
	uint8_t rx_data;
	uint8_t sum=0;
	uint16_t frame_cnt=0;
	uint16_t frame_len=0;


    while (bsp_rs485_recv(rs232_num, &rx_data, 1, 100) == 1)
    {
			pbuff[frame_cnt++] = rx_data;

			if (frame_cnt == 1)
			{
				if (rx_data != 2)
				{
					frame_cnt=0;
				}
			}
			else if(frame_cnt==3)
			{
        memcpy(&frame_len,&pbuff[1],sizeof(frame_len));
			}
			else if(frame_cnt == frame_len)
			{
				sum = make_sum(&pbuff[1],frame_len-3);

				if(sum ==pbuff[frame_len-1])
				{
					return frame_cnt;
				}
				else
					frame_cnt=0;
			}

			if(frame_cnt >= buff_size)
			{
				return 0;
			}
		}



	return -1;
}



void hjsmartCharger_read(charger_data_t *charger_data,uint8_t *err)
{
  int32_t len;
  uint8_t data[6];
  uint16_t val;

  OS_PEND_SEM(charger_inst.sem, osWaitForever);


  data[0] = 1;  // 의미없음
  data[1] = 10; // 상태읽기

  val = 0;
  memcpy(&data[2],&val,2);
  val = 22;  //22바이트만 읽어옴
  memcpy(&data[4],&val,2);
  
  len = make_charger_frame(buff,sizeof(buff),0x50,data,6);

  bsp_rs485_flush_rx(charger_inst.uart_num);
  bsp_rs485_send(charger_inst.uart_num, buff, len);

  len = recv_charger_frame(charger_inst.uart_num, buff, sizeof(buff));

  if (len > 0)
  {
    memcpy(&chg_system, &buff[13], sizeof(SYSTEM_TypeDef));
    charger_data->battery1 = (float)chg_system.BattVolt1 / 1000.0f;
    charger_data->battery2 = (float)chg_system.BattVolt2 / 1000.0f;
    charger_data->load1Current = (float)chg_system.LoadCurr1 / 1000.0f;
    charger_data->load2Current = (float)chg_system.LoadCurr2 / 1000.0f;
    charger_data->load3Current = (float)chg_system.LoadCurr3 / 1000.0f;

    charger_data->solar1Volt = (float)chg_system.SolraVolt1 / 1000.0f;
    charger_data->solar2Volt = (float)chg_system.SolraVolt2 / 1000.0f;
    charger_data->solar1Current = (float)chg_system.SolraCurr1 / 1000.0f;
    charger_data->solar2Current = (float)chg_system.SolraCurr2 / 1000.0f;
    *err = DRV_ERR_NONE;
  }
  else
  {
    *err = DRV_ERR_TIMEOUT;
  }

  OS_POST_SEM(charger_inst.sem);
}



int32_t hj_smartcharger_init(void)
{
  uart_config_t uart_config;

  if (charger_inst.opened)
  {
    return 1;
  }

  uart_config.baud = 57600;
  uart_config.dataLen = UART_DATA_LEN_8;
  uart_config.parity_index = PARITY_NONE;
  uart_config.stop_bit = UART_STOP_BIT_1;

  charger_inst.uart_num = BSP_RS485_D;
  bsp_rs485_init(charger_inst.uart_num, &uart_config);

  charger_inst.opened = true;
  OS_CREATE_BINARY_SEM(charger_inst.sem);

  return 1;
}