#define __STDC_WANT_LIB_EXT1__ 1

#include <time.h>

#include "data_logging.h"
#include "ff.h"

int32_t save_data_to_file(const char *path, const uint8_t *data, uint32_t dataLen,
                          uint32_t offset) ;
int32_t load_data_from_file(const char *path, uint8_t *dst, uint32_t dataLen, uint32_t offset);

long get_minute_offset(DATE_TIME_BUF *ct, int logging_min)
{
  struct tm base_tm = {0};
  time_t t_base, t_now;

  base_tm.tm_year = ct->Year - 1900;
  base_tm.tm_mon = 0;
  base_tm.tm_mday = 1;
  base_tm.tm_hour = 0;
  base_tm.tm_min = 0;
  base_tm.tm_sec = 0;

  t_base = mktime(&base_tm);

  struct tm now_tm = {0};
  now_tm.tm_year = ct->Year - 1900;
  now_tm.tm_mon = ct->Month - 1;
  now_tm.tm_mday = ct->Day;
  now_tm.tm_hour = ct->Hour;
  now_tm.tm_min = ct->Min;
  now_tm.tm_sec = 0;

  t_now = mktime(&now_tm);

  if (t_base == -1 || t_now == -1)
  {
    return -1;
  }

  long seconds_diff = (long)difftime(t_now, t_base);
  return seconds_diff / (logging_min * 60);
}

int32_t save_data(DATE_TIME_BUF *ct,
                  const void* data_ptr, size_t data_size, uint8_t logging_min,
                  eDATA_TYPE_t data_type)
{
  struct tm time_input = {0};
  struct tm prev_tm = {0};
  time_t t_now, t_prev;
  DATE_TIME_BUF nt;

  // 입력 시간 초기화
  time_input.tm_year = ct->Year - 1900;
  time_input.tm_mon = ct->Month - 1;
  time_input.tm_mday = ct->Day;
  time_input.tm_hour = ct->Hour;
  time_input.tm_min = ct->Min;
  time_input.tm_sec = 0;

  t_now = mktime(&time_input);
  if (t_now == -1)
  {
    return 1;
  }
  // 저장할 시점은 입력 시간 - 저장 주기
  t_prev = t_now ;//- (logging_min * 60);//기존 레코더 호환 

  if (localtime_s(&t_prev ,&prev_tm) != 0)
  {

    return 1;
  }

  nt.Year = prev_tm.tm_year + 1900;
  nt.Month = prev_tm.tm_mon + 1;
  nt.Day = prev_tm.tm_mday;
  nt.Hour = prev_tm.tm_hour;
  nt.Min = prev_tm.tm_min;
  nt.Sec = 0;

  // offset 계산
  long offset =  get_minute_offset(&nt, logging_min);
  if (offset < 0)
  {

    return 1;
  }

  // 파일 경로 계산
  int folder_idx = nt.Year % 10;
  char folder_name[10];
  char file_name[128];


 // save_data_to_file("d:\\data.dat", (uint8_t*)data_ptr, data_size, offset);
  return 1;
}


int32_t load_data(DATE_TIME_BUF *ct, uint8_t *pDataBuff,
               uint32_t dataBuffSize, uint32_t readCnt, eDATA_TYPE_t dataType)
{
  if (pDataBuff == NULL || dataBuffSize == 0 || readCnt == 0)
  {
    //printf("load_data: Invalid parameters.\n");
    return 1;
  }

  time_t t_now;
  struct tm time_input = {0};


  time_input.tm_year = ct->Year - 1900;
  time_input.tm_mon = ct->Month - 1;
  time_input.tm_mday = ct->Day;
  time_input.tm_hour = ct->Hour;
  time_input.tm_min = ct->Min;
  time_input.tm_sec = 0;

  t_now = mktime(&time_input);
  if (t_now == -1)
  {

    return 1;
  }

  uint32_t dataSizePerEntry = dataBuffSize / readCnt;
  if (dataSizePerEntry == 0)
  {

    return 1;
  }

  uint8_t *pBuff = pDataBuff;

  for (uint32_t i = 0; i < readCnt; i++)
  {
    // ? offset 계산 기준 시간은 저장 종료 시간 - 저장 주기
    time_t t_prev = t_now - 60;  // 1분 저장 주기 기준 (확장 가능: save_cycle_min * 60)

    struct tm cur_tm;
    if (localtime_s(&t_prev,&cur_tm) != 0)
    {

      return 1;
    }

    int file_year = cur_tm.tm_year + 1900;
    int file_month = cur_tm.tm_mon + 1;
    int file_day = cur_tm.tm_mday;

    // 폴더 및 파일명 계산
    int folder_idx = file_year % 10;
    char folder_name[10];
    char file_name[128];

    //snprintf(folder_name, sizeof(folder_name), "Y%d", folder_idx);
   // snprintf(file_name, sizeof(file_name), "%s/%s_%04d%02d%02d.dat", folder_name,
    //         get_data_type_str(dataType), file_year, file_month, file_day);

    // offset 계산
    struct tm base_tm = {0};
    base_tm.tm_year = file_year - 1900;
    base_tm.tm_mon = 0;
    base_tm.tm_mday = 1;
    base_tm.tm_hour = 0;
    base_tm.tm_min = 0;
    base_tm.tm_sec = 0;

    time_t t_base = mktime(&base_tm);
    if (t_base == -1)
    {
      return 1;
    }

    long seconds_diff = (long)difftime(t_prev, t_base);
    long offsetEntry = seconds_diff / 60;  // 1분 주기 기준
    uint32_t byte_offset = offsetEntry * dataSizePerEntry;

     load_data_from_file(file_name, pBuff, dataSizePerEntry, byte_offset);

    pBuff += dataSizePerEntry;
    t_now += 60;  // 다음 분 → 입력 시간 증가
  }

  return 0;
}

int32_t save_data_to_file(const char *path, const uint8_t *data, uint32_t dataLen, uint32_t offset)
{
  FIL file;
  FRESULT res;
  UINT bytesWritten;

  // 파일 열기 (읽기/쓰기, 없으면 생성)
  res = f_open(&file, path, FA_WRITE | FA_OPEN_ALWAYS);
  if (res != FR_OK)
  {
    return res;
  }

  // 지정 offset으로 이동
  res = f_lseek(&file, offset);
  if (res != FR_OK)
  {

    f_close(&file);
    return res;
  }

  // 데이터 쓰기
  res = f_write(&file, data, dataLen, &bytesWritten);
  if (res != FR_OK || bytesWritten != dataLen)
  {

    f_close(&file);
    return res != FR_OK ? res : FR_INT_ERR;  // partial write → INT_ERR로 변환
  }

  // 파일 닫기
  res = f_close(&file);
  if (res != FR_OK)
  {

    return res;
  }



  return FR_OK;
}


int32_t load_data_from_file(const char *path, uint8_t *dst, uint32_t dataLen, uint32_t offset)
{
  FIL file;
  FRESULT res;
  UINT bytesRead;

  if (dst == NULL || dataLen == 0)
  {

    return FR_INVALID_PARAMETER;
  }

  // 파일 열기 (읽기 전용)
  res = f_open(&file, path, FA_READ);
  if (res != FR_OK)
  {
    return res;
  }

  // 오프셋 이동
  res = f_lseek(&file, offset);
  if (res != FR_OK)
  {
    f_close(&file);
    return res;
  }

  // 데이터 읽기
  res = f_read(&file, dst, dataLen, &bytesRead);
  if (res != FR_OK || bytesRead != dataLen)
  {
    // 부족 부분 0으로 초기화
    if (bytesRead < dataLen)
    {
      //memset(dst + bytesRead, 0, dataLen - bytesRead);
    }
    f_close(&file);
    return (res != FR_OK) ? res : FR_INT_ERR;  
  }

  // 파일 닫기
  res = f_close(&file);
  if (res != FR_OK)
  {

    return res;
  }


  return FR_OK;
}
