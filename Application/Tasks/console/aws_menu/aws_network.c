#include "console_define.h"
#include "console_utile.h"
#include "dev_io.h"
#include "util_memory.h"

#include "config_app.h"

#include "console_scanf.h"

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
  int32_t cnt;

  do
  {
    cnt = select_indexFromList( NULL, print_net_use, 0, false);
    if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK)
    {
      break;
    }
    cnt--;

    switch (cnt)
    {
      case 0:
        if (input_use( &get_config_app()->eth_use))
        {
          WRITE_CFG(eth_use);
        }
        break;
      case 1:
        if (input_use( &get_config_app()->cdma_use))
        {
          if (get_config_app()->cdma_use)
          {
            config.direct_use = 0;
            WRITE_CFG(direct_use);
          }

          WRITE_CFG(cdma_use);
        }
        break;
      case 2:
        if (input_use( &get_config_app()->direct_use))
        {
          if (get_config_app()->direct_use)
          {
            config.cdma_use = 0;
            WRITE_CFG(cdma_use);
          }
          WRITE_CFG(direct_use);
        }
        break;
    }
  } while (1);

  return cnt;
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

int32_t menu_net_eth_remote_set(void)
{
  int32_t cnt;
  int32_t a, b, c, d;
  int32_t dec;

  do
  {
    cnt = select_indexFromList( NULL, print_net_eth_remote_set, 0, false);
    if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
    {
      return cnt;
    }

    cnt--;
    switch (cnt)
    {
      case 0:
        io_printf("xxx.xxx.xxx.xxx:");
        if (console_scanf("%d.%d.%d.%d", &a, &b, &c, &d) == 4)
        {
          config.eth_server_ip[0] = a;
          config.eth_server_ip[1] = b;
          config.eth_server_ip[2] = c;
          config.eth_server_ip[3] = d;
          WRITE_CFG(eth_server_ip);
        }
        break;
      case 1:
        if (input_decimal( 0, 60000, &dec))
        {
          config.eth_server_port = dec;
          WRITE_CFG(eth_server_port);
        }
        break;
    }
  } while (1);
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

int32_t menu_net_eth_default_set(void)
{
  int32_t cnt;
  int32_t a, b, c, d;

  do
  {
    cnt = select_indexFromList( NULL, print_net_eth_default_set, 0, false);
    if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
    {
      return cnt;
    }

    cnt--;
    switch (cnt)
    {
      case 0:  // ip
        io_printf("xxx.xxx.xxx.xxx:");
        if (console_scanf("%d.%d.%d.%d", &a, &b, &c, &d) == 4)
        {
          config.eth_ip[0] = a;
          config.eth_ip[1] = b;
          config.eth_ip[2] = c;
          config.eth_ip[3] = d;
          WRITE_CFG(eth_ip);
        }
        break;
      case 1:  // subnet
        io_printf("xxx.xxx.xxx.xxx:");
        if (console_scanf("%d.%d.%d.%d", &a, &b, &c, &d) == 4)
        {
          config.eth_subnet[0] = a;
          config.eth_subnet[1] = b;
          config.eth_subnet[2] = c;
          config.eth_subnet[3] = d;
          WRITE_CFG(eth_subnet);
        }
        break;
      case 2:  // gateway
        io_printf("xxx.xxx.xxx.xxx:");
        if (console_scanf("%d.%d.%d.%d", &a, &b, &c, &d) == 4)
        {
          config.eth_gateway[0] = a;
          config.eth_gateway[1] = b;
          config.eth_gateway[2] = c;
          config.eth_gateway[3] = d;
          WRITE_CFG(eth_gateway);
        }
        break;
      case 3:  // port
        io_printf("x:");
        if (console_scanf("%d%d", &a) == 1)
        {
          config.eth_local_port = a;

          WRITE_CFG(eth_local_port);
        }
        break;
    }
  } while (1);
}

int32_t menu_net_eth_mode_set(void)
{
  int32_t cnt;


  cnt = select_indexFromList( ethModeList, NULL, _countof(ethModeList), true);
  if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
  {
    return cnt;
  }

  cnt--;

      config.eth_mode = (eETH_MODE_t)cnt;
      WRITE_CFG(eth_mode);

  return cnt;
}

int32_t menu_net_eth_set(void)
{
  int32_t cnt;

  const menu_func menu[] = {menu_net_eth_mode_set, menu_net_eth_remote_set,
                            menu_net_eth_default_set};
  do
  {
    cnt = select_indexFromList( NULL, print_net_eth_set, 0, false);
    if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    switch (cnt)
    {
      case 0:
        cnt = menu_net_eth_mode_set();
        break;
      default:
        cnt = menu[cnt]();
        if (cnt == EXIT_PROGRAM)
        {
          return cnt;
        }
        break;
    }

  } while (1);
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


int32_t menu_net_cdma_set(void)
{
  int32_t cnt;
  int32_t a, b, c, d;
  int32_t dec;
  int32_t (*menu_set)(void) = print_net_cdma_set;


    do
    {
      if (get_config_app()->cdma_model == eCDMA_NTLE9607)
      {
        menu_set = print_net_ntle_set;
      }
      else
      {
        menu_set = print_net_cdma_set;
      }
      cnt = select_indexFromList( NULL, menu_set, 0, false);
      if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
      {
        return cnt;
      }

      cnt--;
      switch (cnt)
      {
        case 0:
          io_printf("xxx.xxx.xxx.xxx:");
          if (console_scanf("%d.%d.%d.%d", &a, &b, &c, &d) == 4)
          {
            config.cdma_server_ip[0] = a;
            config.cdma_server_ip[1] = b;
            config.cdma_server_ip[2] = c;
            config.cdma_server_ip[3] = d;
            WRITE_CFG(cdma_server_ip);
          }
          break;
        case 1:
          if (input_decimal( 0, 60000, &dec))
          {
            config.cdma_port = dec;
            WRITE_CFG(cdma_port);
          }
          break;
        case 2:  // 모델
          cnt = select_indexFromList( cdmaModellList, NULL, _countof(cdmaModellList), true);
          if (cnt > 0)
          {
            cnt--;
            config.cdma_model = (eCDMA_MODEL_t)cnt;
            WRITE_CFG(cdma_model);
          }
          break;
        case 3:
          if (input_use( &get_config_app()->vpn_use))
          {
            WRITE_CFG(vpn_use);
          }
          break;
      }
    } while (1);
}

int32_t print_net_direct_set(void)
{
  int32_t cnt = 0;

  io_printf("%2d.baud     :%d\r\n", cnt++, config.direct_baud);

  return cnt;
}

int32_t menu_net_direct_set(void)
{
  int32_t cnt;
  int32_t dec;

  do
  {
    cnt = select_indexFromList( NULL, print_net_direct_set, 0, false);
    if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK)
    {
      return cnt;
    }

    cnt--;
    switch (cnt)
    {
      case 0:  // baud
        if (input_decimal( 0, 115200, &dec))
        {
          config.direct_baud = dec;
          WRITE_CFG(direct_baud);
        }
        break;
    }
  } while (1);
}

int32_t print_net_set(void)
{
  int32_t cnt = 0;

  io_printf("%2d.이더넷\r\n", cnt++);
  io_printf("%2d.CDMA\r\n", cnt++);
  io_printf("%2d.직접 통신\r\n", cnt++);

  return cnt;
}

int32_t menu_net_set(void)
{
  int32_t cnt;
  const menu_func menu[] = {menu_net_eth_set, menu_net_cdma_set, menu_net_direct_set};
  do
  {
    cnt = select_indexFromList( NULL, print_net_set, 0, false);
    if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
    {
      break;
    }
    cnt--;
    cnt = menu[cnt]();
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      break;
    }
  } while(1);

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

int32_t menu_net_protocol(void)
{
  int32_t cnt;

  cnt = select_indexFromList( protocolList, NULL, _countof(protocolList), true);

  if (cnt > 0)
  {
    config.aws_protocol_type = (eAWS_PROTOCOL_t)(cnt - 1);
    WRITE_CFG(aws_protocol_type);
  }

  return cnt;
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

int32_t menu_network(void)
{
  int32_t cnt;

  const menu_func menu[] = {menu_net_use, menu_net_set, menu_net_protocol, menu_net_vhf};

  while (1)
  {
    cnt = select_indexFromList( NULL, print_menu_net, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      break;
    }
    cnt--;
    cnt = menu[cnt]();
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      break;
    }
  }

  return cnt;
}


int aws_menu_network(void)
{
  #if 0 
  int choice, status;
  int max_number;
  char* menu[] = {"통신방식", "통신설정", "통신 프로토콜"};

  while (1)
  {
    status = choice_menu(24, "네트워크", menu, _countof(menu), &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        aws_menu_display();
        break;
      case 2:
        aws_menu_sensor();
        break;
    }
  }
#else
  menu_network();
#endif
      return 0;
}