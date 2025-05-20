


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "app_file.h"
#include "crc.h"
#include "update_fw.h"
#include "user_heap.h"
#include "dev_io.h"
#include "hj_product_list.h"
#pragma location = 0x20000000
__no_init volatile uint32_t SystemMagicValue;
#define MAGIC_UPDATE_FW 0xA5A5ABAB
typedef struct fwHeader_s
{
  uint32_t ver;           // 섹션 헤더 정보,1
  uint32_t section;       // 펌웨어,const,lib  (1펌웨어,2 const ,3 lib)
  uint32_t hw_code;       // 하드웨어 (1 디바스, 2 M2M)
  uint32_t nick;          // 화진, 비젼
  uint32_t offset;        // 시작주소,0x00008000
  uint32_t len;           // 길이
  uint32_t section_ver;   // section 버전
  uint32_t time;          // 헤더 생성 날짜
  uint32_t restore;       // 0xFFFFFFFF 이면 nick 무시하고 업데이트
  uint32_t pcb_n;         // 적용가능한 PCB 버전
  uint32_t pcb[50];       // PCB 버전 목록
  uint8_t iv[16];         // CBC 초기화 백터
  uint8_t reserved[760];  // 4의 배수 정렬
  uint32_t fw_CRC;
  uint32_t head_CRC;  // 헤더의 헤더의 crc32
} fw_header_t;

bool serach_fw(char buffer[100]);

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
void update_fw(uint8_t local)
{
  FRESULT fret;
  uint8_t *p_buffer=0;
  FSIZE_t file_size = 0;
  char path[100];

  SystemMagicValue = 0;

  if (local == UPDATE_REMOTE)
  {
    snprintf(path, sizeof(path),"%s",UPDATE_FW__REMOTE_PATH);
  }
  else
  {
    if(serach_fw(path)==false)
    {
      return ;
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
      return ;
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
          debug_printf("장비에 적용되는 펌웨어가 아닙니다.\r\n");
        }
          debug_printf("장비가 리셋되면서 펌웨어 업데이트가 자동 진행됩니다.\r\n");
        SystemMagicValue = MAGIC_UPDATE_FW;
      }
      else
      {
        debug_printf("펌웨어 CRC 불일치\r\n");
      }
    }



  }

  if (p_buffer)
  {
    aws_free(p_buffer);
  }

  if (SystemMagicValue == MAGIC_UPDATE_FW)
  {
    //리셋셋
  }
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