

#include <stdint.h>
#include <string.h>

#include "os_define.h"
#include "app_file.h"
#include "usDelay.h"
#include "sdio.h"
#include "dev_io.h"
//파일명 8자리 
const char *remote_path = "0:Firmware/Remote";
const char *user_path   = "0:Firmware/User";
const char *system_log_path = "0:System/log.txt";


static osSemaphoreId_t g_fileSem;



typedef enum
{
    eFAT_ERR_OPEN=0,
    eFAT_ERR_SEEK,
    eFAT_ERR_READ,
    eFAT_ERR_WRITE,
    eFAT_ERR_CLOSE,
    eFAT_ERR_MAX
}eFAT_ERR_t;
uint8_t _fatErr[eFAT_ERR_MAX];// SD 상태 확인용,세마포어 필요

#if 0 
int32_t read_file(char *pPath,uint8_t *pBuff, uint32_t len,uint32_t offset)
{
  uint32_t bw = 0;
  uint32_t quot;
  uint32_t rem;
  uint32_t i;
  FIL rFile;
  FRESULT volatile fr;
enum {READ_SIZE = 4096};

  quot = len/READ_SIZE;
  rem = len %READ_SIZE;
    
  for(int i = 0 ; i< eFAT_ERR_MAX;i++)
  {
    _fatErr[i] = 0xFF;
  }

    OS_SEM_PEND(g_fileSem,osWaitForever);

  do
  {
    fr = f_open(&rFile, pPath, FA_READ);
    if(fr != FR_OK)
    {
      _fatErr[eFAT_ERR_OPEN] = (uint8_t)fr;
      return 0;
    }
        fr = f_lseek(&rFile, offset); 
        if( fr != FR_OK)
        {
            _fatErr[eFAT_ERR_SEEK] = (uint8_t)fr;
            break;
        }

        for(i = 0 ; i < quot; i++)
        {
            fr = f_read(&rFile, &pBuff[i*READ_SIZE], READ_SIZE, &bw);
            if( (fr != FR_OK) ||( bw != READ_SIZE))
            {
                _fatErr[eFAT_ERR_READ] = (uint8_t)fr;
                break;
            } 
            osDelay(2);
        }

        if(fr != FR_OK)
        {
            break;
        }

        if(rem)
        {
            fr = f_read(&rFile, &pBuff[i*READ_SIZE], rem, &bw);
            if( (fr != FR_OK) || (bw != rem))
            {
                _fatErr[eFAT_ERR_READ] = (uint8_t)fr;
                break;
            } 
        }
    } while(0);


    fr = f_close(&rFile); 
    if(fr != FR_OK)
    {
        _fatErr[eFAT_ERR_CLOSE] = (uint8_t)fr;
    }

    OS_SEM_POST(g_fileSem);
    return !(fr == FR_OK);

}
#endif

FRESULT write_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset)
{
  FIL file;
  FRESULT res;
  UINT bytesWritten;

  OS_SEM_PEND(g_fileSem, osWaitForever);


  // 파일 열기 (없으면 생성, 있으면 열기 + 쓰기)
  res = f_open(&file, path, FA_WRITE | FA_OPEN_ALWAYS);
  if (res != FR_OK)
  {
    OS_SEM_POST(g_fileSem);
    return res;  // 실패 시 오류 코드 반환
  }

  // 파일 포인터를 offset 위치로 이동
  res = f_lseek(&file, offset);
  if (res != FR_OK)
  {
    f_close(&file);
    OS_SEM_POST(g_fileSem);
    return res;
  }

  // 데이터 쓰기
  res = f_write(&file, data, dataLen, &bytesWritten);
  if (res != FR_OK || bytesWritten != dataLen)
  {
    f_close(&file);
    OS_SEM_POST(g_fileSem);
    return res != FR_OK ? res : FR_DISK_ERR;
  }

  // 캐시된 데이터 플러시 (안정성 보장)
  res = f_sync(&file);

  // 파일 닫기
  f_close(&file);

  OS_SEM_POST(g_fileSem);
  return res;
}

FRESULT read_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset)
{
  FIL file;
  FRESULT res;
  UINT bytesRead;

  OS_SEM_PEND(g_fileSem, osWaitForever);
  // 파일 열기 (읽기 전용, 없으면 오류)
  res = f_open(&file, path, FA_READ);
  if (res != FR_OK)
  {
    OS_SEM_POST(g_fileSem);
    return res;  // 실패 시 오류 코드 반환
  }

  // 파일 포인터를 offset 위치로 이동
  res = f_lseek(&file, offset);
  if (res != FR_OK)
  {
    f_close(&file);
    OS_SEM_POST(g_fileSem);
    return res;
  }

  // 데이터 읽기
  res = f_read(&file, data, dataLen, &bytesRead);
  if (res != FR_OK || bytesRead != dataLen)
  {
    // 읽을 데이터가 파일 끝(EOF)에 도달했을 수 있음 (정상)
    if (res == FR_OK && bytesRead < dataLen)
    {
      // 남은 부분은 0으로 패딩 (옵션, 필요시)
      for (uint32_t i = bytesRead; i < dataLen; i++)
      {
        data[i] = 0;
      }
    }
    else
    {
      f_close(&file);
      OS_SEM_POST(g_fileSem);
      return res != FR_OK ? res : FR_DISK_ERR;
    }
  }

  // 파일 닫기
  f_close(&file);
  OS_SEM_POST(g_fileSem);
  return FR_OK;
}

FRESULT append_file(char *path, uint8_t *data, uint32_t dataLen)
{
  FIL file;
  FRESULT res;
  UINT bytesWritten;

  OS_SEM_PEND(g_fileSem, osWaitForever);
  // 파일 열기 (쓰기, 없으면 생성, 있으면 파일 끝으로 포인터 이동 후 쓰기)
  res = f_open(&file, path, FA_WRITE | FA_OPEN_APPEND);
  if (res != FR_OK)
  {
    OS_SEM_POST(g_fileSem);
    return res;  // 실패 시 오류 코드 반환
  }

  // 데이터 쓰기 (파일 끝에 추가)
  res = f_write(&file, data, dataLen, &bytesWritten);
  if (res != FR_OK || bytesWritten != dataLen)
  {
    f_close(&file);
    OS_SEM_POST(g_fileSem);
    return res != FR_OK ? res : FR_DISK_ERR;
  }

  // 캐시된 데이터 플러시 (안정성 보장)
  res = f_sync(&file);

  // 파일 닫기
  f_close(&file);
  OS_SEM_POST(g_fileSem);
  return res;
}

#include "ff.h"

// 파일이 존재하면 삭제하는 함수
FRESULT delete_file(const char *fileName)
{
  FILINFO fno;
  FRESULT res;

  // 파일 존재 여부 확인
  res = f_stat(fileName, &fno);
  if (res == FR_NO_FILE)
  {
    // 파일이 없는 경우는 에러 아님
    return FR_OK;
  }
  else if (res != FR_OK)
  {
    // 다른 에러 (경로 오류 등)
    return res;
  }

  // 디렉토리인 경우는 삭제하지 않음
  if (fno.fattrib & AM_DIR)
  {
    return FR_DENIED;  // 디렉토리 삭제 금지
  }

  // 파일 삭제
  res = f_unlink(fileName);
  return res;
}

// FAT 날짜 및 시간 포맷 해석 함수
void print_fat_time(WORD fdate, WORD ftime)
{
  uint16_t year = ((fdate >> 9) & 0x7F) + 1980;
  uint8_t month = (fdate >> 5) & 0x0F;
  uint8_t day = fdate & 0x1F;

  uint8_t hour = (ftime >> 11) & 0x1F;
  uint8_t min = (ftime >> 5) & 0x3F;
  uint8_t sec = (ftime & 0x1F) * 2;

  debug_printf("%04u-%02u-%02u %02u:%02u:%02u", year, month, day, hour, min, sec);
}
FRESULT list_directory(const char *path)
{
  DIR dir;
  FILINFO fno;
  FRESULT res;

  OS_SEM_PEND(g_fileSem, osWaitForever);

  // 디렉토리 열기
  res = f_opendir(&dir, path);
  if (res != FR_OK)
  {
    debug_printf("Failed to open directory: %s (Error: %d)\r\n", path, res);
    OS_SEM_POST(g_fileSem);
    return res;
  }

  // 디렉토리 항목 읽기 루프
  while (1)
  {
    res = f_readdir(&dir, &fno);
    if (res != FR_OK || fno.fname[0] == 0)
    {
      break;
    }

    // 파일/디렉토리 정보 출력
    if (fno.fattrib & AM_DIR)
    {
      debug_printf("[DIR ] %-20s  ", fno.fname);
    }
    else
    {
      debug_printf("[FILE] %-20s  %10llu bytes  ", fno.fname, (unsigned long long)fno.fsize);
    }

    // 날짜/시간 출력
    print_fat_time(fno.fdate, fno.ftime);

    
    // 속성 출력
    debug_printf("  [");
    if (fno.fattrib & AM_RDO)
      debug_printf("R");
    if (fno.fattrib & AM_HID)
      debug_printf("H");
    if (fno.fattrib & AM_SYS)
      debug_printf("S");
    if (fno.fattrib & AM_ARC)
      debug_printf("A");
    debug_printf("]\r\n");
  }

  // 디렉토리 닫기
  f_closedir(&dir);

  OS_SEM_POST(g_fileSem);
  return FR_OK;
}

#include "ff.h"

// 파일 크기를 반환하는 함수 (단위: 바이트)
FRESULT get_file_size(const char *path, FSIZE_t *size)
{
  FILINFO fno;
  FRESULT res;
  OS_SEM_PEND(g_fileSem, osWaitForever);
  res = f_stat(path, &fno);  // 파일 정보 가져오기
  if (res != FR_OK)
  {
    *size = 0;
    OS_SEM_POST(g_fileSem);
    return res;  // 오류 반환
  }

  *size = fno.fsize;  // 파일 크기 설정
  OS_SEM_POST(g_fileSem);
  return FR_OK;
}

#define TEST_BUFFER_SIZE 4096  // 4KB 버퍼 (1MB = 256회 루프)
uint8_t buffer[TEST_BUFFER_SIZE];


FRESULT test_file_rw_speed(const char *path, uint32_t fileSize)
{
  FIL file;
  UINT bytesRW;
  uint32_t totalBytes;
  uint32_t startClk, endClk, elapsed;
  FRESULT res;

  // 테스트용 패턴 채우기 (0xAA 패턴)
  memset(buffer, 0xAA, sizeof(buffer));

  debug_printf("Writing %lu bytes to %s...\r\n", (unsigned long)fileSize, path);

  // 파일 열기 (없으면 생성, 항상 새로쓰기)
  res = f_open(&file, path, FA_WRITE | FA_CREATE_ALWAYS);
  if (res != FR_OK)
  {
    debug_printf("Failed to open file for write (Error: %d)\r\n", res);
    return res;
  }

  // 쓰기 시간 측정 시작
  startClk = HAL_GetTick();

  // 파일 쓰기 루프
  totalBytes = 0;
  while (totalBytes < fileSize)
  {
    UINT writeSize =
        ((fileSize - totalBytes) >= TEST_BUFFER_SIZE) ? TEST_BUFFER_SIZE : (fileSize - totalBytes);

    res = f_write(&file, buffer, writeSize, &bytesRW);
    if (res != FR_OK || bytesRW != writeSize)
    {
      debug_printf("Write error at %lu bytes (Error: %d)\r\n", totalBytes, res);
      f_close(&file);
      return res;
    }

    totalBytes += bytesRW;
  }

  // 데이터 플러시
  f_sync(&file);

  // 쓰기 시간 측정 종료
  endClk = HAL_GetTick();
  elapsed = endClk - startClk;

  debug_printf("Write completed: %lu bytes in %lu ms (%.2f KB/s)\r\n", (unsigned long)fileSize,
               (unsigned long)elapsed,
               (fileSize / (elapsed > 0 ? (elapsed / 1000.0f) : 1.0f)) / 1024.0f);

  f_close(&file);

  // ============================ 읽기 측정 ==============================

  debug_printf("Reading %lu bytes from %s...\r\n", (unsigned long)fileSize, path);

  res = f_open(&file, path, FA_READ);
  if (res != FR_OK)
  {
    debug_printf("Failed to open file for read (Error: %d)\r\n", res);
    return res;
  }

  // 읽기 시간 측정 시작
  startClk = HAL_GetTick();

  totalBytes = 0;
  while (totalBytes < fileSize)
  {
    UINT readSize =
        ((fileSize - totalBytes) >= TEST_BUFFER_SIZE) ? TEST_BUFFER_SIZE : (fileSize - totalBytes);

    res = f_read(&file, buffer, readSize, &bytesRW);
    if (res != FR_OK || bytesRW == 0)
    {
      debug_printf("Read error at %lu bytes (Error: %d)\r\n", totalBytes, res);
      f_close(&file);
      return res;
    }

    totalBytes += bytesRW;
  }

  // 읽기 시간 측정 종료
  endClk = HAL_GetTick();
  elapsed = endClk - startClk;

  debug_printf("Read completed: %lu bytes in %lu ms (%.2f KB/s)\r\n", (unsigned long)fileSize,
               (unsigned long)elapsed,
               (fileSize / (elapsed > 0 ? (elapsed / 1000.0f) : 1.0f)) / 1024.0f);

  f_close(&file);

  return FR_OK;
}

void file_init(void)
{
  MX_SDIO_SD_Init();
  MX_FATFS_Init();

  
  g_fileSem = osSemaphoreNew(1, 1, NULL);  
}