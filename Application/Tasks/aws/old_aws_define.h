

#ifndef OLD_AWS_DEFINE_H
#define OLD_AWS_DEFINE_H

#include <stdint.h>

typedef struct
{
  uint16_t sReal;
  uint16_t sMin;
  uint16_t sMax;
  uint16_t sSpec;
} SENSOR_RIXS_BUF;

typedef struct
{
  uint16_t sReal;
  uint16_t sMin;
  uint16_t sMax;
} SENSOR_RIX_BUF;

typedef struct
{
  uint16_t sReal;
  uint16_t sMax;
} SENSOR_RX_BUF;

typedef struct
{
  uint16_t sReal;
} SENSOR_R_BUF;

typedef struct
{
  SENSOR_RX_BUF mDirection;  // 풍향
  SENSOR_RX_BUF mSpeed;      // 풍속
} SENSOR_WIND_BUF;

typedef struct
{
  uint8_t cMonth;
  uint8_t cDay;
  uint8_t cHour;
  uint8_t cMin;
} LOG_DATE_BUF;

typedef struct
{  //
  LOG_DATE_BUF mDate;
  SENSOR_WIND_BUF mWind;
  SENSOR_RIX_BUF mTemperature;   // 온도
  SENSOR_RIXS_BUF mRainFall;     // 강우
  SENSOR_R_BUF mRainDetect;      // 강우감지
  SENSOR_RIX_BUF mBarometric;    // 기압
  SENSOR_RIX_BUF mHumidity;      // 습도
  SENSOR_RX_BUF mSolarRad;       // 일사
  SENSOR_RX_BUF mSunshine;       // 일조
  SENSOR_R_BUF mSnowFall;        // 적설
  SENSOR_RIX_BUF mGndTemp;       // 지면온도
  SENSOR_RIX_BUF mGrassTemp;     // 초상온도
  SENSOR_RIX_BUF mSoilTemp5cm;   // 지중온도  5cm
  SENSOR_RIX_BUF mSoilTemp10cm;  // 지중온도 10cm
  SENSOR_RIX_BUF mSoilTemp20cm;  // 지중온도 20cm
  SENSOR_RIX_BUF mSoilTemp30cm;  // 지중온도 30cm
  SENSOR_RIX_BUF mSoilTemp50cm;  // 지중온도 50cm
  SENSOR_RIX_BUF mSoilTemp1_0m;  // 지중온도   1m
  SENSOR_RIX_BUF mSoilTemp1_5m;  // 지중온도 1.5m
  SENSOR_RIX_BUF mSoilTemp3_0m;  // 지중온도 3.0m
  SENSOR_RIX_BUF mSoilTemp5_0m;  // 지중온도 5.0m

  SENSOR_RIX_BUF mStatus;
// sReal
#define DCFAIL_BIT 0x0001
#define BATTERYFAIL_BIT 0x0002
#define AC110V_BIT 0x0000  //     2 3:AC 전압    --> 00:110 V, 01:220V, 11:AC Off
#define AC220V_BIT 0x0004
#define ACOFF_BIT 0x000C
#define LOGGERDOOR_BIT 0x0010  //     4 : 로거잠금상태 : 0 : 닫힘 , 1 : 열림
// sMin
#define WINDSPEEDFAIL_BIT 0x0001
#define WINDDIRECFAIL_BIT 0x0002
#define TEMPERATUREFAIL_BIT 0x0004
#define RAINDETECTFAIL_BIT 0x0008
#define RAINFALLFAIL_BIT 0x0010
#define HUMIDITYFAIL_BIT 0x0020
#define BAROMETRICFAIL_BIT 0x0040
#define FANFAIL_BIT 0x0080
// sMax
#define RAINFAIL_BIT 0x0001

} AWS_DATA_STRUCT;

typedef struct
{
  uint16_t sGustDircMax;   // 1분 0.25초 풍속 최대일때의 풍향
  uint16_t sGustSpeedMax;  // 1분 0.25초 풍속 최대
  float uTot;
  float vTot;
  uint64_t lSpeedTot;
  uint16_t sAddCnt;

} SENSORWIND_BUF;
typedef struct
{
  uint32_t nMonthRain;      // 월간강수량
  uint32_t nYearRain;       // 년간강수량
  uint32_t nYearSunshine;   // 연간 일조량
  uint32_t nMonthSunshine;  // 월간 일조량

} NONVOLATILE_BUF;

typedef struct
{
  uint16_t sAvg3Speed[12];      //
  uint16_t sAvg3Direction[12];  //

  uint16_t sAvg10Speed[40];      //
  uint16_t sAvg10Direction[40];  //
  uint16_t sWrFlag[40];

  uint16_t sChangeInCnt;  // Dual Port Ram에서 들어온 Data를 System Memory로
                          // 옮기는 카운트
} SENSORWIND_REAL;

typedef struct
{
  uint16_t sMax;  // 최고
  uint16_t sMin;  // 최소
  uint64_t lTot;  // 평균을 구하기 위한 변수
  uint16_t sAddCnt;
} SENSORPROC_BUF;

typedef struct
{
  // 일간 강수량을 매 0시에 Clear하고 그후에는 일간 강수량을 계속 증가 시킨다
  uint16_t sMinRain;     // 1분 강수량
  uint16_t s10MinRain;   // 10분 강수량
  uint16_t sHourRain;    // 1시간 강수량
  uint16_t sDayRain;     // 일간 강수량
  uint16_t sBefDayRain;  // 전일 강수량

  uint16_t sDayCount;       // Mega에서 올라온 값
  uint16_t sDayCountOld;    // 임시 저장 값
  uint16_t sDayCountFlag;   // 일간 강우량을 새로운 값으로 써넣을시 Dual Port의 동기화
  uint16_t sDayCountValue;  // 써넣어야할 값

} SENSORRAIN_BUF;

typedef struct
{
  uint32_t nSolarTot;  // 평균을 구하기 위한 변수
  uint32_t nSunshineTot;
  uint16_t sAddCnt;
} SENSORSUN_BUF;

typedef struct
{
  uint16_t sSolarVoltage;    // 태양전지 또는 충전 전압
  uint16_t sSolarCurrent;    // 충전 전류
  uint16_t sBatteryVoltage;  // 밧데리 전압
  uint16_t sLoad1Current;    // 부하전류 1(시스템)
  uint16_t sLoad2Current;    // 부하전류 2 기타

} POWERMAN_BUF;

typedef struct
{
  uint16_t sVert;
  uint16_t sHoriz;
} DISPLAYPAGE_BUF;

typedef struct
{
  NONVOLATILE_BUF mNVram;
  SENSORWIND_REAL mRealWind;  // Dual Port Ram에서 들어온 Data
  SENSORWIND_BUF mWind[3];  // 0: 분 , 1: 10분 , 2: 1시간
  SENSORPROC_BUF mTempBuf[3];   // 0: 분 , 1: 10분 , 2: 1시간
  SENSORPROC_BUF mBaroBuf[3];   // 0: 분 , 1: 10분 , 2: 1시간
  SENSORPROC_BUF mHumidBuf[3];  // 0: 분 , 1: 10분 , 2: 1시간

  SENSORRAIN_BUF mRain;
  SENSORSUN_BUF mSun[3];  // 0: 분 , 1: 10분 , 2: 1시간
  SENSORPROC_BUF mGndBuf[3];
  SENSORPROC_BUF mGrassBuf[3];
  SENSORPROC_BUF mSoil5Buf[3];
  SENSORPROC_BUF mSoil10Buf[3];
  SENSORPROC_BUF mSoil20Buf[3];
  SENSORPROC_BUF mSoil30Buf[3];
  SENSORPROC_BUF mSoil50Buf[3];
  SENSORPROC_BUF mSoil100Buf[3];
  SENSORPROC_BUF mSoil150Buf[3];
  uint16_t PnlLedStatus;  // 2:Spare2, 1:Spare1, 0:TcpConnect Led
#define TCPCONNET_LEDBIT 0x0001
#define DOOROPEN_LEDBIT 0x0002
#define SDFAIL_LEDBIT 0x0004

  uint16_t sDiStatus;
  uint16_t sDoStatus;
  uint8_t FatOk;  // SD CARD 상태 1: OK , 0: Fail

  uint8_t cKeyLCDFlag;
  uint64_t lKeyOnLCDTimer;

  uint16_t sCpuStat;  // IO CPU 상태 (0 - 50 roundrobin)
  uint16_t sOldCpuStat;

  uint8_t cTcpipStat;      // Tcpip Ethernet 상태 		0:Normal 1:Connect
  uint8_t cDoorStat;       // DOOR 상태 				0:Open	 1:Close
  uint8_t cACStat;         // AC 상태 					1:Normal 0:Fail
  uint8_t cTTSPacketExch;  // 0:Normal,   1:Runnig

  uint8_t cTcpRTxCnt;     // TCP/IP Packet Count
  uint8_t cTcpRTxCntOld;  // TCP/IP Packet Count
  uint8_t cSerRTxCnt;     // Serial Packet Count(CDMA)
  uint8_t cRfRTxCnt;      // Serial Packet Count(VHF RF)
  uint8_t cCdmaStat;
  uint8_t cCDFlag;         // CDMA  CD Timeout Flag
  uint8_t cModemInitFlag;  // Modem Init Flag			0:Deactive
                           // 1:Active
  uint64_t lCDOldTimer;
  uint32_t CpuTimeCount;

  uint16_t shSnowFallOld;  // 10분 누적 적설량을 구하기위한 10분전 적설(실적설)
                           //		uint16_t	        CDMAHookTmr;

  //		uint8_t			cMegaErrCnt[10];                // Error
  // Count를 10초 이상일 경우 9999로 설정한다(Mega640의 리셋시 대응 하기위함)
  uint8_t cMegaErrCnt[15];  // Error Count를 10초 이상일 경우 9999로
                            // 설정한다(Mega640의 리셋시 대응 하기위함)

  short m_shOffDelayRemain;  // OffDelay Remain Time(sec)
  uint8_t m_cOffDelayFlag;   // Off Delay Flag 1:일때 처리


} SYSTEM_INFO_AWS;

/* ================================  SYSTEM CONFIGRATION
 * =============================== */
/* -------------------------------- Auto Weather System
 * -------------------------------- */
#define BAROMETER_TYPE_JINYANG 0
#define BAROMETER_TYPE_YOUNG 1
typedef struct
{
  uint8_t m_cViDestID;     // 가상 목적지 주소
  uint8_t m_cViSourID;     // 가상 보낸 주소
  uint8_t m_cTransDestID;  // 변환 목적지 주소
  uint8_t m_cTransTrsID;   // 변환 중계 주소
} VIRTUAL_IDGROUP_TABLE;

typedef struct
{
  uint16_t sOffset;      // AD Convertion 최소값
  uint16_t sFull;        // AD Convertion 최고값
  uint16_t sUse;         // 0: Use 1: NotUse
  uint16_t sChanDefine;  // 첨자:chan -> 0:Temp, 1:WindDirc, 2:Humid, 3:Barometric
                         //              4:Solar Rad, 5:SnowFall
} CALIB_BUF;

typedef enum eChargerType_
{
  eCHARGER_HJ,
  eCHARGER_LS1024
} eChargerType_t;

#define TCP_MODE_SERVER 0
#define TCP_MODE_CLIENT 1

typedef struct
{



  uint16_t m_usRainDtOffDelay;  // Rain Detect Sensor Off Delay : Rain Detect의
                                // off시 지연 시간
#ifdef VHFCOMM_DEF
  // 여기서 부터 추가 2012. 07. 03
  // ========== VHF CONFIG ========== //
  uint8_t m_cAlmId;  // 경보국 ID
  uint8_t m_cRev[2];
  uint8_t m_cSystemGrp;   // System Group Number
  uint8_t m_cHstId;       // Loop Back Destination   Id
  uint8_t m_cTrsId;       // Loop Back Destination Transfer Id
  uint8_t m_cAudioLevel;  // Alarm RTU Audio Level Control
  uint16_t m_usTxPttGap;  // PTT Gap Time Configration Memory
  uint8_t m_cRstCnt;      // Host RTU Reset Count
  uint8_t m_cMsgQueIn;    // Message Que Input Count

  uint8_t m_cMessageNum[20][8];  // 경보국 Configration과 같이 하기위함(6개 사용
                                 // 2개 예비)

  // =========== VHF ID Forwarding ============= //
  VIRTUAL_IDGROUP_TABLE m_ViGRP[64];  // 가상ID 포워딩 중계 설정
  ulong m_usVhfTmout;                 // Loop Back시 Time Out시간 설정
  uint8_t m_cToneSec;                 // Tone 지연 시간
  uint8_t m_cNoiseSec;                // Noise 지연 시간

  uint8_t m_cEvSendCount;
#endif
  uint8_t m_cComm2Mode;  // 0:Direct, 1:2G Modem, 2:3G Modem
  uint8_t m_cCDMAIP[4];
  uint16_t m_usCDMATcpPort;          //
  uint16_t m_usHostCommLastTimeMax;  // Host로 부터 통신후 접속 체크하는 시간(Sec)
  uint16_t m_usReConnectTime;        // 접속 실패시 재접속 시도 까지의 시간(Sec)
  uint8_t m_ucBarometerType;         // 시리얼 rs232 타입 기압계 사용
  eChargerType_t m_ucChargerType;    // 0 화진 atemga 연산,1 ls1024
  uint8_t m_ucTcpMode;               // 0 server, 1 client
  uint8_t m_ucServer_ip[4];          // tcp 모듈 사용
  uint16_t m_usServer_port;          // tcp 모듈 사용
  // HWAJIN V2.00
  uint8_t m_ucChUse[60];

} SYSTEM_CONFIG_AWS;

#define WINDSPEED_CHN 36   // Pulse Input 정의
#define TEMPERATURE_CHN 0  // AD Converter Input 정의
#define SUNSHINE_CHN 2     // 일조 센서
#define WINDDIREC_CHN 3    // AD Converter Input 정의
#define HUMIDITY_CHN 4     // 습도
#define BAROMETRIC_CHN 5   // 기압
#define SNOWFALL_CHN 6     // 적설량
#define SOLARRAD_CHN 7     // 일사량
// 추가 2017. 03.22 //
#define SOLITEMP5CM_CHN 8    // 지중온도 5Cm [A08] mSoilTemp5cm
#define SOLITEMP50CM_CHN 9   // 지중온도 50Cm [A09] mSoilTemp50cm
#define SOLITEMP1_0M_CHN 10  // 지중온도 1.0m [A10] mSoilTemp1_0m
#define SOLITEMP1_5M_CHN 11  // 지중온도 1.5m [A11] mSoilTemp1_5m

#define SOLITEMP10CM_CHN 12  // 지중온도 10Cm [ ] mSoilTemp10cm	임시
#define SOLITEMP20CM_CHN 13  // 지중온도 20Cm [ ] mSoilTemp20cm 	임시
#define SOLITEMP30CM_CHN 14  // 지중온도 30Cm [ ] mSoilTemp30cm	임시

#define MEGASPEED_ERR_CHAN 0
#define MEGADIREC_ERR_CHAN 1
#define MEGATEMP_ERR_CHAN 2
#define MEGABARO_ERR_CHAN 3
#define MEGAHUMID_ERR_CHAN 4
#define MEGASOL_ERR_CHAN 5
#define MEGASNOW_ERR_CHAN 6
#define MEGARAIN_ERR_CHAN 7

#define MEGASOLI5TEMP_ERR_CHAN 8
#define MEGASOLI10TEMP_ERR_CHAN 9
#define MEGASOLI20TEMP_ERR_CHAN 10
#define MEGASOLI30TEMP_ERR_CHAN 11

#define MEGASOLI50TEMP_ERR_CHAN 12
#define MEGASOLI1_0TEMP_ERR_CHAN 13
#define MEGASOLI1_5TEMP_ERR_CHAN 14

#define SUNSHINE_DIBIT 0x0002
#define RAINDETECT_DIBIT 0x0001
#define RAINFAIL_DIBIT 0x0004
#define LOGGERDOOR_DIBIT 0x0008
#define RAINFAIL_HALLDIBIT 0x8000

// 강우량 감지센서가 홀소자 방식(2010)
#define RAIN_DETECT_HALL
#endif
