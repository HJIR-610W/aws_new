/**
 * @file lfs_port.c
 * @brief LittleFS porting layer implementation
 *
 * This file implements the callbacks required by littlefs to interface with flash memory.
 */

#include "lfs_port.h"

#include <stdio.h>
#include <string.h>

#include "debug_io.h"
#include "components\serial_flash\at45db.h"
#include "components\fram\fm25cl.h"

#define LITTLEFS_FRAM_EN 1
#define FRAM_LFS_CACHE_SIZE 64
#define FRAM_LOOKAHEAD_SIZE 32

#define FLASH_LFS_CACHE_SIZE 256
#define FLASH_LOOKAHEAD_SIZE 16



#define LFS_LOOKAHEAD_SIZE FRAM_LOOKAHEAD_SIZE
#define LFS_CACHE_SIZE FRAM_LFS_CACHE_SIZE

/* Static buffer for littlefs */
static uint8_t lfs_lookahead_buffer[LFS_LOOKAHEAD_SIZE];
static uint8_t lfs_prog_buffer[LFS_CACHE_SIZE];
static uint8_t lfs_read_buffer[LFS_CACHE_SIZE];

/* LittleFS configuration */
struct lfs_config lfs_cfg;

/**
 * @brief Read a region in a block
 */
static int lfs_flash_read(const struct lfs_config *c, lfs_block_t block,
                          lfs_off_t off, void *buffer, lfs_size_t size)
{
    uint32_t addr;
    int ret;

    (void)c;
   
#if (LITTLEFS_FRAM_EN==1)
    ret = fm25cl_lfs_read(block,off,(uint8_t*)buffer,size);
#else
    ret = at45db_lfs_read(block,off,buffer,size);
#endif

    return (ret == 0) ? LFS_ERR_OK : LFS_ERR_IO;
}

/**
 * @brief Program a region in a block
 */
static int lfs_flash_prog(const struct lfs_config *c, lfs_block_t block,
                          lfs_off_t off, const void *buffer, lfs_size_t size)
{
    uint32_t addr;
    int ret;

    (void)c;

       
#if (LITTLEFS_FRAM_EN==1)
    ret = fm25cl_lfs_prog(block, off,(uint8_t *) buffer, size);
#else
    ret =  at45db_lfs_prog(block, off, buffer, size);
#endif

    return (ret == 0) ? LFS_ERR_OK : LFS_ERR_IO;
}

/**
 * @brief Erase a block
 */
static int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block)
{
    int ret=0;

    (void)c;

    #if (LITTLEFS_FRAM_EN==1)
    #else
    // AT45DB 블록 erase 함수 호출
    ret = at45db_lfs_erase(block);
    #endif

    return (ret == 0) ? LFS_ERR_OK : LFS_ERR_IO;
}

/**
 * @brief Sync the state of the underlying block device
 */
static int lfs_flash_sync(const struct lfs_config *c)
{
    int ret = 0;

    (void)c;



    return (ret == 0) ? LFS_ERR_OK : LFS_ERR_IO;
}

/**
 * @brief Initialize LittleFS configuration
 */
int lfs_port_init(void)
{
    #if (LITTLEFS_FRAM_EN==1)

  fm25lc_init();
    lfs_cfg.read = lfs_flash_read;
    lfs_cfg.prog = lfs_flash_prog;
    lfs_cfg.erase = lfs_flash_erase;
    lfs_cfg.sync = lfs_flash_sync;
    /* Block device configuration */
    lfs_cfg.read_size = 16;
    lfs_cfg.prog_size = 16;
    lfs_cfg.block_size = 128;
    lfs_cfg.block_count = 8192/128;
    lfs_cfg.block_cycles = -1;
    lfs_cfg.cache_size = FRAM_LFS_CACHE_SIZE;
    lfs_cfg.lookahead_size = LFS_LOOKAHEAD_SIZE;
#else
    at45db_chip_info_t at45db_chip_info;

    at45db_init();

    at45db_get_chip_info(&at45db_chip_info);


    memset(&lfs_cfg, 0, sizeof(lfs_cfg));
    lfs_cfg.read = lfs_flash_read;
    lfs_cfg.prog = lfs_flash_prog;
    lfs_cfg.erase = lfs_flash_erase;
    lfs_cfg.sync = lfs_flash_sync;
    /* Block device configuration */
    lfs_cfg.read_size = 16;
    lfs_cfg.prog_size = at45db_chip_info.device_info.page_size_binary;
    lfs_cfg.block_size = at45db_chip_info.device_info.block_size;
    lfs_cfg.block_count = at45db_chip_info.device_info.capacity_bits/at45db_chip_info.device_info.block_size/8;
    lfs_cfg.block_cycles = 500;
    lfs_cfg.cache_size = at45db_chip_info.device_info.page_size_binary;
    lfs_cfg.lookahead_size = 16;
#endif
    /* Buffers */
    lfs_cfg.read_buffer = lfs_read_buffer;
    lfs_cfg.prog_buffer = lfs_prog_buffer;
    lfs_cfg.lookahead_buffer = lfs_lookahead_buffer;


    return 0;
}

/**
 * @brief Deinitialize LittleFS configuration
 */
int lfs_port_deinit(void)
{

    return 0;
}
