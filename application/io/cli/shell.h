
#ifndef _FSL_SHELL_H_
#define _FSL_SHELL_H_

//#include "fsl_common.h"
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
/*!
 * @addtogroup SHELL
 * @{
 */

/*******************************************************************************
 * 정의
 ******************************************************************************/
/*! @brief 히스토리 기능 켜기/끄기 설정 매크로. */
#ifndef SHELL_USE_HISTORY
#define SHELL_USE_HISTORY (0U)
#endif

/*! @brief 히스토리 기능 켜기/끄기 설정 매크로. */
#ifndef SHELL_SEARCH_IN_HIST
#define SHELL_SEARCH_IN_HIST (1U)
#endif

/*! @brief 메서드 스트림 선택 매크로. */
#ifndef SHELL_USE_FILE_STREAM
#define SHELL_USE_FILE_STREAM (0U)
#endif

/*! @brief 자동 완성 기능 켜기/끄기 설정 매크로. */
#ifndef SHELL_AUTO_COMPLETE
#define SHELL_AUTO_COMPLETE (0U)
#endif

/*! @brief 콘솔 버퍼 크기 설정 매크로. */
#ifndef SHELL_BUFFER_SIZE
#define SHELL_BUFFER_SIZE (64U)
#endif

/*! @brief 명령에서 최대 인자 수 설정 매크로. */
#ifndef SHELL_MAX_ARGS
#define SHELL_MAX_ARGS (8U)
#endif

/*! @brief 히스토리 명령의 최대 개수 설정 매크로. */
#ifndef SHELL_HIST_MAX
#define SHELL_HIST_MAX (5U)
#endif

/*! @brief 명령의 최대 개수 설정 매크로. */
#ifndef SHELL_MAX_CMD
#define SHELL_MAX_CMD (10U)
#endif

/*! @brief Shell 사용자 데이터 전송 콜백 프로토타입.*/
typedef void (*send_data_cb_t)(const uint8_t *buf, size_t len);

/*! @brief Shell 사용자 데이터 수신 콜백 프로토타입.*/
typedef int32_t (*recv_data_cb_t)(uint8_t *buf, size_t len,uint32_t timeout_ms);

/*! @brief Shell 사용자 printf 데이터 프로토타입.*/
typedef int (*printf_data_t)(const char *format, ...);
typedef int (*puts_data_t)(const char *str);


/*! @brief 특수 키 처리를 위한 타입. */
typedef enum _fun_key_status
{
    kSHELL_Normal = 0U,   /*!< 일반 키 */
    kSHELL_Special = 1U,  /*!< 특수 키 */
    kSHELL_Function = 2U, /*!< 기능 키 */
} fun_key_status_t;

/*! @brief Shell 환경을 위한 데이터 구조체. */
typedef struct _shell_context_struct
{
    char *prompt;                 /*!< 프롬프트 문자열 */
    enum _fun_key_status stat;    /*!< 특수 키 상태 */
    char line[SHELL_BUFFER_SIZE]; /*!< 콘솔 버퍼 */
    uint8_t cmd_num;              /*!< 사용자 명령 개수 */
    uint8_t l_pos;                /*!< 전체 라인 위치 */
    uint8_t c_pos;                /*!< 현재 라인 위치 */
    printf_data_t printf;
    puts_data_t puts_data_func;
    uint16_t hist_current;                            /*!< 히스토리 버퍼의 현재 히스토리 명령*/
    uint16_t hist_count;                              /*!< 히스토리 버퍼의 전체 히스토리 명령*/
    char hist_buf[SHELL_HIST_MAX][SHELL_BUFFER_SIZE]; /*!< 히스토리 버퍼*/
    bool exit;                                        /*!< 종료 플래그*/
} shell_context_struct, *p_shell_context_t;

/*! @brief 사용자 명령 함수 프로토타입. */
typedef int32_t (*cmd_function_t)(int32_t argc, char **argv);

/*! @brief 사용자 명령 데이터 구조체. */
typedef struct _shell_command_context
{
    const char *pcCommand; /*!< 실행되는 명령. 예: "help". 모두 소문자여야 함. */
    char *pcHelpString;    /*!< 명령 사용 방법을 설명하는 문자열. 명령 자체로 시작하고,
                                    "\r\n"으로 끝나야 함. 예: "help: Returns a list of all the commands\r\n". */
    const cmd_function_t
        pFuncCallBack; /*!< 명령에 의해 생성된 출력을 반환하는 콜백 함수에 대한 포인터. */
    uint8_t cExpectedNumberOfParameters; /*!< 명령이 기대하는 고정된 매개변수 개수, 0일 수 있음. */
} shell_command_context_t;

/*! @brief 명령 목록 구조체. */
typedef struct _shell_command_context_list
{
    const shell_command_context_t *CommandList[SHELL_MAX_CMD]; /*!< 명령 테이블 목록 */
    uint8_t numberOfCommandInList;                             /*!< 목록의 전체 명령 개수 */
} shell_command_context_list_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */

/*!
 * @name Shell 기능 동작
 * @{
 */

/*!
* @brief 클럭 게이트를 활성화하고 설정 구조체에 따라 Shell 모듈을 구성합니다.
*
* 이 함수는 다른 모든 Shell 함수를 호출하기 전에 반드시 호출되어야 합니다.
* 사용자 정의 설정으로 Shell 명령을 호출합니다.
* 아래 예제는 미들웨어 Shell을 설정하는 방법과
* 매개변수를 전달하여 shell_init 함수를 호출하는 방법을 보여줍니다.
* 이것은 예제입니다.
* @code
*   shell_context_struct user_context;
*   shell_init(&user_context, SendDataFunc, ReceiveDataFunc, "SHELL>> ");
* @endcode
* @param context Shell 환경 및 런타임 상태에 대한 포인터.
* @param send_cb 데이터 전송 함수를 콜백하기 위한 포인터.
* @param recv_cb 데이터 수신 함수를 콜백하기 위한 포인터.
* @param prompt  Shell의 문자열 프롬프트
*/
void shell_init(p_shell_context_t context, printf_data_t shell_printf,
                char *prompt);


int32_t shell_register_command(const shell_command_context_t *command_context);
int32_t shell_main(p_shell_context_t context);
int32_t SHELL_recv(p_shell_context_t context,uint32_t *key);

int32_t shell_vscanf_s(const char *fmt_ptr, va_list ap);
int32_t shell_scanf(const char *fmt_ptr, ...);
void shell_scanf_init(void);
void shell_scanf_exit(void);


/* @} */

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* _FSL_SHELL_H_ */

