#include "app_key.h"

#include <string.h>

#include "cmsis_os2.h"

#include "util_memory.h"
#include "bsp_uart.h"
#include "debug_io.h"
#include "cli_key_code.h"
#include "pcb_define.h"
#include "util_time.h"
#define BUTTON_QUEUE_SIZE 5

static int32_t serial_key = -1;

osMessageQueueId_t g_button_queue_id = NULL;


void send_key_cmd(const char *cmd)
{
  if(serial_key !=-1)
  bsp_uart_send(serial_key,(uint8_t *)cmd,strlen(cmd));
}




void app_key_init(void)
{
    uart_config_t uart_config;


    uart_config.dataLen = UART_DATA_LEN_8;
    uart_config.parity_index = PARITY_NONE;
    uart_config.stop_bit = UART_STOP_BIT_1;
#ifdef NOT_USE_LCD
        uart_config.baud = 115200;    
    serial_key = BSP_UART_0_D_SUB_0;
#else
    uart_config.baud = 38400;
        serial_key = BSP_UART_1_TTL_ONLY;    
#endif

    bsp_uart_init(serial_key, &uart_config);

    g_button_queue_id = osMessageQueueNew(BUTTON_QUEUE_SIZE, sizeof(int32_t), NULL);

    if (g_button_queue_id == NULL)
    {
        return;
    }

}

int32_t get_button_key(uint32_t timeout_ms)
{
    int32_t key = -1;
    osStatus_t status;

    if (g_button_queue_id == NULL)
    {
        return KEY_CODE_NONE;
    }

    status = osMessageQueueGet(g_button_queue_id, &key, NULL, timeout_ms);
    
    if (status == osOK)
    {
        return key;
    }

    return KEY_CODE_NONE;
}

void button_put_key(int32_t key)
{
    if (g_button_queue_id == NULL)
    {
        return;
    }

    osMessageQueuePut(g_button_queue_id, &key, 0, 0);
}


static int32_t process_serial_data(uint8_t data)
{
    static uint8_t escape_sequence[3] = {0};
    static int escape_index = 0;
    static uint32_t last_escape_time = 0;
    uint8_t ch;
    uint32_t current_time;

    ch = data;
    current_time = HAL_GetTick();

    // 이스케이프 시퀀스가 특정시간초과하면새롭게 시퀀스 시작 
    if (escape_index > 0 && (current_time - last_escape_time) > 10)
    {
        escape_index = 0;
    }

    // 이스케이프 시퀀스
    if (ch == 0x1B) // ESC
    { 
        escape_sequence[0] = ch;
        escape_index = 1;
        last_escape_time = current_time;
        return KEY_CODE_NONE; 
    }
    else if (escape_index == 1)
    {
        escape_sequence[1] = ch;
        escape_index = 2;
        last_escape_time = current_time;
        return KEY_CODE_NONE; 
    }
    else if (escape_index == 2)
    {
        escape_sequence[2] = ch;
        escape_index = 0;
        
        if (escape_sequence[1] == '[')
        {
            switch (ch)
            {
                case 'A': return KEY_CODE_UP;
                case 'B': return KEY_CODE_DOWN;
                case 'C': return KEY_CODE_RIGHT;
                case 'D': return KEY_CODE_LEFT;
                case 'H': return KEY_CODE_HOME;
                case 'F': return KEY_CODE_END;
                default: return KEY_CODE_UNKNOWN;
            }
        }
        else if (escape_sequence[1] == 'O')
        {
            switch (ch)
            {
                case 'H': return KEY_CODE_HOME;
                case 'F': return KEY_CODE_END;
                default: return KEY_CODE_UNKNOWN;
            }
        }
        
        return KEY_CODE_UNKNOWN;
    }

    escape_index = 0;

    if(ch =='\r')
    {
      return KEY_CODE_ENTER;//0x0D가 전송되면 이것도 ENTER로 처리
    }
    
    //  Ctrl key 조합
    if (ch >= 0x01 && ch <= 0x1A)
    {
        return (int32_t)ch;
    }

    //일반 키
    return (int32_t)ch;
}


uint8_t g_key_info[8];

uint32_t get_key_version(uint8_t *major, uint8_t *minor, uint8_t *patch, uint8_t *release)
{
    uint32_t key_ver=0;

    *major = g_key_info[0];
    *minor = g_key_info[1];
    *patch = g_key_info[2];
    *release = g_key_info[3];

    key_ver = g_key_info[0]<<24 |  g_key_info[1]<<16| g_key_info[2]<<8 | g_key_info[3];

    
    return key_ver;
}

void get_key_build(DATE_TIME_BUF *build)
{
    uint32_t time_stamp;

    memcpy(&time_stamp,&g_key_info[4],4);
    time_cvt_secTotime(time_stamp,build);
}

void inject_key(uint8_t data)
{
    bsp_uart_inject(serial_key,&data,1);
}
void scan_key(void)
{
    uint8_t data;

    int key;

     bsp_uart_recv(serial_key, &data, 1, 0xFFFFFFFF);
  
     /*
     key MCU 버전 정보 수신 처리
     ESC +ENTER를 동시에 1초 이상 누르면 버전 정보가 전송되어 온다.
     */ 
     if(data==0x02)
     {
       bsp_uart_recv(serial_key, g_key_info, 8, 0xFFFFFFFF);
       return;
     }
     key = process_serial_data(data);
  
      if (key != KEY_CODE_NONE) 
      {
        button_put_key(key);
      }

   
}



void enable_left_long_key(void)
{
  send_key_cmd("en_left_long\n");
}

void disable_left_long_key(void)
{
 send_key_cmd("di_left_long\n");
}


