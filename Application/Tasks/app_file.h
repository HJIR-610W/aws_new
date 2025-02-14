

#ifndef APP_FILE_H
#define APP_FILE_H

#include <stdint.h>

#include "fatfs.h"

void file_init(void);

//FA_CREATE_ALWAYS|FA_WRITE
int32_t write_file(char *pPath,uint8_t *pData, uint32_t dataLen,uint32_t offset);
int32_t read_file(char *pPath,uint8_t *pBuff, uint32_t len,uint32_t offset);


extern const char *remote_path;
extern const char *user_path;
extern const char *system_log_path;
#endif