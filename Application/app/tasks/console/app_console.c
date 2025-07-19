

#include "app_console_test.h"
#include "aws_menu.h"
#include "fsl_shell.h"
#include "aws_develop.h"
#include "cli_input.h"
#include "console_define.h"
#include "console_utile.h"

int32_t menu_root(p_shell_context_t ctx, int32_t argc, char **argv)
{
  aws_menu();

  return 0;
}

int32_t menu_develop(p_shell_context_t ctx, int32_t argc, char **argv)
{
  int status;
  int ok;

  status = check_pass("진행 코드 입력해주세요", "1601",&ok);
  if(status == MENU_OK && ok==1)
    aws_menu_develop();
  
  
  return 0;
}