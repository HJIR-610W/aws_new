

#include "app_file.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bsp_delay.h"
#include "debug_io.h"
#include "lfs_port.h"
#include "ff.h"
#include "os_user_def.h"
#include "sdio.h"
#include "user_heap.h"



static osSemaphoreId_t g_file_sem;//fatfs 파일 시스템 보호
#if LFS_ENABLE==1
static osSemaphoreId_t g_lfs_sem;//littlefs 파일 시스템 보호 
#endif

/* LittleFS object */
 lfs_t lfs;

typedef enum
{
    eFAT_ERR_OPEN=0,
    eFAT_ERR_SEEK,
    eFAT_ERR_READ,
    eFAT_ERR_WRITE,
    eFAT_ERR_CLOSE,
    eFAT_ERR_MAX
}eFAT_ERR_t;

uint8_t g_fat_error[eFAT_ERR_MAX];// SD 상태 확인용,세마포어 필요

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
    g_fat_error[i] = 0xFF;
  }

    OS_SEM_PEND(g_file_sem,osWaitForever);

  do
  {
    fr = f_open(&rFile, pPath, FA_READ);
    if(fr != FR_OK)
    {
      g_fat_error[eFAT_ERR_OPEN] = (uint8_t)fr;
      return 0;
    }
        fr = f_lseek(&rFile, offset); 
        if( fr != FR_OK)
        {
            g_fat_error[eFAT_ERR_SEEK] = (uint8_t)fr;
            break;
        }

        for(i = 0 ; i < quot; i++)
        {
            fr = f_read(&rFile, &pBuff[i*READ_SIZE], READ_SIZE, &bw);
            if( (fr != FR_OK) ||( bw != READ_SIZE))
            {
                g_fat_error[eFAT_ERR_READ] = (uint8_t)fr;
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
                g_fat_error[eFAT_ERR_READ] = (uint8_t)fr;
                break;
            } 
        }
    } while(0);


    fr = f_close(&rFile); 
    if(fr != FR_OK)
    {
        g_fat_error[eFAT_ERR_CLOSE] = (uint8_t)fr;
    }

    OS_POST_SEM(g_file_sem);
    return !(fr == FR_OK);

}
#endif

#define FAT_HEAP_USE 1


FRESULT write_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset)
{
  FIL file;
  FRESULT res;
  UINT bytesWritten;



  OS_PEND_SEM(g_file_sem, osWaitForever);

  // 파일 열기 (없으면 생성, 있으면 열기 + 쓰기)
  res = f_open(&file, path, FA_WRITE | FA_OPEN_ALWAYS);
  if (res != FR_OK)
  {
    ERROR_PRINTF("wrtie f_open fail %s", get_fresult((int)res));
    OS_POST_SEM(g_file_sem);
    return res;  // 실패 시 오류 코드 반환
  }

  // 파일 포인터를 offset 위치로 이동
  res = f_lseek(&file, offset);
  if (res != FR_OK)
  {
    f_close(&file);
    OS_POST_SEM(g_file_sem);
    return res;
  }

  // 데이터 쓰기
  res = f_write(&file, data, dataLen, &bytesWritten);
  if (res != FR_OK || bytesWritten != dataLen)
  {
    ERROR_PRINTF("bytesWritten(%d) != dataLen(%d) FRESULT:%d", bytesWritten, dataLen, res);
    f_close(&file);
    OS_POST_SEM(g_file_sem);
    return res != FR_OK ? res : FR_DISK_ERR;
  }

  // 캐시된 데이터 플러시 (안정성 보장)
  res = f_sync(&file);

  // 파일 닫기
  f_close(&file);

  OS_POST_SEM(g_file_sem);
  return res;
}
/*
| 함수          | `FF_USE_LFN = 0` | `FF_USE_LFN = 1` (LFN은 stack에) |
| ----------- | ---------------- | ------------------------------ |
| `f_open()`  | \~320 bytes      | \~820 bytes                    |
| `f_read()`  | \~350 bytes      | \~900 bytes                    |
| `f_write()` | \~400 bytes      | \~950 bytes                    |
| `f_mount()` | \~200 bytes      | \~200 bytes                    |

*/


FRESULT read_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset)
{
#if FAT_HEAP_USE
  FIL *p_file;
  FRESULT res;
  UINT bytesRead;

  //ERROR_PRINTF("read_file %s,%p %p %p\r\n",path,data,&data[dataLen]);
  p_file = pvPortMalloc(sizeof(FIL));

  if(p_file==NULL)
  {
    ERROR_PRINTF("read_file pvPortMalloc fail");
    return (FRESULT)-1;
  }
  OS_PEND_SEM(g_file_sem, osWaitForever);
  // 파일 열기 (읽기 전용, 없으면 오류)
  res = f_open(p_file, path, FA_READ);
  if (res != FR_OK)
  {
    ERROR_PRINTF("read f_open fail %d", res);
    vPortFree(p_file);
    OS_POST_SEM(g_file_sem);
    return res;  // 실패 시 오류 코드 반환
  }

  // 파일 포인터를 offset 위치로 이동
  res = f_lseek(p_file, offset);
  if (res != FR_OK)
  {
    f_close(p_file);
    vPortFree(p_file);
    OS_POST_SEM(g_file_sem);
    return res;
  }

  // 데이터 읽기
  res = f_read(p_file, data, dataLen, &bytesRead);
  if (res != FR_OK || bytesRead != dataLen)
  {
    ERROR_PRINTF("bytesRead(%d) != dataLen(%d) FRESULT:%d", bytesRead, dataLen, res);
    f_close(p_file);
    vPortFree(p_file);
    OS_POST_SEM(g_file_sem);
    return res != FR_OK ? res : FR_DISK_ERR;
  }

  // 파일 닫기
  f_close(p_file);
  vPortFree(p_file);

  OS_POST_SEM(g_file_sem);
  return FR_OK;
#else
  FIL file;
  FRESULT res;
  UINT bytesRead;
  OS_PEND_SEM(g_file_sem, osWaitForever);
  // 파일 열기 (읽기 전용, 없으면 오류)
  

  res = f_open(&file, path, FA_READ);

  
  if (res != FR_OK)
  {

    OS_POST_SEM(g_file_sem);
    return res;  // 실패 시 오류 코드 반환
  }

  // 파일 포인터를 offset 위치로 이동
  res = f_lseek(&file, offset);
  if (res != FR_OK)
  {
    f_close(&file);
    OS_POST_SEM(g_file_sem);
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
      OS_POST_SEM(g_file_sem);
      return res != FR_OK ? res : FR_DISK_ERR;
    }
  }

  // 파일 닫기
  f_close(&file);

  OS_POST_SEM(g_file_sem);

  return FR_OK;
#endif
}

FRESULT append_file(char *path, uint8_t *data, uint32_t dataLen)
{
  FIL file;
  FRESULT res;
  UINT bytesWritten;

  OS_PEND_SEM(g_file_sem, osWaitForever);
  // 파일 열기 (쓰기, 없으면 생성, 있으면 파일 끝으로 포인터 이동 후 쓰기)
  res = f_open(&file, path, FA_WRITE | FA_OPEN_APPEND);
  if (res != FR_OK)
  {
    OS_POST_SEM(g_file_sem);
    return res;  // 실패 시 오류 코드 반환
  }

  // 데이터 쓰기 (파일 끝에 추가)
  res = f_write(&file, data, dataLen, &bytesWritten);
  if (res != FR_OK || bytesWritten != dataLen)
  {
    f_close(&file);
    OS_POST_SEM(g_file_sem);
    return res != FR_OK ? res : FR_DISK_ERR;
  }

  // 캐시된 데이터 플러시 (안정성 보장)
  res = f_sync(&file);

  // 파일 닫기
  f_close(&file);
  OS_POST_SEM(g_file_sem);
  return res;
}



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

//0:backup/test
void make_path(const char *path)
{

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

  dbg_printf("%04u-%02u-%02u %02u:%02u:%02u", year, month, day, hour, min, sec);
}
FRESULT list_directory(const char *path)
{
  DIR dir;
  FILINFO fno;
  FRESULT res;

  OS_PEND_SEM(g_file_sem, osWaitForever);

  // 디렉토리 열기
  res = f_opendir(&dir, path);
  if (res != FR_OK)
  {
    dbg_printf("Failed to open directory: %s (Error: %d)\r\n", path, res);
    OS_POST_SEM(g_file_sem);
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
      dbg_printf("[DIR ] %-20s  ", fno.fname);
    }
    else
    {
      dbg_printf("[FILE] %-20s  %10llu bytes  ", fno.fname, (unsigned long long)fno.fsize);
    }

    // 날짜/시간 출력
    print_fat_time(fno.fdate, fno.ftime);

    
    // 속성 출력
    dbg_printf("  [");
    if (fno.fattrib & AM_RDO)
      dbg_printf("R");
    if (fno.fattrib & AM_HID)
      dbg_printf("H");
    if (fno.fattrib & AM_SYS)
      dbg_printf("S");
    if (fno.fattrib & AM_ARC)
      dbg_printf("A");
    dbg_printf("]\r\n");
  }

  // 디렉토리 닫기
  f_closedir(&dir);

  OS_POST_SEM(g_file_sem);
  return FR_OK;
}

FRESULT get_file_size(const char *path, FSIZE_t *size)
{
  FILINFO fno;
  FRESULT res;
  OS_PEND_SEM(g_file_sem, osWaitForever);
  res = f_stat(path, &fno);  // 파일 정보 가져오기
  if (res != FR_OK)
  {
    *size = 0;
    OS_POST_SEM(g_file_sem);
    return res;  // 오류 반환
  }

  *size = fno.fsize;  // 파일 크기 설정
  OS_POST_SEM(g_file_sem);
  return FR_OK;
}


FRESULT find_files_by_extension(const TCHAR *folder_path, const TCHAR *extension,
                               char found_filenames[][MAX_FILENAME_LEN], int max_filenames_to_store,
                               int *p_files_found_count)
{
  FRESULT res;
  DIR dir;
  FILINFO fno;
  int count = 0;
  TCHAR pattern[32];  // "*.ext" 형태의 패턴을 저장할 버퍼 

  if (p_files_found_count == NULL || found_filenames == NULL || folder_path == NULL ||
      extension == NULL)
  {
    if (p_files_found_count)
      *p_files_found_count = 0;
    return FR_INVALID_PARAMETER;  
  }
  *p_files_found_count = 0;


  if ((strlen("*.") + strlen(extension) + 1) > sizeof(pattern) / sizeof(TCHAR))
  {
    return FR_INVALID_PARAMETER;  // 확장자가 너무 김
  }
  snprintf(pattern,sizeof(pattern), "*.%s", extension); 

  res = f_findfirst(&dir, &fno, folder_path, pattern);

  if (res == FR_OK)
  {
    while (fno.fname[0] != 0 && count < max_filenames_to_store)
    {
      // fno.fname[0] == 0 은 더 이상 일치하는 항목이 없음을 의미
      if (!(fno.fattrib & AM_DIR))
      {  
        size_t fname_len = 0;

        fname_len = strlen(fno.fname);
        if (fname_len < MAX_FILENAME_LEN)
        {

          strcpy(found_filenames[count], fno.fname);
          count++;
        }
        else
        {

#if 0
          dbg_printf("Warning: Filename '%s' is too long and was skipped.\n", fno.fname);
#endif
        }
      }

      res = f_findnext(&dir, &fno);
      if (res != FR_OK)
        break; 
    }
    f_closedir(&dir);  // 검색 완료 후 DIR 객체 닫기
  }

  *p_files_found_count = count;

  if (res == FR_NO_FILE || res == FR_NO_PATH)
  { 
    return FR_OK;
  }
  return res;  // 그 외의 FatFs 에러 코드 반환
}



//========== 파일시스템 테스트 코드 시작==========
#define STATIC_RAM_USE 0 //head사용이 불가능 하면 MCU RAM 사용

#define TEST_BUFFER_SIZE 4096
#if STATIC_RAM_USE
uint8_t buffer[TEST_BUFFER_SIZE];
#endif

FRESULT test_file_rw_speed(const char *path, uint32_t fileSize)
{
  FIL file;
  UINT bytesRW;
  uint32_t totalBytes;
  uint32_t startClk, endClk, elapsed;
  FRESULT res;
#if !STATIC_RAM_USE
  uint8_t *buffer = (uint8_t *)user_malloc(TEST_BUFFER_SIZE);
  if (buffer == NULL)
  {
      dbg_printf("메모리 할당 실패\r\n");
    return FR_OK;
  }
#endif

  memset(buffer, 0xAA, TEST_BUFFER_SIZE);

  dbg_printf("Writing %lu bytes to %s...\r\n", (unsigned long)fileSize, path);

  // 파일 열기 (없으면 생성, 항상 새로쓰기)
  res = f_open(&file, path, FA_WRITE | FA_CREATE_ALWAYS);
  if (res != FR_OK)
  {
    dbg_printf("Failed to open file for write (Error: %d)\r\n", res);
#if !STATIC_RAM_USE
    user_free(buffer);
#endif
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
      dbg_printf("Write error at %lu bytes (Error: %d)\r\n", totalBytes, res);
      f_close(&file);
#if !STATIC_RAM_USE
      user_free(buffer);
#endif
      return res;
    }

    totalBytes += bytesRW;
  }

  // 데이터 플러시
  f_sync(&file);

  // 쓰기 시간 측정 종료
  endClk = HAL_GetTick();
  elapsed = endClk - startClk;

  dbg_printf("Write completed: %lu bytes in %lu ms (%.2f KB/s)\r\n", (unsigned long)fileSize,
               (unsigned long)elapsed,
               (fileSize / (elapsed > 0 ? (elapsed / 1000.0f) : 1.0f)) / 1024.0f);

  f_close(&file);

  // ============================ 읽기 측정 ==============================

  dbg_printf("Reading %lu bytes from %s...\r\n", (unsigned long)fileSize, path);

  res = f_open(&file, path, FA_READ);
  if (res != FR_OK)
  {
    dbg_printf("Failed to open file for read (Error: %d)\r\n", res);
#if !STATIC_RAM_USE
    user_free(buffer);
#endif
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
      dbg_printf("Read error at %lu bytes (Error: %d)\r\n", totalBytes, res);
      f_close(&file);
#if !STATIC_RAM_USE
      user_free(buffer);
#endif
      return res;
    }

    totalBytes += bytesRW;
  }

  // 읽기 시간 측정 종료
  endClk = HAL_GetTick();
  elapsed = endClk - startClk;

  dbg_printf("Read completed: %lu bytes in %lu ms (%.2f KB/s)\r\n", (unsigned long)fileSize,
               (unsigned long)elapsed,
               (fileSize / (elapsed > 0 ? (elapsed / 1000.0f) : 1.0f)) / 1024.0f);

  f_close(&file);
#if !STATIC_RAM_USE
  user_free(buffer);
#endif
  return FR_OK;
}
//========== 파일시스템 테스트 코드 종료==========


void *get_file_sem(void)
{
  return g_file_sem;
}

#if LFS_ENABLE ==1 

void *get_lfs_sem(void)
{
  return g_lfs_sem;
}

int32_t lfs_write_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset)
{
  int err;
  lfs_file_t file;
  lfs_soff_t seek_result;
  lfs_ssize_t size;

  OS_PEND_SEM(g_lfs_sem, osWaitForever);

  // 파일 열기 (없으면 생성, 있으면 열기 + 쓰기)
  err = lfs_file_open(&lfs, &file, path, LFS_O_WRONLY | LFS_O_CREAT);
  if (err < 0)
  {
    ERROR_PRINTF("lfs_file_open fail %d", err);
    OS_POST_SEM(g_lfs_sem);
    return err;
  }

  // 파일 포인터를 offset 위치로 이동
  seek_result = lfs_file_seek(&lfs, &file, offset, LFS_SEEK_SET);
  if (seek_result < 0)
  {
    lfs_file_close(&lfs, &file);
    OS_POST_SEM(g_lfs_sem);
    return seek_result;
  }

  // 데이터 쓰기
  size = lfs_file_write(&lfs, &file, data, dataLen);
  if (size < 0 || (uint32_t)size != dataLen)
  {
    ERROR_PRINTF("size(%d) != dataLen(%d) err:%d", size, dataLen, size);
    lfs_file_close(&lfs, &file);
    OS_POST_SEM(g_lfs_sem);
    return size < 0 ? size : LFS_ERR_IO;
  }

  // 캐시된 데이터 플러시 (안정성 보장)
  err = lfs_file_sync(&lfs, &file);
  if (err < 0)
  {
    lfs_file_close(&lfs, &file);
    OS_POST_SEM(g_lfs_sem);
    return err;
  }

  // 파일 닫기
  err = lfs_file_close(&lfs, &file);

  OS_POST_SEM(g_lfs_sem);
  return err;
}




int32_t lfs_read_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset)
{
  int err;
  lfs_file_t file;
  lfs_soff_t seek_result;
  lfs_ssize_t size;

  OS_PEND_SEM(g_lfs_sem, osWaitForever);

  // 파일 열기 (읽기 전용, 없으면 오류)
  err = lfs_file_open(&lfs, &file, path, LFS_O_RDONLY);
  if (err < 0)
  {
    ERROR_PRINTF("lfs_file_open fail %d", err);
    OS_POST_SEM(g_lfs_sem);
    return err;
  }

  // 파일 포인터를 offset 위치로 이동
  seek_result = lfs_file_seek(&lfs, &file, offset, LFS_SEEK_SET);
  if (seek_result < 0)
  {
    lfs_file_close(&lfs, &file);
    OS_POST_SEM(g_lfs_sem);
    return seek_result;
  }

  // 데이터 읽기
  size = lfs_file_read(&lfs, &file, data, dataLen);
  if (size < 0 || (uint32_t)size != dataLen)
  {
    ERROR_PRINTF("size(%d) != dataLen(%d) err:%d", size, dataLen, size);
    lfs_file_close(&lfs, &file);
    OS_POST_SEM(g_lfs_sem);
    return size < 0 ? size : LFS_ERR_IO;
  }

  // 파일 닫기
  err = lfs_file_close(&lfs, &file);

  OS_POST_SEM(g_lfs_sem);
  return err;
}


int32_t get_lfs_file_size(const char *path, uint32_t *size)
{
  int err;
  lfs_file_t file;
  lfs_soff_t file_size;

  OS_PEND_SEM(g_lfs_sem, osWaitForever);

  // 파일 열기 (읽기 전용)
  err = lfs_file_open(&lfs, &file, path, LFS_O_RDONLY);
  if (err < 0)
  {
    ERROR_PRINTF("lfs_file_open fail %d", err);
    *size = 0;
    OS_POST_SEM(g_lfs_sem);
    return err;
  }

  // 파일 크기 가져오기
  file_size = lfs_file_size(&lfs, &file);
  if (file_size < 0)
  {
    *size = 0;
    lfs_file_close(&lfs, &file);
    OS_POST_SEM(g_lfs_sem);
    return file_size;
  }

  *size = (uint32_t)file_size;

  // 파일 닫기
  err = lfs_file_close(&lfs, &file);

  OS_POST_SEM(g_lfs_sem);
  return err;
}

int32_t lfs_delete_file(const char *fileName)
{
  int err;

  OS_PEND_SEM(g_lfs_sem, osWaitForever);

  // 파일 삭제 (파일이 없어도 에러 반환하지 않음)
  err = lfs_remove(&lfs, fileName);
  if (err < 0 && err != LFS_ERR_NOENT)
  {
    ERROR_PRINTF("lfs_remove fail %d", err);
    OS_POST_SEM(g_lfs_sem);
    return err;
  }

  // 파일이 없는 경우는 에러 아님
  if (err == LFS_ERR_NOENT)
  {
    err = LFS_ERR_OK;
  }

  OS_POST_SEM(g_lfs_sem);
  return err;
}

int32_t lfs_append_file(char *path, uint8_t *data, uint32_t dataLen)
{
  int err;
  lfs_file_t file;
  lfs_ssize_t size;

  OS_PEND_SEM(g_lfs_sem, osWaitForever);

  // 파일 열기 (쓰기, 없으면 생성, 있으면 파일 끝으로 포인터 이동 후 쓰기)
  err = lfs_file_open(&lfs, &file, path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND);
  if (err < 0)
  {
    ERROR_PRINTF("lfs_file_open fail %d", err);
    OS_POST_SEM(g_lfs_sem);
    return err;
  }

  // 데이터 쓰기 (파일 끝에 추가)
  size = lfs_file_write(&lfs, &file, data, dataLen);
  if (size < 0 || (uint32_t)size != dataLen)
  {
    ERROR_PRINTF("size(%d) != dataLen(%d) err:%d", size, dataLen, size);
    lfs_file_close(&lfs, &file);
    OS_POST_SEM(g_lfs_sem);
    return size < 0 ? size : LFS_ERR_IO;
  }

  // 캐시된 데이터 플러시 (안정성 보장)
  err = lfs_file_sync(&lfs, &file);
  if (err < 0)
  {
    lfs_file_close(&lfs, &file);
    OS_POST_SEM(g_lfs_sem);
    return err;
  }

  // 파일 닫기
  err = lfs_file_close(&lfs, &file);

  OS_POST_SEM(g_lfs_sem);
  return err;
}

void printf_lfs_directory(const char *dir)
{
  int err;
  lfs_dir_t lfs_dir;
  struct lfs_info info;
  uint32_t dir_count;
  uint32_t file_count;
  uint32_t total_size;

  dir_count = 0;
  file_count = 0;
  total_size = 0;

  OS_PEND_SEM(g_lfs_sem, osWaitForever);

  // 디렉토리 열기
  err = lfs_dir_open(&lfs, &lfs_dir, dir);
  if (err < 0)
  {
    dbg_printf("Failed to open directory: %s (Error: %d)\r\n", dir, err);
    OS_POST_SEM(g_lfs_sem);
    return;
  }

  dbg_printf("\r\n========== LittleFS Directory: %s ==========\r\n", dir);

  // 디렉토리 항목 읽기 루프
  while (1)
  {
    err = lfs_dir_read(&lfs, &lfs_dir, &info);
    if (err < 0)
    {
      dbg_printf("Error reading directory (Error: %d)\r\n", err);
      break;
    }

    // 더 이상 항목이 없으면 종료
    if (err == 0)
    {
      break;
    }

    // "." 와 ".." 항목은 건너뛰기
    if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0)
    {
      continue;
    }

    // 파일/디렉토리 정보 출력
    if (info.type == LFS_TYPE_DIR)
    {
      dbg_printf("[DIR ] %-30s\r\n", info.name);
      dir_count++;
    }
    else if (info.type == LFS_TYPE_REG)
    {
      dbg_printf("[FILE] %-30s %10lu bytes\r\n", info.name, (unsigned long)info.size);
      file_count++;
      total_size += info.size;
    }
  }

  // 디렉토리 닫기
  lfs_dir_close(&lfs, &lfs_dir);

  OS_POST_SEM(g_lfs_sem);

  // 요약 정보 출력
  dbg_printf("--------------------------------------------\r\n");
  dbg_printf("Total: %lu directories, %lu files\r\n", (unsigned long)dir_count, (unsigned long)file_count);
  dbg_printf("Total size: %lu bytes (%.2f KB)\r\n", (unsigned long)total_size, total_size / 1024.0f);
  dbg_printf("============================================\r\n\r\n");
}

void printf_lfs_info(void)
{
  int err;
  lfs_ssize_t blocks_used;
  uint32_t total_blocks;
  uint32_t block_size;
  uint32_t total_capacity;
  uint32_t used_capacity;
  uint32_t free_capacity;

  OS_PEND_SEM(g_lfs_sem, osWaitForever);

  // 사용 중인 블록 수 확인
  blocks_used = lfs_fs_size(&lfs);

  OS_POST_SEM(g_lfs_sem);

  if (blocks_used < 0)
  {
    dbg_printf("[LFS] Error getting filesystem size (Error: %d)\r\n", blocks_used);
    return;
  }

  total_blocks = lfs_cfg.block_count;
  block_size = lfs_cfg.block_size;
  total_capacity = total_blocks * block_size;
  used_capacity = blocks_used * block_size;
  free_capacity = total_capacity - used_capacity;

  dbg_printf("\r\n========== LittleFS Filesystem Info ==========\r\n");
  dbg_printf("Block size      : %lu bytes\r\n", (unsigned long)block_size);
  dbg_printf("Total blocks    : %lu\r\n", (unsigned long)total_blocks);
  dbg_printf("Used blocks     : %lu\r\n", (unsigned long)blocks_used);
  dbg_printf("Free blocks     : %lu\r\n", (unsigned long)(total_blocks - blocks_used));
  dbg_printf("--------------------------------------------\r\n");
  dbg_printf("Total capacity  : %lu bytes (%.2f KB / %.2f MB)\r\n",
            (unsigned long)total_capacity,
            total_capacity / 1024.0f,
            total_capacity / (1024.0f * 1024.0f));
  dbg_printf("Used capacity   : %lu bytes (%.2f KB / %.2f MB)\r\n",
            (unsigned long)used_capacity,
            used_capacity / 1024.0f,
            used_capacity / (1024.0f * 1024.0f));
  dbg_printf("Free capacity   : %lu bytes (%.2f KB / %.2f MB)\r\n",
            (unsigned long)free_capacity,
            free_capacity / 1024.0f,
            free_capacity / (1024.0f * 1024.0f));
  dbg_printf("Usage           : %.1f%%\r\n", (blocks_used * 100.0f) / total_blocks);
  dbg_printf("==============================================\r\n\r\n");
}


int32_t test_lfs(const char *path, uint32_t fileSize);


void lfs_init(void)
{
  int err;


  g_lfs_sem = osSemaphoreNew(1, 1, NULL);
    
      
  //littlefs 초기화
  err = lfs_port_init();
  if (err)
  {
      dbg_printf("[ERROR] Failed to initialize porting layer\n");
  }

  err = lfs_mount(&lfs, &lfs_cfg);
  if (err)
  {
    dbg_printf("[LFS] Mount failed, formatting...\r\n");
    lfs_format(&lfs, &lfs_cfg);
    err = lfs_mount(&lfs, &lfs_cfg);
  }

  if(err == 0)
  {
    dbg_printf("[LFS] Mount successful\r\n");

    printf_lfs_info();
    printf_lfs_directory("/");

    // 테스트 (필요시 주석 처리)
   // test_lfs("test.txt", 7*1024);
  }
  else
  {
    dbg_printf("[ERROR] LittleFS mount failed (err=%d)\r\n", err);
  }
}

//littlefs 테스트 코드 
int32_t test_lfs(const char *path, uint32_t fileSize)
{
  int err;
  uint8_t *buffer;
  uint8_t *read_buffer;
  uint32_t i;
  uint32_t startClk, endClk, elapsed;
  lfs_file_t file;
  lfs_ssize_t size;
  uint32_t totalBytes;
  uint32_t writeSize;
  enum {   LFS_TEST_BUFFER_SIZE = 4096  };

  // 버퍼 할당
  buffer = (uint8_t *)user_malloc(LFS_TEST_BUFFER_SIZE);
  if (buffer == NULL)
  {
    dbg_printf("Failed to allocate write buffer\r\n");
    return LFS_ERR_NOMEM;
  }

  read_buffer = (uint8_t *)user_malloc(LFS_TEST_BUFFER_SIZE);
  if (read_buffer == NULL)
  {
    dbg_printf("Failed to allocate read buffer\r\n");
    user_free(buffer);
    return LFS_ERR_NOMEM;
  }

  // 쓰기 버퍼 초기화
  memset(buffer, 0xAA, LFS_TEST_BUFFER_SIZE);

  dbg_printf("Writing %lu bytes to %s...\r\n", (unsigned long)fileSize, path);

  OS_PEND_SEM(g_lfs_sem, osWaitForever);

  // 파일 열기 (쓰기, 없으면 생성, 있으면 덮어쓰기)
  err = lfs_file_open(&lfs, &file, path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
  if (err < 0)
  {
    dbg_printf("Failed to open file for write (Error: %d)\r\n", err);
    OS_POST_SEM(g_lfs_sem);
    user_free(buffer);
    user_free(read_buffer);
    return err;
  }

  // 쓰기 시간 측정 시작
  startClk = HAL_GetTick();

  // 파일 쓰기 루프
  totalBytes = 0;
  while (totalBytes < fileSize)
  {
    writeSize = ((fileSize - totalBytes) >= LFS_TEST_BUFFER_SIZE) ? LFS_TEST_BUFFER_SIZE : (fileSize - totalBytes);

    size = lfs_file_write(&lfs, &file, buffer, writeSize);
    if (size < 0 || (uint32_t)size != writeSize)
    {
      dbg_printf("Write error at %lu bytes (Error: %d)\r\n", totalBytes, size);
      lfs_file_close(&lfs, &file);
      OS_POST_SEM(g_lfs_sem);
      user_free(buffer);
      user_free(read_buffer);
      return size < 0 ? size : LFS_ERR_IO;
    }

    totalBytes += size;
  }

  // 데이터 플러시
  lfs_file_sync(&lfs, &file);

  // 쓰기 시간 측정 종료
  endClk = HAL_GetTick();
  elapsed = endClk - startClk;

  dbg_printf("Write completed: %lu bytes in %lu ms (%.2f KB/s)\r\n", (unsigned long)fileSize,
            (unsigned long)elapsed,
            (fileSize / (elapsed > 0 ? (elapsed / 1000.0f) : 1.0f)) / 1024.0f);

  lfs_file_close(&lfs, &file);

  // ============================ 읽기 측정 ==============================

  dbg_printf("Reading %lu bytes from %s...\r\n", (unsigned long)fileSize, path);

  err = lfs_file_open(&lfs, &file, path, LFS_O_RDONLY);
  if (err < 0)
  {
    dbg_printf("Failed to open file for read (Error: %d)\r\n", err);
    OS_POST_SEM(g_lfs_sem);
    user_free(buffer);
    user_free(read_buffer);
    return err;
  }

  // 읽기 시간 측정 시작
  startClk = HAL_GetTick();

  totalBytes = 0;
  while (totalBytes < fileSize)
  {
    writeSize = ((fileSize - totalBytes) >= TEST_BUFFER_SIZE) ? TEST_BUFFER_SIZE : (fileSize - totalBytes);

    size = lfs_file_read(&lfs, &file, read_buffer, writeSize);
    if (size < 0 || size == 0)
    {
      dbg_printf("Read error at %lu bytes (Error: %d)\r\n", totalBytes, size);
      lfs_file_close(&lfs, &file);
      OS_POST_SEM(g_lfs_sem);
      user_free(buffer);
      user_free(read_buffer);
      return size < 0 ? size : LFS_ERR_IO;
    }

    totalBytes += size;
  }

  // 읽기 시간 측정 종료
  endClk = HAL_GetTick();
  elapsed = endClk - startClk;

  dbg_printf("Read completed: %lu bytes in %lu ms (%.2f KB/s)\r\n", (unsigned long)fileSize,
            (unsigned long)elapsed,
            (fileSize / (elapsed > 0 ? (elapsed / 1000.0f) : 1.0f)) / 1024.0f);

  lfs_file_close(&lfs, &file);

  OS_POST_SEM(g_lfs_sem);

  // ============================ 데이터 비교 ==============================

  dbg_printf("Verifying data integrity...\r\n");

  OS_PEND_SEM(g_lfs_sem, osWaitForever);

  err = lfs_file_open(&lfs, &file, path, LFS_O_RDONLY);
  if (err < 0)
  {
    dbg_printf("Failed to open file for verify (Error: %d)\r\n", err);
    OS_POST_SEM(g_lfs_sem);
    user_free(buffer);
    user_free(read_buffer);
    return err;
  }

  totalBytes = 0;
  while (totalBytes < fileSize)
  {
    writeSize = ((fileSize - totalBytes) >= TEST_BUFFER_SIZE) ? TEST_BUFFER_SIZE : (fileSize - totalBytes);

    size = lfs_file_read(&lfs, &file, read_buffer, writeSize);
    if (size < 0 || (uint32_t)size != writeSize)
    {
      dbg_printf("Verify read error at %lu bytes\r\n", totalBytes);
      lfs_file_close(&lfs, &file);
      OS_POST_SEM(g_lfs_sem);
      user_free(buffer);
      user_free(read_buffer);
      return LFS_ERR_IO;
    }

    // 데이터 비교
    for (i = 0; i < writeSize; i++)
    {
      if (read_buffer[i] != 0xAA)
      {
        dbg_printf("Data mismatch at offset %lu: expected 0xAA, got 0x%02X\r\n",
                  totalBytes + i, read_buffer[i]);
        lfs_file_close(&lfs, &file);
        OS_POST_SEM(g_lfs_sem);
        user_free(buffer);
        user_free(read_buffer);
        return LFS_ERR_CORRUPT;
      }
    }

    totalBytes += size;
  }

  lfs_file_close(&lfs, &file);
  OS_POST_SEM(g_lfs_sem);

  dbg_printf("Data verification passed!\r\n");

  user_free(buffer);
  user_free(read_buffer);

  return LFS_ERR_OK;
}

#endif 




void filesystem_init(void)
{
  //fat32 라이브러리 초기화
  MX_SDIO_SD_Init();
  MX_FATFS_Init();

  g_file_sem = osSemaphoreNew(1, 1, NULL);

}

