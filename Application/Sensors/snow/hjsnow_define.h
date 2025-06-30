#ifndef HJSNOW_DEFINE_H

#define HJSNOW_DEFINE_H
#include <stdint.h>

#define TELNUMBER_PASS_MAX 6  // 8
#define TELNUMBER_MAX 12
#define PASSNUMBER_MAX 5
#define IPADDR_MAX 4
#define WATCHDOG_BUFFER_MAX 12  // 32bit size

typedef struct
{
  uint16_t CurDistance[3];  // channel별 실제 측정거리값
  int16_t CurLevel[3];      // channel별 높이
  int16_t CurSnowLevel;     // 3점의 평균한 높이
  uint32_t LastScanTime;    // 마지막으로 스캔한시간

  uint8_t CurConnStat[3];  // 블루투스 Connection 상태
  uint8_t HeaterStat;
#define SNOW_STAT_HEAT_ON 1
#define SNOW_STAT_HEAT_OFF 0
  uint8_t snow_debugprint;  // 0: not print , 1: print
#define SNOW_STAT_DEBUGPRINT_ON 1
#define SNOW_STAT_DEBUGPRINT_OFF 0
  // eTempSens_t 순서로 배치
  int16_t innerTemp;    // 25 	-> 25`C
  int16_t Humidity;     // 30		-> 30%
  int16_t snow_vitemp;  // virtual temp (temp + 40)
                        // 평상시 filter 적용하고 Test시에만 뺄수있다
  uint8_t ComPingCnt[4];
  uint32_t SysResetTime;  // 시스템 파워 온 시간
  int8_t MstMcuInit;      // TFT Mcu가 초기화한 상태
  int8_t cfg_status;      // 0 정상 1 오류
} SYSTEM_TypeDef;         // System

#define PROTOCOL_TYPE_t uint8_t
#define P_TYPE_HJ 0
#define P_TYPE_WEATHERPIA_2 1
#define P_TYPE_WEATHERPIA_013456 2

typedef struct
{
  // Snowfall Config
  uint8_t xModel[3][6];   // bluetooth laser model
                          //		uint8_t  temp_heaton;						// value + 40
                          //		uint8_t  temp_heatoff;					// value + 40
                          //		uint8_t  heat_automan;					// 히터 사용유무(여름에
                          //온도에으한 오동작방지) : 0 : manual , 1 : auto
  uint16_t DeviceID;      //
  uint8_t snow_scantime;  // 0:real , 1:1분 , 5분 , 10분 , 30분, 60분
#define SNOW_HEAT_AUTO 1
#define SNOW_HEAT_MANUAL 0
  uint32_t snow_stddistance;  // 기준 높이 지면으로 부터 장치의 높이

  uint32_t snow_refdistance[3];  // calibration point distance
  uint8_t snow_runflag;          // 0: Not scan , 1: scan

  uint8_t snow_scantemp;  // value + 40 : snow scan 가동 온도 : off온도 : snow_scantemp - 2도 낮은점
  uint8_t snow_scantempauto;  // 0: manual 1: auto mode,
#define SNOW_SCAN_AUTO 1
#define SNOW_SCAN_MANUAL 0

  uint16_t snow_filterlevel;
  uint8_t snow_nofiltermode;  // 1: run mode(filter) , 0:test mode(non filter)
  // 디바이스별 설정값들..
  uint16_t FactorySet;

  uint8_t Com1PingTime;  // 0:off, 1m~60m
  uint8_t Com2PingTime;  // 0:off, 1m~60m
  uint8_t Com3PingTime;  // 0:off, 1m~60m
  uint8_t Com4PingTime;  // TBD

  ///////////////////////////////
  char Password[PASSNUMBER_MAX];             // DTMF 및 SMS TCP/IP 통신시 비밀번호
  uint8_t WatchDogSec[WATCHDOG_BUFFER_MAX];  // 0 ~ 9:off, 10 ~ 240sec,
  PROTOCOL_TYPE_t protocolType;  // 통신 프로토콜, 0 화진(요청 응답) 1 웨더피아(일방전송)
  uint32_t txPeriodSec;  //  웨더피아 전송 주기 sec
  uint8_t snowScanCnt;
} CONFIG_TypeDef;  // Config		LOGMSG_BUFFER_MAX
#endif