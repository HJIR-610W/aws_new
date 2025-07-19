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
  uint16_t CurDistance[3];  // channel蹂??ㅼ젣 痢≪젙嫄곕━媛?
  int16_t CurLevel[3];      // channel蹂??믪씠
  int16_t CurSnowLevel;     // 3?먯쓽 ?됯퇏???믪씠
  uint32_t LastScanTime;    // 留덉?留됱쑝濡??ㅼ틪?쒖떆媛?

  uint8_t CurConnStat[3];  // 釉붾（?ъ뒪 Connection ?곹깭
  uint8_t HeaterStat;
#define SNOW_STAT_HEAT_ON 1
#define SNOW_STAT_HEAT_OFF 0
  uint8_t snow_debugprint;  // 0: not print , 1: print
#define SNOW_STAT_DEBUGPRINT_ON 1
#define SNOW_STAT_DEBUGPRINT_OFF 0
  // eTempSens_t ?쒖꽌濡?諛곗튂
  int16_t innerTemp;    // 25 	-> 25`C
  int16_t Humidity;     // 30		-> 30%
  int16_t snow_vitemp;  // virtual temp (temp + 40)
                        // ?됱긽??filter ?곸슜?섍퀬 Test?쒖뿉留?類꾩닔?덈떎
  uint8_t ComPingCnt[4];
  uint32_t SysResetTime;  // ?쒖뒪???뚯썙 ???쒓컙
  int8_t MstMcuInit;      // TFT Mcu媛 珥덇린?뷀븳 ?곹깭
  int8_t cfg_status;      // 0 ?뺤긽 1 ?ㅻ쪟
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
                          //		uint8_t  heat_automan;					// ?덊꽣 ?ъ슜?좊Т(?щ쫫??
                          //?⑤룄?먯쑝???ㅻ룞?묐갑吏) : 0 : manual , 1 : auto
  uint16_t DeviceID;      //
  uint8_t snow_scantime;  // 0:real , 1:1遺?, 5遺?, 10遺?, 30遺? 60遺?
#define SNOW_HEAT_AUTO 1
#define SNOW_HEAT_MANUAL 0
  uint32_t snow_stddistance;  // 湲곗? ?믪씠 吏硫댁쑝濡?遺???μ튂???믪씠

  uint32_t snow_refdistance[3];  // calibration point distance
  uint8_t snow_runflag;          // 0: Not scan , 1: scan

  uint8_t snow_scantemp;  // value + 40 : snow scan 媛???⑤룄 : off?⑤룄 : snow_scantemp - 2???????
  uint8_t snow_scantempauto;  // 0: manual 1: auto mode,
#define SNOW_SCAN_AUTO 1
#define SNOW_SCAN_MANUAL 0

  uint16_t snow_filterlevel;
  uint8_t snow_nofiltermode;  // 1: run mode(filter) , 0:test mode(non filter)
  // ?붾컮?댁뒪蹂??ㅼ젙媛믩뱾..
  uint16_t FactorySet;

  uint8_t Com1PingTime;  // 0:off, 1m~60m
  uint8_t Com2PingTime;  // 0:off, 1m~60m
  uint8_t Com3PingTime;  // 0:off, 1m~60m
  uint8_t Com4PingTime;  // TBD

  ///////////////////////////////
  char Password[PASSNUMBER_MAX];             // DTMF 諛?SMS TCP/IP ?듭떊??鍮꾨?踰덊샇
  uint8_t WatchDogSec[WATCHDOG_BUFFER_MAX];  // 0 ~ 9:off, 10 ~ 240sec,
  PROTOCOL_TYPE_t protocolType;  // ?듭떊 ?꾨줈?좎퐳, 0 ?붿쭊(?붿껌 ?묐떟) 1 ?⑤뜑?쇱븘(?쇰갑?꾩넚)
  uint32_t txPeriodSec;  //  ?⑤뜑?쇱븘 ?꾩넚 二쇨린 sec
  uint8_t snowScanCnt;
} CONFIG_TypeDef;  // Config		LOGMSG_BUFFER_MAX
#endif