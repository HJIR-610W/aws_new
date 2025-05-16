
#include "test_filesystem.h"

#include <string.h>

#include "App_drivers\app_file.h"
#include "cli_input.h"
#include "dev_io.h"
#include "user_heap.h"


volatile uint8_t *p_data;
#define TEST_FILE_NAME "0:test.txt"
#define TEST_MSG "1234"
#define TEST_MSG_LEN 4
void test_filesystem(void)
{
  char buff[20]="0:";
  char temp[10];
  int ret;



  //파일 쓰기 ,읽기 속도 테스트
  test_file_rw_speed("0:1mb.txt",1024*1024);

  //파일 읽기,쓰기 비교 테스트
  write_file(TEST_FILE_NAME, TEST_MSG, TEST_MSG_LEN, 0);
  read_file(TEST_FILE_NAME, temp, TEST_MSG_LEN, 0);

  if (strncmp(temp, TEST_MSG, TEST_MSG_LEN) == 0)
  {
    debug_printf("파일 쓰기 읽기 정상 \r\n");
  }
  else
  {
    debug_printf("파일 쓰기 읽기 실패 \r\n");
  }


  debug_printf("'0:'목록을 출력합니다.\r\n");
  while(1)
  {
    list_directory(buff);

    debug_printf("경로를 입력하세요>>\r\n");
    ret = cli_scanf_s("%19s", buff, sizeof(buff));

    if (ret == CLI_KEYCODE_CTRL_C)
    {
      debug_printf("테스트 종료\r\n");
      break;
    }
    
  }
}