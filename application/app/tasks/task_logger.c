#include "task_logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "os_user_def.h"
#include "debug_io.h"

/*
 * 설정
 * LOGGER_MAX_LINE_BYTES: 한 메시지의 최대 길이(고정)
 * LOGGER_QUEUE_LENGTH: 큐 깊이(메시지 개수)
 */
#define LOGGER_MAX_LINE_BYTES   (256)
#define LOGGER_QUEUE_LENGTH     (4)

/* static 전역 */
static osMessageQueueId_t s_log_queue = NULL;
static osThreadId_t s_logger_task_id = NULL;

const osThreadAttr_t loggerTask_attributes = {
  .name = "print",
  .stack_size = TASK_STACK(TASK_LOGGER_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_LOGGER_DEF),
};

/* 내부 함수 프로토타입 */
static void printTask(void *arg);

/*
 * 로거 태스크를 초기화합니다.
 */
void loggerTask_init(void)
{
    if (s_logger_task_id != NULL)
    {
        return; // 이미 초기화됨
    }



     
    // 메시지 큐 생성
    s_log_queue = osMessageQueueNew(LOGGER_QUEUE_LENGTH, LOGGER_MAX_LINE_BYTES, NULL);
    if (s_log_queue == NULL)
    {
        // 오류 처리: 큐 생성 실패
        return;
    }

    // 로거 태스크 생성
    s_logger_task_id = osThreadNew(printTask, NULL, &loggerTask_attributes);
    if (s_logger_task_id == NULL)
    {
        (void)osMessageQueueDelete(s_log_queue);
        s_log_queue = NULL;
        // 오류 처리: 스레드 생성 실패
    }
}

int32_t os_vprintf(const char *fmt, va_list ap)
{
    char buf[LOGGER_MAX_LINE_BYTES];
    int n;
    osStatus_t st;

    if (s_log_queue == NULL || fmt == NULL)
    {
        return -1;
    }

    n = vsnprintf(buf, sizeof(buf), fmt, ap);

    if (n < 0)
    {
        return -1;
    }

    /* 메시지가 잘렸을 경우, "..."을 추가하고 안전하게 null 문자로 종료 */
    if (n >= (int)sizeof(buf))
    {
        // "..."와 null 종료 문자를 위한 공간이 있는지 확인
        if (sizeof(buf) >= 4)
        {
            buf[sizeof(buf) - 4] = '.';
            buf[sizeof(buf) - 3] = '.';
            buf[sizeof(buf) - 2] = '.';
            buf[sizeof(buf) - 1] = '\0';
            n = (int)sizeof(buf) - 1; // "..."를 포함한 새로운 문자열 길이를 반영하도록 n 업데이트
        }
        else if (sizeof(buf) >= 1) // 버퍼가 "..."을 넣기에는 너무 작으면, 단순히 null 종료
        {
            buf[sizeof(buf) - 1] = '\0';
            n = (int)sizeof(buf) - 1;
        }
        else
        {
            // 버퍼 크기가 0이면 할 일 없음
            n = 0;
        }
    }

    /*
     * 논블로킹 호출 (timeout = 0).
     * 큐가 가득 찼으면 메시지는 드롭됩니다.
     */
    st = osMessageQueuePut(s_log_queue, buf, 0, 10);
    if (st != osOK)
    {
        return -1;
    }

    return n;
}

/*
 * 문자열을 포맷하여 로그 큐에 넣습니다.
 * 논블로킹: 큐가 가득 차면 메시지를 드롭합니다.
 */
int32_t os_printf(const char *fmt, ...)
{
    va_list ap;
    int32_t result;

    va_start(ap, fmt);
    result = os_vprintf(fmt, ap);
    va_end(ap);

    return result;
}

#include "task_telnet_server.h"


/*
 * 로거 스레드 메인 함수.
 * 큐에서 메시지를 기다려 출력합니다.
 */
static void printTask(void *arg)
{
    (void)arg;

    char rx_buf[LOGGER_MAX_LINE_BYTES];
    osStatus_t st;

    for (;;) {
        // 메시지 큐에서 메시지를 영원히 기다림
        st = osMessageQueueGet(s_log_queue, rx_buf, NULL, osWaitForever);
        if (st == osOK)
        {
            /* 수신된 메시지를 그대로 출력 */
            // prevent infinite loop by calling low-level io_send directly
            size_t len = strlen((char *)rx_buf);
            io_send(get_debug_io(), (uint8_t*)rx_buf, len);
            #if TELNET_MIRROR_USE == 1
            telnet_send((uint8_t*)rx_buf, len);
            #endif
        }
    }
}

/*
 * 문자열을 로그 큐에 넣습니다.
 * 논블로킹: 큐가 가득 차면 메시지를 드롭합니다.
 */
int32_t os_puts(const char *str)
{
    char buf[LOGGER_MAX_LINE_BYTES];
    osStatus_t st;
    size_t len;

    if (s_log_queue == NULL || str == NULL)
    {
        return -1;
    }

    len = strlen(str);

    // 버퍼 크기보다 길면 자릅니다.
    if (len >= sizeof(buf))
    {
        len = sizeof(buf) - 1;
    }
    
    // strncpy is safer
    strncpy(buf, str, len);
    buf[len] = '\0'; // null-terminate

    /*
     * 논블로킹 호출 (timeout = 0).
     * 큐가 가득 찼으면 메시지는 드롭됩니다.
     */
    st = osMessageQueuePut(s_log_queue, buf, 0, 0);
    if (st != osOK)
    {
        return -1;
    }

    return (int32_t)len;
}

/*
 * 단일 문자를 로그 큐에 넣습니다.
 */
void os_put_ch(uint8_t ch)
{
    char buf[2];
    osStatus_t st;

    if (s_log_queue == NULL)
    {
        return;
    }

    buf[0] = ch;
    buf[1] = '\0';

    /*
     * 논블로킹 호출 (timeout = 0).
     * 큐가 가득 찼으면 메시지는 드롭됩니다.
     */
    st = osMessageQueuePut(s_log_queue, buf, 0, 0);
    if (st != osOK)
    {
        // Optionally handle the error, e.g., increment a drop counter
    }
}

/*
 * 바이너리 데이터를 로그 큐에 16진수 문자열로 넣습니다.
 */
void os_debug_send(const uint8_t *data, size_t len)
{
    char line_buf[LOGGER_MAX_LINE_BYTES];
    char byte_str[4]; // "XX " + null
    size_t line_len = 0;
    osStatus_t st;
    
    if (s_log_queue == NULL || data == NULL)
    {
        return;
    }

    line_buf[0] = '\0';

    for (size_t i = 0; i < len; i++)
    {
        snprintf(byte_str, sizeof(byte_str), "%02X ", data[i]);

        // 라인 버퍼가 꽉 찼으면 큐에 넣고 버퍼를 비웁니다.
        if (line_len + strlen(byte_str) >= sizeof(line_buf))
        {
            osMessageQueuePut(s_log_queue, line_buf, 0, 0);
            line_len = 0;
            line_buf[0] = '\0';
        }

        // 버퍼에 바이트 문자열을 추가합니다.
        strncat(line_buf, byte_str, sizeof(line_buf) - line_len - 1);
        line_len += strlen(byte_str);
    }

    // 남은 데이터가 있으면 큐에 넣습니다.
    if (line_len > 0)
    {
    st =     osMessageQueuePut(s_log_queue, line_buf, 0, 0);
    
        if (st != osOK)
    {
        // Optionally handle the error, e.g., increment a drop counter
    }
    }
}
