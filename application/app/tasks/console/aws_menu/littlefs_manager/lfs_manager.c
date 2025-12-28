
/*
littlefs 테스트용 파일

*/

#include "lfs_manager.h"

#include "app_file.h"
#if LFS_ENABLE ==1


#include <string.h>
#include <stdlib.h>


#include "cli_key_code.h"
 
#include "debug_io.h"
#include "lfs.h"
#include "lfs_port.h"
#include "user_heap.h"

static void print_menu(void);
static void cmd_info(void);
static void cmd_list(void);
static void cmd_delete(void);
static void cmd_dump(void);
static void cmd_write(void);
static void cmd_read(void);
static void cmd_test(void);
static void cmd_format(void);
static void hex_dump(uint8_t *data, uint32_t length, uint32_t offset);

//함수 실행시 초기에 파일시스템 정보를 출력한다.
//delete 파일명을 입력하면 파일을 삭제하는 기능 추가(사용자에게 한번더 진행여부 물어 봐야한다.)
//dump 파일명 오프셋 읽을 길이 하면 파일을 오프셋위치에서 길이 만큼 읽어서 hex와 아스키로 출력한다(hex dump)
int32_t menu_littlefs_manager(void)
{
  char cmd[128];
  int ret;

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"\r\n========================================\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"    LittleFS File System Manager\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"========================================\r\n\r\n");

  // 초기 파일시스템 정보 출력
  printf_lfs_info();
  printf_lfs_directory("/");

  while (1)
  {
    print_menu();

      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"\r\nLFS> ");
    ret = shell_scanf("%s", cmd);

    if (ret <= 0)
    {
      continue;
    }

    // 명령어 처리
    if (strcmp(cmd, "info") == 0)
    {
      cmd_info();
    }
    else if (strcmp(cmd, "list") == 0 || strcmp(cmd, "ls") == 0)
    {
      cmd_list();
    }
    else if (strcmp(cmd, "delete") == 0 || strcmp(cmd, "rm") == 0)
    {
      cmd_delete();
    }
    else if (strcmp(cmd, "dump") == 0)
    {
      cmd_dump();
    }
    else if (strcmp(cmd, "write") == 0)
    {
      cmd_write();
    }
    else if (strcmp(cmd, "read") == 0)
    {
      cmd_read();
    }
    else if (strcmp(cmd, "test") == 0)
    {
      cmd_test();
    }
    else if (strcmp(cmd, "format") == 0)
    {
      cmd_format();
    }
    else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0)
    {
      // 메뉴는 다시 출력됨
      continue;
    }
    else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0)
    {
        DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Exiting LittleFS Manager...\r\n");
      break;
    }
    else
    {
        DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Unknown command: %s (type 'help' for commands)\r\n", cmd);
    }
  }

  return 0;
}

static void print_menu(void)
{
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"\r\n--- Commands ---\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"  info          - Show filesystem information\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"  list (ls)     - List files in directory\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"  delete (rm)   - Delete a file\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"  dump          - Hex dump file contents\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"  write         - Write data to file\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"  read          - Read file contents\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"  test          - Run read/write test\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"  format        - Format filesystem (WARNING: erases all data!)\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"  help (?)      - Show this menu\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"  exit (q)      - Exit manager\r\n");
}

static void cmd_info(void)
{
  printf_lfs_info();
}

static void cmd_list(void)
{
  char dir[64];
  int ret;

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Directory path (default: /): ");
  ret = shell_scanf("%s", dir);

  if (ret <= 0 || strlen(dir) == 0)
  {
    strcpy(dir, "/");
  }

  printf_lfs_directory(dir);
}

static void cmd_delete(void)
{
  char confirm;
  char filename[128];
  int err;
  int ret;
  uint32_t size;

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Filename to delete: ");
  ret = shell_scanf("%s", filename);

  if (ret <= 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: Invalid filename\r\n");
    return;
  }

  // 파일 존재 여부 확인
  err = get_lfs_file_size(filename, &size);
  if (err < 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: File '%s' not found (err=%d)\r\n", filename, err);
    return;
  }

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"File '%s' found (size: %lu bytes)\r\n", filename, (unsigned long)size);
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Are you sure you want to delete? (y/n): ");

  ret = shell_scanf("%c", &confirm);

  if (ret <= 0 || (confirm != 'y' && confirm != 'Y'))
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Delete cancelled\r\n");
    return;
  }
#if LFS_ENABLE ==1
  err = lfs_delete_file(filename);
  if (err == 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"File '%s' deleted successfully\r\n", filename);
  }
  else
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: Failed to delete file (err=%d)\r\n", err);
  }
#endif
}

static void cmd_dump(void)
{
  char filename[128];
  int err;
  int ret;
  uint8_t *buffer;
  uint32_t file_size;
  uint32_t length;
  uint32_t offset;

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Filename: ");
  ret = shell_scanf("%s", filename);
  if (ret <= 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: Invalid filename\r\n");
    return;
  }

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Offset (hex): ");
  ret = shell_scanf("%x", &offset);
  if (ret <= 0)
  {
    offset = 0;
  }

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Length (hex): ");
  ret = shell_scanf("%x", &length);
  if (ret <= 0)
  {
    length = 256;
  }

  // 파일 크기 확인
  err = get_lfs_file_size(filename, &file_size);
  if (err < 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: File '%s' not found (err=%d)\r\n", filename, err);
    return;
  }

  if (offset >= file_size)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: Offset 0x%X is beyond file size %lu\r\n", offset, (unsigned long)file_size);
    return;
  }

  // 읽을 길이 조정
  if (offset + length > file_size)
  {
    length = file_size - offset;
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Adjusted length to %lu bytes (until end of file)\r\n", (unsigned long)length);
  }

  // 버퍼 할당
  buffer = (uint8_t *)user_malloc(length);
  if (buffer == NULL)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: Failed to allocate buffer\r\n");
    return;
  }

  // 파일 읽기
  err = lfs_read_file(filename, buffer, length, offset);
  if (err < 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: Failed to read file (err=%d)\r\n", err);
    user_free(buffer);
    return;
  }

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"\r\nHex dump of '%s' (offset: 0x%X, length: %lu bytes):\r\n", filename, offset, (unsigned long)length);
  hex_dump(buffer, length, offset);

  user_free(buffer);
}

static void cmd_write(void)
{
  char data[256];
  char filename[128];
  int err;
  int ret;
  uint32_t offset;

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Filename: ");
  ret = shell_scanf("%s", filename);
  if (ret <= 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: Invalid filename\r\n");
    return;
  }

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Offset (default: 0): ");
  ret = shell_scanf("%u", &offset);
  if (ret <= 0)
  {
    offset = 0;
  }

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Data to write: ");
  ret = shell_scanf("%s", data);
  if (ret <= 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: No data entered\r\n");
    return;
  }

#if LFS_ENABLE ==1
  err = lfs_write_file(filename, (uint8_t *)data, strlen(data), offset);
  if (err == 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Successfully wrote %d bytes to '%s' at offset %lu\r\n", strlen(data), filename, (unsigned long)offset);
  }
  else
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: Failed to write file (err=%d)\r\n", err);
  }
#endif
}

static void cmd_read(void)
{
  char filename[128];
  int err;
  int ret;
  uint8_t *buffer;
  uint32_t file_size;
  uint32_t i;
  uint32_t length;

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Filename: ");
  ret = shell_scanf("%s", filename);
  if (ret <= 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: Invalid filename\r\n");
    return;
  }

  // 파일 크기 확인
  err = get_lfs_file_size(filename, &file_size);
  if (err < 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: File '%s' not found (err=%d)\r\n", filename, err);
    return;
  }

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"File size: %lu bytes\r\n", (unsigned long)file_size);

  if (file_size > 1024)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"File is large. Reading first 1024 bytes only.\r\n");
    length = 1024;
  }
  else
  {
    length = file_size;
  }

  buffer = (uint8_t *)user_malloc(length);
  if (buffer == NULL)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: Failed to allocate buffer\r\n");
    return;
  }
#if LFS_ENABLE ==1
  err = lfs_read_file(filename, buffer, length, 0);
  if (err < 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Error: Failed to read file (err=%d)\r\n", err);
    user_free(buffer);
    return;
  }
#endif

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"\r\nFile contents:\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"--- ASCII ---\r\n");
  for (i = 0; i < length; i++)
  {
    if (buffer[i] >= 32 && buffer[i] <= 126)
    {
        DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"%c", buffer[i]);
    }
    else
    {
        DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,".");
    }
  }
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"\r\n");

  user_free(buffer);
}

static void cmd_test(void)
{
  int ret;
  uint32_t size;

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Test file size (bytes, default: 10240): ");
  ret = shell_scanf("%u", &size);
  if (ret <= 0)
  {
    size = 10240;
  }

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Running test with %lu bytes...\r\n", (unsigned long)size);
  //test_lfs("test.txt", size);
}

static void cmd_format(void)
{
  char confirm[16];
  int err;
  int ret;

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"========================================\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"        WARNING: FORMAT FILESYSTEM\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"========================================\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"This will erase ALL data on the filesystem!\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"All files and directories will be permanently deleted.\r\n");
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"\r\n");

  // 파일시스템 정보 표시
  printf_lfs_info();

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"\r\nType 'YES' (all uppercase) to confirm format: ");
  ret = shell_scanf("%s", confirm);

  if (ret <= 0 || strcmp(confirm, "YES") != 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Format cancelled\r\n");
    return;
  }

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"\r\nFormatting filesystem...\r\n");

  // lfs_cfg는 lfs_port.h에서 extern으로 선언됨
  extern struct lfs_config lfs_cfg;
  extern lfs_t lfs;

  // 파일시스템 언마운트 (format 전에 필요)
  // 주의: 언마운트 실패해도 계속 진행 (이미 언마운트 상태일 수 있음)
  lfs_unmount(&lfs);

  // 포맷 수행
  err = lfs_format(&lfs, &lfs_cfg);
  if (err < 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"ERROR: Format failed (err=%d)\r\n", err);
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Attempting to remount filesystem...\r\n");
    lfs_mount(&lfs, &lfs_cfg);
    return;
  }

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Format successful!\r\n");

  // 다시 마운트
    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Remounting filesystem...\r\n");
  err = lfs_mount(&lfs, &lfs_cfg);
  if (err < 0)
  {
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"ERROR: Mount failed after format (err=%d)\r\n", err);
    return;
  }

    DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"Mount successful!\r\n\r\n");

  // 포맷 후 정보 출력
  printf_lfs_info();
  printf_lfs_directory("/");
}

static void hex_dump(uint8_t *data, uint32_t length, uint32_t offset)
{
  uint32_t i;
  uint32_t j;

  for (i = 0; i < length; i += 16)
  {
    // 주소 출력
      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"%08X: ", offset + i);

    // Hex 출력
    for (j = 0; j < 16; j++)
    {
      if (i + j < length)
      {
          DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"%02X ", data[i + j]);
      }
      else
      {
          DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"   ");
      }

      if (j == 7)
      {
          DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG," ");
      }
    }

      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG," | ");

    // ASCII 출력
    for (j = 0; j < 16 && i + j < length; j++)
    {
      if (data[i + j] >= 32 && data[i + j] <= 126)
      {
          DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"%c", data[i + j]);
      }
      else
      {
          DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,".");
      }
    }

      DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"\r\n");
  }
}

#endif
