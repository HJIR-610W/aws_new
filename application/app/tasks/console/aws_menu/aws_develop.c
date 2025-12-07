
#include "aws_develop.h"

#include "console_define.h"
#include "console_utile.h"
#include "debug_io.h"
#include "util_memory.h"

#include "drv_fram.h"

#include "mcu_debug.h"
#include "const_string.h"
#include "config_app.h"
#include "config_nvm.h"
#include "cli_key_code.h"
#include "app_logging.h"
 
 
#include "console_rtos.h"
#include "divas_protocol_handler.h"
#include "app_console_test.h"
#include "cmsis_os.h"
#include "drv_flash.h"
#include "littlefs_manager\lfs_manager.h"
#include "task_core_debug.h"

extern const char *protocolList[2];
extern const char *cdmaModellList[2];
extern const char* panelList[4] ;


int32_t menu_manage_print_config_all(void)
{
  debug_printf("ID               :%d\r\n", config.id);
  debug_printf("비밀번호         :%d\r\n", config.password);
  debug_printf("충전기 종류      :%s\r\n", ITEM_LIST(config.charger_model, charger_list_eng));
  debug_printf("로그 카운트      :%d\r\n", nvm_get_log_cnt());
  debug_printf("프로토콜          :%s\r\n", ITEM_LIST(config.aws_protocol_type, protocolList));
  debug_printf("이더넷 서브넷    :%d.%d.%d.%d\r\n", config.eth_subnet[0], config.eth_subnet[1],
              config.eth_subnet[2], config.eth_subnet[3]);
  debug_printf("이더넷 게이트웨이:%d.%d.%d.%d\r\n", config.eth_gateway[0], config.eth_gateway[1],
              config.eth_gateway[2], config.eth_gateway[3]);
  debug_printf("이더넷 IP        :%d.%d.%d.%d\r\n", config.eth_ip[0], config.eth_ip[1],
              config.eth_ip[2], config.eth_ip[3]);
  debug_printf("이더넷 원격 서버 :%d.%d.%d.%d\r\n", config.eth_remote_server_ip[0], config.eth_remote_server_ip[1],
              config.eth_remote_server_ip[2], config.eth_remote_server_ip[3]);
  ;

  debug_printf("이더넛 포트      :%d\r\n", config.eth_remote_server_port);

  debug_printf("CDMA 원격 서버   :%d.%d.%d.%d\r\n", config.cdma_server_ip[0],
              config.cdma_server_ip[1], config.cdma_server_ip[2], config.cdma_server_ip[3]);
  debug_printf("CDMA 포트        :%d\r\n", config.cdma_port);

  debug_printf("CDMA 종류        :%s\r\n", ITEM_LIST(config.cdma_model, cdmaModellList));
  debug_printf("이더넷 사용      :%s\r\n", ITEM_LIST((int32_t)config.eth_active, enable_list_kor));
  debug_printf("CDMA 사용        :%s\r\n", ITEM_LIST((int32_t)config.cdma_active, enable_list_kor));
  debug_printf("직접통신         :%s\r\n", ITEM_LIST((int32_t)config.direct_active, enable_list_kor));

  debug_printf("직접통신 속도    :%d\r\n", ITEM_LIST((int32_t)config.direct_baud_index, baud_list_eng)); 
  debug_printf("패널 종류        :%s\r\n", ITEM_LIST(config.panel_model, panelList));
  debug_printf("VHF ID           :%d\r\n", config.vhf_id);
  debug_printf("VHF 그룹         :%d\r\n", config.vhf_group);
  debug_printf("VHF HOST         :%d\r\n", config.vhf_host_id);
  debug_printf("VHF 중계         :%d\r\n", config.vhf_repeater_id);
  debug_printf("VHF PTT 지연     :%d\r\n", config.vhf_ptt_delay);

  return 0;
}


void print_flash(uint32_t start, uint32_t size, uint32_t width)
{
  uint8_t buff[512];
  uint32_t quot;
  uint32_t rem;
  uint32_t i;

  quot = size / 512;
  rem = size % 512;

  for (i = 0; i < quot; i++)
  {
    drv_flash_read(start + i * 512, buff,  512);
    debug_dump(buff, sizeof(buff), start + i * 512, width);
  }

  if (rem)
  {
    drv_flash_read(start + i * 512, buff,  rem);
    debug_dump(buff, rem, start + i * 512, width);
  }
}

void print_fram(uint32_t start, uint32_t size, uint32_t width)
{
  uint8_t buff[512];
  uint32_t quot;
  uint32_t rem;
  uint32_t i;

  quot = size / 512;
  rem = size % 512;

  for (i = 0; i < quot; i++)
  {
    drv_fram_read(start + i * 512, buff, 512);
    debug_dump(buff, sizeof(buff), start + i * 512, width);
  }

  if (rem)
  {
    drv_fram_read(start + i * 512, buff, rem);
    debug_dump(buff, rem, start + i * 512, width);
  }
}

int32_t menu_developer_memory(void)
{
  int32_t status;
  int32_t choice;
  int32_t inCnt;
  int32_t start, size, len;

  const char *memList[] = {"flash", "fram"};

  status = select_index_from_table( memList, NULL, _countof(memList), true,&choice);

  if (status != MENU_OK)
  {
    return status;
  }

  debug_printf("start(HEX),size,len>>");

  inCnt = shell_scanf("%x,%d,%d", &start, &size, &len);
  if (inCnt == EXIT_BACK || inCnt == EXIT_PROGRAM && choice < 0)
  {
    return inCnt;
  }
  
  if(size>512 || len >16)
  {
    return 0;
  }

  switch (choice)
  {
    case 0:  // flash;
      print_flash(start, size, len);
      break;
    case 1:  // fram
      print_fram(start, size, len);
       break;
  }
  return 0;
}



int32_t menu_developer_sensor_config(void)
{
  // char opt[20];
  int32_t cnt = 0;
  int i = 0;

  debug_printf("\r\n");


  cnt = _countof(sensor_name_list);
  return cnt;
}




int32_t menu_developer_logging(void)
{
  int32_t startCnt, endCnt;
  system_log_t log;
  int32_t cnt;
  char buff[LOG_LEN_MAX+1];

  do
  {
    debug_printf("로그 시작 카운트:%d\r\n", logging_get_log_count());
    debug_printf("start,end>>");

    cnt = shell_scanf("%d,%d,%d", &startCnt, &endCnt);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }

    if (cnt == 2)
    {
      for (int32_t i = startCnt; i <= endCnt; i++)
      {

        logging_read_log(i, &log);
        memcpy(buff,log.msg,LOG_LEN_MAX);
        buff[LOG_LEN_MAX]=0;
        debug_printf("%4d,%s\r\n", i,buff);
      }
    }
  } while (1);
}






int32_t menu_update_info(void)
{
  float progress=0.0f;
  uint32_t total_bytes;
  uint32_t received_bytes;


  while(1)
  {
    total_bytes = get_download_file_size();
    received_bytes = get_received_bytes();
    if(total_bytes !=0)
    {
      progress = ((float)received_bytes/(float)total_bytes)*100.0;
    }
    debug_printf("펌웨어 다운:%7d/%7d [%5.2f%%]\r",received_bytes,total_bytes,progress);

    if (debug_get_key(1000) == KEY_CODE_CTRL_Q)
    {
      break;
    }
  }

  return EXIT_BACK;
}

int32_t menu_task_print(void)
{
  uint32_t id;
  int32_t ret;
  debug_printf("printf 하고 싶은 task id입력해주세요\r\n");
  debug_printf("printf 종료하려면 0을 입력\r\n");
  debug_printf(">>");

  ret = debug_scanf_s("%X",&id);
  if(ret <=0)
  {
    return ret;
  }
  set_task_id((void*)id);

  while(1)
  {
    if(debug_get_key(osWaitForever)==KEY_CODE_CTRL_Q)
    {
      set_task_id(0);
      break;
    }
  }

  return EXIT_BACK;
}

int32_t menu_task_print_force(void)
{
  int status;
  int ok=0;

  debug_printf("특정 Task는 1회성 실행으로 task id가 유지 되지 않는다.");
  debug_printf("강제 출력을 하면 task_prinf가 강제 실행된다.\r\n");

  while(1)
  {
    status = view_confirm_continue("task printf 강제출력하겠습니까?",&ok);
    if(status != MENU_OK)
    break;

    if(ok)
    set_forced_print(true);
    else
    set_forced_print(false);
  }

  return status;

}


extern int32_t input_ip(int *a, int *b, int *c, int *d);
int32_t menu_task_telnet(void)
{
  int status;

  int a,b,c,d;
  uint8_t *ip = get_config_app()->dev_telnet_ip;
  uint16_t port =get_config_app()->dev_telnet_port;


  if (get_config_app()->dev_telnet_mode == eTELNET_SERVER)
  {
    debug_printf("텔넷 모드:서버(외부에서 접속해와야함)\r\n");
  }
  else if (get_config_app()->dev_telnet_mode == eTELNET_CLIENT)
  {
    debug_printf("텔넷 모드:클라이언트(중계서버로 접속)\r\n");
  }
  else{
    debug_printf("텔넷 모드:설정 오류\r\n");
  }

  debug_printf("텔넷 중계서버 IP:%d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3]);
  debug_printf("텔넷 PORT(중계,로컬공통):%d\r\n", port);

  debug_printf("텔넷 모드 설정\r\n");
  status = view_input_decimal("텔넷모드(0:서버 1:클라이언트(중계모드))", &a, 0, 1);

  if (status != MENU_OK)
  {
    return status;
  }

  get_config_app()->dev_telnet_mode =(eTELNET_MODE_t)a;
  WRITE_CFG(dev_telnet_mode);

  debug_printf("텔넷 접속할 서버 주소 설정\r\n");

  status =  input_ip( &a, &b, &c, &d);

  if(status !=MENU_OK)
  {
    return status;
  }

  get_config_app()->dev_telnet_ip[0] =a;
  get_config_app()->dev_telnet_ip[1] = b;
  get_config_app()->dev_telnet_ip[2] = c;
  get_config_app()->dev_telnet_ip[3] = d;

  WRITE_CFG(dev_telnet_ip);

  status = view_input_decimal("port",&a,0,65535);

  if (status != MENU_OK)
  {
    return status;
  }

  get_config_app()->dev_telnet_port = a;

  WRITE_CFG(dev_telnet_port);
  return MENU_OK;
}
int32_t aws_menu_develop(void)
{
  int choice, status;


  const char* menu[] = {"인터럽트 설정 확인",
                  "메모리 테스트",
                  "센서 설정 전부 확인",
                  "로그 확인",
                  "테스크 정보",
                  "파일 다운 진행 상태",
                  "TASK 디버깅 출력",
                  "TASK 디버깅 출력 강제",
                  "PCB PIN",
                  "TELNET",
                  "littlefs"};

    while(1)
    {
      status = view_input_combobox( "개발자", menu, _countof(menu),&choice);
      if (status != MENU_OK)
        return status;

      switch (choice)
      {
        case 1:
          PrintAllInterrupts();
        case 2:
          menu_developer_memory();
           break;
        case 3:
          menu_developer_sensor_config();
           break;
        case 4:
          menu_developer_logging();
          break;
       case 5:
        print_task_info();
          break;
        case 6:
          menu_update_info();
           break;
        case 7:
          menu_task_print();
           break;
        case 8:
          menu_task_print_force();
           break;
          case 9:
            pcb_pin();
            break;
        case 10:
         menu_task_telnet();
         break;
       case 11:
#if LFS_ENABLE ==1
         menu_littlefs_manager();
#endif
        break;
      }
  }

}
