

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cmsis_os.h"
#include "config.h"
#include "driver_adc.h"
#include "driver_uart.h"
#include "driver_stm32_uart.h"
#include "driver_485.h"
#include "driver_sdi.h"
#include "driver_fram.h"
#include "stm32f4xx_hal.h"
#include "utile.h"
#include "mcu_utile.h"

#define WRITE_CFG(x) driver_fram_write(g_fram,(uint32_t)OFFSET_OF_STRUCT(test_config_t, x),(uint8_t *)&g_test_config.x,sizeof(g_test_config.x));

uint8_t g_adc_average_cnt = 10;
int32_t g_adc_vref = 5980;//mv

adc_calibraion_t single_cali[32];
adc_calibraion_t diff_cali[8];
driver_t *g_fram = NULL;



typedef struct test_config_s
{
  adc_calibraion_t single_cali[32];
  adc_calibraion_t diff_cali[8];
  int32_t Vref;
}test_config_t;

test_config_t g_test_config;

typedef struct 
{
    char cmd[10];       // "write"
    char target[10];    // "gpio"
    char port[10];      // "porta"
    int pin;            // pin number (1)
    int data;           // data value (0)
} ParsedData;


const osThreadAttr_t testTxTask_attributes = {
  .name = "testTask",
  .stack_size = 2048,//2048바이트가 할당됨 하지만 4바이트 단위로 스택은 구성됨
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
driver_t *g_adc_single=NULL;
driver_t *g_adc_diff=NULL;

  const char *nameList[]={"A_SIG_01","A_SIG_02","A_SIG_RTD_0","NOT USE",
                          "A_SIG_03","A_SIG_04","A_SIG_RTD_1","NOT USE",
                          "A_SIG_05","A_SIG_06","NOT USE","NOT USE",
                          "A_SIG_07","A_SIG_08","NOT USE","NOT USE",
                          "A_SIG_09","A_SIG_10","NOT USE","NOT USE",
                          "A_SIG_11","A_SIG_12","NOT USE","NOT USE",
                          "A_SIG_13","A_SIG_14","NOT USE","NOT USE",
                          "A_SIG_15","A_SIG_16","NOT USE","NOT USE"};

  const char *diff_nameLists[] = {"A_SIG_01 - A_SIG_02",
                                  "A_SIG_03 - A_SIG_04",
                                  "A_SIG_05 - A_SIG_06",
                                  "A_SIG_07 - A_SIG_08",
                                  "A_SIG_09 - A_SIG_10",
                                  "A_SIG_11 - A_SIG_12",
                                  "A_SIG_13 - A_SIG_14",
                                  "A_SIG_15 - A_SIG_16"};
float calculate_gain_adc_single(int32_t ch)
{
  float gain;
  int32_t diff;
  int32_t input_diff;
  test_config_t *cfg= &g_test_config;

  diff =  cfg->single_cali[ch].fullset  - cfg->single_cali[ch].offset ;
  input_diff = cfg->single_cali[ch].fullset_input  - cfg->single_cali[ch].offset_input;

  gain = (float)input_diff/(float)diff/1000.0;

  return gain;//mv

}
float calculate_gain_adc_diff(int32_t ch)
{
  float gain;
  int32_t diff;
  int32_t input_diff;
  test_config_t *cfg= &g_test_config;

  diff =  cfg->diff_cali[ch].fullset  - cfg->diff_cali[ch].offset ;
  input_diff = cfg->diff_cali[ch].fullset_input  - cfg->diff_cali[ch].offset_input;

  gain = (float)input_diff/(float)diff/1000.0;

  return gain;//mv

}

float get_voltage(float gain,int32_t adc,int32_t offset)
{
  float voltage;

  voltage = gain*(adc-offset);

  return voltage;
}





float get_voltage_vref(int32_t adc,int32_t offset)
{
  double lsb_value;
  double Vref ;
  float voltage;

  Vref = g_test_config.Vref/1000.0;

  lsb_value = 2*Vref/16777216;//2^24 = 1677216

  voltage = lsb_value *(adc-offset);

  return voltage;
}

void adc_read_single(int32_t ch)
{
  int32_t adc;
  uint8_t err;
  float gain;
  int32_t offset;

  if(g_adc_single == NULL)
  {
    g_adc_single = driver_adc_open(ADC_ADS1220_SINGLE_CH_0);
  }

  adc = driver_adc_read_average(g_adc_single,ch,&err,g_adc_average_cnt);
  if(err)
  {
    debug_printf("ADC_%d,read err\r\n",ch);
  }
  else
  {
    gain = calculate_gain_adc_single(ch);
    offset = g_test_config.single_cali[ch].offset;
    debug_printf("%12s:%d,%.3fv\r\n",nameList[ch],adc, get_voltage_vref(adc,offset));
  }
}

void adc_read_diff(int32_t ch)
{
  uint8_t err;
  int32_t adc;
  int32_t offset;
  float gain;

  adc = driver_adc_read_average(g_adc_diff,ch+ADC_ADS1220_DIFF_CH_0,&err,g_adc_average_cnt);
  if(err)
  {
    debug_printf("%s,read err\r\n",diff_nameLists[ch]);
  }
  else
  {
    gain = calculate_gain_adc_diff(ch);
    offset = g_test_config.single_cali[ch].offset;
    debug_printf("%s:%10d,%6.3fv\r\n",diff_nameLists[ch],adc, get_voltage_vref(adc,offset));
  }
}

void adc_single_all_test(void)
{
  int32_t adc;
  uint8_t err;
  float gain;
  int32_t offset;
  const uint8_t adc_ch_list[]={0,1,4,5,8,9,12,13,16,17,20,21,24,25,28,29,2,6};

  debug_printf("\r\n");
  for(int i = 0 ; i < _countof(adc_ch_list); i++)
  {
    adc = driver_adc_read_average(g_adc_single,adc_ch_list[i],&err,g_adc_average_cnt);
    if(err)
    {
      debug_printf("%s,read err\r\n",nameList[i]);
    }
    else
    {
      gain = calculate_gain_adc_diff(adc_ch_list[i]);
      offset = g_test_config.single_cali[adc_ch_list[i]].offset;
      debug_printf("%12s:%10d,%6.3fv\r\n",nameList[i],adc, get_voltage_vref(adc,offset));
    }
  }
}

void adc_diff_all_test(void)
{
  uint8_t err;
  int32_t adc;
  int32_t offset;
  float gain;
  const uint8_t adc_ch_list[]={0,1,2,3,4,5,6,7};

  debug_printf("\r\n");
  for(int i = 0 ;i<_countof(adc_ch_list); i++)
  {
    adc = driver_adc_read(g_adc_diff,adc_ch_list[i]+ADC_ADS1220_DIFF_CH_0,&err);
    if(err)
    {
      debug_printf("%s,read err\r\n",diff_nameLists[i]);
    }
    else
    {
    gain = calculate_gain_adc_diff(adc_ch_list[i]);
    offset = g_test_config.diff_cali[adc_ch_list[i]].offset;
    debug_printf("%12s:%10d,%6.3fv\r\n",diff_nameLists[i],adc, get_voltage_vref(adc,offset));

    }
  }
}

#define ADC_OFFSET  0
#define ADC_FULLSET 1
void adc_single_calibration_set(int32_t ch,int32_t setType,int32_t input_voltage)
{
  int32_t adc;
  uint8_t err;

  adc = driver_adc_read_average(g_adc_single,ch,&err,g_adc_average_cnt);

  switch(setType)
  {
    case ADC_OFFSET:
      if(err == 0)
      {
        g_test_config.single_cali[ch].offset = adc;
        g_test_config.single_cali[ch].offset_input = input_voltage;

        WRITE_CFG(single_cali[ch].offset);
        WRITE_CFG(single_cali[ch].offset_input);

        debug_printf("offset:%d\r\n",adc);
      }
      else
      {
        debug_printf("adc read err\r\n");
      }
    break;
    case ADC_FULLSET:
       if(err ==0)
      {
        g_test_config.single_cali[ch].fullset = adc;
        g_test_config.single_cali[ch].fullset_input = input_voltage;



        WRITE_CFG(single_cali[ch].fullset);
        WRITE_CFG(single_cali[ch].fullset_input);

        g_test_config.single_cali[ch].gain = calculate_gain_adc_single(ch);
        WRITE_CFG(single_cali[ch].gain);
        debug_printf("fullset:%d\r\n",adc);
      }
      else
      {
        debug_printf("adc read err\r\n");
      }
    break;
  }
}


void adc_diff_calibration_set(int32_t ch,int32_t setType,int32_t input_voltage)
{
  int32_t adc;
  uint8_t err;

  adc = driver_adc_read_average(g_adc_single,ch,&err,g_adc_average_cnt);

  switch(setType)
  {
    case ADC_OFFSET:
      if(err ==0)
      {
        g_test_config.diff_cali[ch].offset = adc;
        g_test_config.diff_cali[ch].offset_input = input_voltage;

        WRITE_CFG(diff_cali[ch].offset);
        WRITE_CFG(diff_cali[ch].offset_input);

        debug_printf("offset:%d\r\n",adc);
      }
      else
      {
        debug_printf("adc read err\r\n");
      }
    break;
    case ADC_FULLSET:
       if(err ==0)
      {
        g_test_config.diff_cali[ch].fullset = adc;
        g_test_config.diff_cali[ch].fullset_input = input_voltage;

        WRITE_CFG(diff_cali[ch].fullset);
        WRITE_CFG(diff_cali[ch].fullset_input);

        g_test_config.diff_cali[ch].gain = calculate_gain_adc_single(ch);
        WRITE_CFG(diff_cali[ch].gain);
        debug_printf("fullset:%d\r\n",adc);
      }
      else
      {
        debug_printf("adc read err\r\n");
      }
    break;
  }
}




void test_cmd(char *data)
{
  char *list[10];
  int count=0;
  ParsedData cmd;
  uint16_t pin;
  char temp[20];
  char msg[100];
  
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
              HAL_GPIO_WritePin(GPIOA,1<<cmd.pin,(GPIO_PinState)cmd.data);
            break;
            case 'b':
              test_do_init(GPIOB,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOB,1<<cmd.pin,(GPIO_PinState)cmd.data);
            break;
            case 'c':
              test_do_init(GPIOC,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOC,1<<cmd.pin,(GPIO_PinState)cmd.data);
            break;
            case 'd':
              test_do_init(GPIOD,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOD,1<<cmd.pin,(GPIO_PinState)cmd.data);
            break;
            case 'e':
              test_do_init(GPIOE,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOE,1<<cmd.pin,(GPIO_PinState)cmd.data);
            break;
            case 'f':
              test_do_init(GPIOF,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOF,1<<cmd.pin,(GPIO_PinState)cmd.data);
            break;
            case 'g':
              test_do_init(GPIOG,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOG,1<<cmd.pin,(GPIO_PinState)cmd.data);
            break;
            case 'h':
              test_do_init(GPIOH,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOH,1<<cmd.pin,(GPIO_PinState)cmd.data);
            break;
            case 'i':
              test_do_init(GPIOI,1<<cmd.pin);
              HAL_GPIO_WritePin(GPIOI,1<<cmd.pin,(GPIO_PinState)cmd.data);
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
              cnt = driver_rs485_recv(g_rs485_a,(uint8_t *)temp,10,3000);
              if(cnt)
              {
                driver_rs485_sends(g_rs485_a,(uint8_t *)temp,cnt);
              }
                break;
                case 'b':
                driver_rs485_sends(g_rs485_b,"rs485_b",7);
               cnt = driver_rs485_recv(g_rs485_b,(uint8_t *)temp,10,3000);
              if(cnt)
              {
                driver_rs485_sends(g_rs485_b,(uint8_t *)temp,cnt);
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
                strcpy(msg,"UART_STM32_1");
                driver_uart_send(g_stm_uart_1,(uint8_t *)msg,strlen(msg));
                
                cnt = driver_uart_recvs(g_stm_uart_1,(uint8_t *)temp,sizeof(temp),3000);
                if(cnt)
                {
                    driver_uart_send(g_stm_uart_1,(uint8_t *)temp,cnt);
                }
                break;
                case '2':
                strcpy(msg,"UART_QUAD_1");
                driver_uart_send(g_quad_232_1,(uint8_t *)msg,strlen(msg));
                
                cnt = driver_uart_recvs(g_quad_232_1,(uint8_t *)temp,sizeof(temp),3000);
                if(cnt)
                {
                    driver_uart_send(g_quad_232_1,(uint8_t *)temp,cnt);
                }
                
                break;
                case '3':
                strcpy(msg,"UART_EX_TTL_2");
                driver_uart_send(g_quad_ttl_2,(uint8_t *)msg,strlen(msg));
                
                cnt = driver_uart_recvs(g_quad_ttl_2,(uint8_t *)temp,sizeof(temp),3000);
                if(cnt)
                {
                    driver_uart_send(g_quad_ttl_2,(uint8_t *)temp,cnt);
                }
                break;
                case '4':
                strcpy(msg,"UART_EX_232_A_3");
                driver_uart_send(g_232_A,(uint8_t *)msg,strlen(msg));
                
                cnt = driver_uart_recvs(g_232_A,(uint8_t *)temp,sizeof(temp),3000);
                if(cnt)
                {
                    driver_uart_send(g_232_A,(uint8_t *)temp,cnt);
                }
                break;
                case '5':
                strcpy(msg,"UART_EX_232_B_4");
                driver_uart_send(g_232_B,(uint8_t *)msg,strlen(msg));
                cnt = driver_uart_recvs(g_232_B,(uint8_t *)temp,sizeof(temp),3000);
                if(cnt)
                {
                    driver_uart_send(g_232_B,(uint8_t *)temp,cnt);
                }
                break;
                case '6':
                strcpy(msg,"UART_EX_232_C_7");
                driver_uart_send(g_232_C,(uint8_t *)msg,strlen(msg));
                cnt = driver_uart_recvs(g_232_C,(uint8_t *)temp,sizeof(temp),3000);
                if(cnt)
                {
                  driver_uart_send(g_232_C,(uint8_t *)temp,cnt);
                }
                break;
                case '7':
                strcpy(msg,"UART_EX_232_D_8");
                driver_uart_send(g_232_D,(uint8_t *)msg,strlen(msg));
                cnt = driver_uart_recvs(g_232_D,(uint8_t *)temp,sizeof(temp),3000);
                if(cnt)
                {
                  driver_uart_send(g_232_D,(uint8_t *)temp,cnt);
                }
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
      else if(strncmp(list[1],"adc",3)==0)
      {
        if(strncmp(list[2],"single_",7)==0)
        {
            int32_t ch;
            if(sscanf(list[2], "single_%d", &ch)==1)
            {
                adc_read_single(ch);
            }

        }
        else if(strncmp(list[2],"diff_",5)==0)
        {
            int32_t ch;
            if(sscanf(list[2], "diff_%d", &ch)==1)
            {
                adc_read_diff(ch );
            }
        }
        else if(strncmp(list[2],"all_single",10) == 0)
        {
          adc_single_all_test();
        }
        else if(strncmp(list[2],"all_diff", 8) == 0)
        {
            adc_diff_all_test();
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
      else if(strncmp(list[1],"rs232_",6) == 0)
      {
        uart_baud_config_t uart_cfg;
        switch (list[1][6])
        {
        case '1':
          uart_cfg.baud = atoi(list[2]);
          driver_uart_set(g_stm_uart_1,eUART_SET_CONFIG,(void *)&uart_cfg);
          break;
        case '2':
          uart_cfg.baud = atoi(list[2]);
          driver_uart_set(g_quad_232_1,eUART_SET_CONFIG,(void *)&uart_cfg);
        break;
        case '3':
          uart_cfg.baud = atoi(list[2]);
          driver_uart_set(g_quad_ttl_2,eUART_SET_CONFIG,(void *)&uart_cfg);
        break;
        case '4':
          uart_cfg.baud = atoi(list[2]);
          driver_uart_set(g_232_A,eUART_SET_CONFIG,(void *)&uart_cfg);
        break;
        case '5':
          uart_cfg.baud = atoi(list[2]);
          driver_uart_set(g_232_B,eUART_SET_CONFIG,(void *)&uart_cfg);
        break;
        case '6':
          uart_cfg.baud = atoi(list[2]);
          driver_uart_set(g_232_C,eUART_SET_CONFIG,(void *)&uart_cfg);
        break;
        case '7':
          uart_cfg.baud = atoi(list[2]);
          driver_uart_set(g_232_D,eUART_SET_CONFIG,(void *)&uart_cfg);
        break;
        default:
          break;
        }
      }
      else if(strncmp(list[1],"adc",3)==0)
      {
          if(strncmp(list[2],"single_",7)==0)
          {
            int32_t ch;
            int32_t input_voltage;
            int32_t setMode;
            if(sscanf(list[2], "single_%d", &ch)==1)
            {
              if(strncmp(list[3],"offset",6)==0)
              {
                setMode = ADC_OFFSET;
              }
              else if(strncmp(list[3],"fullset",7)==0)
              {
                setMode = ADC_FULLSET;
              }

              if(sscanf(list[4], "%d", &input_voltage)==1)
              {
                  adc_single_calibration_set(ch,setMode,input_voltage);
              }
            }
          }
          else if(strncmp(list[2],"average",7)==0)
          {
            int32_t average_cnt;
              if(sscanf(list[2], "average=%d", &average_cnt)==1)
              {
                g_adc_average_cnt = average_cnt;
                debug_printf("adc average cnt:%d\r\n",g_adc_average_cnt);
              }
          }
          else if(strncmp(list[2],"Vref",4)==0)
          {
            int32_t Vref;
              if(sscanf(list[2], "Vref=%d", &Vref)==1)
              {
                g_adc_vref = Vref;
                debug_printf("adc Vref:%dmV\r\n",g_adc_vref);
                g_test_config.Vref = Vref;
                WRITE_CFG(Vref);
              }
          }
      }
    }
  }
}

extern void set_debug_uart_handle(driver_t *drv);
extern void fram_test(void);
void testTask(void *argument)
{
  uint8_t buff[512];
  osStatus status;
  uint8_t data;
  uint16_t cnt=0;


  g_rs485_a = driver_rs485_open(RS485_A);
  g_rs485_b = driver_rs485_open(RS485_B);
  g_sdi     = driver_sdi_open(SDI_1);

  g_stm_uart_1 = driver_uart_open(UART_STM32_1);
  g_uart3      = driver_uart_open(UART_STM32_3);
  
  g_quad_232_1 = driver_uart_open(UART_EX_232_1);
  g_quad_ttl_2 = driver_uart_open(UART_EX_TTL_2);
  g_232_A =  driver_uart_open(UART_EX_232_A_3);
  g_232_B =  driver_uart_open(UART_EX_232_B_4);
  g_232_C =  driver_uart_open(UART_EX_232_C_7);
  g_232_D =  driver_uart_open(UART_EX_232_D_8);

  
  set_debug_uart_handle(g_uart3);
  
  
  if(g_adc_single == NULL)
  {
    g_adc_single = driver_adc_open(ADC_ADS1220_SINGLE_CH_0);
  }
  
    if(g_adc_diff == NULL)
  {
    g_adc_diff = driver_adc_open(ADC_ADS1220_DIFF_CH_0);
  }



  g_fram = driver_fram_open(FRAM_FM25LC);
  
  driver_fram_read(g_fram,0,(uint8_t *)&g_test_config,sizeof(test_config_t));
  
  while(1)
  {
    if (driver_uart_recvs(g_uart3,&data,1,osWaitForever))
    {
      buff[cnt++] = data;
      if(data=='\n')
      {
        buff[cnt]=0;
        test_cmd((uint8_t *)buff);

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