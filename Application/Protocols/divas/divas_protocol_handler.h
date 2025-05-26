#ifndef DIVAS_PROTOCOL_HANDLER_H
#define DIVAS_PROTOCOL_HANDLER_H

#include <stdint.h>

uint32_t get_download_file_size(void);
uint32_t get_received_bytes(void);

    uint16_t divas_cmd_handler(uint8_t *rx_frame, uint16_t rx_len, uint8_t *tx_frame);
#endif