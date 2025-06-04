
#include <stdio.h>

#include "cmsis_os2.h"
#include "app_file.h"
#include "util_time.h"
#include "app_dataLogging.h"

#include "dev_io.h"

#define AWS_FILE_PATH       "0:Y%02d/M%02d.aws"
#define RAIN_1MIN_FILE_PATH "0:Y%02d/RAIN_01.rcd"
#define SUNSHINE_1MIN_FILE_PATH "0:Y%02d/SUNSHINE_01.rcd"

static osSemaphoreId_t dataLoggingSem;

void get_filePath(uint8_t type, uint16_t year, uint8_t month, char *pOutBuff, uint32_t buffSize)
{
  uint16_t temp;

  temp = year;
  year = temp % 10;
  switch (type)
  {
    case LOGGING_AWS:
      snprintf(pOutBuff, buffSize, AWS_FILE_PATH, year, month);
      break;
    case LOGGING_RAIN_1MIN:
      snprintf(pOutBuff, buffSize, RAIN_1MIN_FILE_PATH, year);
      break;
    case LOGGING_SUNSHINE_1MIN:
      snprintf(pOutBuff, buffSize, SUNSHINE_1MIN_FILE_PATH, year);
      break;
  }
}



void make_rain_1min_path(uint16_t year, char *buffer, int32_t buffer_size)
{
  get_filePath(LOGGING_RAIN_1MIN, year, 0, buffer, buffer_size);
}

void make_sunshine_1min_path(uint16_t year, char *buffer, int32_t buffer_size)
{
  get_filePath(LOGGING_SUNSHINE_1MIN, year, 0, buffer, buffer_size);
}

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


// 하루 기준 오프셋 계산 함수
uint32_t timeToOffsetDay(time_t current_tick, uint8_t min, uint16_t byte)
{
  time_t ts;
  time_t day_start_tick;
  uint16_t year;
  uint8_t month, day;
  uint32_t offset;

  // min 단위로 정렬
  current_tick -= current_tick % (min * 60);

  year = GetYear(current_tick);
  month = GetMonth(current_tick);
  day = GetDay(current_tick);

  day_start_tick = SetTime(year, month, day, 0, 0, 0);


  ts = current_tick - day_start_tick;

  if (ts)
  {
    offset = (ts / (min * 60)) * byte;
  }
  else
  {
    offset = 0;
  }

  return offset;
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
int32_t write_data_month(DATE_TIME_BUF *p_date, void *p_data,uint16_t dataLen, uint8_t type,uint8_t period_min)
{
  char path[70];
  time_t tmCurrent;


  uint16_t year; 
  uint8_t month;
  uint8_t last_day;
  uint32_t year_offset;
  uint32_t month_offset;


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


int32_t read_data_month(DATE_TIME_BUF *p_date, void *p_buff,uint16_t readLen, uint8_t type,uint8_t period_min)
{
  char path[70];
  time_t tmCurrent;


  uint16_t year; 
  uint8_t month;
  uint8_t last_day;
  uint32_t year_offset;
  uint32_t month_offset;


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

int32_t read_data_month_bulk(DATE_TIME_BUF *p_date, void *p_buff, uint16_t readLen, uint8_t type,
                  uint8_t period_min,uint16_t read_cnt)
{
  char path[70];
  time_t tmCurrent;


  uint16_t year;
  uint8_t month;
  uint8_t last_day;
  uint32_t year_offset;
  uint32_t month_offset;


  uint8_t monthList[] = {0, 12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
  uint8_t ret;

  year = p_date->Year;
  month = p_date->Month;

  tmCurrent = SetTime(p_date->Year, p_date->Month, p_date->Day, p_date->Hour, p_date->Min, 0);

  year_offset = timeToOffsetYear(tmCurrent, period_min, readLen);
  month_offset = timeToOffsetMonth(tmCurrent, period_min, readLen);

  if (year_offset == 0)
  {
    year = year - 1;  // 전년도에 저장해야함
  }
  if (month_offset == 0)  // 1일 0시0분 이면 이건 전달 자료임
  {
    last_day = get_last_day(p_date->Year, monthList[month]);
    month_offset =
        timeToOffsetMonth(SetTime(year, monthList[month], last_day, 23, 60 - period_min, 0),
                          period_min, readLen) +
        readLen;
  }

  get_filePath(type, year % 10, month, path, sizeof(path));

  if (osSemaphoreAcquire(dataLoggingSem, osWaitForever) == osOK)
  {
    ret = read_file(path, (uint8_t *)p_buff, readLen * read_cnt, month_offset);
    osSemaphoreRelease(dataLoggingSem);
  }

  return ret;
}
void dataLogging_init(void)
{
  dataLoggingSem = osSemaphoreNew(1, 1, NULL);  
}



uint32_t TimeToAddress(time_t tmStart, uint16_t sec,uint32_t byte)
{
    time_t  ts;
    time_t  tmTemp;
    int32_t     nYear;
    int32_t     nTotalSec;
    uint32_t     nAddress;

    // 입력 일시의 당년 1월 1일 00:00:00을 기준시간으로 함.
    tmStart     -= tmStart % sec;
    nYear       = GetYear(tmStart);
    tmTemp      = SetTime(nYear, 1, 1, 0, 0, 0);

    // (입력 일시 - 기준시각) / (60초 * 10분) * 2 Byte= Address
    ts          = tmStart - tmTemp;
    nTotalSec   = GetTotalSeconds(ts);
   // nAddress    = nTotalSec /(sec/byte);            // 10분단위 데이터를 얻기 위함.
    if(nTotalSec)
    {
      
     nAddress = nTotalSec/sec*byte;                                       // nTotalSec / (10분 * 60초) = Word 데이타 단위 갯수
    }
    else
    {
        nAddress = 0;
    }// Word 데이타 단위 갯수  * 2 = Byte단위 Address

    return nAddress;
}


int32_t write_data_year(DATE_TIME_BUF *pDate, void *pInData, uint32_t dataSize, uint8_t Type,
                      uint32_t periodMin)
{
  char path[70]; 
  time_t tmCurrent;
  uint32_t year, nAddr;
  uint8_t yearList[10] = {9, 0, 1, 2, 3, 4, 5, 6, 7, 8};
  uint8_t ret;
  uint32_t sensorDataSize;

  sensorDataSize = dataSize;

  tmCurrent = SetTime(pDate->Year, pDate->Month, pDate->Day, pDate->Hour, pDate->Min, 0);
  year = GetYear(tmCurrent) % 10;
  nAddr = TimeToAddress(tmCurrent, periodMin * 60, sensorDataSize);

  if (nAddr == 0)  // 해가 바뀌게 되면
  {
    nAddr = TimeToAddress(SetTime(pDate->Year - 1, 12, 31, 23, 60 - periodMin, 0), periodMin * 60,
                          sensorDataSize) +
            sensorDataSize;
    year = yearList[GetYear(tmCurrent) % 10];  // 전년도에 저장해야함
  }


    get_filePath(Type, year, periodMin, path, sizeof(path));

    if (osSemaphoreAcquire(dataLoggingSem, osWaitForever) == osOK)
    {
      ret = write_file(path, (uint8_t *)pInData, sensorDataSize, nAddr);
      osSemaphoreRelease(dataLoggingSem);
    }

    if(ret ==FR_OK)
    {
      return 0;
    }

    return 1;
}

/**
 * @brief   :함수 설명
 * @param   :파라메터 설명
 * @retval  :1 정상, 0에러
 * @note :
 *   1분 저장시 2000-01-01 00:01:00 ~ 2001-01-01 00:00:00  1년 저장,
 *  10분 저장시 2000-01-01 00:10:00 ~ 2001-01-01 00:00:00  1년 저장
 */
uint8_t read_sensorDataMulti(DATE_TIME_BUF *pDate, uint32_t dataSize, int32_t ReadCnt,
                             uint8_t cSystem, uint32_t periodMin, uint8_t *pOutBuff,
                             uint32_t buffSize)  // nReadRainCnt는 Word갯수임
{
  char path[50];
  uint8_t yearList[10] = {9, 0, 1, 2, 3, 4, 5, 6, 7, 8};
  uint8_t ret = 0;
  uint32_t nAddr;
  uint32_t nAddrOld;

  uint32_t year;
  uint32_t yearOld;
  uint32_t offset = 0;
  time_t tmCurrent;

  if (buffSize < (dataSize * ReadCnt))
  {
    return 0;
  }

  tmCurrent = SetTime(pDate->Year, pDate->Month, pDate->Day, pDate->Hour, pDate->Min, 0);
  year = GetYear(tmCurrent) % 10;
  nAddr = TimeToAddress(tmCurrent, 60 * periodMin, dataSize);

  if (nAddr == 0)  // 해가 바뀌면
  {
    nAddrOld = TimeToAddress(SetTime(pDate->Year - 1, 12, 31, 23, 60 - periodMin, 0),
                             periodMin * 60, dataSize) +
               dataSize;
    yearOld = yearList[GetYear(tmCurrent) % 10];  // 전년도 저장 메모리에 연속하여 저장



      get_filePath(cSystem, yearOld, periodMin, path, sizeof(path));

      if (osSemaphoreAcquire(dataLoggingSem, osWaitForever) == osOK)
      {
        ret = read_file(path, (uint8_t *)&pOutBuff[0], dataSize, nAddrOld );
        osSemaphoreRelease(dataLoggingSem);
      }


    if (ReadCnt == 1)
    {
      goto FUNTION_RETURN;
    }
  }
  // 1개 이상 읽기 이며 해가바뀌었으면 이미 이전단계에서 1개는 읽었기에 1개는 빼고 읽음
  if (nAddr == 0)
  {
    ReadCnt = ReadCnt - 1;
    offset = dataSize;
    nAddr = nAddr + dataSize;
  }

    get_filePath(cSystem, year, periodMin, path, sizeof(path));

    if (osSemaphoreAcquire(dataLoggingSem, osWaitForever) == osOK)
    {
      ret = read_file(path, (uint8_t *)&pOutBuff[offset], ReadCnt * dataSize, nAddr);
      osSemaphoreRelease(dataLoggingSem);
    }


FUNTION_RETURN:

  return ret;

}
