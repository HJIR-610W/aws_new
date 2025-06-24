#include "websocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cmsis_os2.h"
#include "task_logging.h"
#include "user_heap.h"
#include "dev_io.h"
// 터미널 브리지 전역 변수
static void (*g_terminal_output_callback)(const char* data, size_t len) = NULL;
static bool g_bridge_initialized = false;
static int g_active_clients = 0;

void terminal_bridge_init(void)
{
    // TODO: 콘솔 시스템 초기화 구현
    // 
    // 구현 방법:
    // 1. 기존 콘솔/CLI 시스템과의 연결 설정
    // 2. 콘솔 입출력 버퍼 초기화
    // 3. 콘솔 명령 파서 초기화
    // 4. 터미널 상태 변수 초기화
    // 5. 필요한 세마포어나 뮤텍스 생성
    // 
    // 예시:
    // - console_init() 호출
    // - cli_init() 호출  
    // - terminal_mutex = osMutexNew(NULL)
    // - console_input_queue = osMessageQueueNew(...)
    
    if (!g_bridge_initialized) {
        g_bridge_initialized = true;
        task_printf("Terminal Bridge: 초기화 완료 - 콘솔 시스템 연결 필요\r\n");
    }
    g_active_clients++;
    task_printf("Terminal Bridge: 클라이언트 연결됨 (총 %d개)\r\n", g_active_clients);
}

void terminal_bridge_send_command(const char* command, size_t len)
{
    // TODO: 웹 터미널에서 받은 명령을 실제 콘솔 시스템으로 전달
    //
    // 구현 방법:
    // 1. 명령어 유효성 검사 (NULL 체크, 길이 체크)
    // 2. 특수 문자 처리 (Ctrl+C, Enter, Backspace 등)
    // 3. 명령어를 콘솔 파서로 전달
    // 4. 명령 실행 결과를 콜백으로 전송
    //
    // 예시:
    // - VT100 이스케이프 시퀀스 파싱
    // - console_execute_command(command) 호출
    // - cli_process_input(command, len) 호출
    // - 결과를 g_terminal_output_callback으로 전송
    
    if (!g_bridge_initialized || !command || len == 0) {
        return;
    }
    
    io_inject((uint8_t *)command,len);
    task_printf("Terminal Bridge: 명령 수신 (%zu bytes): %.*s\r\n", 
                len, (int)len, command);
    
    // 임시 에코 응답 (실제 구현에서는 제거)
    if (g_terminal_output_callback) {
        char response[512];
        int response_len = snprintf(response, sizeof(response), 
                                   "Echo: %.*s\r\n$ ", (int)len, command);
        g_terminal_output_callback(response, response_len);
    }
}

void terminal_bridge_set_output_callback(void (*callback)(const char* data, size_t len))
{
    // TODO: 콘솔 출력을 웹 터미널로 보내는 콜백 함수 설정
    //
    // 구현 방법:
    // 1. 콜백 함수 포인터 저장
    // 2. 기존 콘솔 출력 시스템에 이 콜백 연결
    // 3. printf, puts 등의 출력을 이 콜백으로 리다이렉트
    //
    // 예시:
    // - console_set_output_redirect(callback)
    // - cli_set_output_handler(callback)  
    // - printf 출력을 캡처하여 콜백 호출
    
    g_terminal_output_callback = callback;
    task_printf("Terminal Bridge: 출력 콜백 설정 완료\r\n");
}

void terminal_bridge_cleanup(void)
{
    // TODO: 터미널 브리지 정리 및 콘솔 시스템 연결 해제
    //
    // 구현 방법:
    // 1. 출력 콜백 해제
    // 2. 콘솔 시스템 리다이렉트 해제
    // 3. 생성된 세마포어나 뮤텍스 삭제
    // 4. 메모리 해제
    // 5. 상태 변수 초기화
    //
    // 예시:
    // - console_restore_output()
    // - cli_cleanup()
    // - osMutexDelete(terminal_mutex)
    // - osMessageQueueDelete(console_input_queue)
    
    if (g_active_clients > 0) {
        g_active_clients--;
        task_printf("Terminal Bridge: 클라이언트 연결 해제 (남은 %d개)\r\n", g_active_clients);
    }
    
    if (g_active_clients == 0) {
        g_terminal_output_callback = NULL;
        g_bridge_initialized = false;
        task_printf("Terminal Bridge: 모든 클라이언트 해제됨 - 정리 완료\r\n");
    }
}

// 추가 유틸리티 함수들 (필요에 따라 구현)

void terminal_bridge_send_output(const char* data, size_t len)
{
    // TODO: 콘솔 시스템에서 이 함수를 호출하여 출력을 웹 터미널로 전송
    //
    // 구현 방법:
    // 1. 출력 데이터 유효성 검사
    // 2. 콜백 함수가 설정되어 있는지 확인
    // 3. 콜백 함수 호출하여 웹 터미널로 데이터 전송
    //
    // 사용 예시:
    // printf를 오버라이드하여 이 함수 호출
    // console_printf 등에서 이 함수 호출
    
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
    //
    // 구현 방법:
    // 1. 현재 프롬프트 문자열 생성 (예: "user@device:~$ ")
    // 2. terminal_bridge_send_output 호출
    //
    // 예시:
    // char prompt[] = "AWS-Device$ ";
    // terminal_bridge_send_output(prompt, strlen(prompt));
    
    const char* prompt = "$ ";
    terminal_bridge_send_output(prompt, strlen(prompt));
}