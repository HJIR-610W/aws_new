
#include "menu_network.h"

#include "app_key.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "console_utile.h"
#include "menu_handler.h"
#include "view_driver.h"
#include "util_memory.h"
#include "common\const_string.h"
#include "app_screen.h"
#define SCREEN_COLS 20
#define NETWORK_WD 10

#define M_PRINTF screen_menu_printf_row



#define NETWORK_MENU_ETH_USE 0
#define NETWORK_MENU_CDMA_USE 1
#define NETWORK_MENU_DIRECT_USE 2
#define NETWORK_MENU_ETH_CONFIG 3
#define NETWORK_MENU_CDMA_CONFIG 4
#define NETWORK_MENU_DIRECT_CONFIG 5
#define NETWORK_MENU_AWS_PROTOCOL 6

void draw_network_main_page(screen_menu_t* p_win)
{
  int row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, NETWORK_MENU_ETH_USE);
  M_PRINTF(p_win, row_count++, "%-*s:%s", NETWORK_WD, "Ethernet", 
           ITEM_LIST(get_config_app()->eth_active, enable_list_eng));

  screen_update_list(p_win, row_count, NETWORK_MENU_CDMA_USE);
  M_PRINTF(p_win, row_count++, "%-*s:%s", NETWORK_WD, "CDMA", 
           ITEM_LIST(get_config_app()->cdma_active, enable_list_eng));

  screen_update_list(p_win, row_count, NETWORK_MENU_DIRECT_USE);
  M_PRINTF(p_win, row_count++, "%-*s:%s", NETWORK_WD, "Direct", 
           ITEM_LIST(get_config_app()->direct_active, enable_list_eng));

  screen_update_list(p_win, row_count, NETWORK_MENU_ETH_CONFIG);
  M_PRINTF(p_win, row_count++, "%-*s", NETWORK_WD, "Ethernet");

  screen_update_list(p_win, row_count, NETWORK_MENU_CDMA_CONFIG);
  M_PRINTF(p_win, row_count++, "%-*s", NETWORK_WD, "CDMA");

  screen_update_list(p_win, row_count, NETWORK_MENU_DIRECT_CONFIG);
  M_PRINTF(p_win, row_count++, "%-*s", NETWORK_WD, "Direct");

  screen_update_list(p_win, row_count, NETWORK_MENU_AWS_PROTOCOL);
  M_PRINTF(p_win, row_count++, "%-*s:%s", NETWORK_WD, "Protocol", 
           ITEM_LIST(get_config_app()->aws_protocol_type, protocol_list_eng));

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}

int32_t input_ip_address(const char* title, uint8_t* ip)
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
#define ETH_MENU_MODE 0
#define ETH_MENU_LOCAL_IP 1
#define ETH_MENU_SUBNET 2
#define ETH_MENU_GATEWAY 3
#define ETH_MENU_LOCAL_PORT 4
#define ETH_MENU_REMOTE_IP 5
#define ETH_MENU_REMOTE_PORT 6

void draw_eth_config_page(screen_menu_t* p_win)
{
  int row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, ETH_MENU_MODE);
  M_PRINTF(p_win, row_count++, "%-*s:%s", NETWORK_WD, "Mode", 
           ITEM_LIST(get_config_app()->eth_mode, eth_mode_list_eng));

  screen_update_list(p_win, row_count, ETH_MENU_LOCAL_IP);
  M_PRINTF(p_win, row_count++, "%-*s", NETWORK_WD, "Local IP");

  screen_update_list(p_win, row_count, ETH_MENU_SUBNET);
  M_PRINTF(p_win, row_count++, "%-*s", NETWORK_WD, "Subnet");

  screen_update_list(p_win, row_count, ETH_MENU_GATEWAY);
  M_PRINTF(p_win, row_count++, "%-*s", NETWORK_WD, "Gateway");

  screen_update_list(p_win, row_count, ETH_MENU_LOCAL_PORT);
  M_PRINTF(p_win, row_count++, "%-*s:%d", NETWORK_WD, "Local Port", 
           get_config_app()->eth_local_port);

  screen_update_list(p_win, row_count, ETH_MENU_REMOTE_IP);
  M_PRINTF(p_win, row_count++, "%-*s", NETWORK_WD, "Remote IP");

  screen_update_list(p_win, row_count, ETH_MENU_REMOTE_PORT);
  M_PRINTF(p_win, row_count++, "%-*s:%d", NETWORK_WD, "Remote Port", 
           get_config_app()->eth_remote_server_port);

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}

int32_t setup_eth_config(void)
{
  int32_t choice = 0;
  int32_t status;
  int32_t key;
  screen_menu_t menu;
  int32_t dec;
  int32_t index;

  screen_menu_create(&menu,  "Ethernet");


  while (1)
  {
    draw_eth_config_page(&menu);
    screen_refresh();

    key = get_button_key(1000);

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
        break;
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
  int row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, CDMA_MENU_SERVER_IP);
  M_PRINTF(p_win, row_count++, "%-*s", NETWORK_WD, "Server IP");

  screen_update_list(p_win, row_count, CDMA_MENU_PORT);
  M_PRINTF(p_win, row_count++, "%-*s:%d", NETWORK_WD, "Port", 
           get_config_app()->cdma_port);

  screen_update_list(p_win, row_count, CDMA_MENU_MODEL);
  M_PRINTF(p_win, row_count++, "%-*s:%s", NETWORK_WD, "Model", 
           ITEM_LIST(get_config_app()->cdma_model, cdma_model_list_eng));

  screen_update_list(p_win, row_count, CDMA_MENU_VPN);
  M_PRINTF(p_win, row_count++, "%-*s:%s", NETWORK_WD, "VPN", 
           ITEM_LIST(get_config_app()->vpn_active, enable_list_eng));

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
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

    key = get_button_key(1000);

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
          choice = get_config_app()->vpn_active;
          status = input_active("VPN", &choice);
          if (status == MENU_OK)
          {
            get_config_app()->vpn_active = choice;
            WRITE_CFG(vpn_active);
          }
          break;
      }

      if (status == MENU_ABORT)
        break;
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
  int row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, DIRECT_MENU_BAUD_RATE);
  M_PRINTF(p_win, row_count++, "%-*s:%d", NETWORK_WD, "Baud Rate", 
           get_config_app()->direct_baud);

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
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

    key = get_button_key(1000);

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
        case DIRECT_MENU_BAUD_RATE:
          dec = get_config_app()->direct_baud;
          status = input_decimal("Baud Rate", 1200, 115200, &dec);
          if (status == MENU_OK)
          {
            get_config_app()->direct_baud = dec;
            WRITE_CFG(direct_baud);
          }
          break;
      }

      if (status == MENU_ABORT)
        break;
    }
    else if (key != KEY_CODE_NONE)
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

    key = get_button_key(1000);

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
        case NETWORK_MENU_ETH_USE:
          choice = get_config_app()->eth_active;
          status = input_active("Use Ethernet?", &choice);
          if (status == MENU_OK)
          {
            get_config_app()->eth_active = choice;
            WRITE_CFG(eth_active);
            show_popup("Information", "Applied after reset");
          }
          break;
        case NETWORK_MENU_CDMA_USE:
          choice = get_config_app()->cdma_active;
          status = input_active("Use Cdma?",  &choice);
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
        case NETWORK_MENU_DIRECT_USE:
          choice = get_config_app()->direct_active;
          status = input_active("Use Direct?",  &choice);
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
          if (status == MENU_OK)
          {
            get_config_app()->aws_protocol_type = (eAWS_PROTOCOL_t)choice;
            WRITE_CFG(aws_protocol_type);
          }
          break;
      }
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}