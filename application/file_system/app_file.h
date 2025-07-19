

#ifndef APP_FILE_H
#define APP_FILE_H

#include <stdint.h>

#include "fatfs.h"

#define MAX_FILENAME_LEN 100  // 반환할 파일 이름의 최대 길이
#define MAX_FILES_TO_FIND 1   // 찾을 파일의 최대 개수 (결과를 저장할 배열의 크기)

void filesystem_init(void);

FRESULT write_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset);
FRESULT read_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset);
FRESULT append_file(char *path, uint8_t *data, uint32_t dataLen);
FRESULT list_directory(const char *path);
FRESULT get_file_size(const char *path, FSIZE_t *size);
FRESULT test_file_rw_speed(const char *path, uint32_t fileSize);
FRESULT delete_file(const char *fileName);
FRESULT find_files_by_extension(const TCHAR *folder_path, const TCHAR *extension,
                                char found_filenames[][MAX_FILENAME_LEN],
                                int max_filenames_to_store, int *p_files_found_count);

void *get_file_sem(void);

#endif