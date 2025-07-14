

#include "at45db.h"

#include <string.h>

#include "bsp_delay.h"
#include "bsp_do.h"
#include "bsp_spi.h"
#include "os_user_def.h"
#include "system_err.h"

// AT45DB Command Definitions
#define AT45DB_CMD_DEVICE_ID                0x9F    // Read Device ID
#define AT45DB_CMD_STATUS_REGISTER         0xD7    // Read Status Register

// Buffer Write Commands
#define AT45DB_CMD_BUFFER1_WRITE           0x84    // Buffer 1 Write
#define AT45DB_CMD_BUFFER2_WRITE           0x87    // Buffer 2 Write

// Buffer Read Commands  
#define AT45DB_CMD_BUFFER1_READ            0xD4    // Buffer 1 Read
#define AT45DB_CMD_BUFFER2_READ            0xD6    // Buffer 2 Read

// Buffer to Memory Commands
#define AT45DB_CMD_BUFFER1_TO_MEMORY       0x83    // Buffer 1 to Memory Page Program
#define AT45DB_CMD_BUFFER2_TO_MEMORY       0x86    // Buffer 2 to Memory Page Program

// Memory to Buffer Commands
#define AT45DB_CMD_MEMORY_TO_BUFFER1       0x53    // Memory Page to Buffer 1 Transfer
#define AT45DB_CMD_MEMORY_TO_BUFFER2       0x55    // Memory Page to Buffer 2 Transfer

// Configuration Commands
#define AT45DB_CMD_SECTOR_PROTECT_ENABLE   {0x3d, 0x2a, 0x7f, 0xa9}  // Sector Protection Enable
#define AT45DB_CMD_BINARY_PAGE_SIZE        {0x3d, 0x2a, 0x80, 0xa6}  // Configure Power of 2 Page Size

// Buffer Selection
#define AT45DB_BUFFER1                     0
#define AT45DB_BUFFER2                     1

// Status Register Bit Definitions
#define AT45DB_STATUS_READY_BUSY_BIT       0x80    // Bit 7: Ready/Busy status (1=Ready, 0=Busy)
#define AT45DB_STATUS_COMPARE_BIT          0x40    // Bit 6: Compare result
#define AT45DB_STATUS_DENSITY_MASK         0x3C    // Bits 5-2: Device density
#define AT45DB_STATUS_PROTECT_BIT          0x02    // Bit 1: Sector protection
#define AT45DB_STATUS_PAGE_SIZE_BIT        0x01    // Bit 0: Page size (1=Binary, 0=Standard)

// Address Bit Masks
#define AT45DB_ADDR_HIGH_MASK              0x11    // High address bits mask for buffer addressing

// Timeout and Retry Constants
#define AT45DB_BUSY_TIMEOUT_MS             100     // Maximum busy wait timeout
#define AT45DB_BUSY_POLL_DELAY_US          100     // Delay between busy status polls

// Obsolete defines for backward compatibility
#define DEVICE_ID                          AT45DB_CMD_DEVICE_ID
#define STATUS_REGISTER                    AT45DB_CMD_STATUS_REGISTER
#define BYTE_DUMMY 				0x00 			// Dummy Byte

typedef struct ad45db_instance_s
{
  at45db_chip_info_t chip_info;
  int spi_num;
  int cs_do_num;
  void *sem;
  bool opened;
} at45db_instance_t;



int32_t at45db_write( uint32_t offset, uint8_t *pData, uint32_t dataLen);
void at45db_read( uint32_t offset, uint8_t *pBuff, uint32_t buffSize,
                 uint32_t readLen) ;
void at45db_read_page( uint32_t ReadAddr, uint8_t *readbuff);
void at45db_write_page( uint32_t WriteAddr, uint8_t *writebuff);
static void at45db_delay(uint32_t usec);
static void at45db_write_buffer( uint8_t buffer_choice, uint32_t address, const char *string, uint32_t buf_len);
static void at45db_page_write_cmd( uint32_t page);
static void at45db_buffer_to_memory( uint8_t buffer_choice, uint32_t page);
static void at45db_reg_read( uint8_t cmd, uint8_t *info, uint8_t len);
static void at45db_reg_write( uint8_t *cmd);
static void at45db_wait_ready(void);
static void at45db_memory_to_buffer( uint8_t buffer_choice, uint32_t page);
static void at45db_read_buffer( uint8_t buffer_choice, uint32_t address, char *string, uint16_t buf_len);
at45db_result_t at45db_find_device_info(uint8_t density_code, at45db_device_info_t *device_info);
at45db_result_t at45db_parse_chip_info(uint8_t *chip_info, at45db_chip_info_t *info);

at45db_result_t at45db_initialize(void);

static at45db_instance_t at45db_inst;

void at45db_init(void)
{
  if (at45db_inst.opened)
  {
    return ;
  }

  at45db_inst.opened = true;
  at45db_inst.spi_num = BSP_SPI_1;               
  at45db_inst.cs_do_num  = BSP_DO_FLASH_CS;

  OS_CREATE_BINARY_SEM(at45db_inst.sem);
  bsp_spi_init(at45db_inst.spi_num);

  if (at45db_initialize() != AT45DB_OK)
  {
    DEBUG_PRINTF("Error: AT45DB initialization failed\r\n");
    at45db_inst.opened = false;
    return ;
  }

  at45db_inst.opened = true;
}




static const at45db_device_info_t at45db_device_table[] = {
    {0x00, 1024*1024, 512, 264, 256, "AT45DB011"},
    {0x01, 2048*1024, 1024, 264, 256, "AT45DB021"},
    {0x02, 4096*1024, 2048, 264, 256, "AT45DB041"},
    {0x03, 8192*1024, 4096, 264, 256, "AT45DB081"},
    {0x04, 16384*1024, 4096, 528, 512, "AT45DB161"},
    {0x05, 32768*1024, 8192, 528, 512, "AT45DB321"},
    {0x08, 65536*1024, 8192, 1056, 1024, "AT45DB641"},
    {0x09, 131072*1024, 16384, 1056, 1024, "AT45DB128"},
    {0x0A, 262144*1024, 16384, 2112, 2048, "AT45DB256"}
};

#define AT45DB_DEVICE_COUNT (sizeof(at45db_device_table) / sizeof(at45db_device_table[0]))


static void at45db_delay(uint32_t usec)
{
  bsp_us_delay(usec);
}


/**
 * @brief Write data to AT45DB internal buffer
 * @param drv: Driver instance
 * @param buffer_choice: Buffer selection (AT45DB_BUFFER1 or AT45DB_BUFFER2)  
 * @param address: Buffer address offset
 * @param string: Data to write
 * @param buf_len: Length of data to write
 */
static void at45db_write_buffer( uint8_t buffer_choice, uint32_t address, const char *string, uint32_t buf_len)
{


  uint8_t szCmd[4];

  bsp_spi_pend_sem(at45db_inst.spi_num);
  


  bsp_do_low(at45db_inst.cs_do_num);

  if(buffer_choice == AT45DB_BUFFER2)
  {
    szCmd[0] = AT45DB_CMD_BUFFER2_WRITE;
  }
  else
  {
    szCmd[0] = AT45DB_CMD_BUFFER1_WRITE;
  }

  szCmd[1] = BYTE_DUMMY;
  szCmd[2] = (uint8_t)((address>>8) & AT45DB_ADDR_HIGH_MASK);
  szCmd[3] = (uint8_t)address;

  bsp_spi_send_bytes(at45db_inst.spi_num,szCmd,4);

  bsp_spi_send_bytes(at45db_inst.spi_num,(uint8_t *)string,buf_len);

  bsp_do_high(at45db_inst.cs_do_num);

  bsp_spi_post_sem(at45db_inst.spi_num);
  
}

static void at45db_page_write_cmd( uint32_t page)
{


#ifdef	AT45DB321/*PAGE SIZE == 512 */
  FlashSend_Byte((uint8_t)(page >>7));
  FlashSend_Byte((uint8_t)(page <<1));
#else

  bsp_spi_send_byte(at45db_inst.spi_num,(uint8_t)(page >> 8));
  bsp_spi_send_byte(at45db_inst.spi_num,(uint8_t)(page));

#endif
}

/**
 * @brief Transfer buffer contents to memory page
 * @param drv: Driver instance
 * @param buffer_choice: Buffer selection (AT45DB_BUFFER1 or AT45DB_BUFFER2)
 * @param page: Target page number
 */
static void at45db_buffer_to_memory( uint8_t buffer_choice, uint32_t page)
{

  
  
  bsp_spi_pend_sem(at45db_inst.spi_num);

  bsp_do_low(at45db_inst.cs_do_num);

    if(buffer_choice == AT45DB_BUFFER2)
    {
      bsp_spi_send_byte(at45db_inst.spi_num, AT45DB_CMD_BUFFER2_TO_MEMORY);
    }
    else
    {
      bsp_spi_send_byte(at45db_inst.spi_num, AT45DB_CMD_BUFFER1_TO_MEMORY);
    }

    at45db_page_write_cmd( page);

    bsp_spi_send_byte(at45db_inst.spi_num,BYTE_DUMMY);

  bsp_do_high(at45db_inst.cs_do_num);

  bsp_spi_post_sem(at45db_inst.spi_num);
}


static void at45db_reg_read( uint8_t cmd, uint8_t *info, uint8_t len)
{



  memset(info,  0, len);
  bsp_spi_pend_sem(at45db_inst.spi_num);

  bsp_do_low(at45db_inst.cs_do_num);
  bsp_spi_send_byte(at45db_inst.spi_num,cmd);

  bsp_spi_read_bytes(at45db_inst.spi_num,info,len);


  bsp_do_high(at45db_inst.cs_do_num);

  bsp_spi_post_sem(at45db_inst.spi_num);
}


static void at45db_reg_write( uint8_t *cmd)
{

  bsp_spi_pend_sem(at45db_inst.spi_num);

  bsp_do_low(at45db_inst.cs_do_num);
  bsp_spi_send_bytes(at45db_inst.spi_num,cmd,4);
;

  bsp_do_high(at45db_inst.cs_do_num);

  bsp_spi_post_sem(at45db_inst.spi_num);
  
}


/**
 * @brief Wait for AT45DB to become ready (not busy)
 * @param drv: Driver instance
 * @note Polls the status register until ready bit is set or timeout occurs
 */
static void at45db_wait_ready(void)
{
  uint8_t status_reg;
  uint16_t timeout_count = 0;

  while(1)
  {
    at45db_reg_read( AT45DB_CMD_STATUS_REGISTER, &status_reg, 1);

    // Check if device is ready (bit 7 = 1 means ready)
    if(status_reg & AT45DB_STATUS_READY_BUSY_BIT) break;

    at45db_delay(AT45DB_BUSY_POLL_DELAY_US);
    if(++timeout_count > AT45DB_BUSY_TIMEOUT_MS) {
      DEBUG_PRINTF("Warning: AT45DB busy timeout\r\n");
      break;
    }
  }
}


/**
  * @brief  : Do not use osDelay() when called from USB interrupt
  * @param  :
  * @retval :
  */
// WriteAddr : Page Address (512)
void at45db_write_page(uint32_t WriteAddr, uint8_t *writebuff)
{

  uint16_t pageSize = at45db_inst.chip_info.current_page_size;
  uint8_t readCnt = 1;

  // For devices with smaller page sizes, may need multiple operations
  if (pageSize < 512) {
    readCnt = 512 / pageSize;
  }

  for(uint16_t i = 0; i < readCnt; i++)
  {
    at45db_write_buffer( AT45DB_BUFFER1, 0, (const char *) writebuff, pageSize);
    bsp_us_delay(10);
    at45db_buffer_to_memory( AT45DB_BUFFER1, WriteAddr * readCnt + i);
    at45db_wait_ready();
    writebuff += pageSize;
  }
}

/**
 * @brief Transfer memory page to buffer
 * @param drv: Driver instance  
 * @param buffer_choice: Buffer selection (AT45DB_BUFFER1 or AT45DB_BUFFER2)
 * @param page: Source page number
 */
static void at45db_memory_to_buffer( uint8_t buffer_choice, uint32_t page)
{
      

  bsp_spi_pend_sem(at45db_inst.spi_num);

  bsp_do_low(at45db_inst.cs_do_num);
  
    if(buffer_choice == AT45DB_BUFFER2)
    {
      bsp_spi_send_byte(at45db_inst.spi_num, AT45DB_CMD_MEMORY_TO_BUFFER2);
    }
    else
    {
      bsp_spi_send_byte(at45db_inst.spi_num, AT45DB_CMD_MEMORY_TO_BUFFER1);
    }

  at45db_page_write_cmd( page);

  bsp_spi_send_byte(at45db_inst.spi_num,BYTE_DUMMY);
  
  bsp_do_high(at45db_inst.cs_do_num);

  bsp_spi_post_sem(at45db_inst.spi_num);
}
/**
 * @brief Read data from AT45DB internal buffer
 * @param drv: Driver instance
 * @param buffer_choice: Buffer selection (AT45DB_BUFFER1 or AT45DB_BUFFER2)
 * @param address: Buffer address offset  
 * @param string: Buffer to store read data
 * @param buf_len: Length of data to read
 */
static void at45db_read_buffer( uint8_t buffer_choice, uint32_t address, char *string, uint16_t buf_len)
{
  char szCmd[5];

  bsp_spi_pend_sem(at45db_inst.spi_num);

  bsp_do_low(at45db_inst.cs_do_num);

    if(buffer_choice == AT45DB_BUFFER2)
    {
      szCmd[0] = AT45DB_CMD_BUFFER2_READ;
    }
    else
    {
      szCmd[0] = AT45DB_CMD_BUFFER1_READ;
    }
    szCmd[1] = BYTE_DUMMY;
    szCmd[2] = (uint8_t)((address>>8) & AT45DB_ADDR_HIGH_MASK);
    szCmd[3] = (uint8_t)address;
    szCmd[4] = BYTE_DUMMY;


  bsp_spi_send_bytes(at45db_inst.spi_num,(uint8_t *)szCmd,5);
  bsp_spi_read_bytes(at45db_inst.spi_num,(uint8_t *)string,buf_len);
  
  bsp_do_high(at45db_inst.cs_do_num);
  bsp_spi_post_sem(at45db_inst.spi_num);


}
void at45db_read_page(uint32_t ReadAddr,uint8_t *readbuff)
{
  uint16_t pageSize = at45db_inst.chip_info.current_page_size;
  uint8_t readCnt = 1;

  // For devices with smaller page sizes, may need multiple operations
  if (pageSize < 512) {
    readCnt = 512 / pageSize;
  }
  
  for(uint16_t i = 0; i < readCnt; i++)
  {
    at45db_memory_to_buffer( AT45DB_BUFFER1, (uint32_t) ReadAddr * readCnt + i);
    at45db_wait_ready();
    at45db_read_buffer( AT45DB_BUFFER1, 0, (char*) readbuff, pageSize);
    at45db_wait_ready();
    readbuff += pageSize;
  }
}

const char *_eicpart=NULL;//"AT45DB641E-SHN2B"; /* IC Part Number */
at45db_result_t at45db_initialize(void)
{
  static const uint8_t protect_enable[4] = AT45DB_CMD_SECTOR_PROTECT_ENABLE;
  static const uint8_t page_binary_mode[4] = AT45DB_CMD_BINARY_PAGE_SIZE;

    uint8_t reg=0;
    uint8_t chip_info[5]={0,0,0,0,0};

    
  // Device ID Information
  // Byte[0] Manufacturer ID  0x1F
  // Byte[1] Device ID        0x28
  // Byte[2] Device ID        0x00
  // Byte[3] Extended Device Information String Leghth 0x01
  // Byte[4] EDI Byte 1                                0x00 
    

  
  at45db_reg_read( AT45DB_CMD_DEVICE_ID, chip_info, 5);

  at45db_result_t result = at45db_parse_chip_info(chip_info, &at45db_inst.chip_info);
  if (result != AT45DB_OK) {
    DEBUG_PRINTF("Error: Failed to parse chip info\r\n");
    return result;
  }

  osDelay(1);

  at45db_reg_read( AT45DB_CMD_STATUS_REGISTER, &reg, 1);
  
  // Check if binary page size mode is enabled
  if ((reg & AT45DB_STATUS_PAGE_SIZE_BIT) == 0) {
    DEBUG_PRINTF("Switching to binary page size mode...\r\n");
    at45db_reg_write( (uint8_t *)protect_enable);
    at45db_wait_ready();
    at45db_reg_write( (uint8_t *)page_binary_mode);
    at45db_wait_ready();
    
    // Verify the switch
    at45db_reg_read( AT45DB_CMD_STATUS_REGISTER, &reg, 1);
    if ((reg & AT45DB_STATUS_PAGE_SIZE_BIT) == 0) {
      DEBUG_PRINTF("Error: Failed to switch to binary page size mode\r\n");
      return AT45DB_ERROR;
    }
  }

  // Set current page size and other info
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

at45db_result_t at45db_get_chip_info( at45db_chip_info_t *info)
{

  if (!at45db_inst.chip_info.is_initialized)
  {
    return AT45DB_INIT_FAILED;
  }
  
  memcpy(info, &at45db_inst.chip_info, sizeof(at45db_chip_info_t));
  return AT45DB_OK;
}

#define FLASH_PAGE_SIZE_BASE 512

/**
 * @brief Flash read like RAM access
 */
void at45db_read( uint32_t offset, uint8_t *p_buff, uint32_t buff_size,
                 uint32_t read_len)
{

  uint32_t page_size = at45db_inst.chip_info.current_page_size;
  uint8_t buff[2112]; // Maximum possible page size for AT45DB256
  uint32_t page_quot = offset / page_size;
  uint32_t page_rem = offset % page_size;
  uint32_t remain = page_size - page_rem;
  uint32_t read_cnt = 0;

  // Check buffer size
  if (read_len > buff_size)
  {
    read_len = buff_size;
  }

  OS_PEND_SEM(at45db_inst.sem, osWaitForever);

  // Case 1: When start position is not page-aligned (unaligned)
  if (page_rem > 0)
  {
    at45db_read_page( page_quot, buff);
    uint32_t first_read = (read_len < remain) ? read_len : remain;
    memcpy(&p_buff[0], &buff[page_rem], first_read);
    page_quot++;
    read_cnt += first_read;
  }

  // Case 2: Read full pages
  while ((read_len - read_cnt) >= page_size)
  {
    at45db_read_page( page_quot, buff);
    memcpy(&p_buff[read_cnt], buff, page_size);
    page_quot++;
    read_cnt += page_size;
  }

  // Case 3: Read partial last page (if any remaining)
  if (read_len > read_cnt)
  {
    at45db_read_page( page_quot, buff);
    memcpy(&p_buff[read_cnt], buff, read_len - read_cnt);
  }

  OS_POST_SEM(at45db_inst.sem);
}


int32_t at45db_write( uint32_t offset, uint8_t *p_data, uint32_t data_len)
{

  uint32_t page_size = at45db_inst.chip_info.current_page_size;
  uint8_t buff[2112]; // Maximum possible page size for AT45DB256
  uint32_t page_quot = offset / page_size;
  uint32_t page_rem = offset % page_size;
  uint32_t remain = page_size - page_rem;
  uint32_t written = 0;

  OS_PEND_SEM(at45db_inst.sem, osWaitForever);

  // When start position is not page-aligned (unaligned)
  if (page_rem > 0)
  {
    at45db_read_page( page_quot, buff);
    uint32_t first_write = (data_len < remain) ? data_len : remain;
    memcpy(&buff[page_rem], &p_data[0], first_write);
    at45db_write_page( page_quot, buff);
    page_quot++;
    written += first_write;
  }

  // Write full pages
  while ((data_len - written) >= page_size)
  {
    memcpy(buff, &p_data[written], page_size);
    at45db_write_page( page_quot, buff);
    page_quot++;
    written += page_size;
  }

  // Write partial last page (if any remaining)
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
// Utility Functions (Information and Parsing)
//=============================================================================

/**
 * @brief Find device information by density code
 * @param density_code: AT45DB density code from device ID
 * @param device_info: Pointer to store device information
 * @return AT45DB_OK if found, AT45DB_UNSUPPORTED if not supported
 */
at45db_result_t at45db_find_device_info(uint8_t density_code, at45db_device_info_t *device_info)
{
  for (int i = 0; i < AT45DB_DEVICE_COUNT; i++) {
    if (at45db_device_table[i].density_code == density_code) {
      memcpy(device_info, &at45db_device_table[i], sizeof(at45db_device_info_t));
      return AT45DB_OK;
    }
  }
  return AT45DB_UNSUPPORTED;
}

/**
 * @brief Parse chip information from device ID response
 * @param chip_info: Raw chip info bytes from device
 * @param info: Pointer to store parsed chip information
 * @return AT45DB_OK if successful, error code otherwise
 * @note This function displays detailed chip information via DEBUG_PRINTF
 */
at45db_result_t at45db_parse_chip_info(uint8_t *chip_info, at45db_chip_info_t *info)
{
  const char *manufacturer = "Unknown";
  const char *family = "Unknown";
  const char *subcode = "Standard (00h)";
  const char *variant = "00000";
  const char *revision_str = "Unknown";
  
  
  (void)manufacturer;
  (void)family;
  (void)subcode;
  (void)variant;
  (void)revision_str;

  uint8_t id0 = chip_info[0];  // Manufacturer ID
  uint8_t id1 = chip_info[1];  // Device ID Byte 1
  uint8_t id2 = chip_info[2];  // Device ID Byte 2
  uint8_t edi_len = chip_info[3];
  uint8_t edi_byte1 = chip_info[4];

  (void)id2;
  (void)edi_len;
  
  // Validate manufacturer ID
  if (id0 != 0x1F) {
    DEBUG_PRINTF("Error: Invalid Manufacturer ID: 0x%02X\r\n", id0);
    return AT45DB_ERROR;
  }
  manufacturer = "Atmel / Renesas";

  // Validate family code
  if ((id1 >> 5) != 0x01) {
    DEBUG_PRINTF("Error: Invalid Family Code: 0x%02X\r\n", id1 >> 5);
    return AT45DB_ERROR;
  }
  family = "AT45DBxxx (DataFlash)";

  uint8_t density_code = id1 & 0x1F;
  
  // Device info lookup
  at45db_result_t result = at45db_find_device_info(density_code, &info->device_info);
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

  uint8_t rev_code = edi_byte1 & 0x1F;
  if (rev_code == 0)
    revision_str = "00000 (Initial Version)";
  else
    revision_str = "Unknown Version";

  DEBUG_PRINTF("    - Device Revision : %02X (%s)\r\n", rev_code, revision_str);
  
  return AT45DB_OK;
}