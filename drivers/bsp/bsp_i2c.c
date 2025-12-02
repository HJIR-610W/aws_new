

#include "bsp_i2c.h"
#include "pcb_define.h"
#include "system_err.h"
#include "os_user_def.h"

#define I2C_TIMEOUT 1000

typedef struct i2c_instance_s
{
  osMutexId_t *lock;
  bool opened;
  I2C_HandleTypeDef i2c;
} i2c_instance_t;

i2c_instance_t i2c_instance[STM32_I2C_MAX] = {
    [STM32_I2C_1] = {.i2c.Instance = I2C1, .opened = false, .lock = 0},
    [STM32_I2C_2] = {.i2c.Instance = I2C2, .opened = false, .lock = 0}};

void MX_I2C_Init(int num)
{
  HAL_StatusTypeDef status = HAL_OK;
  I2C_HandleTypeDef *hi2c;

  hi2c = &i2c_instance[num].i2c;

   if (hi2c->Instance == I2C1)
  {
    hi2c->Instance = I2C1;
    hi2c->Init.ClockSpeed = 100000;  // 표준 속도 100KHz
    hi2c->Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c->Init.OwnAddress1 = 0;
    hi2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c->Init.OwnAddress2 = 0;
    hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  }
  else if (hi2c->Instance == I2C2)
  {
    hi2c->Instance = I2C2;
    hi2c->Init.ClockSpeed = 100000;  // 표준 속도 100KHz
    hi2c->Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c->Init.OwnAddress1 = 0;
    hi2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c->Init.OwnAddress2 = 0;
    hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  }
  status = HAL_I2C_Init(hi2c);

  if (status != HAL_OK)
  {
    ERROR_PRINTF("i2c[%d] init err:%d", num, status);
  }
}




void i2c1_bus_recovery(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // 1. SCL/SDA 핀을 GPIO로 재설정 (Open-Drain)
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  // SCL 초기화
  GPIO_InitStruct.Pin = I2C1_SCL_Pin;
  HAL_GPIO_Init(I2C1_SCL_GPIO_Port, &GPIO_InitStruct);

  // SDA는 입력으로 설정 (상태 감지용)
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pin = I2C1_SDA_Pin;
  HAL_GPIO_Init(I2C1_SDA_GPIO_Port, &GPIO_InitStruct);

  // 2. SDA 상태 확인
  if (HAL_GPIO_ReadPin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin) == GPIO_PIN_RESET)
  {
    // 3. SDA가 LOW인 경우: SCL을 pulsing하여 버스 복구 시도
    for (int i = 0; i < 9; i++)
    {
      HAL_GPIO_WritePin(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, GPIO_PIN_SET);
      HAL_Delay(1);  // 최소 4us 이상, 여기선 1ms
      HAL_GPIO_WritePin(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, GPIO_PIN_RESET);
      HAL_Delay(1);
    }

    // 4. STOP 조건 시도: SDA high 상태로 SCL을 high로 하면서 SDA도 high
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pin = I2C1_SDA_Pin;
    HAL_GPIO_Init(I2C1_SDA_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
  }

  // 5. 핀을 원래대로 복구 (I2C 모드로 다시 초기화 필요)
  HAL_GPIO_DeInit(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin);
  HAL_GPIO_DeInit(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin);
}


void i2c2_bus_recovery(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // 1. SCL/SDA 핀을 GPIO Open-Drain 모드로 전환
  __HAL_RCC_GPIOH_CLK_ENABLE();

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  // SCL = PH4
  GPIO_InitStruct.Pin = I2C2_SCL_Pin;
  HAL_GPIO_Init(I2C2_SCL_GPIO_Port, &GPIO_InitStruct);

  // SDA = PH5를 입력으로 설정하여 상태 감지
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pin = I2C2_SDA_Pin;
  HAL_GPIO_Init(I2C2_SDA_GPIO_Port, &GPIO_InitStruct);

  // 2. SDA 상태가 LOW이면 복구 진행
  if (HAL_GPIO_ReadPin(I2C2_SDA_GPIO_Port, I2C2_SDA_Pin) == GPIO_PIN_RESET)
  {
    // 3. SCL 클럭을 9번 출력하여 stuck 해제 시도
    for (int i = 0; i < 9; i++)
    {
      HAL_GPIO_WritePin(I2C2_SCL_GPIO_Port, I2C2_SCL_Pin, GPIO_PIN_SET);
      HAL_Delay(1);
      HAL_GPIO_WritePin(I2C2_SCL_GPIO_Port, I2C2_SCL_Pin, GPIO_PIN_RESET);
      HAL_Delay(1);
    }

    // 4. STOP 조건 강제: SDA를 high로 유지하며 SCL high
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pin = I2C2_SDA_Pin;
    HAL_GPIO_Init(I2C2_SDA_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(I2C2_SCL_GPIO_Port, I2C2_SCL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(I2C2_SDA_GPIO_Port, I2C2_SDA_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
  }

  // 5. 핀 복구: HAL_I2C_Init() 전에 GPIO 해제 필요
  HAL_GPIO_DeInit(I2C2_SCL_GPIO_Port, I2C2_SCL_Pin);
  HAL_GPIO_DeInit(I2C2_SDA_GPIO_Port, I2C2_SDA_Pin);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if(i2cHandle->Instance==I2C1)
  {
    i2c1_bus_recovery();

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = I2C1_SCL_Pin|I2C1_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(I2C1_SCL_GPIO_Port, &GPIO_InitStruct);

    __HAL_RCC_I2C1_CLK_ENABLE();

  }
  else if(i2cHandle->Instance==I2C2)
  {
     i2c2_bus_recovery();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    GPIO_InitStruct.Pin = I2C2_SCL_Pin|I2C2_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(I2C2_SDA_GPIO_Port, &GPIO_InitStruct);

    __HAL_RCC_I2C2_CLK_ENABLE();

  }

}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
    __HAL_RCC_I2C1_CLK_DISABLE();

    HAL_GPIO_DeInit(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin);

    HAL_GPIO_DeInit(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin);
  }
  else if(i2cHandle->Instance==I2C2)
  {
    __HAL_RCC_I2C2_CLK_DISABLE();

    HAL_GPIO_DeInit(I2C2_SCL_GPIO_Port, I2C2_SCL_Pin);

    HAL_GPIO_DeInit(I2C2_SDA_GPIO_Port, I2C2_SDA_Pin);

  }

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

int32_t bsp_i2c_init(uint32_t num)
{

  if(i2c_instance[num].opened == false)
  {
    OS_CREATE_MUTEX(i2c_instance[num].lock);
    MX_I2C_Init(num);
    i2c_instance[num].opened = true;
    return 0;
  }

  return 1;

}

int32_t bsp_i2c_send(int num, uint32_t address,uint8_t reg,const uint8_t *pData,uint16_t dataLen)
{
  HAL_StatusTypeDef status;
  I2C_HandleTypeDef *handle;

  OS_MUTEX_LOCK(i2c_instance[num].lock, osWaitForever);
  handle = &i2c_instance[num].i2c;

  status = HAL_I2C_Mem_Write(handle, address<<1,reg, I2C_MEMADD_SIZE_8BIT,(uint8_t *)pData, dataLen, I2C_TIMEOUT);
  if (status != HAL_OK)
  {
    uint32_t sr1;
    uint32_t sr2;
    sr1 = handle->Instance->SR1;
    sr2 = handle->Instance->SR2;

    ERROR_PRINTF("i2c error %d,SR1:0x%08X,SR2:0x%08X", status, sr1, sr2);
    
    (void)sr1;
    (void)sr2;
    
    if (handle->Instance == I2C1)
    {
      i2c1_bus_recovery();
    }
    else if (handle->Instance == I2C2)
    {
      i2c2_bus_recovery();
    }
    HAL_I2C_MspInit(handle);
  }

  OS_MUTEX_UNLOCK(i2c_instance[num].lock);

  return status;
}
int32_t bsp_i2c_read(int num ,uint32_t address,uint8_t reg,uint8_t *pData,uint16_t readCnt)
{
  I2C_HandleTypeDef*handle;
  HAL_StatusTypeDef status;

  OS_MUTEX_LOCK(i2c_instance[num].lock, osWaitForever);


  handle = &i2c_instance[num].i2c;
  status = HAL_I2C_Mem_Read(handle, address << 1, reg, I2C_MEMADD_SIZE_8BIT, pData, readCnt,
                            I2C_TIMEOUT);
  if (status != HAL_OK)
  {
    uint32_t sr1;
    uint32_t sr2;
    
    sr1 = handle->Instance->SR1;
    sr2 = handle->Instance->SR2;

    ERROR_PRINTF("i2c error %d,SR1:0x%08X,SR2:0x%08X", status, sr1, sr2);
    (void)sr1;
    (void)sr2;
    if (handle->Instance == I2C1)
    {
      i2c1_bus_recovery();
    }
    else if (handle->Instance == I2C2)
    {
      i2c2_bus_recovery();
    }
    HAL_I2C_MspInit(handle);
  }

  OS_MUTEX_UNLOCK(i2c_instance[num].lock);

  return status;
}
int32_t bsp_i2c_recv_byte(int num,uint8_t address,uint8_t *pBuff,uint32_t readCnt)
{
  HAL_StatusTypeDef status = HAL_OK;
  I2C_HandleTypeDef *handle;

  OS_MUTEX_LOCK(i2c_instance[num].lock, osWaitForever);
  
  handle = &i2c_instance[num].i2c;

  status = HAL_I2C_Master_Receive(handle, address << 1, pBuff, readCnt, 1000);
  if (status != HAL_OK)
  {
    uint32_t sr1;
    uint32_t sr2;
    sr1 = handle->Instance->SR1;
    sr2 = handle->Instance->SR2;

    ERROR_PRINTF("i2c error %d,SR1:0x%08X,SR2:0x%08X", status, sr1, sr2);
    (void)sr1;
    (void)sr2;
    if (handle->Instance == I2C1)
    {
      i2c1_bus_recovery();
    }
    else if (handle->Instance == I2C2)
    {
      i2c2_bus_recovery();
    }
    HAL_I2C_MspInit(handle);
  }

  OS_MUTEX_UNLOCK(i2c_instance[num].lock);

  return status;
}
int32_t bsp_i2c_send_byte(int num,uint8_t address,uint8_t *pData,uint32_t dataLen)
{
  HAL_StatusTypeDef status = HAL_OK;
  I2C_HandleTypeDef *handle;

  OS_MUTEX_LOCK(i2c_instance[num].lock, osWaitForever);

  handle = &i2c_instance[num].i2c;
  status = HAL_I2C_Master_Transmit(handle, address<<1, pData, dataLen, 1000);
  if (status != HAL_OK)
  {
    uint32_t sr1;
    uint32_t sr2;
    sr1 = handle->Instance->SR1;
    sr2 = handle->Instance->SR2;
    
    ERROR_PRINTF("i2c error %d,SR1:0x%08X,SR2:0x%08X", status,sr1,sr2);
    (void)sr1;
    (void)sr2;
    if (handle->Instance == I2C1)
    {
      i2c1_bus_recovery();
    }
    else if (handle->Instance == I2C2)
    {
      i2c2_bus_recovery();
    }
    HAL_I2C_MspInit(handle);
  }

  OS_MUTEX_UNLOCK(i2c_instance[num].lock);

  return status;

}

