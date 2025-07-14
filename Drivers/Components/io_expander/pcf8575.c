
#include "pcf8575.h"

#include <string.h>


#include "os_user_def.h"
#include "system_err.h"
#include "bsp_i2c.h"
#define PCF8575_0X20 0
#define PCF8575_MAX 1

typedef struct pcf8575_instance_s
{
  bool opened;
  int i2c_num;
  void *sem;
  uint16_t address;
  uint16_t dir;  // 핀 방향: 1=INPUT(읽기), 0=OUTPUT(쓰기)
  uint8_t pin[16];  // 핀 번호 매핑 배열: pin[물리핀번호] = 논리핀번호
  uint16_t output_cache;  // OUTPUT 핀들의 현재 상태
  uint16_t input_cache;   // INPUT 핀들의 마지막 읽은 상태
  uint16_t initial_output;  // 초기 출력값
  bool in_cache_valid;
  bool out_cache_valid;
} pcf8575_inst_t;

pcf8575_inst_t pcf8575_inst[PCF8575_MAX] = {[PCF8575_0X20] = {.opened = false,
                                                              .dir = 0x00FF,  // 0~7 입력 8~15 출력
                                                              .address = 0x20,
                                                              .in_cache_valid = false,
                                                              .out_cache_valid = false,
                                                              .output_cache = 0xFF00,
                                                              .input_cache = 0,
                                                              .initial_output = 0xFF00}};

pcf8575_result_t pcf8575_write_port(int number, uint16_t port_data)
{
  uint8_t data[2];
  int32_t result;

  OS_PEND_SEM(pcf8575_inst[number].sem, osWaitForever);

  data[0] = (uint8_t)(port_data & 0xFF);
  data[1] = (uint8_t)((port_data >> 8) & 0xFF);

  result = bsp_i2c_send_byte(pcf8575_inst[number].i2c_num, pcf8575_inst[number].address, data, 2);
  
  if(result == 0)
  {
    pcf8575_inst[number].output_cache = port_data;
    pcf8575_inst[number].out_cache_valid = true;
    result = PCF8575_OK;
  }
  else
  {
    result = PCF8575_ERROR_I2C;
  }

  OS_POST_SEM(pcf8575_inst[number].sem);
  return (pcf8575_result_t)result;
}

pcf8575_result_t pcf8575_read_port(int number, uint16_t *port_data)
{
  uint8_t data[2];
  int result;

  OS_PEND_SEM(pcf8575_inst[number].sem, osWaitForever);

  result = bsp_i2c_recv_byte(pcf8575_inst[number].i2c_num, pcf8575_inst[number].address, data, 2);
  
  if(result == 0)
  {
    *port_data = ((uint16_t)data[0]) | (((uint16_t)data[1]) << 8);
    pcf8575_inst[number].input_cache = *port_data;
    pcf8575_inst[number].in_cache_valid = true;
    result = PCF8575_OK;
  } else {
    result = PCF8575_ERROR_I2C;
  }

  OS_POST_SEM(pcf8575_inst[number].sem);

  return (pcf8575_result_t)result; 
}




int find_pin(uint8_t *pin, int num)
{
  for(int i = 0; i < 16; i++)
  {
    if(pin[i] == num)
    {
      return i;
    }
  }
  return -1;
}

pcf8575_result_t pcf8575_set_pin_direction(int inst, int pin, bool is_input)
{
  uint16_t current_data;
  pcf8575_result_t result;
  

  result = pcf8575_read_port(inst, &current_data);
  if(result != PCF8575_OK) {
    return result;
  }
  
  if(is_input) {
    // PCF8575 quasi-bidirectional I/O: 입력으로 사용하려면 반드시 HIGH(1)로 설정
    current_data |= (1 << pin);   // HIGH = INPUT 모드 (weak pull-up 활성화)
    pcf8575_inst[inst].dir |= (1 << pin);  // 방향 플래그 설정
  } else {
    // OUTPUT 핀은 실제 출력값에 따라 제어됨 (기본 LOW로 설정)
    current_data &= ~(1 << pin);  // LOW = OUTPUT 모드
    pcf8575_inst[inst].dir &= ~(1 << pin);  // 방향 플래그 설정
  }
  
  result = pcf8575_write_port(inst, current_data);
  if(result == PCF8575_OK) {
    // 캐시 업데이트
    if(is_input) {
      pcf8575_inst[inst].output_cache |= (1 << pin);
    } else {
      pcf8575_inst[inst].output_cache &= ~(1 << pin);
    }
  }
  
  return result;
}

pcf8575_result_t pcf8575_write_single_pin(int number, int pin, int high)
{
  uint16_t port_data;
  pcf8575_result_t result;
  

  if(pcf8575_inst[number].dir & (1 << pin)) {
    return PCF8575_ERROR_WRONG_DIRECTION;
  }


  result = pcf8575_read_port(number, &port_data);
  if(result != PCF8575_OK) {
    return result;
  }
 

  if(high)
  {
    port_data |= (1 << pin);
  } else {
    port_data &= ~(1 << pin);
  }

  return pcf8575_write_port(number, port_data);
}

pcf8575_result_t pcf8575_write_pin(int number, int high)
{
  int pin;
  
  for(int inst = 0; inst < PCF8575_MAX; inst++)
  {
    
    pin = find_pin(pcf8575_inst[inst].pin, number);
    if(pin != -1)
    {
      // OUTPUT 핀인지 확인
      if((pcf8575_inst[inst].dir & (1 << pin)) == 0)
      {
        return pcf8575_write_single_pin(inst, pin, high);
      } else {
        return PCF8575_ERROR_WRONG_DIRECTION;
      }
    }
  }
  
  return PCF8575_ERROR_INVALID_PIN;
}



int32_t pcf8575_read_pin(int number)
{
  int ret=-1;
  int pin;
  uint16_t port_data = 0;
  pcf8575_result_t result;
  
  for(int inst = 0; inst < PCF8575_MAX; inst++)
  {
    
    pin = find_pin(pcf8575_inst[inst].pin, number);
    if(pin != -1)
    {
      // INPUT 핀인지 확인
      if((pcf8575_inst[inst].dir & (1 << pin)) != 0)
      {
        result = pcf8575_read_port(inst, &port_data);
        if(result == PCF8575_OK)
        {
          ret = ((port_data & (1 << pin)) > 0) ? 1 : 0;
          return ret;
        }
        return result;
      } else {
        return PCF8575_ERROR_WRONG_DIRECTION;
      }
    }
  }
  
  return PCF8575_ERROR_INVALID_PIN;
}


pcf8575_result_t pcf8575_init(void)
{
  pcf8575_result_t result;

  if (pcf8575_inst[PCF8575_0X20].opened)
  {
    return PCF8575_OK;  
  }


  pcf8575_inst[PCF8575_0X20].i2c_num = STM32_I2C_2;


  pcf8575_inst[PCF8575_0X20].pin[0] = DI_PCF8575_0;
  pcf8575_inst[PCF8575_0X20].pin[1] = DI_PCF8575_1;
  pcf8575_inst[PCF8575_0X20].pin[2] = DI_PCF8575_2;
  pcf8575_inst[PCF8575_0X20].pin[3] = DI_PCF8575_3;
  pcf8575_inst[PCF8575_0X20].pin[4] = DI_PCF8575_4;
  pcf8575_inst[PCF8575_0X20].pin[5] = DI_PCF8575_5;
  pcf8575_inst[PCF8575_0X20].pin[6] = DI_PCF8575_6;
  pcf8575_inst[PCF8575_0X20].pin[7] = DI_PCF8575_7;

  pcf8575_inst[PCF8575_0X20].pin[8] = DO_PCF8575_0;
  pcf8575_inst[PCF8575_0X20].pin[9] = DO_PCF8575_1;
  pcf8575_inst[PCF8575_0X20].pin[10] = DO_PCF8575_2;
  pcf8575_inst[PCF8575_0X20].pin[11] = DO_PCF8575_3;
  pcf8575_inst[PCF8575_0X20].pin[12] = DO_PCF8575_4;
  pcf8575_inst[PCF8575_0X20].pin[13] = DO_PCF8575_5;
  pcf8575_inst[PCF8575_0X20].pin[14] = DO_PCF8575_6;
  pcf8575_inst[PCF8575_0X20].pin[15] = DO_PCF8575_7;

  // 세마포어 생성
  OS_CREATE_BINARY_SEM(pcf8575_inst[PCF8575_0X20].sem);

  // 세마포어 생성 실패 체크 (매크로에서 처리하지만 추가 검증)
  if (pcf8575_inst[PCF8575_0X20].sem == NULL)
  {
    return PCF8575_ERROR_SEMAPHORE;
  }



  // PCF8575 quasi-bidirectional I/O 특성에 따른 초기 설정:
  // - INPUT 핀들: 반드시 HIGH(1)로 설정하여 weak pull-up 활성화 (입력 모드)
  // - OUTPUT 핀들: 설정된 초기값으로 설정
  uint16_t initial_state =
      pcf8575_inst[PCF8575_0X20].initial_output | pcf8575_inst[PCF8575_0X20].dir;

  bsp_i2c_init(pcf8575_inst[PCF8575_0X20].i2c_num);
  
  result = pcf8575_write_port(PCF8575_0X20, initial_state);
  if (result != PCF8575_OK)
  {
    return result;
  }

   pcf8575_inst[PCF8575_0X20].opened = true;

  return PCF8575_OK;


}

