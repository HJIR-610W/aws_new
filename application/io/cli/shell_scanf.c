


#include "console_scanf.h"

#include <stdarg.h>
#include <stdio.h>

#include "cmsis_os.h"
#include "fsl_shell.h"
#include "fsl_debug_console.h"
#include "debug_io.h"
#include "cli_key_code.h"

shell_context_struct *g_scanf_context = NULL;

int32_t console_scanf(const char *fmt_ptr, ...)
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
         result = vsscanf(g_scanf_context->line, fmt_ptr, ap);
        }
    }

END_FUNC:
    va_end(ap);

    return result;
}


void console_scanf_init( void )
{
    if(g_scanf_context==NULL)
    {
        g_scanf_context = (shell_context_struct *)pvPortMalloc(sizeof(shell_context_struct));
    }
}

void console_scanf_exit(void)
{
    vPortFree(g_scanf_context);
    g_scanf_context = NULL;
}
