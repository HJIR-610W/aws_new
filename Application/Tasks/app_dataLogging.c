
#include <stdio.h>

#include "cmsis_os2.h"

#include "app_file.h"

#include "utile_time.h"

#include "app_dataLogging.h"
#define AWS_FILE_PATH        "0:Y%02d/M%02d.aws"


static osSemaphoreId_t dataLoggingSem;


/**
 * @brief 현재 월에서 부터 현재 시간까지 byte크기 만큼 offset
 */
uint32_t timeToOffsetMonth(time_t currnet_tick, uint8_t min,uint16_t byte)
{
  time_t  ts;
  time_t  month_start_tick;
  uint16_t year;
  uint8_t month;
  uint32_t offset;

  currnet_tick     -= currnet_tick % (min*60);
  year   = GetYear(currnet_tick);
  month  = GetMonth(currnet_tick);
  month_start_tick = SetTime(year, month, 1, 0, 0, 0);

  ts = currnet_tick - month_start_tick;

  if(ts)
  {
    offset = (ts/(min*60))*byte;
  }
  else
  {
    offset = 0;
  }
  
  return offset;
}


uint32_t timeToOffsetYear(time_t currnet_tick, uint8_t min,uint16_t byte)
{
  time_t  ts;
  time_t  year_start_tick;
  uint16_t year;
  uint8_t month;
  uint32_t offset;

  currnet_tick     -= currnet_tick % (min*60);
  year   = GetYear(currnet_tick);
  year_start_tick = SetTime(year, 1, 1, 0, 0, 0);

  ts = currnet_tick - year_start_tick;

  if(ts)
  {
    offset = (ts/(min*60))*byte;
  }
  else
  {
    offset = 0;
  }
  
  return offset;
}



void get_filePath(uint8_t type,uint8_t year,uint8_t month,char *pOutBuff,uint32_t buffSize)
{
    uint16_t temp;

    temp = year;
    year = temp %10;
    switch(type)
    {
        case LOGGING_AWS:
            snprintf(pOutBuff, buffSize, AWS_FILE_PATH,year,month);
            break;
    }
}


int get_last_day(int year, int month)
{
  if(month == 2)
  {
    // 윤년 체크
    if((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
      return 29;
    else
      return 28;
  }
  else if(month == 4 || month == 6 || month == 9 || month == 11)
  {
    return 30;
  }
  else
  {
    return 31;
  }
}

/**
 * @brief 데이터 저장 1개의 데이터만 저장됨
 * 
 * 월단위로 저장
 * 2025-01-01 00:00:00 데이터는 2024-12-31 마지막 자료로 저장
 * 2025-02-01 00:00:00 dms 2025-01-31 마지막 자료로 저장장
 */
int32_t write_data(DATE_TIME_BUF *p_date, void *p_data,uint16_t dataLen, uint8_t type,uint8_t period_min)
{
  char path[70];
  time_t tmCurrent;
  uint8_t hour;
  uint8_t min;
  uint16_t year; 
  uint8_t month;
  uint8_t last_day;
  uint32_t year_offset;
  uint32_t month_offset;
  uint8_t retry;
  uint8_t yearList[10]={9,0,1,2,3,4,5,6,7,8};
  uint8_t monthList[]={0,12,1,2,3,4,5,6,7,8,9,10,11};
  uint8_t ret;

  year  = p_date->Year;
  month = p_date->Month; 

  tmCurrent  = SetTime(p_date->Year, p_date->Month, p_date->Day, p_date->Hour, p_date->Min, 0);

  year_offset  = timeToOffsetYear(tmCurrent,period_min, dataLen);
  month_offset = timeToOffsetMonth(tmCurrent,period_min, dataLen);

  if(year_offset == 0)
  {
    year  = year-1; // 전년도에 저장해야함

  }
  if(month_offset == 0)// 1일 0시0분 이면 이건 전달 자료임
  {
    last_day = get_last_day(p_date->Year,monthList[month]);
    month_offset = timeToOffsetMonth(SetTime(year, monthList[month], last_day, 23, 60-period_min, 0), period_min, dataLen) + dataLen;
  }

  get_filePath(type,year%10,month,path,sizeof(path));

  if(osSemaphoreAcquire(dataLoggingSem, osWaitForever) == osOK)
  {
    ret =write_file(path, (uint8_t *)p_data,dataLen,month_offset);
    osSemaphoreRelease(dataLoggingSem);
  }

    
  return ret;

}
int32_t read_data(DATE_TIME_BUF *p_date, void *p_buff,uint16_t readLen, uint8_t type,uint8_t period_min)
{
  char path[70];
  time_t tmCurrent;
  uint8_t hour;
  uint8_t min;
  uint16_t year; 
  uint8_t month;
  uint8_t last_day;
  uint32_t year_offset;
  uint32_t month_offset;
  uint8_t retry;
  uint8_t yearList[10]={9,0,1,2,3,4,5,6,7,8};
  uint8_t monthList[]={0,12,1,2,3,4,5,6,7,8,9,10,11};
  uint8_t ret;

  year  = p_date->Year;
  month = p_date->Month; 

  tmCurrent  = SetTime(p_date->Year, p_date->Month, p_date->Day, p_date->Hour, p_date->Min, 0);

  year_offset  = timeToOffsetYear(tmCurrent,period_min, readLen);
  month_offset = timeToOffsetMonth(tmCurrent,period_min, readLen);

  if(year_offset == 0)
  {
    year  = year-1; // 전년도에 저장해야함

  }
  if(month_offset == 0)// 1일 0시0분 이면 이건 전달 자료임
  {
    last_day = get_last_day(p_date->Year,monthList[month]);
    month_offset = timeToOffsetMonth(SetTime(year, monthList[month], last_day, 23, 60-period_min, 0), period_min, readLen) + readLen;
  }

  get_filePath(type,year%10,month,path,sizeof(path));

  if(osSemaphoreAcquire(dataLoggingSem, osWaitForever) == osOK)
  {
    ret =read_file(path, (uint8_t *)p_buff,readLen,month_offset);
    osSemaphoreRelease(dataLoggingSem);
  }

    
  return ret;
}

void dataLogging_init(void)
{
  dataLoggingSem = osSemaphoreNew(1, 1, NULL);  
}