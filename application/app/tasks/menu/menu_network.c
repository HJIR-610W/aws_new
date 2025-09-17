
#include "menu_network.h"

#include "app_key.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "const_string.h"
#include "console_utile.h"
#include "menu_handler.h"
#include "view_driver.h"
#include "util_memory.h"
#include "common\const_string.h"
#include "app_screen.h"


#define SCREEN_COLS 21
#define NETWORK_WD 8

/*
Network
*Interface Type
 Settings Type
 Protocol:KMA3

Interface Type
*Ethernet:ENB
 CDMA    :DSB
 Direct  :ENB

 Settings Type
 *Ethernet
  CDMA
  Direct

Ethernet
*Mode :Server
 Local IP
 Subnet
 Gateway
 Local Port:9000
 Remote IP
 Remote Port:6000

CDMA
*Server IP
 Port :0
 Model:NTLE9607
 VPN  :Disabled

Direct
*Baud Rate:19200


 */


 #define NETWORK_ETH_EN 0
#define NETWORK_CDMA_EN 1
#define NETWORK_DIRECT_EN 2
void draw_network_mode_main_page(screen_menu_t *p_win)
{
  uint8_t eth_active = get_config_app()->eth_active;
  uint8_t cdma_active = get_config_app()->cdma_active;
  uint8_t direct_active = get_config_app()->direct_active;

  screen_menu_start(p_win);
  screen_menu_printf(p_win, NETWORK_ETH_EN, "%-*s:%s", NETWORK_WD, "Ethernet", ITEM_LIST(eth_active, enable_list_eng));
  screen_menu_printf(p_win, NETWORK_CDMA_EN, "%-*s:%s", NETWORK_WD, "CDMA", ITEM_LIST(cdma_active, enable_list_eng));
  screen_menu_printf(p_win, NETWORK_DIRECT_EN, "%-*s:%s", NETWORK_WD, "Direct", ITEM_LIST(direct_active, enable_list_eng));
  screen_menu_clear(p_win);
}


#define NETWORK_MENU_NET_EN 0
#define NETWORK_MENU_ETH_CONFIG 1
#define NETWORK_MENU_CDMA_CONFIG 2
#define NETWORK_MENU_DIRECT_CONFIG 3
#define NETWORK_MENU_AWS_PROTOCOL 4

//Net mode:[DIRE][ETH]
void draw_network_main_page(screen_menu_t* p_win)
{
  char buff[13]={"Not Used"};
  int32_t len=0;

  screen_menu_start(p_win);
  
  if(get_config_app()->eth_active)
  {
    len += snprintf(&buff[len],sizeof(buff),"%s","[ETH]");
  }
   if (get_config_app()->cdma_active)
  {
    len += snprintf(&buff[len], sizeof(buff)-len, "%s", "[CDMA]");
  }
   if (get_config_app()->direct_active)
  {
    len += snprintf(&buff[len], sizeof(buff)-len, "%s", "[DRCT]");
  }

  screen_menu_printf(p_win, NETWORK_MENU_NET_EN, "%-*s:%s", NETWORK_WD, "Net Mode", buff);
  screen_menu_printf(p_win, NETWORK_MENU_ETH_CONFIG, "%-*s", NETWORK_WD, "Ethernet");
  screen_menu_printf(p_win, NETWORK_MENU_CDMA_CONFIG, "%-*s", NETWORK_WD, "CDMA");
  screen_menu_printf(p_win, NETWORK_MENU_DIRECT_CONFIG, "%-*s", NETWORK_WD, "Direct");
  screen_menu_printf(p_win, NETWORK_MENU_AWS_PROTOCOL, "%-*s:%s", NETWORK_WD, "Protocol",ITEM_LIST(get_config_app()->aws_protocol_type, protocol_list_eng));
  screen_menu_clear(p_win);
}

char user_fmt[2][21] = {{"    000.000.000     "}, {"    000.000.000     "}};
typedef struct pos_s
{
  char row;
  char col;
}lcd_pos_t;

lcd_pos_t user_fmt_pos[20];
int user_fmt_pos_cnt=0;

/*
"    MAC Address     "
"    000.000.000     "
"    000.000.000     "
*/

extern size_t utf8_strlen(const char* s);
int32_t input_mac_address(const char *title, uint8_t *mac)
{
  int row_offset = 1;
  int total_width = 20; // 좌우 여백 및 메뉴 번호 고려
  int key;
  int cur_row_pos;
  int cur_col_pos;
  int pos_cnt=0;
  // 타이틀 가운데 정렬
  int title_len = utf8_strlen(title);
  int title_padding = (total_width - 2 - title_len) / 2;

  int blink_state = 1;
  char display_char;

  screen_clear();
  screen_printf(0, title_padding, "%s", title);

  for (int row = 0; row < 2; row++)
  {
    for (int col = 0; col < 20; col++)
    {
      if (user_fmt[row][col] == '0')
      {
        user_fmt_pos[user_fmt_pos_cnt].row = row;
        user_fmt_pos[user_fmt_pos_cnt].col = col;
        user_fmt_pos_cnt++;
      }
    }
  }

  snprintf(user_fmt[0], sizeof(user_fmt[0]),"    %03d.%03d.%03d     ", mac[0], mac[1], mac[2]);
  snprintf(user_fmt[1],sizeof(user_fmt[0]), "    %03d.%03d.%03d     ", mac[3], mac[4], mac[5]);

  screen_printf(row_offset, 0, "%s", user_fmt[0]);
  screen_printf(1 + row_offset, 0, "%s", user_fmt[1]);

  cur_col_pos = user_fmt_pos[0].col;
  cur_row_pos = user_fmt_pos[0].row+row_offset;

  screen_refresh();

  while(1)
  {
    screen_refresh();

    key = get_menu_key(500);
    blink_state = !blink_state;

    screen_printf(row_offset, 0, "%s", user_fmt[0]);
    screen_printf(row_offset+1, 0, "%s", user_fmt[1]);
    display_char = blink_state ? user_fmt[cur_row_pos-row_offset][cur_col_pos] : ' ';
    screen_put_ch(cur_row_pos, cur_col_pos, display_char);

    switch (key)
    {
    case KEY_CODE_CTRL_C:
      return MENU_BACK;

    case KEY_CODE_CTRL_Q:
      return MENU_ABORT;

    case KEY_CODE_ENTER:
    {
      int a, b, c;
      sscanf(user_fmt[0], "    %03d.%03d.%03d     ", &a, &b, &c);
      mac[0] = a;
      mac[1] = b;
      mac[2] = c;
      sscanf(user_fmt[1], "    %03d.%03d.%03d     ", &a, &b, &c);
      mac[3] = a;
      mac[4] = b;
      mac[5] = c;
      return MENU_OK;
    }
      break;
    case KEY_CODE_RIGHT:
      if (pos_cnt < user_fmt_pos_cnt-1)
      {
        pos_cnt++;
        cur_col_pos = user_fmt_pos[pos_cnt].col;
        cur_row_pos = user_fmt_pos[pos_cnt].row + row_offset;
        blink_state=1;
      }
            break;
      case KEY_CODE_LEFT:
        if (pos_cnt>0)
        {
          pos_cnt--;
          cur_col_pos = user_fmt_pos[pos_cnt].col;
          cur_row_pos = user_fmt_pos[pos_cnt].row + row_offset;
          blink_state = 1;
        }

      default :
       if (key >= '0' && key <= '9')
      {
         user_fmt[cur_row_pos-row_offset][cur_col_pos] = key;
         if (pos_cnt < user_fmt_pos_cnt - 1)
         {
           pos_cnt++;
           cur_col_pos = user_fmt_pos[pos_cnt].col;
           cur_row_pos = user_fmt_pos[pos_cnt].row + row_offset;
           blink_state = 1;
         }
     }
        break;
    }

  }

}


int32_t input_ip_address(const char *title, uint8_t *ip)
{
int a, b, c, d;
string_fmt_t strfmt;
int32_t status;

strfmt.fmt = "%3d.%3d.%3d.%3d";
snprintf(strfmt.data, sizeof(strfmt.data), "%03d.%03d.%03d.%03d", ip[0], ip[1], ip[2], ip[3]);

status = input_fmt(&strfmt, title);
if (status == MENU_OK)
{
  if (sscanf(strfmt.data, "%d.%d.%d.%d", &a, &b, &c, &d) == 4)
  {
    if (a >= 0 && a <= 255 && b >= 0 && b <= 255 && c >= 0 && c <= 255 && d >= 0 && d <= 255)
    {
      ip[0] = a;
      ip[1] = b;
      ip[2] = c;
      ip[3] = d;
    }
    else
    {
      status = MENU_ERROR;
    }
  }
  else
  {
    status = MENU_ERROR;
  }
  }

  return status;
}

// 이더넷 메뉴 정의
#define ETH_MENU_MODE        0
#define ETH_MENU_LOCAL_IP    1
#define ETH_MENU_SUBNET      2
#define ETH_MENU_GATEWAY     3
#define ETH_MENU_MAC         4
#define ETH_MENU_LOCAL_PORT  5
#define ETH_MENU_REMOTE_IP   6
#define ETH_MENU_REMOTE_PORT 7

void draw_eth_config_page(screen_menu_t* p_win)
{
  eETH_MODE_t eth_mode;

  eth_mode  = get_config_app()->eth_mode;
  screen_menu_start(p_win);
  screen_menu_printf(p_win, ETH_MENU_LOCAL_IP, "%-*s", NETWORK_WD, "Local IP");
  screen_menu_printf(p_win, ETH_MENU_SUBNET, "%-*s", NETWORK_WD, "Subnet");
  screen_menu_printf(p_win, ETH_MENU_GATEWAY, "%-*s", NETWORK_WD, "Gateway");
  screen_menu_printf(p_win, ETH_MENU_MAC, "%-*s", NETWORK_WD, "MAC");
  screen_menu_printf(p_win, ETH_MENU_MODE, "%-*s:%s", NETWORK_WD, "Mode", ITEM_LIST(get_config_app()->eth_mode, eth_mode_list_eng));
  if(eth_mode == eETH_MODE_SERVER)
  {
  screen_menu_printf(p_win, ETH_MENU_LOCAL_PORT, "%-*s:%d", NETWORK_WD, "Listen Port", get_config_app()->eth_local_port);
  }
  else
  {
  screen_menu_printf(p_win, ETH_MENU_REMOTE_IP, "%-*s", NETWORK_WD, "Remote Server IP");
  screen_menu_printf(p_win, ETH_MENU_REMOTE_PORT, "%-*s:%d", NETWORK_WD, "Remote Port",get_config_app()->eth_remote_server_port);
  }
  screen_menu_clear(p_win);
}

int32_t setup_eth_config(void)
{
  int32_t choice = 0;
  int32_t status;
  int32_t key;
  int32_t dec;
  int32_t index;
  screen_menu_t menu;

  screen_menu_create(&menu,  "Ethernet");


  while (1)
  {
    draw_eth_config_page(&menu);
    screen_refresh();

    key = get_menu_key(WAIT_FOREVER);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }

    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
        case ETH_MENU_MODE:
          choice = get_config_app()->eth_mode;
          status = input_combobox("Eth Mode",eth_mode_list_eng, _countof(eth_mode_list_eng), &choice);
          if (status == MENU_OK)
          {
            get_config_app()->eth_mode = (eETH_MODE_t)choice;
            WRITE_CFG(eth_mode);
            show_popup("Information", "Applied after reset");
          }
          break;
        case ETH_MENU_LOCAL_IP:
          status = input_ip_address("Local IP", get_config_app()->eth_ip);
          if (status == MENU_OK)
          {
            WRITE_CFG(eth_ip);
            show_popup("Information", "Applied after reset");
          }
          break;
        case ETH_MENU_SUBNET:
          status = input_ip_address("Subnet", get_config_app()->eth_subnet);
          if (status == MENU_OK)
          {
            WRITE_CFG(eth_subnet);
            show_popup("Information", "Applied after reset");
          }
          break;
        case ETH_MENU_GATEWAY:
          status = input_ip_address("Gateway", get_config_app()->eth_gateway);
          if (status == MENU_OK)
          {
            WRITE_CFG(eth_gateway);
            show_popup("Information", "Applied after reset");
          }
          break;
        case ETH_MENU_MAC:
          status = input_mac_address("MAC", get_config_app()->eth_mac);
          if (status == MENU_OK)
          {
            WRITE_CFG(eth_mac);
            show_popup("Information", "Applied after reset");
          }
          break;

          break;
        
        case ETH_MENU_LOCAL_PORT:
          dec = get_config_app()->eth_local_port;
          status = input_decimal("Local Port", 0, 65535, &dec);
          if (status == MENU_OK)
          {
            get_config_app()->eth_local_port = dec;
            WRITE_CFG(eth_local_port);
            show_popup("Information", "Applied after reset");
          }
          break;
        case ETH_MENU_REMOTE_IP:
          status = input_ip_address("Remote IP", get_config_app()->eth_remote_server_ip);
          if (status == MENU_OK)
          {
            WRITE_CFG(eth_remote_server_ip);
          }
          break;
        case ETH_MENU_REMOTE_PORT:
          dec = get_config_app()->eth_remote_server_port;
          status = input_decimal("Remote Port", 0, 65535, &dec);
          if (status == MENU_OK)
          {
            get_config_app()->eth_remote_server_port = dec;
            WRITE_CFG(eth_remote_server_port);
          }
          break;
      }

      if (status == MENU_ABORT)
        return status;
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}

// CDMA 메뉴 정의
#define CDMA_MENU_SERVER_IP 0
#define CDMA_MENU_PORT 1
#define CDMA_MENU_MODEL 2
#define CDMA_MENU_VPN 3

void draw_cdma_config_page(screen_menu_t* p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, CDMA_MENU_SERVER_IP, "%-*s", NETWORK_WD, "Remote Server IP");
  screen_menu_printf(p_win, CDMA_MENU_PORT, "%-*s:%d", NETWORK_WD, "Remote Port",get_config_app()->cdma_port);
  screen_menu_printf(p_win, CDMA_MENU_MODEL, "%-*s:%s", NETWORK_WD, "Model",ITEM_LIST(get_config_app()->cdma_model, cdma_model_list_eng));
  screen_menu_printf(p_win, CDMA_MENU_VPN, "%-*s:%s", NETWORK_WD, "VPN",ITEM_LIST(get_config_app()->cdma_vpn_active, enable_list_eng));
  screen_menu_clear(p_win);
}

int32_t setup_cdma_config(void)
{
  int32_t choice = 0;
  int32_t status;
  int32_t key;
  screen_menu_t menu;
  int32_t dec;
  int32_t index;

  screen_menu_create(&menu, "CDMA");
;

  while (1)
  {
    draw_cdma_config_page(&menu);
    screen_refresh();

    key = get_menu_key(WAIT_FOREVER);

    if (key == KEY_CODE_CTRL_Q)
    {
      break;
    }
    else if (key == KEY_CODE_CTRL_C)
    {
      break;
    }

    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
        case CDMA_MENU_SERVER_IP:
          status = input_ip_address("Server IP", get_config_app()->cdma_server_ip);
          if (status == MENU_OK)
          {
            WRITE_CFG(cdma_server_ip);
          }
          break;
        case CDMA_MENU_PORT:
          dec = get_config_app()->cdma_port;
          status = input_decimal("Port", 0, 65535, &dec);
          if (status == MENU_OK)
          {
            get_config_app()->cdma_port = dec;
            WRITE_CFG(cdma_port);
          }
          break;
        case CDMA_MENU_MODEL:
          choice = get_config_app()->cdma_model;
          status = input_combobox("Cdma Model",cdma_model_list_eng, _countof(cdma_model_list_eng), &choice);
          if (status == MENU_OK)
          {
            get_config_app()->cdma_model =(eCDMA_MODEL_t)choice;
            WRITE_CFG(cdma_model);
            show_popup("Information", "Applied after reset");
          }
          break;
        case CDMA_MENU_VPN:
          choice = get_config_app()->cdma_vpn_active;
          status = input_active("VPN", &choice);
          if (status == MENU_OK)
          {
            get_config_app()->cdma_vpn_active = choice;
            WRITE_CFG(cdma_vpn_active);
          }
          break;
      }

      if (status == MENU_ABORT)
        return status;
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}

// DIRECT 메뉴 정의
#define DIRECT_MENU_BAUD_RATE 0

void draw_direct_config_page(screen_menu_t* p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, DIRECT_MENU_BAUD_RATE, "%-*s:%s", NETWORK_WD, "Baud Rate",ITEM_LIST(get_config_app()->direct_baud_index,g_baud_list_eng));
  screen_menu_clear(p_win);
}





int32_t setup_direct_config(void)
{
  int32_t status;
  int32_t key;
  screen_menu_t menu;
  int32_t dec;
  int32_t index;
  

  screen_menu_create(&menu, "Direct");


  while (1)
  {
    draw_direct_config_page(&menu);
    screen_refresh();

    key = get_menu_key(WAIT_FOREVER);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }


    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
        case DIRECT_MENU_BAUD_RATE:
          index = get_config_app()->direct_baud_index;
          status = input_combobox("Baud Rate",g_baud_list_eng,_countof(g_baud_list_eng),&index);

          if (status == MENU_OK)
          {
            get_config_app()->direct_baud_index = (eUART_BAUD_t)index;
            WRITE_CFG(direct_baud_index);
            show_popup("Information", "Applied after reset");
          }
          break;
      }

      if (status == MENU_ABORT)
        return status;
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}

int32_t setup_menu_network_mode(void)
{
  int32_t choice = 0;
  int32_t status;
  int32_t key;
  screen_menu_t menu;
  int32_t index;

  screen_menu_create(&menu, "Network Mode");

  while (1)
  {
    draw_network_mode_main_page(&menu);
    screen_refresh();

    key = get_menu_key(WAIT_FOREVER);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }

    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
      case NETWORK_ETH_EN:
        choice = get_config_app()->eth_active;
        status = input_active("Use Ethernet?", &choice);
        if (status == MENU_OK)
        {
          get_config_app()->eth_active = choice;
          WRITE_CFG(eth_active);
          show_popup("Information", "Applied after reset");
        }
        break;
      case NETWORK_CDMA_EN:
        choice = get_config_app()->cdma_active;
        status = input_active("Use Cdma?", &choice);
        if (status == MENU_OK)
        {
          get_config_app()->cdma_active = choice;
          if (choice && get_config_app()->direct_active)
          {
            get_config_app()->direct_active = 0;
            WRITE_CFG(direct_active);
          }
          WRITE_CFG(cdma_active);
          show_popup("Information", "Applied after reset");
        }
        break;
      case NETWORK_DIRECT_EN:
        choice = get_config_app()->direct_active;
        status = input_active("Use Direct?", &choice);
        if (status == MENU_OK)
        {
          get_config_app()->direct_active = choice;
          if (choice && get_config_app()->cdma_active)
          {
            get_config_app()->cdma_active = 0;
            WRITE_CFG(cdma_active);
          }
          WRITE_CFG(direct_active);
          show_popup("Information", "Applied after reset");
        }
        break;
       }

      if (status == MENU_ABORT)
        return status;
    }
    else if (key != KEY_CODE_UNKNOWN)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}

int32_t setup_menu_network(void)
{
  int32_t choice = 0;
  int32_t status;
  int32_t key;
  screen_menu_t menu;
  int32_t index;

  screen_menu_create(&menu, "Network");


  while (1)
  {
    draw_network_main_page(&menu);
    screen_refresh();

    key = get_menu_key(WAIT_FOREVER);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }

    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
        case NETWORK_MENU_NET_EN:
        status = setup_menu_network_mode();
        break;
         case NETWORK_MENU_ETH_CONFIG:
          status = setup_eth_config();
          break;
        case NETWORK_MENU_CDMA_CONFIG:
          status = setup_cdma_config();
          break;
        case NETWORK_MENU_DIRECT_CONFIG:
          status = setup_direct_config();
          break;
        case NETWORK_MENU_AWS_PROTOCOL:
          choice = get_config_app()->aws_protocol_type;
          status = input_combobox("AWS Protocols",protocol_list_eng, _countof(protocol_list_eng), &choice);
          if (status != MENU_OK)
          break;

            get_config_app()->aws_protocol_type = (eAWS_PROTOCOL_t)choice;
            WRITE_CFG(aws_protocol_type);

          break;
      }

      if (status==MENU_ABORT)
      return status;
    }
    else if (key != KEY_CODE_UNKNOWN)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}