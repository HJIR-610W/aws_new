#include "app_button.h"
#include "cmsis_os2.h"

#include "util_memory.h"
#include "driver_uart.h"
#include "dev_io.h"
#include "cli_key_code.h"
#include "pcb_define.h"


// Button message queue size
#define BUTTON_QUEUE_SIZE 16

static driver_t *serial_key = NULL;

// Message queue handle (exported for other modules)
osMessageQueueId_t button_queue_handle = NULL;



// Message queue attributes
const osMessageQueueAttr_t button_queue_attributes = {
    .name = "button_queue"
};

void app_button_init(void)
{
    uart_config_t uart_config;

    // UART configuration for serial key input
    uart_config.baud = 19200;
    uart_config.dataLen = 8;
    uart_config.parityIdx = 0;
    uart_config.stop_bit = 1;

    serial_key = driver_uart_open(UART_8_CDMA, &uart_config);

    // Create message queue for button events
    button_queue_handle = osMessageQueueNew(BUTTON_QUEUE_SIZE, sizeof(int32_t), &button_queue_attributes);
    
    if (button_queue_handle == NULL) {
        // Handle error - queue creation failed
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

    // Get key from message queue
    status = osMessageQueueGet(button_queue_handle, &key, NULL, timeout_ms);
    
    if (status == osOK) {
        return key;
    }
    
    return -1;
}

void button_put_key(int32_t key)
{
    if (button_queue_handle == NULL) {
        return;
    }

    // Put key into message queue (non-blocking)
    osMessageQueuePut(button_queue_handle, &key, 0, 0);
}

// Process received data and convert to key codes
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

    // Reset escape sequence
    escape_index = 0;

    // Handle Ctrl key combinations
    if (ch >= 0x01 && ch <= 0x1A) {
        return (int32_t)ch;
    }

    // Handle regular characters
    return (int32_t)ch;
}



// Legacy function for compatibility
void button_init(void)
{
    app_button_init();
}


// Legacy function for compatibility (called every 100ms)
void scan_button(void)
{
  int len;
  uint8_t data[10];
  int key;
        if (serial_key != NULL) {
            len = driver_uart_recv(serial_key, data, _countof(data), 0);
            
            if (len > 0) {
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
}