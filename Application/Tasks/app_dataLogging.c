
#include <stdio.h>

#include "cmsis_os2.h"

#include "app_file.h"

#include "utile_time.h"

#define AWS_FILE_PATH        "0:Y%02d/M%02d.aws"


static osSemaphoreId_t dataLoggingSem;

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


#define LOGGING_AWS  0 


void get_filePath(uint8_t type,uint32_t year,uint32_t month,char *pOutBuff,uint32_t buffSize)
{
    uint32_t temp;

    temp = year;
    year = temp %10;
    switch(type)
    {
        case LOGGING_AWS:
            snprintf(pOutBuff, buffSize, AWS_FILE_PATH,year,month);
            break;
    }
}
void write_sensorData(DATE_TIME_BUF *pDate, void *pInData,uint32_t dataSize, uint8_t Type,uint32_t periodMin)
{
	char path[70];			// "0:Weather/01.log"
	time_t tmCurrent;
	uint32_t year, nAddr;
	uint8_t retry;
    uint8_t yearList[10]={9,0,1,2,3,4,5,6,7,8};
    uint8_t ret;
    uint32_t sensorDataSize;

    sensorDataSize = dataSize;


	tmCurrent  = SetTime(pDate->Year, pDate->Month, pDate->Day, pDate->Hour, pDate->Min, 0);
	year       = GetYear(tmCurrent) % 10;
	nAddr      = TimeToAddress(tmCurrent,periodMin*60, sensorDataSize);

	if(nAddr == 0)//해가 바뀌게 되면
	{
        nAddr = TimeToAddress(SetTime(pDate->Year - 1, 12, 31, 23, 60-periodMin, 0), periodMin*60, sensorDataSize) + sensorDataSize;
        year  = yearList[GetYear(tmCurrent)%10]; // 전년도에 저장해야함
	}

	retry = 3;
	do
	{
        get_filePath(Type,year,periodMin,path,sizeof(path));

        if(    osSemaphoreAcquire(dataLoggingSem, osWaitForever) == osOK)
        {
            ret =write_file(path, (uint8_t *)pInData,sensorDataSize,nAddr);
            osSemaphoreRelease(dataLoggingSem);
        }
	    if(ret)
	    {
	        break; // OK
	    }
	}while(--retry);
    

}


void dataLogging_init(void)
{
  dataLoggingSem = osSemaphoreNew(1, 1, NULL);  
}