#ifndef TERMINAL_BRIDGE_H
#define TERMINAL_BRIDGE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// 터미널 브리지 기본 인터페이스 함수들
void terminal_bridge_init(void);
void terminal_bridge_send_command(const char* command, size_t len);
void terminal_bridge_set_output_callback(void (*callback)(const char* data, size_t len));
void terminal_bridge_cleanup(void);

// 추가 유틸리티 함수들
void terminal_bridge_send_output(const char* data, size_t len);
bool terminal_bridge_is_initialized(void);
void terminal_bridge_send_prompt(void);

#endif // TERMINAL_BRIDGE_H