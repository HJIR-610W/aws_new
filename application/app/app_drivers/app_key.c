#include "app_key.h"
#include "cmsis_os2.h"

#include "util_memory.h"
#include "bsp_uart.h"
#include "dev_io.h"
#include "cli_key_code.h"
#include "pcb_define.h"

#define BUTTON_QUEUE_SIZE 5

static int32_t serial_key = -1;

    osMessageQueueId_t button_queue_handle = NULL;

void app_key_init(void)
{
    uart_config_t uart_config;

    uart_config.baud = 19200;
    uart_config.dataLen = UART_DATA_LEN_8;
    uart_config.parityIdx = PARITY_NONE;
    uart_config.stop_bit = UART_STOP_BIT_1;

    serial_key = BSP_UART_0_D_SUB_0;
    bsp_uart_init(serial_key, &uart_config);

    button_queue_handle = osMessageQueueNew(BUTTON_QUEUE_SIZE, sizeof(int32_t), NULL);

    if (button_queue_handle == NULL)
    {
        return;
    }

}

int32_t get_button_key(uint32_t timeout_ms)
{
    int32_t key = -1;
    osStatus_t status;

    if (button_queue_handle == NULL) {
        return -1;
    }


    status = osMessageQueueGet(button_queue_handle, &key, NULL, timeout_ms);
    
    if (status == osOK) {
        return key;
    }
    
    return -1;
}

void button_put_key(int32_t key)
{
    if (button_queue_handle == NULL)
    {
        return;
    }

    osMessageQueuePut(button_queue_handle, &key, 0, 0);
}


static int32_t process_serial_data(uint8_t *data, int len)
{
    static uint8_t escape_sequence[3] = {0};
    static int escape_index = 0;
    static uint32_t last_escape_time = 0;
    
    if (len <= 0) {
        return KEY_CODE_NONE;
    }

    uint8_t ch = data[0];
    uint32_t current_time = HAL_GetTick();

    // Reset escape sequence if timeout occurred
    if (escape_index > 0 && (current_time - last_escape_time) > 100) {
        escape_index = 0;
    }

    // Handle escape sequences
    if (ch == 0x1B) { // ESC character
        escape_sequence[0] = ch;
        escape_index = 1;
        last_escape_time = current_time;
        return KEY_CODE_NONE; // Wait for more characters
    }
    else if (escape_index == 1) {
        escape_sequence[1] = ch;
        escape_index = 2;
        last_escape_time = current_time;
        return KEY_CODE_NONE; // Wait for more characters
    }
    else if (escape_index == 2) {
        escape_sequence[2] = ch;
        escape_index = 0;
        
        // Process complete escape sequence
        if (escape_sequence[1] == '[') {
            switch (ch) {
                case 'A': return KEY_CODE_UP;
                case 'B': return KEY_CODE_DOWN;
                case 'C': return KEY_CODE_RIGHT;
                case 'D': return KEY_CODE_LEFT;
                case 'H': return KEY_CODE_HOME;
                case 'F': return KEY_CODE_END;
                default: return KEY_CODE_UNKNOWN;
            }
        }
        else if (escape_sequence[1] == 'O') {
            switch (ch) {
                case 'H': return KEY_CODE_HOME;
                case 'F': return KEY_CODE_END;
                default: return KEY_CODE_UNKNOWN;
            }
        }
        
        return KEY_CODE_UNKNOWN;
    }


    escape_index = 0;

    // Handle Ctrl key combinations
    if (ch >= 0x01 && ch <= 0x1A) {
        return (int32_t)ch;
    }

    // Handle regular characters
    return (int32_t)ch;
}


void scan_key(void)
{
    int len;
    uint8_t data[10];
    int key;


    len = bsp_uart_recv(serial_key, data, _countof(data), 0);

    if (len > 0)
    {
    // Process each received byte
    for (int i = 0; i < len; i++) {
        key = process_serial_data(&data[i], 1);
        
        if (key != KEY_CODE_NONE) {
            // Put processed key into message queue
            button_put_key(key);
        }
    }
    }

}