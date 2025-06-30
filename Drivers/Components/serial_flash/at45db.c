

#include "at45db.h"

#include <string.h>

#include "cmsis_os.h"
#include "driver_do.h"
#include "driver_flash_define.h"
#include "driver_spi.h"
#include "os_user_def.h"
#include "system_err.h"
#include "usDelay.h"


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

typedef struct ad45db_cfg_s
{
  driver_t *spi_io;
  driver_t *cs_io;
  at45db_chip_info_t chip_info;

} at45db_cfg_t;


// Forward declarations - Core driver functions
int32_t at45db_write(driver_t *drv, uint32_t offset, uint8_t *pData, uint32_t dataLen);
void at45db_read(driver_t *drv, uint32_t offset, uint8_t *pBuff, uint32_t buffSize,
                 uint32_t readLen) ;
void at45db_read_page(driver_t *drv, uint32_t ReadAddr, uint8_t *readbuff);
void at45db_write_page(driver_t *drv, uint32_t WriteAddr, uint8_t *writebuff);
at45db_result_t at45db_init(driver_t *drv);

// Forward declarations - Internal hardware functions
static void at45db_delay(uint32_t usec);
static void at45db_write_buffer(driver_t *drv, uint8_t buffer_choice, uint32_t address, const char *string, uint32_t buf_len);
static void at45db_page_write_cmd(driver_t *drv, uint32_t page);
static void at45db_buffer_to_memory(driver_t *drv, uint8_t buffer_choice, uint32_t page);
static void at45db_reg_read(driver_t *drv, uint8_t cmd, uint8_t *info, uint8_t len);
static void at45db_reg_write(driver_t *drv, uint8_t *cmd);
static void at45db_wait_ready(driver_t *drv);
static void at45db_memory_to_buffer(driver_t *drv, uint8_t buffer_choice, uint32_t page);
static void at45db_read_buffer(driver_t *drv, uint8_t buffer_choice, uint32_t address, char *string, uint16_t buf_len);

// Forward declarations - Utility functions
at45db_result_t at45db_find_device_info(uint8_t density_code, at45db_device_info_t *device_info);
at45db_result_t at45db_parse_chip_info(uint8_t *chip_info, at45db_chip_info_t *info);


const static flash_api_t at45db_api = {
      .read_page = at45db_read_page,
      .write_page = at45db_write_page,
      .write = at45db_write,
      .read = at45db_read
    };

static driver_t ad45db;
static at45db_cfg_t cfg;

driver_t *at45db_open(int32_t num)
{
  if (ad45db.opened)
  {
    return &ad45db;
  }

  ad45db.opened = true;
  ad45db.api = &at45db_api;
  ad45db.cfg = &cfg;
  ad45db.instance_id = num;
  ad45db.driver_type = eDRIVER_FLASH;
  ad45db.name = "at45db";

  cfg.spi_io = driver_spi_open(STM_SPI_1);                // Hardware configuration required for IC operation
  cfg.cs_io  = driver_do_open(DO_FLASH_CS, 0);              // Hardware configuration required for IC operation

  OS_CREATE_BINARY_SEM(ad45db.sem);

  if (at45db_init(&ad45db) != AT45DB_OK) {
    DEBUG_PRINTF("Error: AT45DB initialization failed\r\n");
    ad45db.opened = false;
    return NULL;
  }
  return &ad45db;
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
  usDelay(usec);
}


/**
 * @brief Write data to AT45DB internal buffer
 * @param drv: Driver instance
 * @param buffer_choice: Buffer selection (AT45DB_BUFFER1 or AT45DB_BUFFER2)  
 * @param address: Buffer address offset
 * @param string: Data to write
 * @param buf_len: Length of data to write
 */
static void at45db_write_buffer(driver_t *drv, uint8_t buffer_choice, uint32_t address, const char *string, uint32_t buf_len)
{
  at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;

  uint8_t szCmd[4];

  driverex_spi_pend_sem(cfg->spi_io);

  
  driver_do_low(cfg->cs_io);

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

  driverex_spi_send_bytes(cfg->spi_io,szCmd,4);

  driverex_spi_send_bytes(cfg->spi_io,(uint8_t *)string,buf_len);

  driver_do_high(cfg->cs_io);

  driverex_spi_post_sem(cfg->spi_io);
  
}

static void at45db_page_write_cmd(driver_t *drv, uint32_t page)
{
    at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;

#ifdef	AT45DB321/*PAGE SIZE == 512 */
  FlashSend_Byte((uint8_t)(page >>7));
  FlashSend_Byte((uint8_t)(page <<1));
#else

  driverex_spi_send_byte(cfg->spi_io,(uint8_t)(page >> 8));
  driverex_spi_send_byte(cfg->spi_io,(uint8_t)(page));

#endif
}

/**
 * @brief Transfer buffer contents to memory page
 * @param drv: Driver instance
 * @param buffer_choice: Buffer selection (AT45DB_BUFFER1 or AT45DB_BUFFER2)
 * @param page: Target page number
 */
static void at45db_buffer_to_memory(driver_t *drv, uint8_t buffer_choice, uint32_t page)
{
  at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;
  
  
  driverex_spi_pend_sem(cfg->spi_io);

  driver_do_low(cfg->cs_io);

    if(buffer_choice == AT45DB_BUFFER2)
    {
      driverex_spi_send_byte(cfg->spi_io, AT45DB_CMD_BUFFER2_TO_MEMORY);
    }
    else
    {
      driverex_spi_send_byte(cfg->spi_io, AT45DB_CMD_BUFFER1_TO_MEMORY);
    }

    at45db_page_write_cmd(drv, page);

    driverex_spi_send_byte(cfg->spi_io,BYTE_DUMMY);

  driver_do_high(cfg->cs_io);

  driverex_spi_post_sem(cfg->spi_io);
}


static void at45db_reg_read(driver_t *drv, uint8_t cmd, uint8_t *info, uint8_t len)
{
  at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;


  memset(info,  0, len);
  driverex_spi_pend_sem(cfg->spi_io);

  driver_do_low(cfg->cs_io);
  driverex_spi_send_byte(cfg->spi_io,cmd);

  driverex_spi_read_bytes(cfg->spi_io,info,len);


  driver_do_high(cfg->cs_io);

  driverex_spi_post_sem(cfg->spi_io);
}


static void at45db_reg_write(driver_t *drv, uint8_t *cmd)
{
  at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;



  driverex_spi_pend_sem(cfg->spi_io);

  driver_do_low(cfg->cs_io);
  driverex_spi_send_bytes(cfg->spi_io,cmd,4);
;

  driver_do_high(cfg->cs_io);

  driverex_spi_post_sem(cfg->spi_io);
  
}


/**
 * @brief Wait for AT45DB to become ready (not busy)
 * @param drv: Driver instance
 * @note Polls the status register until ready bit is set or timeout occurs
 */
static void at45db_wait_ready(driver_t *drv)
{
  uint8_t status_reg;
  uint16_t timeout_count = 0;

  while(1)
  {
    at45db_reg_read(drv, AT45DB_CMD_STATUS_REGISTER, &status_reg, 1);

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
void at45db_write_page(driver_t *drv,uint32_t WriteAddr, uint8_t *writebuff)
{
  at45db_cfg_t *cfg = (at45db_cfg_t*)drv->cfg;
  uint16_t pageSize = cfg->chip_info.current_page_size;
  uint8_t readCnt = 1;

  // For devices with smaller page sizes, may need multiple operations
  if (pageSize < 512) {
    readCnt = 512 / pageSize;
  }

  for(uint16_t i = 0; i < readCnt; i++)
  {
    at45db_write_buffer(drv, AT45DB_BUFFER1, 0, (const char *) writebuff, pageSize);
    usDelay(10);
    at45db_buffer_to_memory(drv, AT45DB_BUFFER1, WriteAddr * readCnt + i);
    at45db_wait_ready(drv);
    writebuff += pageSize;
  }
}

/**
 * @brief Transfer memory page to buffer
 * @param drv: Driver instance  
 * @param buffer_choice: Buffer selection (AT45DB_BUFFER1 or AT45DB_BUFFER2)
 * @param page: Source page number
 */
static void at45db_memory_to_buffer(driver_t *drv, uint8_t buffer_choice, uint32_t page)
{
  
    at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;
    

  driverex_spi_pend_sem(cfg->spi_io);

  driver_do_low(cfg->cs_io);
  
    if(buffer_choice == AT45DB_BUFFER2)
    {
      driverex_spi_send_byte(cfg->spi_io, AT45DB_CMD_MEMORY_TO_BUFFER2);
    }
    else
    {
      driverex_spi_send_byte(cfg->spi_io, AT45DB_CMD_MEMORY_TO_BUFFER1);
    }

  at45db_page_write_cmd(drv, page);

  driverex_spi_send_byte(cfg->spi_io,BYTE_DUMMY);
  
  driver_do_high(cfg->cs_io);

  driverex_spi_post_sem(cfg->spi_io);
}
/**
 * @brief Read data from AT45DB internal buffer
 * @param drv: Driver instance
 * @param buffer_choice: Buffer selection (AT45DB_BUFFER1 or AT45DB_BUFFER2)
 * @param address: Buffer address offset  
 * @param string: Buffer to store read data
 * @param buf_len: Length of data to read
 */
static void at45db_read_buffer(driver_t *drv, uint8_t buffer_choice, uint32_t address, char *string, uint16_t buf_len)
{
  char szCmd[5];
    at45db_cfg_t *cfg=(at45db_cfg_t*)drv->cfg;


  driverex_spi_pend_sem(cfg->spi_io);

  driver_do_low(cfg->cs_io);

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


  driverex_spi_send_bytes(cfg->spi_io,(uint8_t *)szCmd,5);
  driverex_spi_read_bytes(cfg->spi_io,(uint8_t *)string,buf_len);
  
  driver_do_high(cfg->cs_io);
  driverex_spi_post_sem(cfg->spi_io);


}
void at45db_read_page(driver_t *drv,uint32_t ReadAddr,uint8_t *readbuff)
{
  at45db_cfg_t *cfg = (at45db_cfg_t*)drv->cfg;
  uint16_t pageSize = cfg->chip_info.current_page_size;
  uint8_t readCnt = 1;

  // For devices with smaller page sizes, may need multiple operations
  if (pageSize < 512) {
    readCnt = 512 / pageSize;
  }
  
  for(uint16_t i = 0; i < readCnt; i++)
  {
    at45db_memory_to_buffer(drv, AT45DB_BUFFER1, (uint32_t) ReadAddr * readCnt + i);
    at45db_wait_ready(drv);
    at45db_read_buffer(drv, AT45DB_BUFFER1, 0, (char*) readbuff, pageSize);
    at45db_wait_ready(drv);
    readbuff += pageSize;
  }
}

const char *_eicpart=NULL;//"AT45DB641E-SHN2B"; /* IC Part Number */
at45db_result_t at45db_init(driver_t *drv)
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
    
  at45db_cfg_t *cfg = (at45db_cfg_t*)drv->cfg;
  
  at45db_reg_read(drv, AT45DB_CMD_DEVICE_ID, chip_info, 5);

  at45db_result_t result = at45db_parse_chip_info(chip_info, &cfg->chip_info);
  if (result != AT45DB_OK) {
    DEBUG_PRINTF("Error: Failed to parse chip info\r\n");
    return result;
  }

  osDelay(1);

  at45db_reg_read(drv, AT45DB_CMD_STATUS_REGISTER, &reg, 1);
  
  // Check if binary page size mode is enabled
  if ((reg & AT45DB_STATUS_PAGE_SIZE_BIT) == 0) {
    DEBUG_PRINTF("Switching to binary page size mode...\r\n");
    at45db_reg_write(drv, (uint8_t *)protect_enable);
    at45db_wait_ready(drv);
    at45db_reg_write(drv, (uint8_t *)page_binary_mode);
    at45db_wait_ready(drv);
    
    // Verify the switch
    at45db_reg_read(drv, AT45DB_CMD_STATUS_REGISTER, &reg, 1);
    if ((reg & AT45DB_STATUS_PAGE_SIZE_BIT) == 0) {
      DEBUG_PRINTF("Error: Failed to switch to binary page size mode\r\n");
      return AT45DB_ERROR;
    }
  }

  // Set current page size and other info
  cfg->chip_info.is_binary_mode = (reg & AT45DB_STATUS_PAGE_SIZE_BIT) ? true : false;
  cfg->chip_info.current_page_size = cfg->chip_info.is_binary_mode ? 
                                     cfg->chip_info.device_info.page_size_binary : 
                                     cfg->chip_info.device_info.page_size_standard;
  cfg->chip_info.total_capacity_bytes = (cfg->chip_info.device_info.capacity_bits / 8);
  cfg->chip_info.is_initialized = true;

  DEBUG_PRINTF("AT45DB initialization completed successfully\r\n");
  DEBUG_PRINTF("  Mode: %s\r\n", cfg->chip_info.is_binary_mode ? "Binary" : "Standard");
  DEBUG_PRINTF("  Current page size: %u bytes\r\n", cfg->chip_info.current_page_size);
  DEBUG_PRINTF("  Total capacity: %lu bytes\r\n", cfg->chip_info.total_capacity_bytes);

  at45db_delay(10);
  
  return AT45DB_OK;

}

at45db_result_t at45db_get_chip_info(driver_t *drv, at45db_chip_info_t *info)
{
  if (!drv || !info) {
    return AT45DB_ERROR;
  }
  
  at45db_cfg_t *cfg = (at45db_cfg_t*)drv->cfg;
  if (!cfg->chip_info.is_initialized) {
    return AT45DB_INIT_FAILED;
  }
  
  memcpy(info, &cfg->chip_info, sizeof(at45db_chip_info_t));
  return AT45DB_OK;
}

#define FLASH_PAGE_SIZE_BASE 512

/**
 * @brief Flash read like RAM access
 */
void at45db_read(driver_t *drv, uint32_t offset, uint8_t *p_buff, uint32_t buff_size,
                 uint32_t read_len)
{
  at45db_cfg_t *cfg = (at45db_cfg_t*)drv->cfg;
  uint32_t page_size = cfg->chip_info.current_page_size;
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

  OS_PEND_SEM(drv->sem, osWaitForever);

  // Case 1: When start position is not page-aligned (unaligned)
  if (page_rem > 0)
  {
    at45db_read_page(drv, page_quot, buff);
    uint32_t first_read = (read_len < remain) ? read_len : remain;
    memcpy(&p_buff[0], &buff[page_rem], first_read);
    page_quot++;
    read_cnt += first_read;
  }

  // Case 2: Read full pages
  while ((read_len - read_cnt) >= page_size)
  {
    at45db_read_page(drv, page_quot, buff);
    memcpy(&p_buff[read_cnt], buff, page_size);
    page_quot++;
    read_cnt += page_size;
  }

  // Case 3: Read partial last page (if any remaining)
  if (read_len > read_cnt)
  {
    at45db_read_page(drv, page_quot, buff);
    memcpy(&p_buff[read_cnt], buff, read_len - read_cnt);
  }

  OS_POST_SEM(drv->sem);
}


int32_t at45db_write(driver_t *drv, uint32_t offset, uint8_t *p_data, uint32_t data_len)
{
  at45db_cfg_t *cfg = (at45db_cfg_t*)drv->cfg;
  uint32_t page_size = cfg->chip_info.current_page_size;
  uint8_t buff[2112]; // Maximum possible page size for AT45DB256
  uint32_t page_quot = offset / page_size;
  uint32_t page_rem = offset % page_size;
  uint32_t remain = page_size - page_rem;
  uint32_t written = 0;

  OS_PEND_SEM(drv->sem, osWaitForever);

  // When start position is not page-aligned (unaligned)
  if (page_rem > 0)
  {
    at45db_read_page(drv, page_quot, buff);
    uint32_t first_write = (data_len < remain) ? data_len : remain;
    memcpy(&buff[page_rem], &p_data[0], first_write);
    at45db_write_page(drv, page_quot, buff);
    page_quot++;
    written += first_write;
  }

  // Write full pages
  while ((data_len - written) >= page_size)
  {
    memcpy(buff, &p_data[written], page_size);
    at45db_write_page(drv, page_quot, buff);
    page_quot++;
    written += page_size;
  }

  // Write partial last page (if any remaining)
  if (data_len > written)
  {
    at45db_read_page(drv, page_quot, buff);
    memcpy(buff, &p_data[written], data_len - written);
    at45db_write_page(drv, page_quot, buff);
  }

  OS_POST_SEM(drv->sem);
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