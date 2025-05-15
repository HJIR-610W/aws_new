

#ifndef APP_FILE_H
#define APP_FILE_H

#include <stdint.h>

#include "fatfs.h"

void file_init(void);

FRESULT write_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset);
FRESULT read_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset);
FRESULT append_file(char *path, uint8_t *data, uint32_t dataLen);
FRESULT list_directory(const char *path);
FRESULT get_file_size(const char *path, FSIZE_t *size);
FRESULT test_file_rw_speed(const char *path, uint32_t fileSize);
FRESULT delete_file(const char *fileName);

extern const char *remote_path;
extern const char *user_path;
extern const char *system_log_path;
#endif