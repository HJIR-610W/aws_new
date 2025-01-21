


#include <stdarg.h>

#include "cmsis_os.h"

#include "console_scanf.h"
#include "fsl_shell.h"



shell_context_struct *pxShell_context_struct = NULL;


extern  int DbgConsole_ScanfFormattedData(const char *line_ptr,const char *format, va_list args_ptr);

int32_t console_scanf(const char *fmt_ptr, ...)
{
    int32_t cnt;
    va_list ap;
    int32_t result =-1;
    uint32_t key=0;

    va_start(ap, fmt_ptr);

    if(pxShell_context_struct)
    {

        cnt =  SHELL_recv(pxShell_context_struct,&key);
        
        
        
        if(key == KEY_EXIT_PROGRAM)
        {
            result  = -3;//exit
            goto END_FUNC;
        }
        else if( key == KEY_PREV_PROGRAM)
        {
          result = -1;//back
          goto END_FUNC;
        }

        if(cnt == 0)
        {
            result  = 0;
            goto END_FUNC;
        }
        else
        {
            result = DbgConsole_ScanfFormattedData(pxShell_context_struct->line,(char *) fmt_ptr, ap);
        }
    }

END_FUNC:
    va_end(ap);

    return result;
}





void console_scanf_init( p_shell_context_t context)
{

    if(pxShell_context_struct==NULL)
    {
        pxShell_context_struct = (shell_context_struct *)pvPortMalloc(sizeof(shell_context_struct));
    }

    if(pxShell_context_struct)
    {
        SHELL_input_init(  pxShell_context_struct,  context->send_data_func,  context->recv_data_func,  context->printf);
    }
}

void console_scanf_exit(void)
{
    vPortFree(pxShell_context_struct);
    pxShell_context_struct = NULL;
}