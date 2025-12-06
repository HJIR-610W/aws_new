#ifndef TERMINAL_BRIDGE_H
#define TERMINAL_BRIDGE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "io_interface.h"

// 터미널 브리지 기본 인터페이스 함수들
void terminal_bridge_init(void);
void terminal_bridge_send_command(const char* command, size_t len);
void terminal_bridge_set_output_callback(void (*callback)(const char* data, size_t len));
void terminal_bridge_cleanup(void);

// 추가 유틸리티 함수들

bool terminal_bridge_is_initialized(void);
void terminal_bridge_send_prompt(void);
void telnet_send(const uint8_t *data,size_t len );

void telnet_io_init(void);
int telnet_io_send(io_if_t *io,const uint8_t *data,size_t len );
int telnet_io_recv(io_if_t *io,uint8_t *buffer,size_t len,uint32_t timeout_ms);
void telnet_io_flush(io_if_t *io);
void telnet_io_inject(io_if_t *io,uint8_t *data,size_t len);


#endif // TERMINAL_BRIDGE_H