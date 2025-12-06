/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sd_diskio.c
  * @brief   SD Disk I/O driver
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "system_err.h"

#include <string.h>
#include <stdio.h>

#include "bsp_delay.h"
#include "journal_manager.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define QUEUE_SIZE         (uint32_t) 10
#define READ_CPLT_MSG      (uint32_t) 1
#define WRITE_CPLT_MSG     (uint32_t) 2
/*
==================================================================
enable the defines below to send custom rtos messages
when an error or an abort occurs.
Notice: depending on the HAL/SD driver the HAL_SD_ErrorCallback()
may not be available.
See BSP_SD_ErrorCallback() and BSP_SD_AbortCallback() below
==================================================================

#define RW_ERROR_MSG       (uint32_t) 3
#define RW_ABORT_MSG       (uint32_t) 4
*/
/*
 * 다음 Timeout 값은 BSP_SD_ReadCpltCallback() 또는 BSP_SD_WriteCpltCallback()에서
 * 오류가 발생했을 경우, 애플리케이션으로 제어를 되돌려주기 위해 유용합니다.
 * 기본적으로 이 값은 BSP 플랫폼 드라이버에 정의된 값이며, 정의되지 않은 경우 30초로 설정됩니다.
 */

#define SD_TIMEOUT 2 * 1000

#define SD_DEFAULT_BLOCK_SIZE 512

/*
 * Depending on the use case, the SD card initialization could be done at the
 * application level: if it is the case define the flag below to disable
 * the BSP_SD_Init() call in the SD_Initialize() and add a call to
 * BSP_SD_Init() elsewhere in the application.
 */
/* USER CODE BEGIN disableSDInit */
/* #define DISABLE_SD_INIT */
/* USER CODE END disableSDInit */

/*
 * when using cacheable memory region, it may be needed to maintain the cache
 * validity. Enable the define below to activate a cache maintenance at each
 * read and write operation.
 * Notice: This is applicable only for cortex M7 based platform.
 */
/* USER CODE BEGIN enableSDDmaCacheMaintenance */
/* #define ENABLE_SD_DMA_CACHE_MAINTENANCE  1 */
/* USER CODE END enableSDDmaCacheMaintenance */

/*
* Some DMA requires 4-Byte aligned address buffer to correctly read/write data,
* in FatFs some accesses aren't thus we need a 4-byte aligned scratch buffer to correctly
* transfer data
*/
/* USER CODE BEGIN enableScratchBuffer */
 //#define ENABLE_SCRATCH_BUFFER 
/* USER CODE END enableScratchBuffer */

/* Private variables ---------------------------------------------------------*/
#if defined(ENABLE_SCRATCH_BUFFER)
#if defined (ENABLE_SD_DMA_CACHE_MAINTENANCE)
ALIGN_32BYTES(static uint8_t scratch[BLOCKSIZE]); // 32-Byte aligned for cache maintenance
#else
__ALIGN_BEGIN static uint8_t scratch[BLOCKSIZE] __ALIGN_END;
#endif
#endif
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

#if (osCMSIS <= 0x20000U)
static osMessageQId SDQueueID = NULL;
#else
static osMessageQueueId_t SDQueueID = NULL;
#endif
/* Private function prototypes -----------------------------------------------*/
static DSTATUS SD_CheckStatus(BYTE lun);
DSTATUS SD_initialize (BYTE);
DSTATUS SD_status (BYTE);
DRESULT SD_read (BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
DRESULT SD_write (BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
DRESULT SD_ioctl (BYTE, BYTE, void*);
#endif  /* _USE_IOCTL == 1 */

const Diskio_drvTypeDef  SD_Driver =
{
  SD_initialize,
  SD_status,
  SD_read,
#if  _USE_WRITE == 1
  SD_write,
#endif /* _USE_WRITE == 1 */

#if  _USE_IOCTL == 1
  SD_ioctl,
#endif /* _USE_IOCTL == 1 */
};

uint8_t g_sd_diskio_error=0;

static int SD_CheckStatusWithTimeout(uint32_t timeout)
{
  uint32_t timer;
  /* block until SDIO peripheral is ready again or a timeout occur */
#if (osCMSIS <= 0x20000U)
  timer = osKernelSysTick();
  while( osKernelSysTick() - timer < timeout)
#else
  timer = osKernelGetTickCount();
  while( osKernelGetTickCount() - timer < timeout)
#endif
  {
    if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
    {
      return 0;
    }
    osDelay(100);
  }

  return -1;
}

static DSTATUS SD_CheckStatus(BYTE lun)
{
  Stat = STA_NOINIT;

  if(BSP_SD_GetCardState() == SD_TRANSFER_OK)
  {
    Stat &= ~STA_NOINIT;
  }

  return Stat;
}

/**
  * @brief  Initializes a Drive
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS SD_initialize(BYTE lun)
{
Stat = STA_NOINIT;

  /*
   * check that the kernel has been started before continuing
   * as the osMessage API will fail otherwise
   */
#if (osCMSIS <= 0x20000U)
  if(osKernelRunning())
#else
  if(osKernelGetState() == osKernelRunning)
#endif
  {
#if !defined(DISABLE_SD_INIT)

    if(BSP_SD_Init() == MSD_OK)
    {
      Stat = SD_CheckStatus(lun);
    }

#else
    Stat = SD_CheckStatus(lun);
#endif

    /*
    * if the SD is correctly initialized, create the operation queue
    * if not already created
    */

    if (Stat != STA_NOINIT)
    {
      if (SDQueueID == NULL)
      {
 #if (osCMSIS <= 0x20000U)
      osMessageQDef(SD_Queue, QUEUE_SIZE, uint16_t);
      SDQueueID = osMessageCreate (osMessageQ(SD_Queue), NULL);
#else
      SDQueueID = osMessageQueueNew(QUEUE_SIZE, 2, NULL);
#endif
      }

      if (SDQueueID == NULL)
      {
        Stat |= STA_NOINIT;
      }
    }
  }
  
  if(Stat == 0)
  {
    journal_init_and_recover();
  }

  return Stat;
}

/**
  * @brief  Gets Disk Status
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS SD_status(BYTE lun)
{
  return SD_CheckStatus(lun);
}



/* USER CODE BEGIN beforeReadSection */
/* can be used to modify previous code / undefine following code / add new code */
/* USER CODE END beforeReadSection */
/**
  * @brief  Reads Sector(s)
  * @param  lun : not used
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */

DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
  uint8_t ret;
  uint16_t event=0;
  uint32_t timer;
  DRESULT res = RES_ERROR;
  osStatus_t status;

  if (BSP_PlatformIsDetected() == SD_NOT_PRESENT)
  {
    g_sd_diskio_error = 1;
    return RES_ERROR;
  }

    if (SD_CheckStatusWithTimeout(SD_TIMEOUT) < 0)
    {
      g_sd_diskio_error = 1;

      return res;
    }

    /* Fast path cause destination buffer is correctly aligned */


  ret = BSP_SD_ReadBlocks_DMA((uint32_t *)buff, (uint32_t)(sector), count);

  if (ret == MSD_OK)
  {
      status = osMessageQueueGet(SDQueueID, (void *)&event, NULL, SD_TIMEOUT);
      if ((status == osOK) && (event == READ_CPLT_MSG))
      {
        timer = osKernelGetTickCount();

        /* block until SDIO IP is ready or a timeout occur */
        while(osKernelGetTickCount() - timer <SD_TIMEOUT)
        {
           if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
           {
              res = RES_OK;
              break;
            }
         }
      }
      else
      {
        ERROR_PRINTF("SD read fail %p(sector:%d read count:%d)\r\n", buff,sector, count);
        g_sd_diskio_error = 5;
      }
  }
  

  if(res !=RES_OK)
  {
    g_sd_diskio_error = 2;
  }
  return res;
}


int read_sd_sector(char *buffer,uint32_t sector)
{
  return (int)SD_read(0,buffer, sector, 1);
}

/* USER CODE BEGIN beforeWriteSection */
/* can be used to modify previous code / undefine following code / add new code */
/* USER CODE END beforeWriteSection */
/**
  * @brief  Writes Sector(s)
  * @param  lun : not used
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
#include "journal_manager.h" // 저널링 함수 선언 포함

// journal_manager.h 에 extern 으로 선언되어 있습니다.
// extern bool SD_write_sector(uint32_t lba, const uint8_t *sector_data); 

/**
 * @brief SD 카드에 순수하게 단일 섹터를 쓰는 함수 (저널링 로직 포함하지 않음)
 * 이 함수는 journal_init_and_recover() 에서 복구 목적으로 호출됩니다.
 * @param sector 쓰기 시작 섹터 (LBA)
 * @param buff 쓰고자 하는 512바이트 섹터 데이터
 * @return true: 쓰기 성공, false: 쓰기 실패
 */
bool SD_write_sector(uint32_t sector, const uint8_t *buff)
{
    // FATFS diskio.c의 SD_write 함수에서 가져온 핵심 쓰기 로직 재사용
    uint16_t event;
    osStatus_t status;
    uint32_t timer;
    const UINT count = 1; // 항상 단일 섹터 쓰기

    // 1. SD 카드 감지 및 상태 확인 (FATFS diskio와 동일)
    if (BSP_PlatformIsDetected() == SD_NOT_PRESENT) {
        return false;
    }
    if (SD_CheckStatusWithTimeout(SD_TIMEOUT) < 0) {
        return false;
    }

    // 2. DMA 쓰기 요청 (저널링 로직 제외)
    if (BSP_SD_WriteBlocks_DMA((uint32_t*)buff, (uint32_t)sector, count) == MSD_OK)
    {
        // 3. DMA 전송 완료 대기 (RTOS 메시지 큐 대기)
        status = osMessageQueueGet(SDQueueID, (void *)&event, NULL, SD_TIMEOUT);
        
        if ((status == osOK) && (event == WRITE_CPLT_MSG))
        {
            // 4. SDIO IP 상태 확인 (실제 카드 쓰기 완료 확인)
            timer = osKernelGetTickCount();
            while(osKernelGetTickCount() - timer < SD_TIMEOUT)
            {
                if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
                {
                    // 쓰기 성공
                    return true;
                }
            }
        }
    }
    
    // 쓰기 실패
    return false;
}



DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
    DRESULT res = RES_ERROR;
    uint32_t timer;
    uint16_t event;
    osStatus_t status;


    // 1. SD 카드 감지 및 상태 확인 (기존 로직)
    if (BSP_PlatformIsDetected() == SD_NOT_PRESENT)
    {
        g_sd_diskio_error = 1;
        return RES_ERROR;
    }

    if (SD_CheckStatusWithTimeout(SD_TIMEOUT) < 0)
    {
        g_sd_diskio_error = 3;
        return res;
    }
    
    // 단일 섹터 쓰기(count=1)에만 저널링을 적용
    if (count == 1) 
    {
        // 2SD 카드 쓰기 전에 FRAM에 백업합니다.
        if (!journal_backup_sector(sector, buff))
        {
            g_sd_diskio_error = 5; // 저널 오류 코드
            return RES_ERROR;
        }
    }

    if (BSP_SD_WriteBlocks_DMA((uint32_t*)buff, (uint32_t)(sector), count) == MSD_OK)
    {
        // 4. DMA 전송 완료 대기
        status = osMessageQueueGet(SDQueueID, (void *)&event, NULL, SD_TIMEOUT);
        
        if ((status == osOK) && (event == WRITE_CPLT_MSG))
        {
            timer = osKernelGetTickCount();
            while(osKernelGetTickCount() - timer < SD_TIMEOUT)
            {
                if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
                {
                    res = RES_OK;
                    break;
                }
            }
        }
    }
    



    if (res == RES_OK) 
    {
        if (count == 1) {
            // Commit: SD 카드 쓰기 및 상태 확인이 모두 성공했으므로 FRAM 백업본을 무효화합니다.
            if (!journal_commit()) {
                // Commit 실패 시, 데이터는 일관되나 다음 리셋 시 불필요한 Recovery 시도 발생 가능.
                // 데이터 무결성 자체는 유지되므로 RES_OK 반환 (치명적인 오류로 처리하려면 RES_ERROR 반환 가능)
                // g_sd_diskio_error = 6; // 저널 커밋 오류 코드 (필요하다면)
            }
        }
    }
    else 
    {
        g_sd_diskio_error = 4;
    }
    
    return res;
}
 #endif /* _USE_WRITE == 1 */

/* USER CODE BEGIN beforeIoctlSection */
/* can be used to modify previous code / undefine following code / add new code */
/* USER CODE END beforeIoctlSection */
/**
  * @brief  I/O control operation
  * @param  lun : not used
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff)
{
  DRESULT res = RES_ERROR;
  BSP_SD_CardInfo CardInfo;

  if (Stat & STA_NOINIT) return RES_NOTRDY;

  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC :
    res = RES_OK;
    break;

  /* Get number of sectors on the disk (DWORD) */
  case GET_SECTOR_COUNT :
    BSP_SD_GetCardInfo(&CardInfo);
    *(DWORD*)buff = CardInfo.LogBlockNbr;
    res = RES_OK;
    break;

  /* Get R/W sector size (WORD) */
  case GET_SECTOR_SIZE :
    BSP_SD_GetCardInfo(&CardInfo);
    *(WORD*)buff = CardInfo.LogBlockSize;
    res = RES_OK;
    break;

  /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE :
    BSP_SD_GetCardInfo(&CardInfo);
    *(DWORD*)buff = CardInfo.LogBlockSize / SD_DEFAULT_BLOCK_SIZE;
    res = RES_OK;
    break;

  default:
    res = RES_PARERR;
  }

  return res;
}
#endif /* _USE_IOCTL == 1 */

/* USER CODE BEGIN afterIoctlSection */
/* can be used to modify previous code / undefine following code / add new code */
/* USER CODE END afterIoctlSection */

/* USER CODE BEGIN callbackSection */
/* can be used to modify / following code or add new code */
/* USER CODE END callbackSection */
/**
  * @brief Tx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void BSP_SD_WriteCpltCallback(void)
{

  /*
   * No need to add an "osKernelRunning()" check here, as the SD_initialize()
   * is always called before any SD_Read()/SD_Write() call
   */
#if (osCMSIS < 0x20000U)
   osMessagePut(SDQueueID, WRITE_CPLT_MSG, 0);
#else
   const uint16_t msg = WRITE_CPLT_MSG;
   osMessageQueuePut(SDQueueID, (const void *)&msg, NULL, 0);
#endif
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void BSP_SD_ReadCpltCallback(void)
{
  /*
   * No need to add an "osKernelRunning()" check here, as the SD_initialize()
   * is always called before any SD_Read()/SD_Write() call
   */
#if (osCMSIS < 0x20000U)
   osMessagePut(SDQueueID, READ_CPLT_MSG, 0);
#else
   const uint16_t msg = READ_CPLT_MSG;
   osMessageQueuePut(SDQueueID, (const void *)&msg, NULL, 0);
#endif
}

/* USER CODE BEGIN ErrorAbortCallbacks */
/*
void BSP_SD_AbortCallback(void)
{
#if (osCMSIS < 0x20000U)
   osMessagePut(SDQueueID, RW_ABORT_MSG, 0);
#else
   const uint16_t msg = RW_ABORT_MSG;
   osMessageQueuePut(SDQueueID, (const void *)&msg, NULL, 0);
#endif
}
*/
/* USER CODE END ErrorAbortCallbacks */

/* USER CODE BEGIN lastSection */
/* can be used to modify / undefine previous code or add new code */
/* USER CODE END lastSection */
