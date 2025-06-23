


#include <stdint.h>


#include "cmsis_os.h"
#include "pRtuBinRcv.h"
#include "fw_sdcd.h"

extern osSemaphoreId ShRamCrcSema;


uint8_t * g_fwBuff =0;
uint32_t g_fwLen=0;





uint8_t check_firmwareFile(void)
{
  const char *fwFilePath;
  uint8_t err = 0;

  fwFilePath = get_remoteFwFilePath();

  if(osSemaphoreWait(ShRamCrcSema, osWaitForever) == osOK)
  {
    err = UpdateFile_CRC32_Check((char *)fwFilePath, NULL, NULL);
    osSemaphoreRelease(ShRamCrcSema);
    err |= check_firmware();
    if(err)
    {
      return err;
    }
  }
  return err;
}