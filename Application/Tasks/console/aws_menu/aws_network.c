#include "console_define.h"
#include "console_utile.h"
#include "dev_io.h"
#include "util_memory.h"

#include "config_app.h"

#include "console_scanf.h"
#include "cli_input.h"

#define AWS_MENU_NET_WIDTH 30

const char *ethModeList[] = {"클라이언트", "서버"};
const char *cdmaModellList[] = {"NTLE9607", "TX700"};
const char *protocolList[] = {"KMA2", "KMA3"};

int32_t print_net_use(void)
{
  int32_t cnt = 3;

  io_printf(" 0.이더넷  :%s\r\n", ITEM_LIST((int)get_config_app()->eth_use, enableList));
  io_printf(" 1.CDMA    :%s\r\n", ITEM_LIST((int)get_config_app()->cdma_use, enableList));
  io_printf(" 2.직접통신:%s\r\n", ITEM_LIST((int)get_config_app()->direct_use, enableList));

  return cnt;
}

int32_t menu_net_use(void)
{

  int32_t choice;
  int32_t status;
  do
  {
    status = select_indexFromList( NULL, print_net_use, 0, false,&choice);
    if (status != MENU_OK)
    {
      break;
    }


    switch (choice)
    {
      case 0:
        status =  choice_enable( &get_config_app()->eth_use);
        if(status != MENU_OK)
        break;

          WRITE_CFG(eth_use);
        break;
      case 1:
       status =choice_enable( &get_config_app()->cdma_use);
       if(status !=MENU_OK)
       break;
 
          if (get_config_app()->cdma_use)
          {
            config.direct_use = 0;
            WRITE_CFG(direct_use);
          }

          WRITE_CFG(cdma_use);

        break;
      case 2:
        status = choice_enable( &get_config_app()->direct_use);
        if(status != MENU_OK)
        break;
          if (get_config_app()->direct_use)
          {
            config.cdma_use = 0;
            WRITE_CFG(cdma_use);
          }
          WRITE_CFG(direct_use);
        break;
    }
    if(status != MENU_OK)
    break;
  } while (1);

  return status;
}

int32_t print_net_eth_set(void)
{
  int32_t cnt = 0;
  io_printf("%2d.방식         :%s \r\n", cnt++, ITEM_LIST(get_config_app()->eth_mode, ethModeList));
  io_printf("%2d.수집 서버 정보\r\n", cnt++);
  io_printf("%2d.기본 구성\r\n", cnt++);

  return cnt;
}

int32_t print_net_eth_remote_set(void)
{
  int32_t cnt = 0;
  uint8_t *ip = get_config_app()->eth_server_ip;

  io_printf("%2d.ip      :%d.%d.%d.%d\r\n", cnt++, ip[0], ip[1], ip[2], ip[3]);
  io_printf("%2d.port    :%d\r\n", cnt++, get_config_app()->eth_server_port);
  return cnt;
}




int32_t print_net_eth_default_set(void)
{
  int32_t cnt = 4;
  uint8_t *ip = config.eth_ip;
  uint8_t *gw = config.eth_gateway;
  uint8_t *subnet = config.eth_subnet;
  uint16_t local_port = config.eth_local_port;

  io_printf(" 0.ip      :%d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3]);
  io_printf(" 1.subnet  :%d.%d.%d.%d\r\n", subnet[0], subnet[1], subnet[2], subnet[3]);
  io_printf(" 2.gateway :%d.%d.%d.%d\r\n", gw[0], gw[1], gw[2], gw[3]);
  io_printf(" 3.port    :%d\r\n", local_port);
  return cnt;
}

int32_t input_ip(int *a, int *b, int *c, int *d)
{
  int32_t status;

  while(1)
  {
    io_printf("IP(xxx.xxx.xxx.xxx)");
    io_printf("입력:");
    status = cli_scanf_s("%d.%d.%d.%d", a, b, c, d);

    if (status == CLI_KEYCODE_CTRL_C )
    {
      status = MENU_BACK;
      break;
    }
    else if(status==CLI_KEYCODE_CTRL_Q)
    {
      status = MENU_ABORT;
      break; 
    }
    else if(status == 4)
    {
      status = MENU_OK;
      break;
    }
    io_printf("값이 입력되지 않았습니다.");
  }

  return status;
}

int32_t menu_net_eth_default_set(void)
{

  int32_t a, b, c, d;
  int32_t status;
  int32_t choice;

  do
  {
    status = select_indexFromList( NULL, print_net_eth_default_set, 0, false,&choice);
    if (status != MENU_OK)
    {
      break;
    }

    switch (choice)
    {
      case 0:  // ip
        status = input_ip(&a, &b, &c, &d);
        if(status !=MENU_OK)
        break;
          config.eth_ip[0] = a;
          config.eth_ip[1] = b;
          config.eth_ip[2] = c;
          config.eth_ip[3] = d;
          WRITE_CFG(eth_ip);
        
        break;
      case 1:  // subnet
        status = input_ip(&a, &b, &c, &d);
        if (status != MENU_OK)
          break;
        config.eth_subnet[0] = a;
        config.eth_subnet[1] = b;
        config.eth_subnet[2] = c;
        config.eth_subnet[3] = d;
        WRITE_CFG(eth_subnet);

        break;
      case 2:  // gateway
        status = input_ip(&a, &b, &c, &d);
        if (status != MENU_OK)
          break;
        config.eth_gateway[0] = a;
        config.eth_gateway[1] = b;
        config.eth_gateway[2] = c;
        config.eth_gateway[3] = d;
        WRITE_CFG(eth_gateway);
        break;
      case 3:  // port
       status = input_decimal_prompt("포트",&a,0,100000);
        if(status != MENU_OK)
          break;
          config.eth_local_port = a;
         WRITE_CFG(eth_local_port);
        break;
    }
  } while (1);
  
  return status;
}

int32_t menu_net_eth_mode_set(void)
{
  int32_t status;
  int32_t choice;
  
  status = select_indexFromList( ethModeList, NULL, _countof(ethModeList), true,&choice);
  if (status == MENU_OK)
  {
    config.eth_mode = (eETH_MODE_t)choice;
    WRITE_CFG(eth_mode);
  }
  
  return status;
}


int32_t print_net_cdma_set(void)
{
  int32_t cnt = 0;
  uint8_t *ip = config.cdma_server_ip;
  int32_t port = config.cdma_port;

  io_printf("%2d.ip      :%d.%d.%d.%d\r\n", cnt++, ip[0], ip[1], ip[2], ip[3]);
  io_printf("%2d.port    :%d\r\n", cnt++, port);
  io_printf("%2d.model   :%s\r\n", cnt++, ITEM_LIST(config.cdma_model, cdmaModellList));
  return cnt;
}

int32_t print_net_ntle_set(void)
{
  int32_t cnt = 0;
  uint8_t *ip = config.cdma_server_ip;
  int32_t port = config.cdma_port;

  io_printf("%2d.ip      :%d.%d.%d.%d\r\n", cnt++, ip[0], ip[1], ip[2], ip[3]);
  io_printf("%2d.port    :%d\r\n", cnt++, port);
  io_printf("%2d.model   :%s\r\n", cnt++, ITEM_LIST(config.cdma_model, cdmaModellList));
  io_printf("%2d.VPN     :%s\r\n", cnt++, ITEM_LIST(config.vpn_use, enableList));
  return cnt;
}


int32_t print_net_direct_set(void)
{
  int32_t cnt = 0;

  io_printf("%2d.baud     :%d\r\n", cnt++, config.direct_baud);

  return cnt;
}



int32_t print_net_set(void)
{
  int32_t cnt = 0;

  io_printf("%2d.이더넷\r\n", cnt++);
  io_printf("%2d.CDMA\r\n", cnt++);
  io_printf("%2d.직접 통신\r\n", cnt++);

  return cnt;
}



int32_t print_menu_vhf(void)
{
  int32_t cnt = 0;

  io_printf("%2d.그룹         :%d\r\n", cnt++, config.vhf_group);
  io_printf("%2d.VHF ID       :%d\r\n", cnt++, config.vhf_id);
  io_printf("%2d.중계 ID      :%d\r\n", cnt++, config.vhf_repeater_id);
  io_printf("%2d.통제 ID      :%d\r\n", cnt++, config.vhf_host_id);
  io_printf("%2d.PTT 시간(ms) :%d\r\n", cnt++, config.vhf_ptt_delay);
  io_printf("%2d.VHF 가상 설정\r\n", cnt++);
  io_printf("%2d.VHF 루프 테스트\r\n", cnt++);
  io_printf("%2d.VHF 톤 테스트\r\n", cnt++);

  return cnt;
}

int32_t menu_net_vhf_vir_set(void) { return 0; }
int32_t menu_net_vhf_loop_test(void) { return 0; }

int32_t menu_net_vhf_tone_test(void) { return 0; }


#if 0 
int32_t menu_net_vhf(void)
{
  int32_t cnt;
  //  int32_t ret;
  int32_t dec;

  do
  {
    cnt = select_indexFromList( NULL, print_menu_vhf, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
    cnt--;

    switch (cnt)
    {
      case 0:  // 그룹
        if (input_decimal( 0, 255, &dec))
        {
          config.vhf_group = dec;
          WRITE_CFG(vhf_group);
        }
        break;
      case 1:  // id
        if (input_decimal( 0, 255, &dec))
        {
          config.vhf_id = dec;
          WRITE_CFG(vhf_id);
        }
        break;
      case 2:  // 중계
        if (input_decimal( 0, 255, &dec))
        {
          config.vhf_repeater_id = dec;
          WRITE_CFG(vhf_repeater_id);
        }
        break;
      case 3:  // 호스트
        if (input_decimal( 0, 255, &dec))
        {
          config.vhf_host_id = dec;
          WRITE_CFG(vhf_host_id);
        }
        break;
      case 4:  // ptt
        if (input_decimal( 0, 255, &dec))
        {
          config.vhf_ptt_delay = dec;
          WRITE_CFG(vhf_ptt_delay);
        }
        break;
      case 5:  //
        menu_net_vhf_vir_set();
        break;
      default:
        break;
    }

  } while (cnt != EXIT_PROGRAM);

  return cnt;
}
#endif
int32_t menu_net_protocol(void)
{

  int32_t status;
  int32_t choice;
  
  status = select_indexFromList( protocolList, NULL, _countof(protocolList), true,&choice);

  if(status ==MENU_OK)
  {
    config.aws_protocol_type = (eAWS_PROTOCOL_t)(choice);
    WRITE_CFG(aws_protocol_type);
  }

  return status;
}
int32_t print_menu_net(void)
{
  int32_t cnt = 0;
  char buff[50] = {0};

  make_comList(buff, sizeof(buff));

  io_printf("%2d.통신 방식:%s\r\n", cnt++, buff);
  io_printf("%2d.통신 설정\r\n", cnt++);
  io_printf("%2d.통신 프로토콜:%s\r\n", cnt++,
              ITEM_LIST(get_config_app()->aws_protocol_type, protocolList));
  io_printf("%2d.VHF\r\n", cnt++);

  return cnt;
}



/*
IP:192.168.1.1
PORT:1234
*/
#define ETH_REMOTE_SERVER_CNT 2
int32_t aws_eth_remote_server_info(void)
{
  int choice, status;
  char buff[ETH_REMOTE_SERVER_CNT][20];
  char *menu[ETH_REMOTE_SERVER_CNT];
  int menu_cnt = 0;

int a,b,c,d;
int dec;

  for (int i = 0; i < ETH_REMOTE_SERVER_CNT; i++)
  {
    menu[i] = buff[i];
  }

  while (1)
  {
    menu_cnt = 0;
    uint8_t *ip = get_config_app()->eth_server_ip;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "IP:%d.%d.%d.%d",ip[0], ip[1],
             ip[2], ip[3]);
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "PORT:%d", get_config_app()->eth_server_port);
    menu_cnt++;

    status = choice_menu(24, "수집서버 설정", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        status = input_ip(&a, &b, &c, &d);
        if(status != MENU_OK)
        break;
          config.eth_server_ip[0] = a;
          config.eth_server_ip[1] = b;
          config.eth_server_ip[2] = c;
          config.eth_server_ip[3] = d;
          WRITE_CFG(eth_server_ip);

        break;
      case 2:
        status = input_decimal_prompt("포트",&dec,0, 60000);
        if(status != MENU_OK)
        break;
          config.eth_server_port = dec;
          WRITE_CFG(eth_server_port);

        break;
    }

    if (status != MENU_OK)
    {
      break;
    }
  }

  return status;
}

#define ETH_DEFAUNT_CNT 4
int32_t aws_eth_default(void)
{
  int choice, status;
  char buff[ETH_DEFAUNT_CNT][20];
  char *menu[ETH_DEFAUNT_CNT];
  int menu_cnt = 0;

  int a, b, c, d;


  for (int i = 0; i < ETH_DEFAUNT_CNT; i++)
  {
    menu[i] = buff[i];
  }

  while (1)
  {
    menu_cnt = 0;
    uint8_t *ip = get_config_app()->eth_ip;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "IP     :%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    menu_cnt++;
    ip = get_config_app()->eth_subnet;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "SUBNET :%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    menu_cnt++;
    ip = get_config_app()->eth_gateway;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "GATEWAY:%d.%d.%d.%d", ip[0], ip[1], ip[2],
             ip[3]);
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "PORT   :%d",
             get_config_app()->eth_local_port);
    menu_cnt++;

    status = choice_menu(24, "이더넷 기본 설정", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
      {
        case 1:  // ip
          status = input_ip( &a, &b, &c, &d);
          if(status != MENU_OK)
          break;

            config.eth_ip[0] = a;
            config.eth_ip[1] = b;
            config.eth_ip[2] = c;
            config.eth_ip[3] = d;
            WRITE_CFG(eth_ip);

          break;
        case 2:  // subnet
          status = input_ip(&a, &b, &c, &d);
          if (status != MENU_OK)
            break;
          config.eth_subnet[0] = a;
          config.eth_subnet[1] = b;
          config.eth_subnet[2] = c;
          config.eth_subnet[3] = d;
          WRITE_CFG(eth_subnet);

          break;
        case 3:  // gateway
          status = input_ip(&a, &b, &c, &d);
          if (status != MENU_OK)
            break;
          config.eth_gateway[0] = a;
          config.eth_gateway[1] = b;
          config.eth_gateway[2] = c;
          config.eth_gateway[3] = d;
          WRITE_CFG(eth_gateway);

          break;
        case 4:  // port
          status = input_decimal_prompt("포트",&a,0,100000);
          if(status != MENU_OK)
          break;
            config.eth_local_port = a;
            WRITE_CFG(eth_local_port);
               break;
      }

    if (status != MENU_OK)
    {
      break;
    }
  }

  return status;
}

#define ETH_CFG_CNT 3
int32_t aws_network_config_eth(void)
{
  int choice, status;

  char buff[ETH_CFG_CNT][20];


  char *menu[ETH_CFG_CNT];
  int menu_cnt = 0;


  for (int i = 0; i < ETH_CFG_CNT; i++)
  {
    menu[i] = buff[i];
  }

  while (1)
  {
    menu_cnt = 0;

    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "방식:%s", ITEM_LIST(get_config_app()->eth_mode, ethModeList));
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "수집 서버 정보");
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "기본 구성");
    menu_cnt++;

    status = choice_menu(24, "네트워크", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        status = choice_menu(30,"이더넷 방식",(char**)ethModeList,_countof(ethModeList),&choice);
        if(status != MENU_OK)
        {
          break;
        }
        config.eth_mode = (eETH_MODE_t)(choice-1);
        WRITE_CFG(eth_mode);
        break;
      case 2:
        status = aws_eth_remote_server_info();
        break;
      case 3:
        status = aws_eth_default();
        break;
    }

    if (status == MENU_ABORT)
    {
      break;
    }
  }

  return status;
}

#define AWS_CDMA_CNT 4
int32_t aws_network_config_cdma(void)
{
  int choice, status;
  char buff[AWS_CDMA_CNT][30];
  char *menu[AWS_CDMA_CNT];
  int menu_cnt = 0;

  int a, b, c, d;
  int dec;

  for (int i = 0; i < AWS_CDMA_CNT; i++)
  {
    menu[i] = buff[i];
  }

  do
  {
    menu_cnt = 0;
    uint8_t *ip = get_config_app()->cdma_server_ip;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]),      "IP   :%d.%d.%d.%d", ip[0], ip[1], ip[2],
             ip[3]);
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "PORT :%d", get_config_app()->cdma_port);
    menu_cnt++;

    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "MODEL:%s",
             ITEM_LIST(config.cdma_model, cdmaModellList));
    menu_cnt++;
    if (config.cdma_model == eCDMA_NTLE9607)
    {
      snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "VPN   :%s",
               ITEM_LIST(config.vpn_use, enableList));
    menu_cnt++;
    }

    status = choice_menu(24, "CDMA 설정", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      break;

    switch (choice)
    {
      case 1:
       status = input_ip(&a, &b, &c, &d);
       if(status !=MENU_OK)
       break;
          config.cdma_server_ip[0] = a;
          config.cdma_server_ip[1] = b;
          config.cdma_server_ip[2] = c;
          config.cdma_server_ip[3] = d;
          WRITE_CFG(cdma_server_ip);

        break;
      case 2:
        status = input_decimal_prompt("포트",&dec,0,10000);
        if(status != MENU_OK)
        break;

          config.cdma_port = dec;
          WRITE_CFG(cdma_port);

        break;
      case 3:  // 모델
        status = choice_menu(30,"CDMA 모델",(char**)cdmaModellList,  _countof(cdmaModellList), &choice);
        if( status == MENU_ABORT)
        break;
        if(status ==MENU_BACK)
        continue;

          config.cdma_model = (eCDMA_MODEL_t)(choice - 1);
          WRITE_CFG(cdma_model);


        break;
      case 4:
        status = choice_enable(&get_config_app()->vpn_use);
        if (status == MENU_ABORT)
          break;
        if (status == MENU_BACK)
          continue;
        WRITE_CFG(vpn_use);
        break;
    }

   }while(1);
  
  return status;
}

int32_t aws_network_config_direct(void)
{
  int choice, status;
  char buff[AWS_CDMA_CNT][20];
  char *menu[AWS_CDMA_CNT];
  int menu_cnt = 0;


  int dec;

  for (int i = 0; i < AWS_CDMA_CNT; i++)
  {
    menu[i] = buff[i];
  }

  while (1)
  {
    menu_cnt = 0;
    uint8_t *ip = get_config_app()->cdma_server_ip;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "통신 속도   :%d", config.direct_baud);
    menu_cnt++;

    status = choice_menu(24, "직접통신(RS232)", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        status = input_decimal_prompt("BAUDRATE",&dec,1200,115200);
        if(status== MENU_OK)
        {
          config.direct_baud = dec;
          WRITE_CFG(direct_baud);
        }
        break;
    }

    if (status != MENU_OK)
    {
      break;
    }
  }

  return status;
}

int32_t aws_network_config(void)
{
  int choice, status;

  char *menu[] = {"이더넷", "CDMA", "직접통신"};

  while (1)
  {
    status = choice_menu(AWS_MENU_NET_WIDTH, "상세 설정", menu, _countof(menu), &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        status = aws_network_config_eth();
        break;
      case 2:
        status = aws_network_config_cdma();
        break;
      case 3:
        status = aws_network_config_direct();
      break;
    }

    if(status ==MENU_ABORT)
    {
      break;
    }
  }

  return status;
}

#define MENU_CNT 4
int32_t aws_network_use(void)
{
  int choice, status;

  char buff[MENU_CNT][20];
  char buffer[20];

  char *menu[MENU_CNT];
  int menu_cnt = 0;


  for (int i = 0; i < MENU_CNT; i++)
  {
    menu[i] = buff[i];
  }

  while (1)
  {
    menu_cnt = 0;

    make_comList(buffer, sizeof(buffer));
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "이더넷  :%s",
             ITEM_LIST((int)get_config_app()->eth_use, enableList));
    menu_cnt++;

    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "CDMA    :%s",
             ITEM_LIST((int)get_config_app()->cdma_use, enableList));
    menu_cnt++;

    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "직접통신:%s",
             ITEM_LIST((int)get_config_app()->direct_use, enableList));
    menu_cnt++;

    status = choice_menu(24, "사용 여부", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        status = choice_enable(&get_config_app()->eth_use);
        if (status == MENU_ABORT)
          break;
        if (status == MENU_BACK)
          continue;
        WRITE_CFG(eth_use);
        break;
      case 2:
        status = choice_enable(&get_config_app()->cdma_use);
        if (status == MENU_ABORT)
          break;
        if (status == MENU_BACK)
          continue;
        if (get_config_app()->direct_use)
        {
          get_config_app()->direct_use = 0;
          WRITE_CFG(direct_use);
        }
        WRITE_CFG(cdma_use);
        break;
      case 3:
        status = choice_enable(&get_config_app()->direct_use);
        if (status == MENU_ABORT)
          break;
        if (status == MENU_BACK)
          continue;
        if(get_config_app()->cdma_use)
        {
          get_config_app()->cdma_use = 0;
          WRITE_CFG(cdma_use);
        }
        WRITE_CFG(direct_use);
        break;
    }

    if (status != MENU_OK)
    {
      break;
    }
  }

  return status;
}

int aws_menu_network(void)
{
  int choice, status;

  char buff[MENU_CNT][30];
  char buffer[50];

  char *menu[MENU_CNT];
  int menu_cnt = 0;


  for (int i = 0; i < MENU_CNT; i++)
  {
    menu[i] = buff[i];
  }

  while (1)
  {
    menu_cnt = 0;

    make_comList(buffer, sizeof(buffer));
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "통신 방식:%s",buffer);
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "통신 설정");
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "통신 프로토콜:%s",
             ITEM_LIST(get_config_app()->aws_protocol_type, protocolList));
      menu_cnt++; 

    status = choice_menu(24, "네트워크", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        status = aws_network_use();
        break;
      case 2:
        status = aws_network_config();
         break;
      case 3:
        status = choice_menu(40,"AWS 프로토콜",(char **)protocolList,_countof(protocolList),&choice);
        if (status == MENU_ABORT)
        {
          break;
        }
        config.aws_protocol_type = (eAWS_PROTOCOL_t)(choice - 1);
        WRITE_CFG(aws_protocol_type);
        break;
        break;
    }

    if (status == MENU_ABORT)
    {
      break;
    }
  }

  return status;
}