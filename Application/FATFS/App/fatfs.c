


#include "fatfs.h"
#include "dev_io.h"

#include "utile_time.h"

uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */
uint8_t retUSER;    /* Return value for USER */
char USERPath[4];   /* USER logical drive path */
FATFS USERFatFS;    /* File system object for USER logical drive */
FIL USERFile;       /* File object for USER */

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
    FRESULT res;  
    

  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);

  retUSER = FATFS_LinkDriver(&USER_Driver, USERPath);


    res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
    if (res != FR_OK)
    {
        debug_printf("Failed to mount SD card. Error: %d\n", res);
    }
}



/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
#include <time.h>

#include "ff.h"


DWORD get_fattime(void)
{
  time_t now = time_cvt_timestamp(&Date_Time);
  struct tm *t = localtime(&now);

  DWORD fattime = 0;

  fattime |= ((DWORD)(t->tm_year - 80) & 0x7F) << 25;  // (tm_year: 1900년 기준, FAT는 1980년 기준)
  fattime |= ((DWORD)(t->tm_mon + 1) & 0x0F) << 21;    // tm_mon: 0~11 → 1~12
  fattime |= ((DWORD)t->tm_mday & 0x1F) << 16;         // 일
  fattime |= ((DWORD)t->tm_hour & 0x1F) << 11;         // 시
  fattime |= ((DWORD)t->tm_min & 0x3F) << 5;           // 분
  fattime |= ((DWORD)(t->tm_sec / 2) & 0x1F);          // 초 (2초 단위)

  return fattime;
}
