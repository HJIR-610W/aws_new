
#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdint.h>
#include <string.h>

#include "ff.h"

FRESULT write_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset);
FRESULT read_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset);
FRESULT append_file(char *path, uint8_t *data, uint32_t dataLen);
FRESULT list_directory(const char *path);

#endif

