#include "app_key.h"
#include "cmsis_os2.h"

#include "util_memory.h"
#include "bsp_uart.h"
#include "dev_io.h"
#include "cli_key_code.h"
#include "pcb_define.h"

#define BUTTON_QUEUE_SIZE 5

static int32_t serial_key = -1;

osMessageQueueId_t g_button_queue_id = NULL;

void app_key_init(void)
{
    uart_config_t uart_config;

    uart_config.baud = 38400;
    uart_config.dataLen = UART_DATA_LEN_8;
    uart_config.parityIdx = PARITY_NONE;
    uart_config.stop_bit = UART_STOP_BIT_1;

    serial_key = BSP_UART_1_TTL_ONLY;
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


static int32_t process_serial_data(uint8_t *data, int len)
{
    static uint8_t escape_sequence[3] = {0};
    static int escape_index = 0;
    static uint32_t last_escape_time = 0;
    uint8_t ch;
    uint32_t current_time;

    if (len <= 0)
    {
        return KEY_CODE_NONE;
    }

    ch = data[0];
     current_time = HAL_GetTick();

    // 이스케이프 시퀀스가 특정시간초과하면새롭게 시퀀스 시작 
    if (escape_index > 0 && (current_time - last_escape_time) > 100)
    {
        escape_index = 0;
    }

    // 이스케이프 시퀀스
    if (ch == 0x1B)
    { // ESC 
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

    //  Ctrl key 조합
    if (ch >= 0x01 && ch <= 0x1A)
    {
        return (int32_t)ch;
    }

    //일반 키
    return (int32_t)ch;
}


void scan_key(void)
{
    uint8_t data[5];
    int len;
    int key;

    len = bsp_uart_recv(serial_key, data, _countof(data), 0);

    for (int i = 0; i < len; i++)
    {
        key = process_serial_data(&data[i], 1);
    
        if (key != KEY_CODE_NONE) 
        {

            button_put_key(key);
        }
    }
}