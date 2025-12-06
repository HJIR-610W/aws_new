#include "websocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cmsis_os2.h"
#include "task_logging.h"
#include "user_heap.h"
#include "debug_io.h"
// 터미널 브리지 전역 변수
static void (*g_terminal_output_callback)(const char* data, size_t len) = NULL;
static bool g_bridge_initialized = false;


void terminal_bridge_init(void)
{
    // TODO: 콘솔 시스템 초기화 구현

    if (!g_bridge_initialized) {
        g_bridge_initialized = true;
        task_printf("Terminal Bridge: 초기화 완료 - 콘솔 시스템 연결 필요\r\n");
    }


}

void terminal_bridge_send_command(const char* command, size_t len)
{
    // TODO: 웹 터미널에서 받은 명령을 실제 콘솔 시스템으로 전달

    
    if (!g_bridge_initialized || !command || len == 0) {
        return;
    }
    
    dbg_inject((uint8_t *)command,len);

    
  
}

void terminal_bridge_set_output_callback(void (*callback)(const char* data, size_t len))
{
    // TODO: 콘솔 출력을 웹 터미널로 보내는 콜백 함수 설정
    
    g_terminal_output_callback = callback;
    task_printf("Terminal Bridge: 출력 콜백 설정 완료\r\n");
}

void terminal_bridge_cleanup(void)
{
    // TODO: 터미널 브리지 정리 및 콘솔 시스템 연결 해제

        g_terminal_output_callback = NULL;
        g_bridge_initialized = false;

}

// 추가 유틸리티 함수들 (필요에 따라 구현)

void terminal_bridge_send_output(const char* data, size_t len)
{
    // TODO: 콘솔 시스템에서 이 함수를 호출하여 출력을 웹 터미널로 전송

    
    if (g_terminal_output_callback && data && len > 0) {
        g_terminal_output_callback(data, len);
    }
}

bool terminal_bridge_is_initialized(void)
{
    // TODO: 터미널 브리지 초기화 상태 확인
    return g_bridge_initialized;
}

void terminal_bridge_send_prompt(void)
{
    // TODO: 프롬프트 문자열을 웹 터미널로 전송
    
    const char* prompt = "$ ";
    terminal_bridge_send_output(prompt, strlen(prompt));
}