
#include <string.h>  // for memcmp

#include "dev_io.h"
#include "driver_flash.h"

#define TEST_ADDR 0
#define TEST_SIZE 600

void test_flash(void)
{
  driver_t *flash;
  uint8_t write_data[TEST_SIZE] ;
  uint8_t read_data[TEST_SIZE] = {0};

  flash = driver_flash_open(FALSH_AT45DB);
  if (flash == NULL)
  {
    io_printf("Flash 드라이버 열기 실패\r\n");
    return;
  }

  for (int i = 0; i < TEST_SIZE;i++)
  {
    write_data[i] = i;
  }
    // 쓰기
    driver_flash_write(flash, TEST_ADDR, write_data, sizeof(write_data));


  // 읽기
  driver_flash_read(flash, TEST_ADDR, read_data, sizeof(read_data), sizeof(read_data));


  // 비교
  if (memcmp(write_data, read_data, TEST_SIZE) == 0)
  {
    io_printf("Flash 테스트 성공 \r\n");
  }
  else
  {
    io_printf("Flash 테스트 실패 \r\n");
    io_printf("쓰기값: ");
    LOG_MEM(write_data, TEST_SIZE, 0, 160);
    io_printf("읽은값: ");
    LOG_MEM(read_data, TEST_SIZE, 0, 160);
  }
}
