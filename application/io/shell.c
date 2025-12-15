
#define __STDC_WANT_LIB_EXT1__ 1
#include <assert.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "shell.h"
#include "debug_io.h"
#include "cli_key_code.h"
/*******************************************************************************
 * 정의
 ******************************************************************************/

#define KET_DEL (0x7FU)

/*******************************************************************************
 * 프로토타입
 ******************************************************************************/
static int32_t HelpCommand( int32_t argc, char **argv); /*!< 도움말 명령 */

static int32_t ExitCommand( int32_t argc, char **argv); /*!< 종료 명령 */

static int32_t ParseLine(const char *cmd, uint32_t len, char *argv[SHELL_MAX_ARGS]); /*!< 라인 파싱 명령 */

static int32_t StrCompare(const char *str1, const char *str2, int32_t count); /*!< 문자열 비교 명령 */

static void ProcessCommand(p_shell_context_t context, const char *cmd); /*!< 명령 처리 */

static void GetHistoryCommand(p_shell_context_t context, uint8_t hist_pos); /*!< 명령 히스토리 가져오기 */

static void AutoComplete(p_shell_context_t context); /*!< 자동 완성 명령 */



static int32_t StrLen(const char *str); /*!< 문자열 길이 가져오기 */

static char *StrCopy(char *dest, const char *src, int32_t count); /*!< 문자열 복사 */

/*******************************************************************************
 * 변수
 ******************************************************************************/
static const shell_command_context_t xHelpCommand = {"help", "\r\n\"help\": Lists all the registered commands\r\n",
                                                     HelpCommand, 0};

static const shell_command_context_t xExitCommand = {"exit", "\r\n\"exit\": Exit program\r\n", ExitCommand, 0};

static shell_command_context_list_t g_RegisteredCommands;

static char g_paramBuffer[SHELL_BUFFER_SIZE];

/*******************************************************************************
 * 코드
 ******************************************************************************/
void shell_init(
    p_shell_context_t context,   printf_data_t shell_printf, char *prompt)
{

    assert(prompt != NULL);
    assert(shell_printf != NULL);

    /* 컨텍스트 초기화 */
    memset_s(context, sizeof(shell_context_struct),0,sizeof(shell_context_struct));
    context->prompt = prompt;

    shell_register_command(&xHelpCommand);
    shell_register_command(&xExitCommand);
}

bool shell_exit=false;
int32_t shell_loop(p_shell_context_t context)
{
    uint8_t ch;
    int32_t i;

    if (!context)
    {
        return -1;
    }

    shell_exit = false;
    debug_printf(context->prompt);

    while (1)
    {
        if (shell_exit)
        {
            break;
        }
        debug_get_ch(&ch);


        /* 특수 키 */
        if (ch == KEY_CODE_ESC)
        {
            context->stat = kSHELL_Special;
            continue;
        }
        else if (context->stat == kSHELL_Special)
        {
            /* 기능 키 */
            if (ch == '[')
            {
                context->stat = kSHELL_Function;
                continue;
            }
            context->stat = kSHELL_Normal;
        }
        else if (context->stat == kSHELL_Function)
        {
            context->stat = kSHELL_Normal;

            switch ((uint8_t)ch)
            {
                /* 히스토리 동작 */
                case 'A': /* 위쪽 키 */
                    GetHistoryCommand(context, context->hist_current);
                    if (context->hist_current < (context->hist_count - 1))
                    {
                        context->hist_current++;
                    }
                    break;
                case 'B': /* 아래쪽 키 */
                    GetHistoryCommand(context, context->hist_current);
                    if (context->hist_current > 0)
                    {
                        context->hist_current--;
                    }
                    break;
                case 'D': /* 왼쪽 키 */
                    if (context->c_pos)
                    {
                        debug_printf("\b");
                        context->c_pos--;
                    }
                    break;
                case 'C': /* 오른쪽 키 */
                    if (context->c_pos < context->l_pos)
                    {
                        debug_printf("%c", context->line[context->c_pos]);
                        context->c_pos++;
                    }
                    break;
                default:
                    break;
            }
            continue;
        }
        /* 탭 키 처리 */
        else if (ch == '\t')
        {
#if SHELL_AUTO_COMPLETE
            /* 커서를 라인 시작으로 이동 */
            for (i = 0; i < context->c_pos; i++)
            {
                debug_printf("\b");
            }
            /* 자동 완성 수행 */
            AutoComplete(context);
            /* 끝 위치로 이동 */
            context->c_pos = context->l_pos = StrLen(context->line);
#endif
            continue;
        }
#if SHELL_SEARCH_IN_HIST
        /* 히스토리에서 명령 검색 */
        else if ((ch == '`') && (context->l_pos == 0) && (context->line[0] == 0x00))
        {
        }
#endif
        /* 백스페이스 키 처리 */
        else if ((ch == KET_DEL) || (ch == '\b'))
        {
            /* 최소 한 문자가 있어야 함 */
            if (context->c_pos == 0)
            {
                continue;
            }

            context->l_pos--;
            context->c_pos--;

            if (context->l_pos > context->c_pos)
            {
                memmove_s(&context->line[context->c_pos], sizeof(context->line)-context->c_pos,
                &context->line[context->c_pos + 1],
                        context->l_pos - context->c_pos);
                context->line[context->l_pos] = 0;
                debug_printf("\b%s  \b", &context->line[context->c_pos]);

                /* 위치 재설정 */
                for (i = context->c_pos; i <= context->l_pos; i++)
                {
                    debug_printf("\b");
                }
            }
            else /* 일반 백스페이스 동작 */
            {
                debug_printf("\b \b");
                context->line[context->l_pos] = 0;
            }
            continue;
        }
        else
        {
        }

        /* 입력이 너무 긺 */
        if (context->l_pos >= (SHELL_BUFFER_SIZE - 1))
        {
            context->l_pos = 0;
        }

        /* 라인 끝 처리, 중단 */
        if ((ch == '\r') || (ch == '\n'))
        {
            debug_printf("\r\n");
            ProcessCommand(context, context->line);
            /* 모든 매개변수 재설정 */
            context->c_pos = context->l_pos = 0;
            context->hist_current = 0;
            debug_printf(context->prompt);
            memset_s(context->line,  sizeof(context->line),0,sizeof(context->line));
            continue;
        }

        /* 일반 문자 */
        if (context->c_pos < context->l_pos)
        {
            memmove_s(&context->line[context->c_pos + 1], sizeof(context->line) - context->c_pos -1,
                   &context->line[context->c_pos],
                    context->l_pos - context->c_pos);
            context->line[context->c_pos] = ch;
            debug_printf("%s", &context->line[context->c_pos]);
            /* 커서를 새 위치로 이동 */
            for (i = context->c_pos; i < context->l_pos; i++)
            {
                debug_printf("\b");
            }
        }
        else
        {
            context->line[context->l_pos] = ch;
            debug_printf("%c", ch);
        }

        ch = 0;
        context->l_pos++;
        context->c_pos++;
    }
    return 0;
}

static int32_t HelpCommand(  int32_t argc, char **argv)
{
    uint8_t i = 0;

    for (i = 0; i < g_RegisteredCommands.numberOfCommandInList; i++)
    {
        debug_printf(g_RegisteredCommands.CommandList[i]->pcHelpString);
    }
    return 0;
}

static int32_t ExitCommand( int32_t argc, char **argv)
{
    /* 경고 생략 */
    debug_printf("\r\nShell exited\r\n");
    shell_exit = true;
    return 0;
}

static void ProcessCommand(p_shell_context_t context, const char *cmd)
{
    static const shell_command_context_t *tmpCommand = NULL;
    static const char *tmpCommandString;
    int32_t argc;
    char *argv[SHELL_BUFFER_SIZE];
    uint8_t flag = 1;
    uint8_t tmpCommandLen;
    uint8_t tmpLen;
    uint8_t i = 0;

    tmpLen = StrLen(cmd);
    argc = ParseLine(cmd, tmpLen, argv);

    if ((tmpCommand == NULL) && (argc > 0))
    {
        for (i = 0; i < g_RegisteredCommands.numberOfCommandInList; i++)
        {
            tmpCommand = g_RegisteredCommands.CommandList[i];
            tmpCommandString = tmpCommand->pcCommand;
            tmpCommandLen = StrLen(tmpCommandString);
            /* 공백 또는 문자열 끝과 비교 */
            if ((cmd[tmpCommandLen] == ' ') || (cmd[tmpCommandLen] == 0x00))
            {
                if (StrCompare(tmpCommandString, argv[0], tmpCommandLen) == 0)
                {
                    if ((tmpCommand->cExpectedNumberOfParameters == 0) && (argc == 1))
                    {
                        flag = 0;
                    }
                    else if (tmpCommand->cExpectedNumberOfParameters > 0)
                    {
                        if ((argc - 1) == tmpCommand->cExpectedNumberOfParameters)
                        {
                            flag = 0;
                        }
                    }
                    else
                    {
                        flag = 1;
                    }
                    break;
                }
            }
        }
    }

    if ((tmpCommand != NULL) && (flag == 1U))
    {
      debug_printf("\r\nType 'help' to see the list of commands.\r\n\r\n");

      tmpCommand = NULL;
    }
    else if (tmpCommand != NULL)
    {
        tmpLen = StrLen(cmd);
        /* 마지막 명령과 비교. 다를 경우 히스토리 버퍼에 추가 */
        if (tmpLen != StrCompare(cmd, context->hist_buf[0], StrLen(cmd)))
        {
            for (i = SHELL_HIST_MAX - 1; i > 0; i--)
            {
    memset_s(context->hist_buf[i], SHELL_BUFFER_SIZE,'\0',SHELL_BUFFER_SIZE);
                tmpLen = StrLen(context->hist_buf[i - 1]);
                StrCopy(context->hist_buf[i], context->hist_buf[i - 1], tmpLen);
            }
            memset_s(context->hist_buf[0], SHELL_BUFFER_SIZE, '\0' , SHELL_BUFFER_SIZE);
            tmpLen = StrLen(cmd);
            StrCopy(context->hist_buf[0], cmd, tmpLen);
            if (context->hist_count < SHELL_HIST_MAX)
            {
                context->hist_count++;
            }
        }
        tmpCommand->pFuncCallBack( argc, argv);
        tmpCommand = NULL;
    }
    else
    {
      debug_printf("\r\nType 'help' to see the available commands.\r\n\r\n");

      tmpCommand = NULL;
    }
}

static void GetHistoryCommand(p_shell_context_t context, uint8_t hist_pos)
{
    uint8_t i;
    uint32_t tmp;

    if (context->hist_buf[0][0] == '\0')
    {
        context->hist_current = 0;
        return;
    }
    if (hist_pos >= SHELL_HIST_MAX)
    {
        hist_pos = SHELL_HIST_MAX - 1;
    }
    tmp = StrLen(context->line);
    /* 현재 내용이 있으면 지우기 */
    if (tmp > 0)
    {
        memset_s(context->line,sizeof(context->line),'\0' , tmp);
        for (i = 0; i < tmp; i++)
        {
            debug_printf("\b \b");
        }
    }

    context->l_pos = StrLen(context->hist_buf[hist_pos]);
    context->c_pos = context->l_pos;
    StrCopy(context->line, context->hist_buf[hist_pos], context->l_pos);
    debug_printf(context->hist_buf[hist_pos]);
}

static void AutoComplete(p_shell_context_t context)
{
    int32_t len;
    int32_t minLen;
    uint8_t i = 0;
    const shell_command_context_t *tmpCommand = NULL;
    const char *namePtr;
    const char *cmdName;

    minLen = 0;
    namePtr = NULL;

    if (!StrLen(context->line))
    {
        return;
    }
    debug_printf("\r\n");
    /* 빈 탭, 모든 명령 나열 */
    if (context->line[0] == '\0')
    {
        HelpCommand( 0, NULL);
        return;
    }
    /* 자동 완성 수행 */
    for (i = 0; i < g_RegisteredCommands.numberOfCommandInList; i++)
    {
        tmpCommand = g_RegisteredCommands.CommandList[i];
        cmdName = tmpCommand->pcCommand;
        if (StrCompare(context->line, cmdName, StrLen(context->line)) == 0)
        {
            if (minLen == 0)
            {
                namePtr = cmdName;
                minLen = StrLen(namePtr);
                /* 가능한 일치 항목 표시 */
                debug_printf("%s\r\n", cmdName);
                continue;
            }
            len = StrCompare(namePtr, cmdName, StrLen(namePtr));
            if (len < 0)
            {
                len = len * (-1);
            }
            if (len < minLen)
            {
                minLen = len;
            }
        }
    }
    /* 자동 완성 문자열 */
    if (namePtr)
    {
        StrCopy(context->line, namePtr, minLen);
    }
    debug_printf("%s%s", context->prompt, context->line);
    return;
}

static char *StrCopy(char *dest, const char *src, int32_t count)
{
    char *ret = dest;
    int32_t i = 0;

    for (i = 0; i < count; i++)
    {
        dest[i] = src[i];
    }

    return ret;
}

static int32_t StrLen(const char *str)
{
    int32_t i = 0;

    while (*str)
    {
        str++;
        i++;
    }
    return i;
}

static int32_t StrCompare(const char *str1, const char *str2, int32_t count)
{
    while (count--)
    {
        if (*str1++ != *str2++)
        {
            return *(unsigned char *)(str1 - 1) - *(unsigned char *)(str2 - 1);
        }
    }
    return 0;
}

static int32_t ParseLine(const char *cmd, uint32_t len, char *argv[SHELL_MAX_ARGS])
{
    uint32_t argc;
    char *p;
    uint32_t position;

    /* 매개변수 초기화 */
    memset_s(g_paramBuffer, sizeof(g_paramBuffer),'\0', len + 1);
    StrCopy(g_paramBuffer, cmd, len);

    p = g_paramBuffer;
    position = 0;
    argc = 0;

    while (position < len)
    {
        /* 모든 공백 건너뛰기 */
        while (((char)(*p) == ' ') && (position < len))
        {
            *p = '\0';
            p++;
            position++;
        }
        /* 문자열 시작 처리 */
        if (*p == '"')
        {
            p++;
            position++;
            argv[argc] = p;
            argc++;
            /* 이 문자열 건너뛰기 */
            while ((*p != '"') && (position < len))
            {
                p++;
                position++;
            }
            /* '"' 건너뛰기 */
            *p = '\0';
            p++;
            position++;
        }
        else /* 일반 문자 */
        {
            argv[argc] = p;
            argc++;
            while (((char)*p != ' ') && ((char)*p != '\t') && (position < len))
            {
                p++;
                position++;
            }
        }
    }
    return argc;
}

int32_t shell_register_command(const shell_command_context_t *command_context)
{
    int32_t result = 0;

    /* 명령 목록에 공간이 있는 경우 */
    if (g_RegisteredCommands.numberOfCommandInList < SHELL_MAX_CMD)
    {
        g_RegisteredCommands.CommandList[g_RegisteredCommands.numberOfCommandInList++] = command_context;
    }
    else
    {
        result = -1;
    }
    return result;
}








static void inputCommand(p_shell_context_t context)
{
    uint8_t tmpLen;
    uint8_t i = 0;

    for (i = SHELL_HIST_MAX - 1; i > 0; i--)
    {
         memset_s(context->hist_buf[i],sizeof(context->hist_buf[i]), '\0', SHELL_BUFFER_SIZE);
        tmpLen = strnlen_s(context->hist_buf[i - 1], sizeof(context->hist_buf[0]));
         StrCopy(context->hist_buf[i], context->hist_buf[i - 1], tmpLen);
    }

    memset_s(context->hist_buf[0], sizeof(context->hist_buf[0]), '\0', SHELL_BUFFER_SIZE);
    tmpLen = strnlen_s(context->line, sizeof(context->line));
    StrCopy(context->hist_buf[0], context->line, tmpLen);
    if (context->hist_count < SHELL_HIST_MAX)
    {
        context->hist_count++;
    }
}

int32_t SHELL_recv(p_shell_context_t context,uint32_t *key)
{
    uint8_t ch;
    int32_t i;

    if (context==NULL)
    {
        return -1;
    }

    memset_s(context->line, sizeof(context->line), 0x00,sizeof(context->line));
    context->exit = false;

    *key = 0;
    while (1)
    {
        if (context->exit)
        {
            break;
        }

        debug_get_ch(&ch);

        

        if(ch== KEY_CODE_CTRL_C)
        {
            debug_puts("\r\nCtrl+c\r\n");
            context->c_pos = context->l_pos = 0;
            context->hist_current = 0;
            *key = KEY_CODE_CTRL_C;
            return 0;
        }
        else if(ch == KEY_CODE_CTRL_Q)
        {
            debug_puts("\r\nCtrl+q\r\n");
            context->c_pos = context->l_pos = 0;
            context->hist_current = 0;
            *key = KEY_CODE_CTRL_Q;
            return 0;
        }
        else if(ch == KEY_CODE_ESC)
        {
            context->stat = kSHELL_Special;
            continue;
        }
        else if (context->stat == kSHELL_Special)
        {
            /* 기능 키 */
            if (ch == '[')
            {
                context->stat = kSHELL_Function;
                continue;
            }
            context->stat = kSHELL_Normal;
        }
        else if (context->stat == kSHELL_Function)
        {
            context->stat = kSHELL_Normal;

            switch ((uint8_t)ch)
            {
                /* 히스토리 동작 */
                case 'A': /* 위쪽 키 */
                    GetHistoryCommand(context, context->hist_current);
                    if (context->hist_current < (context->hist_count - 1))
                    {
                        context->hist_current++;
                    }
                    break;
                case 'B': /* 아래쪽 키 */
                    GetHistoryCommand(context, context->hist_current);
                    if (context->hist_current > 0)
                    {
                        context->hist_current--;
                    }
                    break;
                case 'D': /* 왼쪽 키 */
                    if (context->c_pos)
                    {
                        debug_printf("\b");
                        context->c_pos--;
                    }
                    break;
                case 'C': /* 오른쪽 키 */
                    if (context->c_pos < context->l_pos)
                    {
                        debug_printf("%c", context->line[context->c_pos]);
                        context->c_pos++;
                    }
                    break;
                default:
                    break;
            }
            continue;
        }
        /* 탭 키 처리 */
        else if (ch == '\t')
        {
#if SHELL_AUTO_COMPLETE
            /* 커서를 라인 시작으로 이동 */
            for (i = 0; i < context->c_pos; i++)
            {
                debug_put_ch('\b');
            }
            /* 자동 완성 수행 */
            AutoComplete(context);
            /* 끝 위치로 이동 */
            context->c_pos = context->l_pos = strnlen_s(context->line, sizeof(context->line));
#endif
            continue;
        }
#if SHELL_SEARCH_IN_HIST
        /* 히스토리에서 명령 검색 */
        else if ((ch == '`') && (context->l_pos == 0) && (context->line[0] == 0x00))
        {
        }
#endif
        /* 백스페이스 키 처리 */
        else if ((ch == KET_DEL) || (ch == '\b'))
        {
            /* 최소 한 문자가 있어야 함 */
            if (context->c_pos == 0)
            {
                continue;
            }

            context->l_pos--;
            context->c_pos--;

            if (context->l_pos > context->c_pos)
            {
                memmove_s(&context->line[context->c_pos], sizeof(context->line) - context->c_pos,
                &context->line[context->c_pos + 1],
                        context->l_pos - context->c_pos);
                context->line[context->l_pos] = 0;
                debug_printf("\b%s  \b", &context->line[context->c_pos]);

                /* 위치 재설정 */
                for (i = context->c_pos; i <= context->l_pos; i++)
                {
                    debug_printf("\b");
                }
            }
            else /* 일반 백스페이스 동작 */
            {
                debug_printf("\b \b");
                context->line[context->l_pos] = 0;
            }
            continue;
        }
        else
        {
        }

        /* 입력이 너무 긺 */
        if (context->l_pos >= (SHELL_BUFFER_SIZE - 1))
        {
            context->l_pos = 0;
        }

        /* 라인 끝 처리, 중단 */
        if ((ch == '\r') || (ch == '\n'))
        {
            debug_printf("\r\n");
            context->c_pos = context->l_pos = 0;
            context->hist_current = 0;

           inputCommand(context);
            return strnlen_s(context->line, sizeof(context->line));
        }

        /* 일반 문자 */
        if (context->c_pos < context->l_pos)
        {
            memmove_s(&context->line[context->c_pos + 1], sizeof(context->line) - context->c_pos - 1,
                    &context->line[context->c_pos],
                    context->l_pos - context->c_pos);
            context->line[context->c_pos] = ch;
            debug_printf("%s", &context->line[context->c_pos]);
            /* 커서를 새 위치로 이동 */
            for (i = context->c_pos; i < context->l_pos; i++)
            {

                debug_put_ch('\b');
            }
        }
        else
        {
            context->line[context->l_pos] = ch;
            debug_printf("%c", ch);
        }

        ch = 0;
        context->l_pos++;
        context->c_pos++;
    }
    return 0;
}



shell_context_struct *g_scanf_context = NULL;

int32_t shell_scanf(const char *fmt_ptr, ...)
{
    int32_t cnt;
    int32_t result =-1;
    uint32_t key=0;
    va_list ap;

    va_start(ap, fmt_ptr);

    if(g_scanf_context)
    {
        cnt =  SHELL_recv(g_scanf_context,&key);

        if(key == KEY_CODE_CTRL_Q || key ==KEY_CODE_CTRL_C)
        {
            result = key;
            goto END_FUNC;
        }

        if(cnt == 0)
        {
            result  = 0;
            goto END_FUNC;
        }
        else
        {
         result = vsscanf_s(g_scanf_context->line, fmt_ptr, ap);
        }
    }

END_FUNC:
    va_end(ap);

    return result;
}

int32_t shell_vscanf_s(const char *fmt_ptr, va_list ap)
{
    int32_t cnt;
    int32_t result = -1;
    uint32_t key = 0;

    if(g_scanf_context)
    {
        cnt = SHELL_recv(g_scanf_context, &key);

        if(key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
        {
            result = key;
            goto END_FUNC;
        }

        if(cnt == 0)
        {
            result = 0;
            goto END_FUNC;
        }
        else
        {
            result = vsscanf_s(g_scanf_context->line, fmt_ptr, ap);
        }
    }

END_FUNC:
    return result;
}


void shell_scanf_init( void )
{
    if(g_scanf_context==NULL)
    {
        g_scanf_context = (shell_context_struct *)pvPortMalloc(sizeof(shell_context_struct));
    }
}

void shell_scanf_exit(void)
{
    vPortFree(g_scanf_context);
    g_scanf_context = NULL;
}
