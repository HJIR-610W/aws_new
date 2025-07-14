



#include <string.h>

#include "cmsis_os2.h"

#include "pcb_define.h"
#include "hjsmartCharger.h"
#include "driver_uart.h"
#include "util_time.h"
#include "util_memory.h"
#include "os_user_def.h"


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



void  hjsmartCharger_read(driver_t *chg,charger_data_t *data,uint8_t *err);


charger_api_t hjcharger_api ={.read = hjsmartCharger_read};

typedef struct hjsmartCharger_cfg_s
{
  driver_t *rs232_io;
}hjsmartCharger_cfg_t;

driver_t hjsmartCharger_driver;
hjsmartCharger_cfg_t hjsmartCharger_cfg;

driver_t *hjsmartCharger_open(int32_t num,void *opt)
{
  uart_config_t uart_config;

  if(hjsmartCharger_driver.opened)
  {
    return &hjsmartCharger_driver;
  }


    uart_config.baud = 57600;
    uart_config.dataLen   = 8;
    uart_config.parityIdx = 0;
    uart_config.stop_bit  = 1;

    hjsmartCharger_driver.opened = true;
    hjsmartCharger_driver.api = &hjcharger_api;
    hjsmartCharger_cfg.rs232_io = driver_uart_open(UART_2_EXT_A,&uart_config);

    hjsmartCharger_driver.cfg = &hjsmartCharger_cfg;

  OS_CREATE_BINARY_SEM(hjsmartCharger_driver.sem);


  return &hjsmartCharger_driver;
}


uint32_t Make_SmartChgFrame(uint8_t *pBuff, uint32_t buffSize, uint8_t cmd, uint8_t *pData, uint32_t dataLen)
{
	uint32_t cnt = 0;
	uint16_t frameLen;
	static uint8_t seq = 0;
	frameLen = 12 + 2 + dataLen;
	DATE_TIME_BUF curTime;
	uint8_t sum = 0;
	uint32_t i;


	time_get(&curTime);

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


int32_t recv_smartCharger(void *rs232_driver,uint8_t *pbuff,int32_t buffSize)
{
	uint8_t rxData;
	uint8_t sum=0;
	uint16_t frameCnt=0;
	uint16_t packetLen=0;
	uint32_t startTime;


	startTime = HAL_GetTick();

	while(1)
  {
		while(driver_uart_recv(rs232_driver, &rxData,1,20)==1)
		{
			pbuff[frameCnt++] = rxData;

			if (frameCnt == 1)
			{
				if (rxData != 2)
				{
					frameCnt=0;
				}
			}
			else if(frameCnt==3)
			{
                memcpy(&packetLen,&pbuff[1],sizeof(packetLen));
			}
			else if(frameCnt == packetLen)
			{
				sum = make_sum(&pbuff[1],packetLen-3);

				if(sum ==pbuff[packetLen-1])
				{
					return frameCnt;
				}
				else
					frameCnt=0;
			}

			if(frameCnt >= buffSize)
			{
				return 0;
			}
		}

    if((HAL_GetTick() -startTime) > 50)
    {
      break;
    }
	}

	return -1;
}

SYSTEM_TypeDef chg_system;//TODO:heap으로 변경
static uint8_t buff[sizeof(SYSTEM_TypeDef)+20]; //TODO:heap으로 변경
void hjsmartCharger_read(driver_t *chg,charger_data_t *charger_data,uint8_t *err)
{
  hjsmartCharger_cfg_t *cfg = chg->cfg;
  int32_t len;
  uint8_t data[6];
  uint16_t val;


  data[0] = 1;  // 의미없음
  data[1] = 10; // 상태읽기

  val = 0;
  memcpy(&data[2],&val,2);
  val = 22;  //22바이트만 읽어옴
  memcpy(&data[4],&val,2);
  
  len = Make_SmartChgFrame(buff,sizeof(buff),0x50,data,6);


    OS_PEND_SEM(chg->sem,osWaitForever);

  
  driver_uart_flush_rx(cfg->rs232_io);
  driver_uart_send(cfg->rs232_io,buff,len);

  len = recv_smartCharger(cfg->rs232_io,buff,sizeof(buff));

  if(len > 0)
  {
    memcpy(&chg_system,&buff[13],sizeof(SYSTEM_TypeDef));
    charger_data->battery1     = (float)chg_system.BattVolt1/1000.0f;
    charger_data->battery2     = (float)chg_system.BattVolt2/1000.0f;
    charger_data->load1Current = (float)chg_system.LoadCurr1/1000.0f;
    charger_data->load2Current = (float)chg_system.LoadCurr2/1000.0f;
    charger_data->load3Current = (float)chg_system.LoadCurr3/1000.0f;

    charger_data->solar1Volt    = (float)chg_system.SolraVolt1/1000.0f;
    charger_data->solar2Volt    = (float)chg_system.SolraVolt2/1000.0f;
    charger_data->solar1Current = (float)chg_system.SolraCurr1/1000.0f;
    charger_data->solar2Current = (float)chg_system.SolraCurr2/1000.0f;
    *err = DRV_ERR_NONE;
  }
  else
  {
    *err = DRV_ERR_TIMEOUT;
  } 

  OS_POST_SEM(chg->sem);

}