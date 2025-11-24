

#ifndef APP_FILE_H
#define APP_FILE_H

#include <stdint.h>

#include "fatfs.h"

#define MAX_FILENAME_LEN 100  // ��ȯ�� ���� �̸��� �ִ� ����
#define MAX_FILES_TO_FIND 1   // ã�� ������ �ִ� ���� (����� ������ �迭�� ũ��)

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
void make_path(const char *path);



void *get_file_sem(void);



int32_t lfs_write_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset);
int32_t lfs_read_file(char *path, uint8_t *data, uint32_t dataLen, uint32_t offset);
int32_t lfs_append_file(char *path, uint8_t *data, uint32_t dataLen);
int32_t get_lfs_file_size(const char *path, uint32_t *size);
int32_t lfs_delete_file(const char *fileName);
void printf_lfs_directory(const char *dir);
void printf_lfs_info(void);


#endif