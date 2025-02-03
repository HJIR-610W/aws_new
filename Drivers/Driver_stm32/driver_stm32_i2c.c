

#include "stm32f4xx_hal.h"
#include "driver_stm32_i2c.h"
#include "cmsis_os2.h"
#include "pcb_define.h"



#define I2C_TIMEOUT 1000
typedef struct i2c_cfg_s
{
  I2C_HandleTypeDef *handle;//STM SPI HANDLE
}i2c_cfg_t;


I2C_HandleTypeDef hi2c1={.Instance=I2C1};
I2C_HandleTypeDef hi2c2={.Instance=I2C2};


i2c_cfg_t g_stm32_cfg[2]={{.handle=&hi2c1},{.handle =&hi2c2}};
driver_t g_stm32_i2c[2];




/* I2C1 init function */
void MX_I2C2_Init(void)
{
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    //    Error_Handler(__FILE__,__LINE__);
  }

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C2)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOH_CLK_ENABLE();

    GPIO_InitStruct.Pin = I2C2_CLK_Pin|I2C2_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(I2C2_SDA_GPIO_Port, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C2_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C2)
  {
    __HAL_RCC_I2C1_CLK_DISABLE();

    HAL_GPIO_DeInit(I2C2_SDA_GPIO_Port, I2C2_CLK_Pin);

    HAL_GPIO_DeInit(I2C2_SDA_GPIO_Port, I2C2_SDA_Pin);

  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

driver_t * driver_stm32_i2c_open(uint32_t num,void *opt)
{

  switch (num)
  {
  case STM32_I2C_1:

    break;
  case STM32_I2C_2:
  
  if(g_stm32_i2c[num].opened ==false)
  {
    g_stm32_i2c[num].opened = true;
    MX_I2C2_Init();
    g_stm32_cfg[STM32_I2C_2].handle = &hi2c2;
    g_stm32_i2c[STM32_I2C_2].cfg = &g_stm32_cfg[STM32_I2C_2];

    if(g_stm32_i2c[num].sem == NULL)
    {
      g_stm32_i2c[num].sem  =   osSemaphoreNew(1, 1, NULL); 
    }
  }

  break;
  default:

    break;
  }

  return &g_stm32_i2c[num];
  
}

int32_t stm32_i2c_send(driver_t *drv, uint32_t address,uint8_t reg,const uint8_t *pData,uint16_t dataLen)
{
  HAL_StatusTypeDef status;
  i2c_cfg_t *cfg = (i2c_cfg_t *)drv->cfg;

  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
  status = HAL_I2C_Mem_Write(cfg->handle, address<<1,reg, I2C_MEMADD_SIZE_8BIT,(uint8_t *)pData, dataLen, I2C_TIMEOUT);
 
  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }

  return status;
}
int32_t stm32_i2c_read(driver_t *drv,uint32_t address,uint8_t reg,uint8_t *pData,uint16_t readCnt)
{
  i2c_cfg_t *cfg = (i2c_cfg_t *)drv->cfg;
  HAL_StatusTypeDef status;

  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }

  status = HAL_I2C_Mem_Read(cfg->handle, address<<1, reg, I2C_MEMADD_SIZE_8BIT, pData, readCnt, I2C_TIMEOUT); 
  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }
return status;
}
int32_t stm32_i2c_recv_byte(driver_t *drv,uint8_t address,uint8_t *pBuff,uint32_t readCnt)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  i2c_cfg_t *cfg = (i2c_cfg_t *)drv->cfg;
  
    if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
  status = HAL_I2C_Master_Receive(cfg->handle, address<<1, pBuff, readCnt, 1000);
  if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }

return status;
}
int32_t stm32_i2c_send_byte(driver_t *drv,uint8_t address,uint8_t *pData,uint32_t dataLen)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  i2c_cfg_t *cfg = (i2c_cfg_t *)drv->cfg;

  if(drv->sem)
  {
    osSemaphoreAcquire(drv->sem, osWaitForever);
  }
  status = HAL_I2C_Master_Transmit(cfg->handle, address<<1, pData, dataLen, 1000);
      if(drv->sem)
  {
    osSemaphoreRelease(drv->sem);
  }
  return status;

}

