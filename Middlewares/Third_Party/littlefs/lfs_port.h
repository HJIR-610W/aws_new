/**
 * @file lfs_port.h
 * @brief LittleFS porting layer for flash memory
 *
 * This file contains the glue code between littlefs and the flash wrapper.
 */

#ifndef LFS_PORT_H
#define LFS_PORT_H

#include "lfs.h"


#ifdef __cplusplus
extern "C" {
#endif

/* LittleFS configuration structure */
extern struct lfs_config lfs_cfg;

/**
 * @brief Initialize LittleFS configuration
 * @return 0 on success, error code otherwise
 */
int lfs_port_init(void);

/**
 * @brief Deinitialize LittleFS configuration
 * @return 0 on success, error code otherwise
 */
int lfs_port_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* LFS_PORT_H */
