

#include <stdint.h>

#include "os_define.h"
#include "app_file.h"
#include "usDelay.h"
#include "sdio.h"

//파일명 8자리 
const char *remote_path = "0:Firmware/Remote";
const char *user_path   = "0:Firmware/User";
const char *system_log_path = "0:System/log.txt";


static osSemaphoreId_t g_fileSem;


int32_t write_file(char *pPath,uint8_t *pData, uint32_t dataLen,uint32_t offset)
{
    uint32_t bw = 0;
    FIL wFile;
    FRESULT volatile fr;
  
    OS_SEM_PEND(g_fileSem,osWaitForever);

    do
    {
        fr = f_open(&wFile, pPath, FA_WRITE);
        if(fr != FR_OK)
        {
            break;
        }

        fr = f_lseek(&wFile, offset); 
        if( fr != FR_OK)
        {
            break;
        }

        fr = f_write(&wFile, pData, dataLen, &bw);
        if( fr != FR_OK)
        {
            break;
        } 

        if(dataLen != bw)
        {
            break;
        }

    } while(0);

    if(wFile.obj.fs != NULL)
    {
        fr = f_close(&wFile); 

    }

    OS_SEM_POST(g_fileSem);

    return !(fr==FR_OK);//fr_ok이면 1인데 이것의 반전인 0을 리턴 , 즉 0이면 정상 
}


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




uint8_t buff[10000]={"test"};

  uint32_t start_time1;
  uint32_t  elased_time1;
  uint16_t fileLen=4;
void file_test(void)
{
  const char *path  = "0:100.bin";

  
      start_time1 = mcu_get_clk();
  write_file((char *)path,buff,fileLen,105407990);

         elased_time1 =cal_elapsed_us(start_time1);
         
    read_file((char *)path,(uint8_t *)buff,5,0);
    

       osDelay(1);
}



void file_init(void)
{
  MX_SDIO_SD_Init();
  MX_FATFS_Init();

  
  g_fileSem = osSemaphoreNew(1, 1, NULL);  
}