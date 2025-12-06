
/*
This file must be encoded in EUC-KR
*/
#include "cli_input.h"
#include "config_app.h"
#include "console_define.h"
#include "console_scanf.h"
#include "console_utile.h"
#include "const_string.h"
#include "debug_io.h"
#include "util_memory.h"

#define AWS_MENU_NET_WIDTH 30

const char *eth_mode_list[] = {"Client", "Server"};
const char *cdma_model_list[] = {"NTLE9607", "TX700"};
const char *protocol_list[] = {"KMA2", "KMA3"};



int32_t input_ip(int *a, int *b, int *c, int *d)
{
  int32_t status;

  while(1)
  {
    debug_printf("IP(xxx.xxx.xxx.xxx)");
    debug_printf("입력:");
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
    debug_printf("값이 입력되지 않았습니다.");
  }

  return status;
}

int32_t input_mac(int *a, int *b, int *c, int *d,int *e,int *f)
{
  int32_t status;

  while (1)
  {
    debug_printf("MAC(xxx.xxx.xxx.xxx.xxx.xxx)");
    debug_printf("입력:");
    status = cli_scanf_s("%d.%d.%d.%d.%d.%d", a, b, c, d,e,f);

    if (status == CLI_KEYCODE_CTRL_C)
    {
      status = MENU_BACK;
      break;
    }
    else if (status == CLI_KEYCODE_CTRL_Q)
    {
      status = MENU_ABORT;
      break;
    }
    else if (status == 4)
    {
      status = MENU_OK;
      break;
    }
    debug_printf("값이 입력되지 않았습니다.");
  }

  return status;
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
    uint8_t *ip = get_config_app()->eth_remote_server_ip;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "IP:%d.%d.%d.%d",ip[0], ip[1],
             ip[2], ip[3]);
    menu_cnt++;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "PORT:%d", get_config_app()->eth_remote_server_port);
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
          config.eth_remote_server_ip[0] = a;
          config.eth_remote_server_ip[1] = b;
          config.eth_remote_server_ip[2] = c;
          config.eth_remote_server_ip[3] = d;
          WRITE_CFG(eth_remote_server_ip);
        break;
      case 2:
        status = input_decimal_prompt("포트",&dec,0, 60000);
        if(status != MENU_OK)
        break;
          config.eth_remote_server_port = dec;
          WRITE_CFG(eth_remote_server_port);
        break;
    }

    if (status == MENU_ABORT)
    {
      break;
    }
  }

  return status;
}

#define ETH_DEFAUNT_CNT 5
int32_t aws_eth_default(void)
{
  int choice, status;
  char buff[ETH_DEFAUNT_CNT][30];
  char *menu[ETH_DEFAUNT_CNT];
  int menu_cnt = 0;
  int a, b, c, d,e,f;
  uint8_t *p_mac;

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
    p_mac = get_config_app()->eth_mac;
    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "MAC:%d.%d.%d.%d.%d.%d", p_mac[0], p_mac[1], p_mac[2],
             p_mac[3], p_mac[4], p_mac[5]);
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
            debug_printf_color(IO_COLOR_RED, "리셋 후 적용됩니다\r\n");
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
          debug_printf_color(IO_COLOR_RED, "리셋 후 적용됩니다\r\n");
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
          debug_printf_color(IO_COLOR_RED, "리셋 후 적용됩니다\r\n");
          break;
        case 4: // mac
          status = input_mac(&a, &b, &c, &d,&e,&f);
          if (status != MENU_OK)
            break;
          config.eth_mac[0] = a;
          config.eth_mac[1] = b;
          config.eth_mac[2] = c;
          config.eth_mac[3] = d;
          config.eth_mac[4] = e;
          config.eth_mac[5] = f;
          WRITE_CFG(eth_mac);
          debug_printf_color(IO_COLOR_RED, "리셋 후 적용됩니다\r\n");
          break;
        case 5:  // port
          status = input_decimal_prompt("포트",&a,0,100000);
          if(status != MENU_OK)
          break;
            config.eth_local_port = a;
            WRITE_CFG(eth_local_port);
            debug_printf_color(IO_COLOR_RED, "리셋 후 적용됩니다\r\n");
            break;
      }

    if (status == MENU_ABORT)
    {
      break;
    }
  }

  return status;
}

#define ETH_DEFAULT    0
#define ETH_COM_TYPE   1
#define ETH_SERVER_CFG 2

#define ETH_CFG_CNT     3

int32_t aws_network_config_eth(void)
{
  int choice, status;
  char buff[ETH_CFG_CNT][40];
  char *menu[ETH_CFG_CNT];
  int menu_cnt = 0;


  for (int i = 0; i < ETH_CFG_CNT; i++)
  {
    menu[i] = buff[i];
  }

  while (1)
  {
    menu_cnt = 0;
    snprintf(buff[menu_cnt++], sizeof(buff[menu_cnt]), "기본 구성");

    snprintf(buff[menu_cnt++], sizeof(buff[menu_cnt]), "방식:%s", ITEM_LIST(get_config_app()->eth_mode, eth_mode_list));
 
    if (get_config_app()->eth_mode == eETH_MODE_SERVER)
    {
      snprintf(buff[menu_cnt++], sizeof(buff[menu_cnt]), "수집 서버 정보");

    }

    status = choice_menu(24, "네트워크", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      break;
    switch (choice)
    {
    case ETH_DEFAULT:
      status = aws_eth_default();
      break;
    case ETH_COM_TYPE:
      status = choice_menu(30, "이더넷 방식", (char **)eth_mode_list, _countof(eth_mode_list), &choice);
      if (status != MENU_OK)
      {
        break;
      }
      config.eth_mode = (eETH_MODE_t)(choice - 1);
      WRITE_CFG(eth_mode);
      debug_printf_color(IO_COLOR_RED, "리셋 후 적용됩니다\r\n");
      break;
    case ETH_SERVER_CFG:
      status = aws_eth_remote_server_info();
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
             ITEM_LIST(config.cdma_model, cdma_model_list));
    menu_cnt++;
    if (config.cdma_model == eCDMA_NTLE9607)
    {
      snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "VPN   :%s",
               ITEM_LIST(config.cdma_vpn_active, enableList));
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
        status = choice_menu(30,"CDMA 모델",(char**)cdma_model_list,  _countof(cdma_model_list), &choice);
        if( status != MENU_OK)
        break;
          config.cdma_model = (eCDMA_MODEL_t)(choice-1);
          WRITE_CFG(cdma_model);
          debug_printf_color(IO_COLOR_RED, "리셋 후 적용됩니다\r\n");
          break;
      case 4:
        status = choice_enable(&get_config_app()->cdma_vpn_active);
        if (status != MENU_OK)
          break;
        WRITE_CFG(cdma_vpn_active);
      
        break;
    }

    if (status == MENU_ABORT)
      break;

  } while (1);

  return status;
}

#define LABEL_W 24
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

    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "통신 속도   :%s",ITEM_LIST(config.direct_baud_index,baud_list_eng));
    menu_cnt++;

    status = choice_menu(LABEL_W, "직접통신(RS232)", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      break;

    switch (choice)
    {
      case 1:
        choice = get_config_app()->direct_baud_index;
        status = choice_menu(30, "통신 속도", (char **)baud_list_eng, _countof(baud_list_eng), &choice);
        if(status != MENU_OK)
        break;

        config.direct_baud_index = (eUART_BAUD_t)choice;
        WRITE_CFG(direct_baud_index);
        debug_printf_color(IO_COLOR_RED, "리셋 후 적용됩니다\r\n");
        break;
    }

    if (status == MENU_ABORT)
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
      break;

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

  char buff[MENU_CNT][50];
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
             ITEM_LIST((int)get_config_app()->eth_active, enableList));
    menu_cnt++;

    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "CDMA    :%s",
             ITEM_LIST((int)get_config_app()->cdma_active, enableList));
    menu_cnt++;

    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "직접통신:%s",
             ITEM_LIST((int)get_config_app()->direct_active, enableList));
    menu_cnt++;

    status = choice_menu(24, "사용 여부", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        status = choice_enable(&get_config_app()->eth_active);
        if (status != MENU_OK)
          break;
        WRITE_CFG(eth_active);
        debug_printf_color(IO_COLOR_RED, "리셋 후 적용됩니다\r\n");
        break;
      case 2:
        status = choice_enable(&get_config_app()->cdma_active);
        if (status != MENU_OK)
          break;
        if (get_config_app()->direct_active)
        {
          get_config_app()->direct_active = 0;
          WRITE_CFG(direct_active);
        }
        WRITE_CFG(cdma_active);
        debug_printf_color(IO_COLOR_RED, "리셋 후 적용됩니다\r\n");
        break;
      case 3:
        status = choice_enable(&get_config_app()->direct_active);
        if (status != MENU_OK)
          break;
        if(get_config_app()->cdma_active)
        {
          get_config_app()->cdma_active = 0;
          WRITE_CFG(cdma_active);
        }
        WRITE_CFG(direct_active);
        debug_printf_color(IO_COLOR_RED, "리셋 후 적용됩니다\r\n");
        break;
    }

    if (status == MENU_ABORT)
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
             ITEM_LIST(get_config_app()->aws_protocol_type, protocol_list));
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
        status = choice_menu(40,"AWS 프로토콜",(char **)protocol_list,_countof(protocol_list),&choice);
        if (status != MENU_OK)
          break;

          config.aws_protocol_type = (eAWS_PROTOCOL_t)(choice - 1);
        WRITE_CFG(aws_protocol_type);
        break;
    }

    if (status == MENU_ABORT)
    {
      break;
    }
  }

  return status;
}