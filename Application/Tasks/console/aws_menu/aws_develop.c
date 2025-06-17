
#include "aws_develop.h"

#include "console_define.h"
#include "console_utile.h"
#include "dev_io.h"
#include "util_memory.h"

#include "app_flash.h"
#include "mcu_debug.h"

#include "config_app.h"
#include "config_nvm.h"
#include "cli_key_code.h"
#include "app_logging.h"
#include "console_scanf.h"
#include "console_scanf.h"
#include "cli_input.h"
#include "console_rtos.h"
#include "divas_protocol_handler.h"
#include "app_console_test.h"

extern const char* g_chgList[2];
extern const char *protocolList[2];
extern const char *cdmaModellList[2];
extern const char* panelList[4] ;


int32_t menu_manage_print_config_all(void)
{
  io_printf("ID               :%d\r\n", config.id);
  io_printf("비밀번호         :%d\r\n", config.password);
  io_printf("충전기 종류      :%s\r\n", ITEM_LIST(config.charger_model, g_chgList));
  io_printf("로그 카운트      :%d\r\n", nvm_get_log_cnt());
  io_printf("프로토콜          :%s\r\n", ITEM_LIST(config.aws_protocol_type, protocolList));
  io_printf("이더넷 서브넷    :%d.%d.%d.%d\r\n", config.eth_subnet[0], config.eth_subnet[1],
              config.eth_subnet[2], config.eth_subnet[3]);
  io_printf("이더넷 게이트웨이:%d.%d.%d.%d\r\n", config.eth_gateway[0], config.eth_gateway[1],
              config.eth_gateway[2], config.eth_gateway[3]);
  io_printf("이더넷 IP        :%d.%d.%d.%d\r\n", config.eth_ip[0], config.eth_ip[1],
              config.eth_ip[2], config.eth_ip[3]);
  io_printf("이더넷 원격 서버 :%d.%d.%d.%d\r\n", config.eth_remote_server_ip[0], config.eth_remote_server_ip[1],
              config.eth_remote_server_ip[2], config.eth_remote_server_ip[3]);
  ;

  io_printf("이더넛 포트      :%d\r\n", config.eth_remote_server_port);

  io_printf("CDMA 원격 서버   :%d.%d.%d.%d\r\n", config.cdma_server_ip[0],
              config.cdma_server_ip[1], config.cdma_server_ip[2], config.cdma_server_ip[3]);
  io_printf("CDMA 포트        :%d\r\n", config.cdma_port);

  io_printf("CDMA 종류        :%s\r\n", ITEM_LIST(config.cdma_model, cdmaModellList));
  io_printf("이더넷 사용      :%s\r\n", ITEM_LIST((int32_t)config.eth_use, enableList));
  io_printf("CDMA 사용        :%s\r\n", ITEM_LIST((int32_t)config.cdma_use, enableList));
  io_printf("직접통신         :%s\r\n", ITEM_LIST((int32_t)config.direct_use, enableList));

  io_printf("직접통신 속도    :%d\r\n", config.direct_baud);
  io_printf("패널 종류        :%s\r\n", ITEM_LIST(config.panel_model, panelList));
  io_printf("VHF ID           :%d\r\n", config.vhf_id);
  io_printf("VHF 그룹         :%d\r\n", config.vhf_group);
  io_printf("VHF HOST         :%d\r\n", config.vhf_host_id);
  io_printf("VHF 중계         :%d\r\n", config.vhf_repeater_id);
  io_printf("VHF PTT 지연     :%d\r\n", config.vhf_ptt_delay);

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
    flash_read(start + i * 512, buff, 512, 512);
    LOG_MEM(buff, sizeof(buff), start + i * 512, width);
  }

  if (rem)
  {
    flash_read(start + i * 512, buff, 512, rem);
    LOG_MEM(buff, rem, start + i * 512, width);
  }
}

int32_t menu_developer_memory(void)
{
  int32_t status;
  int32_t choice;
  int32_t inCnt;
  int32_t start, size, len;

  const char *memList[] = {"flash", "fram"};

  status = select_indexFromList( memList, NULL, _countof(memList), true,&choice);

  if (status != MENU_OK)
  {
    return status;
  }

  io_printf("start,size,len>>");

  inCnt = console_scanf("%d,%d,%d", &start, &size, &len);
  if (inCnt == EXIT_BACK || inCnt == EXIT_PROGRAM && choice < 0)
  {
    return inCnt;
  }

  switch (choice)
  {
    case 0:  // flash;
      print_flash(start, size, len);
      break;
    case 1:  // fram
      break;
  }
  return 0;
}



int32_t menu_developer_sensor_config(void)
{
  // char opt[20];
  int32_t cnt = 0;
  int i = 0;

  io_printf("\r\n");

#if 1
  cnt = _countof(sensor_name_list);

  for (i = 0; i < cnt; i++)
  {
    io_printf("%13s:%d",sensor_name_list[i], config.sensor[i].configCnt);
    for (int j = 0; j < 4; j++)
    {
      io_printf("[%-15s.%d]", ITEM_LIST(config.sensor[i].config[j][0], g_sensor_model_list),
                  config.sensor[i].config[j][1]);
    }

    io_printf("\r\n");
  }

#endif
  cnt = _countof(sensor_name_list);
  return cnt;
}




int32_t menu_developer_logging(void)
{
  int32_t startCnt, endCnt;
  sysLog_t log;
  int32_t cnt;
  //int32_t year, month, day, hour, min, sec;

  do
  {
    io_printf("로그 시작 카운트:%d\r\n", logging_get_logCnt());
    io_printf("start,end>>");

    cnt = console_scanf("%d,%d,%d", &startCnt, &endCnt);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }

    if (cnt == 2)
    {
      for (int32_t i = startCnt; i <= endCnt; i++)
      {
        logging_read_log(i, &log);
        io_printf("%4d,%s\r\n", i,log.msg);
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
    io_printf("펌웨어 다운:%7d/%7d [%5.2f%%]\r",received_bytes,total_bytes,progress);

    if (get_key(1000) == KEY_CODE_CTRL_Q)
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
  io_printf("printf 하고 싶은 task id입력해주세요\r\n");
  io_printf("printf 종료하려면 0을 입력\r\n");
  io_printf(">>");

  ret = cli_scanf_s("%X",&id);
  if(ret <=0)
  {
    return ret;
  }
  set_task_id((void*)id);

  while(1)
  {
    if(get_key(osWaitForever)==KEY_CODE_CTRL_Q)
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

  io_printf("특정 Task는 1회성 실행으로 task id가 유지 되지 않는다.");
  io_printf("강제 출력을 하면 task_prinf가 강제 실행된다.\r\n");

  while(1)
  {
    status = confirm_continue("task printf 강제출력하겠습니까?",&ok);
    if(status != MENU_OK)
    break;

    if(ok)
    set_forced_print(true);
    else
    set_forced_print(false);
  }

  return status;

}


int32_t aws_menu_develop(void)
{
  int choice, status;


  char* menu[] = {"인터럽트 설정 확인",
                  "메모리 테스트",
                  "센서 설정 전부 확인",
                  "로그 확인",
                  "테스크 정보",
                  "파일 다운 진행 상태",
                  "TASK 디버깅 출력",
                  "TASK 디버깅 출력 강제",
                  "PCB PIN"};

    while(1)
    {
      status = choice_menu(24, "개발자", menu, _countof(menu),&choice);
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
          print_task_info();
          break;
        case 5:
          menu_update_info();
           break;
        case 6:
          menu_task_print();
           break;
        case 7:
          menu_task_print_force();
           break;
          case 8:
            pcb_pin();
            break;
      }
  }

}
