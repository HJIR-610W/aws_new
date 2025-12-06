

#include "app_console_test.h"
#include "aws_menu.h"
#include "shell.h"
#include "aws_develop.h"
 
#include "console_define.h"
#include "console_utile.h"

int32_t menu_root( int32_t argc, char **argv)
{
  aws_menu();

  return 0;
}

int32_t menu_develop( int32_t argc, char **argv)
{
  int status;
  int ok;

  status = check_pass("비밀번호를 입력해주세요", "0725",&ok);
  if(status == MENU_OK && ok==1)
    aws_menu_develop();
  
  
  return 0;
}