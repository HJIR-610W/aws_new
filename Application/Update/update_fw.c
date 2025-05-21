


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "app_file.h"
#include "crc.h"
#include "update_fw.h"
#include "user_heap.h"
#include "dev_io.h"
#include "hj_product_list.h"
#include "app_version.h"
#pragma location = 0x20000000
__no_init volatile uint32_t SystemMagicValue;

typedef struct fwHeader_s
{
  uint32_t ver;          // 섹션 헤더 정보,1
  uint32_t section;      // 펌웨어,const,lib  (1펌웨어,2 const ,3 lib)
  uint32_t hw_code;      // 하드웨어 (1 디바스, 2 M2M)
  uint32_t nick;         // 화진, 비젼
  uint32_t offset;       // 시작주소,0x00008000
  uint32_t len;          // 길이
  uint32_t section_ver;  // section 버전
  uint32_t time;         // 헤더 생성 날짜
  uint32_t restore;      // 0xFFFFFFFF 이면 nick 무시하고 업데이트
  uint32_t pcb_n;        // 적용가능한 PCB 버전
  uint32_t pcb[50];      // PCB 버전 목록
  uint8_t reserved[5];   // 4의 배수 정렬
  uint32_t fw_CRC;
  uint32_t head_CRC;  // 헤더의 헤더의 crc32
} fw_header_t;

bool g_firmware_update_required=false;

bool get_firmware_update(void)
{
  return g_firmware_update_required;
}

void set_firmware_update(void)
{
  g_firmware_update_required=true;
}


bool serach_fw(char buffer[100]);
void set_magic_value(uint32_t value);

bool check_fw(uint8_t *p_fw_data, uint32_t len)
{
  uint32_t crc;
  fw_header_t *p_header = (fw_header_t *)p_fw_data;

  crc = crc32_hw_with_padding(p_fw_data + sizeof(fw_header_t),len-sizeof(fw_header_t));

  if(crc == p_header->fw_CRC)
  {
    return true;
  }

  return false;

}

void print_fw_header(fw_header_t *p_header)
{

}
#define FW_SIZE_MAX 524288



uint8_t check_firmware(uint8_t local)
{
  FRESULT fret;
  uint8_t *p_buffer=0;
  FSIZE_t file_size = 0;
  char path[100];
  uint32_t pcb_version;
  uint8_t pcb_ok=0;
  SystemMagicValue = 0;

  if (local == UPDATE_REMOTE)
  {
    snprintf(path, sizeof(path),"%s",UPDATE_FW__REMOTE_PATH);
  }
  else
  {
    if(serach_fw(path)==false)
    {
      return FW_FILE_OPEN_ERR;
    }
    debug_printf("%s\r\n", path);
  }

  fret = get_file_size(path, &file_size);

  if (fret == FR_OK && (file_size < FW_SIZE_MAX))
  {
    debug_printf("크기:%d\r\n",file_size);
    p_buffer = aws_malloc(file_size);

    if (p_buffer ==NULL)
    {
      return FW_FILE_MEM_ERR;
    }
    fret = read_file(path, p_buffer, file_size, 0);
    if(fret == FR_OK)
    {
      uint32_t crc;
      fw_header_t *p_header = (fw_header_t *) p_buffer;

      crc = crc32_hw_with_padding(p_buffer + sizeof(fw_header_t), file_size - sizeof(fw_header_t));
      
      if (crc == p_header->fw_CRC)
      {
        if (p_header->hw_code != HW_NEW_ASW)
        {
          if (p_buffer)
          {
            aws_free(p_buffer);
          }
          debug_printf("제품 불일치\r\n");
          return FW_ERR_MFG;
        }

        if (p_header->nick != NICK_NEW_ASW_HJ)
        {
          if (p_buffer)
          {
            aws_free(p_buffer);
          }
          debug_printf("별칭 불일치\r\n");
          return FW_ERR_AREA;
        }

        pcb_version = PCB_VERSION;
        for (int i = 0; i < p_header->pcb_n; i++)
        {
          if (pcb_version == p_header->pcb[i])
          {
            pcb_ok = 1;
            break;
          }
        }

        if(pcb_ok==0)
        {
          if (p_buffer)
          {
            aws_free(p_buffer);
          }
          debug_printf("PCB 버전 불일치\r\n");
          return FW_ERR_PCB;
        }
      }
      else
      {
        if (p_buffer)
        {
          aws_free(p_buffer);
        }
        debug_printf("CRC 불일치\r\n");
        return FW_FILE_CRC_ERR;
      }
    }

  }
  return 0;
}


bool serach_fw(char buffer[100])
{
  char bin_filenames[MAX_FILES_TO_FIND][MAX_FILENAME_LEN];
  int num_files_found = 0;

  FRESULT fret;

  fret = (FRESULT)find_files_by_extension("0:Firmware/User", "bin", bin_filenames,
                                                  MAX_FILES_TO_FIND, &num_files_found);

  if (fret == FR_OK && num_files_found)
  {
    snprintf(buffer,100,"0:Firmware/User/%s",bin_filenames[0]);
    return true;
  }

  return false;
}



void set_magic_value(uint32_t value)
{
  SystemMagicValue = value;
}