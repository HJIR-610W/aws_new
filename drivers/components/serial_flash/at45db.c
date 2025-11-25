/**
 ******************************************************************************
 * @file    at45db.c
 * @brief   AT45DB DataFlash 시리얼 플래시 메모리 드라이버
 * @details AT45DB321/641 시리얼 플래시 메모리 제어 드라이버
 *          - 지원 디바이스: AT45DB321, AT45DB641E
 *          - SPI 통신 인터페이스
 *          - 바이너리 페이지 모드 지원
 * @version 1.0.0
 * @date    2025-11-01
 ******************************************************************************
 */

#include "at45db.h"

#include <string.h>

#include "bsp_delay.h"
#include "bsp_do.h"
#include "bsp_spi.h"
#include "os_user_def.h"
#include "system_err.h"

// AT45DB 명령어 정의
#define AT45DB_CMD_DEVICE_ID                0x9F    // 디바이스 ID 읽기
#define AT45DB_CMD_STATUS_REGISTER         0xD7    // 상태 레지스터 읽기

// 버퍼 쓰기 명령어
#define AT45DB_CMD_BUFFER1_WRITE           0x84    // 버퍼 1 쓰기
#define AT45DB_CMD_BUFFER2_WRITE           0x87    // 버퍼 2 쓰기

// 버퍼 읽기 명령어
#define AT45DB_CMD_BUFFER1_READ            0xD4    // 버퍼 1 읽기
#define AT45DB_CMD_BUFFER2_READ            0xD6    // 버퍼 2 읽기

// 버퍼에서 메모리로 전송 명령어
#define AT45DB_CMD_BUFFER1_TO_MEMORY       0x83    // 버퍼 1을 메모리 페이지로 프로그램
#define AT45DB_CMD_BUFFER2_TO_MEMORY       0x86    // 버퍼 2를 메모리 페이지로 프로그램

// 메모리에서 버퍼로 전송 명령어
#define AT45DB_CMD_MEMORY_TO_BUFFER1       0x53    // 메모리 페이지를 버퍼 1로 전송
#define AT45DB_CMD_MEMORY_TO_BUFFER2       0x55    // 메모리 페이지를 버퍼 2로 전송

// 설정 명령어
#define AT45DB_CMD_SECTOR_PROTECT_ENABLE   {0x3d, 0x2a, 0x7f, 0xa9}  // 섹터 보호 활성화
#define AT45DB_CMD_BINARY_PAGE_SIZE        {0x3d, 0x2a, 0x80, 0xa6}  // 2의 거듭제곱 페이지 크기 설정

// 버퍼 선택
#define AT45DB_BUFFER1                     0
#define AT45DB_BUFFER2                     1

// 상태 레지스터 비트 정의
#define AT45DB_STATUS_READY_BUSY_BIT       0x80    // 비트 7: Ready/Busy 상태 (1=준비, 0=바쁨)
#define AT45DB_STATUS_COMPARE_BIT          0x40    // 비트 6: 비교 결과
#define AT45DB_STATUS_DENSITY_MASK         0x3C    // 비트 5-2: 디바이스 용량
#define AT45DB_STATUS_PROTECT_BIT          0x02    // 비트 1: 섹터 보호
#define AT45DB_STATUS_PAGE_SIZE_BIT        0x01    // 비트 0: 페이지 크기 (1=바이너리, 0=표준)

// 주소 비트 마스크
#define AT45DB_ADDR_HIGH_MASK              0x11    // 버퍼 주소 지정용 상위 주소 비트 마스크

// 타임아웃 및 재시도 상수
#define AT45DB_BUSY_TIMEOUT_MS             100     // 최대 대기 타임아웃
#define AT45DB_BUSY_POLL_DELAY_US          100     // 상태 폴링 간 지연시간

// 페이지 크기 상수
#define AT45DB_MAX_PAGE_SIZE               512     // AT45DB256 최대 페이지 크기 (바이트)

// 하위 호환성을 위한 구형 정의
#define DEVICE_ID                          AT45DB_CMD_DEVICE_ID
#define STATUS_REGISTER                    AT45DB_CMD_STATUS_REGISTER
#define BYTE_DUMMY 				0x00 			// 더미 바이트

typedef struct ad45db_instance_s
{
  at45db_chip_info_t chip_info;
  int spi_num;
  int cs_do_num;
  void *sem;
  bool opened;
} at45db_instance_t;



int32_t at45db_write(uint32_t offset, uint8_t *p_data, uint32_t data_len);
void at45db_read(uint32_t offset, uint8_t *p_buff, uint32_t read_len);
void at45db_read_page(uint32_t read_addr, uint8_t *read_buff);
void at45db_write_page(uint32_t write_addr, uint8_t *write_buff);
static void at45db_delay(uint32_t usec);
static void at45db_write_buffer(uint8_t buffer_choice, uint32_t address, const char *string, uint32_t buf_len);
static void at45db_page_write_cmd(uint32_t page);
static void at45db_buffer_to_memory(uint8_t buffer_choice, uint32_t page);
static void at45db_reg_read(uint8_t cmd, uint8_t *info, uint8_t len);
static void at45db_reg_write(uint8_t *cmd);
static void at45db_wait_ready(void);
static void at45db_memory_to_buffer(uint8_t buffer_choice, uint32_t page);
static void at45db_read_buffer(uint8_t buffer_choice, uint32_t address, char *string, uint16_t buf_len);
at45db_result_t at45db_find_device_info(uint8_t density_code, at45db_device_info_t *device_info);
at45db_result_t at45db_parse_chip_info(uint8_t *chip_info, at45db_chip_info_t *info);

at45db_result_t at45db_initialize(void);

static at45db_instance_t at45db_inst;

void at45db_init(void)
{
  if (at45db_inst.opened)
  {
    return;
  }

  at45db_inst.spi_num = BSP_SPI_1;
  at45db_inst.cs_do_num = BSP_DO_FLASH_CS;

  OS_CREATE_BINARY_SEM(at45db_inst.sem);
  bsp_spi_init(at45db_inst.spi_num);

  if (at45db_initialize() != AT45DB_OK)
  {
    DEBUG_PRINTF("Error: AT45DB initialization failed\r\n");
    at45db_inst.opened = false;
    return;
  }

  at45db_inst.opened = true;
}




static const at45db_device_info_t at45db_device_table[] = {
    /* density_code, capacity_bits, total_pages, page_std, page_bin, name */
    {0x02, 1 * 1024 * 1024, 512, 264, 256, 2048,"AT45DB011"},     /* 1Mbit */
    {0x03, 2 * 1024 * 1024, 1024, 264, 256,2048, "AT45DB021"},    /* 2Mbit */
    {0x04, 4 * 1024 * 1024, 2048, 264, 256, 2048,"AT45DB041"},    /* 4Mbit */
    {0x05, 8 * 1024 * 1024, 4096, 264, 256, 2048,"AT45DB081"},    /* 8Mbit */
    {0x06, 16 * 1024 * 1024, 4096, 528, 512, 2048,"AT45DB161"},   /* 16Mbit */
    {0x07, 32 * 1024 * 1024, 8192, 528, 512, 2048,"AT45DB321"},   /* 32Mbit */
    {0x08, 64 * 1024 * 1024, 32768, 264, 256, 2048,"AT45DB641E"}  /* 64Mbit */
    /* AT45DB128(1024B), AT45DB256(2048B): 메모리 제약으로 미지원 */
};

#define AT45DB_DEVICE_COUNT (sizeof(at45db_device_table) / sizeof(at45db_device_table[0]))


static void at45db_delay(uint32_t usec)
{
  bsp_us_delay(usec);
}


/**
 * @brief AT45DB 내부 버퍼에 데이터 쓰기
 * @param buffer_choice: 버퍼 선택 (AT45DB_BUFFER1 또는 AT45DB_BUFFER2)
 * @param address: 버퍼 주소 오프셋
 * @param string: 쓸 데이터
 * @param buf_len: 쓸 데이터 길이
 */
static void at45db_write_buffer( uint8_t buffer_choice, uint32_t address, const char *string, uint32_t buf_len)
{
  uint8_t cmd_buff[4];

  bsp_spi_pend_sem(at45db_inst.spi_num);

  bsp_do_low(at45db_inst.cs_do_num);
  at45db_delay(1);

  if (buffer_choice == AT45DB_BUFFER2)
  {
    cmd_buff[0] = AT45DB_CMD_BUFFER2_WRITE;
  }
  else
  {
    cmd_buff[0] = AT45DB_CMD_BUFFER1_WRITE;
  }

  cmd_buff[1] = BYTE_DUMMY;
  cmd_buff[2] = BYTE_DUMMY;
  cmd_buff[3] = (uint8_t)address;

  bsp_spi_send_bytes(at45db_inst.spi_num, cmd_buff, 4);

  bsp_spi_send_bytes(at45db_inst.spi_num, (uint8_t *)string, buf_len);
  at45db_delay(1);
  bsp_do_high(at45db_inst.cs_do_num);

  bsp_spi_post_sem(at45db_inst.spi_num);
}

static void at45db_page_write_cmd(uint32_t page)
{
  /* AT45DB321은 특별한 페이지 주소 형식 사용 */
  if (at45db_inst.chip_info.device_info.density_code == 0x07) /* AT45DB321 */
  {
    /* AT45DB321: PA14-PA7 | PA6-PA0 + don't care */
    bsp_spi_send_byte(at45db_inst.spi_num, (uint8_t)(page >> 7));
    bsp_spi_send_byte(at45db_inst.spi_num, (uint8_t)(page << 1));
  }
  else
  {
    /* 다른 디바이스: 표준 16비트 페이지 주소 */
    bsp_spi_send_byte(at45db_inst.spi_num, (uint8_t)(page >> 8));
    bsp_spi_send_byte(at45db_inst.spi_num, (uint8_t)(page));
  }
}

/**
 * @brief 버퍼 내용을 메모리 페이지로 전송
 * @param buffer_choice: 버퍼 선택 (AT45DB_BUFFER1 또는 AT45DB_BUFFER2)
 * @param page: 대상 페이지 번호
 */
static void at45db_buffer_to_memory(uint8_t buffer_choice, uint32_t page)
{
  bsp_spi_pend_sem(at45db_inst.spi_num);

  bsp_do_low(at45db_inst.cs_do_num);
  at45db_delay(1);

  if (buffer_choice == AT45DB_BUFFER2)
  {
    bsp_spi_send_byte(at45db_inst.spi_num, AT45DB_CMD_BUFFER2_TO_MEMORY);
  }
  else
  {
    bsp_spi_send_byte(at45db_inst.spi_num, AT45DB_CMD_BUFFER1_TO_MEMORY);
  }

  at45db_page_write_cmd(page);

  bsp_spi_send_byte(at45db_inst.spi_num, BYTE_DUMMY);
  at45db_delay(1);
  bsp_do_high(at45db_inst.cs_do_num);

  bsp_spi_post_sem(at45db_inst.spi_num);
}


static void at45db_reg_read(uint8_t cmd, uint8_t *info, uint8_t len)
{
  memset(info, 0, len);
  bsp_spi_pend_sem(at45db_inst.spi_num);

  bsp_do_low(at45db_inst.cs_do_num);
  at45db_delay(1);

  bsp_spi_send_byte(at45db_inst.spi_num, cmd);

  bsp_spi_read_bytes(at45db_inst.spi_num, info, len);

  at45db_delay(1);
  bsp_do_high(at45db_inst.cs_do_num);

  bsp_spi_post_sem(at45db_inst.spi_num);
}


static void at45db_reg_write(uint8_t *cmd)
{
  bsp_spi_pend_sem(at45db_inst.spi_num);

  bsp_do_low(at45db_inst.cs_do_num);
  at45db_delay(1);

  bsp_spi_send_bytes(at45db_inst.spi_num, cmd, 4);

  at45db_delay(1);
  bsp_do_high(at45db_inst.cs_do_num);

  bsp_spi_post_sem(at45db_inst.spi_num);
}


/**
 * @brief AT45DB가 준비될 때까지 대기 (바쁘지 않을 때까지)
 * @note 준비 비트가 설정되거나 타임아웃이 발생할 때까지 상태 레지스터를 폴링
 */
static void at45db_wait_ready(void)
{
  uint16_t timeout_count = 0;
  uint8_t status_reg;

  while(1)
  {
    at45db_reg_read( AT45DB_CMD_STATUS_REGISTER, &status_reg, 1);
    osDelay(1);
    // 디바이스가 준비되었는지 확인 (비트 7 = 1이면 준비됨)
    if(status_reg & AT45DB_STATUS_READY_BUSY_BIT) break;


    if(++timeout_count > AT45DB_BUSY_TIMEOUT_MS) {
      DEBUG_PRINTF("Warning: AT45DB busy timeout\r\n");
      break;
    }

  }
}


/**
 * @brief 페이지 쓰기
 * @param write_addr: 페이지 주소
 * @param write_buff: 쓸 데이터 버퍼
 * @note USB 인터럽트에서 호출 시 osDelay() 사용 금지
 */
void at45db_write_page(uint32_t write_addr, uint8_t *write_buff)
{
  const uint16_t page_size = at45db_inst.chip_info.current_page_size;

  /* 매개변수 유효성 검사 */
  if (write_buff == NULL)
  {
    return;
  }

  at45db_write_buffer(AT45DB_BUFFER1, 0, (const char *)write_buff, page_size);
  osDelay(1);
  at45db_buffer_to_memory(AT45DB_BUFFER1, write_addr);
  at45db_wait_ready();
}

/**
 * @brief 메모리 페이지를 버퍼로 전송
 * @param buffer_choice: 버퍼 선택 (AT45DB_BUFFER1 또는 AT45DB_BUFFER2)
 * @param page: 소스 페이지 번호
 */
static void at45db_memory_to_buffer(uint8_t buffer_choice, uint32_t page)
{
  bsp_spi_pend_sem(at45db_inst.spi_num);
  bsp_do_low(at45db_inst.cs_do_num);

  at45db_delay(1);

  if (buffer_choice == AT45DB_BUFFER2)
  {
    bsp_spi_send_byte(at45db_inst.spi_num, AT45DB_CMD_MEMORY_TO_BUFFER2);
  }
  else
  {
    bsp_spi_send_byte(at45db_inst.spi_num, AT45DB_CMD_MEMORY_TO_BUFFER1);
  }

  at45db_page_write_cmd(page);

  bsp_spi_send_byte(at45db_inst.spi_num, BYTE_DUMMY);
  at45db_delay(1);
  bsp_do_high(at45db_inst.cs_do_num);

  bsp_spi_post_sem(at45db_inst.spi_num);
}
/**
 * @brief AT45DB 내부 버퍼에서 데이터 읽기
 * @param buffer_choice: 버퍼 선택 (AT45DB_BUFFER1 또는 AT45DB_BUFFER2)
 * @param address: 버퍼 주소 오프셋
 * @param string: 읽은 데이터를 저장할 버퍼
 * @param buf_len: 읽을 데이터 길이
 */
static void at45db_read_buffer( uint8_t buffer_choice, uint32_t address, char *string, uint16_t buf_len)
{
  uint8_t cmd_buff[5];

  bsp_spi_pend_sem(at45db_inst.spi_num);

  bsp_do_low(at45db_inst.cs_do_num);
  at45db_delay(1);

  if (buffer_choice == AT45DB_BUFFER2)
  {
    cmd_buff[0] = AT45DB_CMD_BUFFER2_READ;
  }
  else
  {
    cmd_buff[0] = AT45DB_CMD_BUFFER1_READ;
  }
  cmd_buff[1] = BYTE_DUMMY;
  cmd_buff[2] = (uint8_t)((address >> 8) & AT45DB_ADDR_HIGH_MASK);
  cmd_buff[3] = (uint8_t)address;
  cmd_buff[4] = BYTE_DUMMY;

  bsp_spi_send_bytes(at45db_inst.spi_num, (uint8_t *)cmd_buff, 5);
  bsp_spi_read_bytes(at45db_inst.spi_num, (uint8_t *)string, buf_len);
  at45db_delay(1);
  bsp_do_high(at45db_inst.cs_do_num);
  bsp_spi_post_sem(at45db_inst.spi_num);
}
void at45db_read_page(uint32_t read_addr, uint8_t *read_buff)
{
  const uint16_t page_size = at45db_inst.chip_info.current_page_size;

  /* 매개변수 유효성 검사 */
  if (read_buff == NULL)
  {
    return;
  }

  at45db_memory_to_buffer(AT45DB_BUFFER1, read_addr);
  at45db_wait_ready();
  at45db_read_buffer(AT45DB_BUFFER1, 0, (char *)read_buff, page_size);
}

/**
 * @brief AT45DB 초기화
 * @return AT45DB_OK: 성공, AT45DB_ERROR: 실패
 * @note 디바이스 ID 확인, 바이너리 페이지 모드 전환
 */
at45db_result_t at45db_initialize(void)
{
  static const uint8_t protect_enable[4] = AT45DB_CMD_SECTOR_PROTECT_ENABLE;
  static const uint8_t page_binary_mode[4] = AT45DB_CMD_BINARY_PAGE_SIZE;
  at45db_result_t result;
  uint8_t chip_info[5] = {0, 0, 0, 0, 0};
  uint8_t reg = 0;

  // 디바이스 ID 정보
  // Byte[0] 제조사 ID       0x1F
  // Byte[1] 디바이스 ID     0x28
  // Byte[2] 디바이스 ID     0x00
  // Byte[3] 확장 정보 길이  0x01
  // Byte[4] EDI Byte 1      0x00

  at45db_reg_read(AT45DB_CMD_DEVICE_ID, chip_info, 5);

  result = at45db_parse_chip_info(chip_info, &at45db_inst.chip_info);
  if (result != AT45DB_OK) {
    DEBUG_PRINTF("Error: Failed to parse chip info\r\n");
    return result;
  }

  osDelay(1);

  at45db_reg_read( AT45DB_CMD_STATUS_REGISTER, &reg, 1);

  // 바이너리 페이지 크기 모드가 활성화되어 있는지 확인
  if ((reg & AT45DB_STATUS_PAGE_SIZE_BIT) == 0) {
    DEBUG_PRINTF("Switching to binary page size mode...\r\n");
    at45db_reg_write( (uint8_t *)protect_enable);
    at45db_wait_ready();
    at45db_reg_write( (uint8_t *)page_binary_mode);
    at45db_wait_ready();

    // 전환 확인
    at45db_reg_read( AT45DB_CMD_STATUS_REGISTER, &reg, 1);
    if ((reg & AT45DB_STATUS_PAGE_SIZE_BIT) == 0) {
      DEBUG_PRINTF("Error: Failed to switch to binary page size mode\r\n");
      return AT45DB_ERROR;
    }
  }

  // 현재 페이지 크기 및 기타 정보 설정
  at45db_inst.chip_info.is_binary_mode = (reg & AT45DB_STATUS_PAGE_SIZE_BIT) ? true : false;
  at45db_inst.chip_info.current_page_size = at45db_inst.chip_info.is_binary_mode ? 
                                     at45db_inst.chip_info.device_info.page_size_binary : 
                                     at45db_inst.chip_info.device_info.page_size_standard;
  at45db_inst.chip_info.total_capacity_bytes = (at45db_inst.chip_info.device_info.capacity_bits / 8);
  at45db_inst.chip_info.is_initialized = true;

  DEBUG_PRINTF("AT45DB initialization completed successfully\r\n");
  DEBUG_PRINTF("  Mode: %s\r\n", at45db_inst.chip_info.is_binary_mode ? "Binary" : "Standard");
  DEBUG_PRINTF("  Current page size: %u bytes\r\n", at45db_inst.chip_info.current_page_size);
  DEBUG_PRINTF("  Total capacity: %lu bytes\r\n", at45db_inst.chip_info.total_capacity_bytes);

  at45db_delay(10);
  
  return AT45DB_OK;

}

at45db_result_t at45db_get_chip_info(at45db_chip_info_t *info)
{
  /* 매개변수 유효성 검사 */
  if (info == NULL)
  {
    return AT45DB_ERROR;
  }

  if (!at45db_inst.chip_info.is_initialized)
  {
    return AT45DB_INIT_FAILED;
  }

  memcpy(info, &at45db_inst.chip_info, sizeof(at45db_chip_info_t));
  return AT45DB_OK;
}

/**
 * @brief RAM 접근처럼 플래시 읽기
 * @param offset: 읽기 시작 오프셋
 * @param p_buff: 읽은 데이터를 저장할 버퍼
 * @param read_len: 읽을 데이터 길이
 */
void at45db_read(uint32_t offset, uint8_t *p_buff, uint32_t read_len)
{
  const uint32_t page_size = at45db_inst.chip_info.current_page_size;
  uint32_t page_quot;
  uint32_t page_rem;
  uint32_t remain;
  uint32_t read_cnt = 0;
  uint32_t first_read;
  uint8_t buff[AT45DB_MAX_PAGE_SIZE];

  /* 매개변수 유효성 검사 */
  if (p_buff == NULL || read_len == 0)
  {
    return;
  }

  /* 초기화 상태 체크 */
  if (!at45db_inst.chip_info.is_initialized)
  {
    DEBUG_PRINTF("Error: AT45DB not initialized\r\n");
    return;
  }

  /* 경계 체크 */
  if (offset >= at45db_inst.chip_info.total_capacity_bytes ||
      (offset + read_len) > at45db_inst.chip_info.total_capacity_bytes)
  {
    DEBUG_PRINTF("Error: Read offset out of range (offset=0x%08lX, len=%lu)\r\n",
                 offset, read_len);
    return;
  }

  page_quot = offset / page_size;
  page_rem = offset % page_size;
  remain = page_size - page_rem;

  OS_PEND_SEM(at45db_inst.sem, osWaitForever);

  // 경우 1: 첫 페이지 부분 읽기 (페이지 정렬되지 않은 경우)
  if (page_rem > 0)
  {
    at45db_read_page( page_quot, buff);
    first_read = (read_len < remain) ? read_len : remain;
    memcpy(&p_buff[0], &buff[page_rem], first_read);
    page_quot++;
    read_cnt += first_read;
  }

  // 경우 2: 전체 페이지 읽기
  while ((read_len - read_cnt) >= page_size)
  {
    at45db_read_page( page_quot, buff);
    memcpy(&p_buff[read_cnt], buff, page_size);
    page_quot++;
    read_cnt += page_size;
  }

  // 경우 3: 마지막 페이지 부분 읽기 (남은 데이터가 있는 경우)
  if (read_len > read_cnt)
  {
    at45db_read_page( page_quot, buff);
    memcpy(&p_buff[read_cnt], buff, read_len - read_cnt);
  }

  OS_POST_SEM(at45db_inst.sem);
}



/**
 * @brief RAM 접근처럼 플래시 쓰기
 * @param offset: 쓰기 시작 오프셋
 * @param p_data: 쓸 데이터
 * @param data_len: 쓸 데이터 길이
 * @return 0: 성공
 */
int32_t at45db_write(uint32_t offset, uint8_t *p_data, uint32_t data_len)
{
  const uint32_t page_size = at45db_inst.chip_info.current_page_size;
  uint32_t page_quot;
  uint32_t page_rem;
  uint32_t remain;
  uint32_t written = 0;
  uint32_t first_write;
  uint8_t buff[AT45DB_MAX_PAGE_SIZE];

  /* 매개변수 유효성 검사 */
  if (p_data == NULL || data_len == 0)
  {
    return -1;
  }

  /* 초기화 상태 체크 */
  if (!at45db_inst.chip_info.is_initialized)
  {
    DEBUG_PRINTF("Error: AT45DB not initialized\r\n");
    return -1;
  }

  /* 경계 체크 */
  if (offset >= at45db_inst.chip_info.total_capacity_bytes ||
      (offset + data_len) > at45db_inst.chip_info.total_capacity_bytes)
  {
    DEBUG_PRINTF("Error: Write offset out of range (offset=0x%08lX, len=%lu)\r\n",
                 offset, data_len);
    return -1;
  }

  page_quot = offset / page_size;
  page_rem = offset % page_size;
  remain = page_size - page_rem;

  OS_PEND_SEM(at45db_inst.sem, osWaitForever);

  // 시작 위치가 페이지 정렬되지 않은 경우
  if (page_rem > 0)
  {
    at45db_read_page( page_quot, buff);
    first_write = (data_len < remain) ? data_len : remain;
    memcpy(&buff[page_rem], &p_data[0], first_write);
    at45db_write_page( page_quot, buff);
    page_quot++;
    written += first_write;
  }

  // 전체 페이지 쓰기
  while ((data_len - written) >= page_size)
  {
    memcpy(buff, &p_data[written], page_size);
    at45db_write_page( page_quot, buff);
    page_quot++;
    written += page_size;
    osDelay(1);
  }

  // 마지막 페이지 부분 쓰기 (남은 데이터가 있는 경우)
  if (data_len > written)
  {
    at45db_read_page( page_quot, buff);
    memcpy(buff, &p_data[written], data_len - written);
    at45db_write_page( page_quot, buff);
  }

  OS_POST_SEM(at45db_inst.sem);
  return 0;
}

//=============================================================================
// 유틸리티 함수 (정보 및 파싱)
//=============================================================================

/**
 * @brief 밀도 코드로 디바이스 정보 찾기
 * @param density_code: 디바이스 ID에서 얻은 AT45DB 밀도 코드
 * @param device_info: 디바이스 정보를 저장할 포인터
 * @return AT45DB_OK: 찾음, AT45DB_UNSUPPORTED: 지원되지 않음
 */
at45db_result_t at45db_find_device_info(uint8_t density_code, at45db_device_info_t *device_info)
{
  int i;

  for (i = 0; i < AT45DB_DEVICE_COUNT; i++) {
    if (at45db_device_table[i].density_code == density_code) {
      memcpy(device_info, &at45db_device_table[i], sizeof(at45db_device_info_t));
      return AT45DB_OK;
    }
  }
  return AT45DB_UNSUPPORTED;
}

/**
 * @brief 디바이스 ID 응답에서 칩 정보 파싱
 * @param chip_info: 디바이스에서 받은 원시 칩 정보 바이트
 * @param info: 파싱된 칩 정보를 저장할 포인터
 * @return AT45DB_OK: 성공, 오류 코드: 실패
 * @note 이 함수는 DEBUG_PRINTF를 통해 상세한 칩 정보를 표시합니다
 */
at45db_result_t at45db_parse_chip_info(uint8_t *chip_info, at45db_chip_info_t *info)
{
  const char *manufacturer = "Unknown";
  const char *family = "Unknown";
  const char *subcode = "Standard (00h)";
  const char *variant = "00000";
  const char *revision_str = "Unknown";
  uint8_t id0 = chip_info[0];      // 제조사 ID
  uint8_t id1 = chip_info[1];      // 디바이스 ID Byte 1
  uint8_t id2 = chip_info[2];      // 디바이스 ID Byte 2
  uint8_t edi_len = chip_info[3];  // 확장 정보 길이
  uint8_t edi_byte1 = chip_info[4];
  uint8_t density_code;
  uint8_t rev_code;
  at45db_result_t result;

  (void)manufacturer;
  (void)family;
  (void)subcode;
  (void)variant;
  (void)revision_str;
  (void)id2;
  (void)edi_len;

  // 제조사 ID 확인
  if (id0 != 0x1F) {
    DEBUG_PRINTF("Error: Invalid Manufacturer ID: 0x%02X\r\n", id0);
    return AT45DB_ERROR;
  }
  manufacturer = "Atmel / Renesas";

  // 패밀리 코드 확인
  if ((id1 >> 5) != 0x01) {
    DEBUG_PRINTF("Error: Invalid Family Code: 0x%02X\r\n", id1 >> 5);
    return AT45DB_ERROR;
  }
  family = "AT45DBxxx (DataFlash)";

  density_code = id1 & 0x1F;

  // 디바이스 정보 조회
  result = at45db_find_device_info(density_code, &info->device_info);
  if (result != AT45DB_OK) {
    DEBUG_PRINTF("Error: Unsupported density code: 0x%02X\r\n", density_code);
    return AT45DB_UNSUPPORTED;
  }

  // Display device information
  DEBUG_PRINTF("Flash Memory\r\n");
  DEBUG_PRINTF("  Manufacturer ID : 0x%02X (%s)\r\n", id0, manufacturer);
  DEBUG_PRINTF("  Device ID Byte 1: 0x%02X\r\n", id1);
  DEBUG_PRINTF("    - Family Code : 0x%02X (%s)\r\n", id1 >> 5, family);
  DEBUG_PRINTF("    - Density Code: 0x%02X (%s)\r\n", density_code, info->device_info.device_name);
  DEBUG_PRINTF("  Capacity: %lu bits (%lu KB)\r\n", info->device_info.capacity_bits, info->device_info.capacity_bits / 8192);
  DEBUG_PRINTF("  Total Pages: %lu\r\n", info->device_info.total_pages);
  DEBUG_PRINTF("  Page Size: %u bytes (standard) / %u bytes (binary)\r\n", 
               info->device_info.page_size_standard, info->device_info.page_size_binary);
  DEBUG_PRINTF("  Device ID Byte 2: 0x%02X\r\n", id2);
  DEBUG_PRINTF("    - Sub Code     : 0x%02X (%s)\r\n", id2 >> 3, subcode);
  DEBUG_PRINTF("    - Variant Code : 0x%02X (%s)\r\n", id2 & 0x07, variant);
  DEBUG_PRINTF("  Extended Info Len: 0x%02X (EDI Byte Count)\r\n", edi_len);
  DEBUG_PRINTF("  EDI Byte[0]      : 0x%02X\r\n", edi_byte1);


  rev_code = edi_byte1 & 0x1F;
  if (rev_code == 0)
    revision_str = "00000 (Initial Version)";
  else
    revision_str = "Unknown Version";

  DEBUG_PRINTF("    - Device Revision : %02X (%s)\r\n", rev_code, revision_str);
  
  return AT45DB_OK;
}

/**
 * @brief 검증 및 재시도 기능이 있는 안전한 쓰기
 * @param offset: 쓰기 시작 오프셋
 * @param p_data: 쓸 데이터
 * @param data_len: 쓸 데이터 길이
 * @return 0: 성공, -1: 잘못된 매개변수, -2: 검증 실패
 */
int32_t at45db_write_safe(uint32_t offset, uint8_t *p_data, uint32_t data_len)
{
  // 검증 및 재시도를 포함한 256바이트 청크 단위 쓰기
  const uint32_t chunk_size = 256;
  const uint8_t max_retry = 3; // 최대 3회 시도
  uint32_t written = 0;
  uint32_t current_chunk_size;
  uint8_t verify_buff[AT45DB_MAX_PAGE_SIZE];
  uint8_t retry_count;

  DEBUG_PRINTF("[at45db_write_safe] start\r\n");

  // 매개변수 유효성 검사
  if (p_data == NULL || data_len == 0)
  {
    DEBUG_PRINTF("[at45db_write_safe] ERROR: Invalid parameters\r\n");
    return -1;
  }

  // 검증을 포함한 청크 단위 데이터 쓰기
  while (written < data_len)
  {
    // 현재 청크 크기 계산 (마지막 청크 처리)
    current_chunk_size = (data_len - written) > chunk_size ? chunk_size : (data_len - written);

    retry_count = 0;

    // 현재 청크 재시도 루프
    while (retry_count < max_retry)
    {
      // 청크 쓰기
      at45db_write(offset + written, &p_data[written], current_chunk_size);

      // 읽기 및 검증
      at45db_read(offset + written, verify_buff, current_chunk_size);

      // 쓴 데이터와 읽은 데이터 비교
      if (memcmp(&p_data[written], verify_buff, current_chunk_size) == 0)
      {
        // 검증 성공
        break;
      }
      else
      {
        // 검증 실패, 재시도
        DEBUG_PRINTF("[at45db_write_safe] Verify failed: offset=0x%08lX, size=%lu, retry=%d\r\n",
                     offset + written, current_chunk_size, retry_count);
        retry_count++;
      }
    }

    // 모든 재시도 후 청크 쓰기 실패 확인
    if (retry_count >= max_retry)
    {
      DEBUG_PRINTF("[at45db_write_safe] CRITICAL: Write failed after %d retries at offset=0x%08lX\r\n",
                   max_retry, offset + written);
      return -2; // 오프셋에서 검증 실패: (offset + written)
    }

    written += current_chunk_size;
  }

  DEBUG_PRINTF("[at45db_write_safe] finish\r\n");
  return 0; // 성공
}




int at45db_lfs_prog( uint32_t block,uint32_t off,const uint8_t *buffer,uint32_t size)
{
  uint32_t chunk;
  uint32_t flash_addr;
  uint32_t page;
  uint32_t page_offset;
  uint32_t page_size;

    page_size = at45db_inst.chip_info.current_page_size;
    // block은 littlefs 블록 번호 (블록 크기 = 2048바이트)
    flash_addr = block * 2048 + off;
    while (size > 0) {

        // 1) 현재 페이지 계산
        page = flash_addr / page_size;
        page_offset = flash_addr % page_size;

        // 이번에 쓸 수 있는 최대 길이 (페이지 경계 안에서)
        chunk = page_size - page_offset;
        if (chunk > size) chunk = size;

        //-----------------------
        // 2) Page → Buffer 복사
        //-----------------------
        at45db_memory_to_buffer(AT45DB_BUFFER1, page);
  at45db_wait_ready();
        //-----------------------
        // 3) Buffer 내부에 원하는 오프셋부터 chunk 만큼 덮어쓰기
        //-----------------------
        at45db_write_buffer(AT45DB_BUFFER1, page_offset, (const char *)buffer, chunk);
          osDelay(1);
        //-----------------------
        // 4) Buffer → Page Program (Page 전체 Commit)
        //-----------------------
        at45db_buffer_to_memory(AT45DB_BUFFER1, page);
        at45db_wait_ready();
        // 다음 반복을 위한 포인터 이동
        flash_addr += chunk;
        buffer     += chunk;
        size        -= chunk;
    }

    return 0;
}

int at45db_lfs_read(uint32_t block, uint32_t off, uint8_t *buffer, uint32_t size)
{
  uint32_t flash_addr;
  uint32_t page_size;

  page_size = at45db_inst.chip_info.current_page_size;

  // block은 littlefs 블록 번호 (블록 크기 = 2048바이트)
  flash_addr = block * 2048 + off;

  while (size > 0)
  {
    // 현재 페이지 계산
    uint32_t page = flash_addr / page_size;
    uint32_t page_offset = flash_addr % page_size;

    // 페이지 경계 내에서 읽을 수 있는 최대 길이
    uint32_t chunk = page_size - page_offset;
    if (chunk > size)
      chunk = size;

    //-----------------------------------
    // 1) Buffer ← Page 복사
    //-----------------------------------
    // DataFlash: Page → Buffer Copy
    at45db_memory_to_buffer(AT45DB_BUFFER1, page);
    at45db_wait_ready();
    //-----------------------------------
    // 2) Buffer 내부에서 원하는 오프셋부터 chunk 만큼 읽기
    //-----------------------------------
    at45db_read_buffer(AT45DB_BUFFER1, page_offset, buffer, chunk);

    // 다음을 위한 업데이트
    flash_addr += chunk;
    buffer += chunk;
    size -= chunk;
  }

  return 0;
}

/**
 * @brief LittleFS 블록 지우기 (0xFF로 채움)
 * @param block: 블록 번호
 * @return 0: 성공, 음수: 실패
 * @note AT45DB는 하드웨어 블록 erase가 없으므로 0xFF로 채워서 erase 효과 구현
 *       블록 크기 = 2048 바이트 = 8 페이지 (페이지 크기 256바이트 기준)
 */
int at45db_lfs_erase(uint32_t block)
{
#if 0 
  uint8_t erase_buffer[256];
  uint32_t i;
  uint32_t page;
  uint32_t page_size;
  uint32_t pages_per_block;

  page_size = at45db_inst.chip_info.current_page_size;
  pages_per_block = 2048 / page_size;  // 블록 크기 2048바이트 / 페이지 크기

  // 0xFF로 채워진 버퍼 생성
  memset(erase_buffer, 0xFF, sizeof(erase_buffer));

  // 블록 내의 모든 페이지를 0xFF로 채움
  for (i = 0; i < pages_per_block; i++)
  {
    page = block * pages_per_block + i;

    // Buffer에 0xFF 쓰기
    at45db_write_buffer(AT45DB_BUFFER1, 0, (const char *)erase_buffer, page_size);
    at45db_wait_ready();

    // Buffer를 페이지에 프로그램
    at45db_buffer_to_memory(AT45DB_BUFFER1, page);
    at45db_wait_ready();
  }
#endif
  return 0;
}

/**
 * @brief RAM처럼 연속 쓰기 가능한 고급 쓰기 함수
 * @param offset: 쓰기 시작 오프셋 (절대 주소)
 * @param p_data: 쓸 데이터
 * @param data_len: 쓸 데이터 길이
 * @return 0: 성공, -1: 실패
 * @note at45db_lfs_prog와 동일한 방식으로 페이지 경계를 넘어 연속 쓰기 가능
 *       내부 버퍼를 활용하여 Read-Modify-Write 방식으로 동작
 */
int32_t at45db_write_adv(uint32_t offset, uint8_t *p_data, uint32_t data_len)
{
  uint32_t chunk;
  uint32_t flash_addr;
  uint32_t page;
  uint32_t page_offset;
  uint32_t page_size;

  /* 매개변수 유효성 검사 */
  if (p_data == NULL || data_len == 0)
  {
    return -1;
  }

  /* 초기화 상태 체크 */
  if (!at45db_inst.chip_info.is_initialized)
  {
    DEBUG_PRINTF("Error: AT45DB not initialized\r\n");
    return -1;
  }

  /* 경계 체크 */
  if (offset >= at45db_inst.chip_info.total_capacity_bytes ||
      (offset + data_len) > at45db_inst.chip_info.total_capacity_bytes)
  {
    DEBUG_PRINTF("Error: Write offset out of range (offset=0x%08lX, len=%lu)\r\n",
                 offset, data_len);
    return -1;
  }

  page_size = at45db_inst.chip_info.current_page_size;
  flash_addr = offset;

  OS_PEND_SEM(at45db_inst.sem, osWaitForever);

  while (data_len > 0)
  {
    // 1) 현재 페이지 계산
    page = flash_addr / page_size;
    page_offset = flash_addr % page_size;

    // 이번에 쓸 수 있는 최대 길이 (페이지 경계 안에서)
    chunk = page_size - page_offset;
    if (chunk > data_len)
      chunk = data_len;

    //-----------------------
    // 2) Page → Buffer 복사
    //-----------------------
    at45db_memory_to_buffer(AT45DB_BUFFER1, page);
    at45db_wait_ready();

    //-----------------------
    // 3) Buffer 내부에 원하는 오프셋부터 chunk 만큼 덮어쓰기
    //-----------------------
    at45db_write_buffer(AT45DB_BUFFER1, page_offset, (const char *)p_data, chunk);
    //osDelay(1);

    //-----------------------
    // 4) Buffer → Page Program (Page 전체 Commit)
    //-----------------------
    at45db_buffer_to_memory(AT45DB_BUFFER1, page);
    at45db_wait_ready();

    // 다음 반복을 위한 포인터 이동
    flash_addr += chunk;
    p_data += chunk;
    data_len -= chunk;
  }

  OS_POST_SEM(at45db_inst.sem);

  return 0;
}


/**
 * @brief RAM처럼 연속 읽기 가능한 고급 읽기 함수
 * @param address: 읽기 시작 주소 (절대 주소)
 * @param buffer: 읽은 데이터를 저장할 버퍼
 * @param size: 읽을 데이터 길이
 * @return 0: 성공, -1: 실패
 * @note at45db_lfs_read와 동일한 방식으로 페이지 경계를 넘어 연속 읽기 가능
 *       내부 버퍼를 활용하여 페이지별로 읽기
 */
int at45db_read_adv(uint32_t address, uint8_t *buffer, uint32_t size)
{
  uint32_t chunk;
  uint32_t flash_addr;
  uint32_t page;
  uint32_t page_offset;
  uint32_t page_size;

  /* 매개변수 유효성 검사 */
  if (buffer == NULL || size == 0)
  {
    return -1;
  }

  /* 초기화 상태 체크 */
  if (!at45db_inst.chip_info.is_initialized)
  {
    DEBUG_PRINTF("Error: AT45DB not initialized\r\n");
    return -1;
  }

  page_size = at45db_inst.chip_info.current_page_size;
  flash_addr = address;

  /* 경계 체크 */
  if (flash_addr >= at45db_inst.chip_info.total_capacity_bytes ||
      (flash_addr + size) > at45db_inst.chip_info.total_capacity_bytes)
  {
    DEBUG_PRINTF("Error: Read offset out of range (offset=0x%08lX, len=%lu)\r\n",
                 flash_addr, size);
    return -1;
  }

  OS_PEND_SEM(at45db_inst.sem, osWaitForever);

  while (size > 0)
  {
    // 현재 페이지 계산
    page = flash_addr / page_size;
    page_offset = flash_addr % page_size;

    // 페이지 경계 내에서 읽을 수 있는 최대 길이
    chunk = page_size - page_offset;
    if (chunk > size)
      chunk = size;

    //-----------------------------------
    // 1) Buffer ← Page 복사
    //-----------------------------------
    // DataFlash: Page → Buffer Copy
    at45db_memory_to_buffer(AT45DB_BUFFER1, page);
    at45db_wait_ready();

    //-----------------------------------
    // 2) Buffer 내부에서 원하는 오프셋부터 chunk 만큼 읽기
    //-----------------------------------
    at45db_read_buffer(AT45DB_BUFFER1, page_offset, (char *)buffer, chunk);

    // 다음을 위한 업데이트
    flash_addr += chunk;
    buffer += chunk;
    size -= chunk;
  }

  OS_POST_SEM(at45db_inst.sem);

  return 0;
}