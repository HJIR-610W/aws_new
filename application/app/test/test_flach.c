#include <string.h>
#include <stdio.h>

#include "debug_io.h"
#include "drv_flash.h"
#include "user_heap.h"
#include "stm32f4xx_hal.h"

#define FLASH_CAPACITY (8 * 1024 * 1024)  // 8MB


/*
Writing 8388608 bytes to flash at address 0x00000000...
Write completed: 8388608 bytes in 322872 ms (25.37 KB/s)
Reading 8388608 bytes from flash at address 0x00000000...
Read completed: 8388608 bytes in 123821 ms (66.16 KB/s)
*/


// test_lfs 함수 참고하여 작성한 flash 메모리 테스트 함수
int32_t test_flash_range(uint32_t start_addr, uint32_t test_size)
{
  int32_t err;
  uint8_t *write_buffer;
  uint8_t *read_buffer;
  uint32_t i;
  uint32_t startClk, endClk, elapsed;
  uint32_t totalBytes;
  uint32_t chunkSize;
  enum { FLASH_TEST_BUFFER_SIZE = 4096 };

  // 주소 범위 검증
  if ((start_addr + test_size) > FLASH_CAPACITY)
  {
    debug_printf("Error: Test range exceeds flash capacity (8MB)\r\n");
    return -1;
  }

  // 버퍼 할당
  write_buffer = (uint8_t *)user_malloc(FLASH_TEST_BUFFER_SIZE);
  if (write_buffer == NULL)
  {
    debug_printf("Failed to allocate write buffer\r\n");
    return -1;
  }

  read_buffer = (uint8_t *)user_malloc(FLASH_TEST_BUFFER_SIZE);
  if (read_buffer == NULL)
  {
    debug_printf("Failed to allocate read buffer\r\n");
    user_free(write_buffer);
    return -1;
  }

  // 쓰기 버퍼 초기화 (0xAA 패턴)
  memset(write_buffer, 0xAA, FLASH_TEST_BUFFER_SIZE);

  debug_printf("Writing %lu bytes to flash at address 0x%08lX...\r\n",
            (unsigned long)test_size, (unsigned long)start_addr);

  // 쓰기 시간 측정 시작
  startClk = HAL_GetTick();

  // 플래시 쓰기 루프
  totalBytes = 0;
  while (totalBytes < test_size)
  {
    chunkSize = ((test_size - totalBytes) >= FLASH_TEST_BUFFER_SIZE)
                ? FLASH_TEST_BUFFER_SIZE
                : (test_size - totalBytes);

    err = drv_flash_write(start_addr + totalBytes, write_buffer, chunkSize);
    if (err < 0)
    {
      debug_printf("Write error at offset %lu (Error: %ld)\r\n", totalBytes, (long)err);
      user_free(write_buffer);
      user_free(read_buffer);
      return err;
    }

    totalBytes += chunkSize;
  }

  // 쓰기 시간 측정 종료
  endClk = HAL_GetTick();
  elapsed = endClk - startClk;

  debug_printf("Write completed: %lu bytes in %lu ms (%.2f KB/s)\r\n",
            (unsigned long)test_size,
            (unsigned long)elapsed,
            (test_size / (elapsed > 0 ? (elapsed / 1000.0f) : 1.0f)) / 1024.0f);

  // ============================ 읽기 측정 ==============================

  debug_printf("Reading %lu bytes from flash at address 0x%08lX...\r\n",
            (unsigned long)test_size, (unsigned long)start_addr);

  // 읽기 시간 측정 시작
  startClk = HAL_GetTick();

  totalBytes = 0;
  while (totalBytes < test_size)
  {
    chunkSize = ((test_size - totalBytes) >= FLASH_TEST_BUFFER_SIZE)
                ? FLASH_TEST_BUFFER_SIZE
                : (test_size - totalBytes);

    drv_flash_read(start_addr + totalBytes, read_buffer, chunkSize);

    totalBytes += chunkSize;
  }

  // 읽기 시간 측정 종료
  endClk = HAL_GetTick();
  elapsed = endClk - startClk;

  debug_printf("Read completed: %lu bytes in %lu ms (%.2f KB/s)\r\n",
            (unsigned long)test_size,
            (unsigned long)elapsed,
            (test_size / (elapsed > 0 ? (elapsed / 1000.0f) : 1.0f)) / 1024.0f);

  // ============================ 데이터 검증 ==============================

  debug_printf("Verifying data integrity...\r\n");

  totalBytes = 0;
  while (totalBytes < test_size)
  {
    chunkSize = ((test_size - totalBytes) >= FLASH_TEST_BUFFER_SIZE)
                ? FLASH_TEST_BUFFER_SIZE
                : (test_size - totalBytes);

    drv_flash_read(start_addr + totalBytes, read_buffer, chunkSize);

    // 데이터 비교
    for (i = 0; i < chunkSize; i++)
    {
      if (read_buffer[i] != 0xAA)
      {
        debug_printf("Data mismatch at offset %lu: expected 0xAA, got 0x%02X\r\n",
                  totalBytes + i, read_buffer[i]);
        user_free(write_buffer);
        user_free(read_buffer);
        return -1;
      }
    }

    totalBytes += chunkSize;
  }

  debug_printf("Data verification passed!\r\n");
  debug_printf("Flash test completed successfully\r\n");

  user_free(write_buffer);
  user_free(read_buffer);

  return 0;
}


void test_flash(void)
{
  test_flash_range(0,1024*1024);
}
