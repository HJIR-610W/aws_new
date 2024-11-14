

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cmsis_os.h"
#include "driver_uart.h"
#include "driver_stm32_uart.h"
#include "driver_485.h"
#include "driver_sdi.h"
#include "stm32f4xx_hal.h"

#include "mcu_utile.h"
typedef struct {
    char cmd[10];       // "write"
    char target[10];    // "gpio"
    char port[10];      // "porta"
    int pin;            // pin number (1)
    int data;           // data value (0)
} ParsedData;


const osThreadAttr_t testTxTask_attributes = {
  .name = "testTask",
  .stack_size = 1024,
  .priority = (osPriority_t) osPriorityNormal,
};


extern int32_t debug_printf(const char * pFmt, ...);
ParsedData parse_serial_data(const char *input) {
    ParsedData result;
    memset(&result, 0, sizeof(result));  // 구조체 초기화

    // 수신된 데이터를 복사하여 strtok에 사용할 문자열 생성
    char buffer[100];
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    // 첫 번째 ','를 기준으로 나눠서 cmd, target, port 추출
    char *token = strtok(buffer, ",");
    if (token) {
        sscanf(token, "%9[^,]", result.cmd);  // cmd 파싱
    }

    token = strtok(NULL, ",");
    if (token) {
        sscanf(token, "%9[^,]", result.target);  // target 파싱
    }

    token = strtok(NULL, ",");
    if (token) {
        sscanf(token, "%9[^,]", result.port);  // port 파싱
    }

    // 나머지 토큰을 "="로 나누어 pin과 data 추출
    while ((token = strtok(NULL, ",")) != NULL) {
        char key[10];
        int value;

        sscanf(token, "%9[^=]=%d", key, &value);

        if (strcmp(key, "pin") == 0) {
            result.pin = value;
        } else if (strcmp(key, "data") == 0) {
            result.data = value;
        }
    }

    return result;
}

#define BUFFER_SIZE 100
#define MAX_TOKENS 10
void split_string( char *input,  char *list[], int *count) {
    // 입력 문자열을 복사하여 사용 (strtok는 원본 문자열을 수정하므로)
    static char buffer[BUFFER_SIZE];
    strncpy(buffer, input, BUFFER_SIZE - 1);
    buffer[BUFFER_SIZE - 1] = '\0';

    // 초기화
    *count = 0;

    // ','를 기준으로 문자열을 분리하여 list 배열에 포인터 저장
    char *token = strtok(buffer, ",");
    while (token != NULL && *count < MAX_TOKENS) {
        list[*count] = token;
        (*count)++;
        token = strtok(NULL, ",");
    }
}



void test_do_init(GPIO_TypeDef *port,uint32_t pin)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  board_clk_gpio(port);
  GPIO_InitStruct.Pin = pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(port ,&GPIO_InitStruct);
}
void test_di_init(GPIO_TypeDef *port,uint32_t pin)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  

  
  board_clk_gpio(port);
  GPIO_InitStruct.Pin = pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(port ,&GPIO_InitStruct);
}



driver_t *g_rs485_a     = NULL;
driver_t *g_rs485_b     = NULL;
driver_t *g_sdi         = NULL;
driver_t *g_uart3       = NULL;
driver_t * g_stm_uart_1 = NULL;
driver_t * g_quad_232_1 = NULL;
driver_t * g_quad_ttl_2 = NULL;
driver_t * g_232_A      = NULL;
driver_t * g_232_B      = NULL;
driver_t * g_232_C      = NULL;
driver_t * g_232_D      = NULL;


void test_cmd(char *data)
{
  char *list[10];
  int count=0;
  ParsedData cmd;
  uint16_t pin;
  char temp[20];
  
  uint16_t cnt;

  split_string(data,list,&count);
  if(count)
  {
    if(strncmp(list[0],"write",5)==0)
    {
      if(strncmp(list[1],"gpio",4)==0)
      {
        if(strncmp(list[2],"port",4)==0)
        {
          cmd = parse_serial_data(data);
          switch(list[2][4])
          {
            case 'a':
              test_do_init(GPIOA,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOA,1<<cmd.pin,cmd.data);
            break;
            case 'b':
              test_do_init(GPIOB,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOB,1<<cmd.pin,cmd.data);
            break;
            case 'c':
              test_do_init(GPIOC,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOC,1<<cmd.pin,cmd.data);
            break;
            case 'd':
              test_do_init(GPIOD,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOD,1<<cmd.pin,cmd.data);
            break;
            case 'e':
              test_do_init(GPIOE,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOE,1<<cmd.pin,cmd.data);
            break;
            case 'f':
              test_do_init(GPIOF,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOF,1<<cmd.pin,cmd.data);
            break;
            case 'g':
              test_do_init(GPIOG,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOG,1<<cmd.pin,cmd.data);
            break;
            case 'h':
              test_do_init(GPIOH,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOH,1<<cmd.pin,cmd.data);
            break;
            case 'i':
              test_do_init(GPIOI,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOI,1<<cmd.pin,cmd.data);
            break;


          }
        }
        
      }
      else if(strncmp(list[1],"rs485",5)==0)
      {
         if(strncmp(list[2],"port",4)==0)
         {
              cmd = parse_serial_data(data);
              switch(list[2][4])
              {
                case 'a':
                driver_rs485_sends(g_rs485_a,"rs485_a",7);
              cnt = driver_rs485_recv(g_rs485_a,temp,10,3000);
              if(cnt)
              {
                driver_rs485_sends(g_rs485_a,temp,cnt);
              }
                break;
                case 'b':
                driver_rs485_sends(g_rs485_b,"rs485_b",7);
               cnt = driver_rs485_recv(g_rs485_b,temp,10,3000);
              if(cnt)
              {
                driver_rs485_sends(g_rs485_b,temp,cnt);
              }
                break;
              }
         }
      }
      else if(strncmp(list[1],"sdi",3)==0)
      {
        driver_sdi_sends(g_sdi,"sdi",3);
      }
      else if(strncmp(list[1],"rs232",3)==0)
      {
              cmd = parse_serial_data(data);
              switch(list[2][4])
              {
                case '1':
             
                break;
              }
      }
    }
    if(strncmp(list[0],"read",4)==0)
    {
      if(strncmp(list[1],"gpio",4)==0)
      {
        if(strncmp(list[2],"port",4)==0)
        {
                    cmd = parse_serial_data(data);
          switch(list[2][4])
          {
            case 'a':
              test_di_init(GPIOA,1<<cmd.pin);
              pin = HAL_GPIO_ReadPin(GPIOA,1<<cmd.pin);
              debug_printf("%s,pin%d=%d\r\n",cmd.port,cmd.pin,pin);
            break;
            case 'b':
              test_di_init(GPIOB,1<<cmd.pin);
              pin = HAL_GPIO_ReadPin(GPIOB,1<<cmd.pin);
              debug_printf("%s,pin%d=%d\r\n",cmd.port,cmd.pin,pin);
            break;
            case 'c':
              test_di_init(GPIOC,1<<cmd.pin);
              pin = HAL_GPIO_ReadPin(GPIOC,1<<cmd.pin);
              debug_printf("%s,pin%d=%d\r\n",cmd.port,cmd.pin,pin);
            break;
            case 'd':
              test_di_init(GPIOD,1<<cmd.pin);
              pin = HAL_GPIO_ReadPin(GPIOD,1<<cmd.pin);
              debug_printf("%s,pin%d=%d\r\n",cmd.port,cmd.pin,pin);
            break;
            case 'e':
              test_di_init(GPIOE,1<<cmd.pin);
              pin = HAL_GPIO_ReadPin(GPIOE,1<<cmd.pin);
              debug_printf("%s,pin%d=%d\r\n",cmd.port,cmd.pin,pin);
            break;
            case 'f':
              test_di_init(GPIOF,1<<cmd.pin);
              pin = HAL_GPIO_ReadPin(GPIOF,1<<cmd.pin);
              debug_printf("%s,pin%d=%d\r\n",cmd.port,cmd.pin,pin);
            break;
            case 'g':
              test_do_init(GPIOG,1<<cmd.pin);
              pin = HAL_GPIO_ReadPin(GPIOG,1<<cmd.pin);
              debug_printf("%s,pin%d=%d\r\n",cmd.port,cmd.pin,pin);
            break;
            case 'h':
              test_di_init(GPIOH,1<<cmd.pin);
              pin = HAL_GPIO_ReadPin(GPIOH,1<<cmd.pin);
              debug_printf("%s,pin%d=%d\r\n",cmd.port,cmd.pin,pin);
            break;
            case 'i':
              test_di_init(GPIOI,1<<cmd.pin);
              pin = HAL_GPIO_ReadPin(GPIOI,1<<cmd.pin);
              debug_printf("%s,pin%d=%d\r\n",cmd.port,cmd.pin,pin);
            break;

              
            break;

          }

        }
        
      }
    }
    if(strncmp(list[0],"set",3)==0)
    {
                int32_t baud;
      if(strncmp(list[1],"rs485",5)==0)
      {
        switch (list[1][5])
        {
        case 'a':

          baud = atoi(list[2]);
          driver_rs485_set(g_rs485_a,RS485_CMD_SET_BAUD,(void *)baud);
          break;
        case 'b':
          baud = atoi(list[2]);
          driver_rs485_set(g_rs485_b,RS485_CMD_SET_BAUD,(void *)baud);
        break;
        default:
          break;
        }


      }
    }
  }
}

extern void set_debug_uart_handle(driver_t *drv);
void testTask(void *argument)
{
  uint8_t buff[512];

  osStatus status;
  uint8_t data;
  uint16_t cnt=0;
  g_uart3 = stm32_uart_open(UART_STM32_3);

  

  g_rs485_a = driver_rs485_open(RS485_A);
  g_rs485_a = driver_rs485_open(RS485_B);
  g_sdi     = driver_sdi_open(SDI_1);


  g_stm_uart_1 = driver_uart_open(UART_STM32_1);
  g_quad_232_1 =  driver_uart_open(UART_EX_232_1);
  g_quad_ttl_2 =  driver_uart_open(UART_EX_TTL_2);
  g_232_A =  driver_uart_open(UART_EX_232_A_3);
  g_232_B =  driver_uart_open(UART_EX_232_B_4);
  g_232_C =  driver_uart_open(UART_EX_232_C_7);
  g_232_D =  driver_uart_open(UART_EX_232_D_8);

  
    set_debug_uart_handle(g_stm_uart_1);
  while(1)
  {
    if (stm32_uart_recv_byte(g_stm_uart_1,&data,osWaitForever))
    {
      buff[cnt++] = data;
      if(data=='\n')
      {
        buff[cnt]=0;
        test_cmd(buff);

        memset(buff,0,sizeof(buff));
        cnt = 0;
      }
      if(cnt==sizeof(buff))
      {
        cnt = 0;
      }
    }
  }
 
}



void testTask_init(void)
{
 
  osThreadNew(testTask, NULL, &testTxTask_attributes);

}