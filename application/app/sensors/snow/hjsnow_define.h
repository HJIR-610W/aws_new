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
  uint16_t CurDistance[3]; 
  int16_t CurLevel[3];     
  int16_t CurSnowLevel;    
  uint32_t LastScanTime;   

  uint8_t CurConnStat[3];  
  uint8_t HeaterStat;
#define SNOW_STAT_HEAT_ON 1
#define SNOW_STAT_HEAT_OFF 0
  uint8_t snow_debugprint;  // 0: not print , 1: print
#define SNOW_STAT_DEBUGPRINT_ON 1
#define SNOW_STAT_DEBUGPRINT_OFF 0
  
  int16_t innerTemp;    // 25 	-> 25`C
  int16_t Humidity;     // 30		-> 30%
  int16_t snow_vitemp;  // virtual temp (temp + 40)
                       
  uint8_t ComPingCnt[4];
  uint32_t SysResetTime; 
  int8_t MstMcuInit;      
  int8_t cfg_status;     
} SYSTEM_TypeDef;         

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
                        
                        
  uint16_t DeviceID;      //
  uint8_t snow_scantime;  
#define SNOW_HEAT_AUTO 1
#define SNOW_HEAT_MANUAL 0
  uint32_t snow_stddistance;  

  uint32_t snow_refdistance[3];  // calibration point distance
  uint8_t snow_runflag;          // 0: Not scan , 1: scan

  uint8_t snow_scantemp; 
  uint8_t snow_scantempauto;  // 0: manual 1: auto mode,
#define SNOW_SCAN_AUTO 1
#define SNOW_SCAN_MANUAL 0

  uint16_t snow_filterlevel;
  uint8_t snow_nofiltermode;  // 1: run mode(filter) , 0:test mode(non filter)

  uint16_t FactorySet;

  uint8_t Com1PingTime;  // 0:off, 1m~60m
  uint8_t Com2PingTime;  // 0:off, 1m~60m
  uint8_t Com3PingTime;  // 0:off, 1m~60m
  uint8_t Com4PingTime;  // TBD

  ///////////////////////////////
  char Password[PASSNUMBER_MAX];          
  uint8_t WatchDogSec[WATCHDOG_BUFFER_MAX];  // 0 ~ 9:off, 10 ~ 240sec,
  PROTOCOL_TYPE_t protocolType; 
  uint32_t txPeriodSec;  
  uint8_t snowScanCnt;
} CONFIG_TypeDef;  
#endif