
#include "menu_network.h"

#include "app_key.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "console_utile.h"
#include "menu_handler.h"
#include "view_driver.h"
#include "util_memory.h"
#include "string\const_string.h"

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
  M_PRINTF(p_win, row_count++, "%-*s:%s", NETWORK_WD, "Eth Use", 
           ITEM_LIST(get_config_app()->eth_use, enable_list_eng));

  screen_update_list(p_win, row_count, NETWORK_MENU_CDMA_USE);
  M_PRINTF(p_win, row_count++, "%-*s:%s", NETWORK_WD, "CDMA Use", 
           ITEM_LIST(get_config_app()->cdma_use, enable_list_eng));

  screen_update_list(p_win, row_count, NETWORK_MENU_DIRECT_USE);
  M_PRINTF(p_win, row_count++, "%-*s:%s", NETWORK_WD, "Direct Use", 
           ITEM_LIST(get_config_app()->direct_use, enable_list_eng));

  screen_update_list(p_win, row_count, NETWORK_MENU_ETH_CONFIG);
  M_PRINTF(p_win, row_count++, "%-*s", NETWORK_WD, "Eth Config");

  screen_update_list(p_win, row_count, NETWORK_MENU_CDMA_CONFIG);
  M_PRINTF(p_win, row_count++, "%-*s", NETWORK_WD, "CDMA Config");

  screen_update_list(p_win, row_count, NETWORK_MENU_DIRECT_CONFIG);
  M_PRINTF(p_win, row_count++, "%-*s", NETWORK_WD, "Direct Config");

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

int32_t setup_eth_config(void)
{
  int32_t choice = 0;
  int32_t status;
  int32_t key;
  screen_menu_t menu;
  int32_t dec;

  screen_menu_create(&menu, 8, 20);

  const char* eth_menu[] = {
    "Mode", "Local IP", "Subnet", "Gateway", 
    "Local Port", "Remote IP", "Remote Port"
  };

  while (1)
  {
    screen_clear();
    status = print_menu_list(eth_menu, _countof(eth_menu), &choice);
    
    if (status != MENU_OK)
      break;

    switch (choice)
    {
      case 0: // Mode
        choice = get_config_app()->eth_mode;
        status = print_menu_list(eth_mode_list_eng, _countof(eth_mode_list_eng), &choice);
        if (status == MENU_OK)
        {
          get_config_app()->eth_mode = choice;
          WRITE_CFG(eth_mode);
        }
        break;
      case 1: // Local IP
        status = input_ip_address("Local IP", get_config_app()->eth_ip);
        if (status == MENU_OK)
        {
          WRITE_CFG(eth_ip);
        }
        break;
      case 2: // Subnet
        status = input_ip_address("Subnet", get_config_app()->eth_subnet);
        if (status == MENU_OK)
        {
          WRITE_CFG(eth_subnet);
        }
        break;
      case 3: // Gateway
        status = input_ip_address("Gateway", get_config_app()->eth_gateway);
        if (status == MENU_OK)
        {
          WRITE_CFG(eth_gateway);
        }
        break;
      case 4: // Local Port
        dec = get_config_app()->eth_local_port;
        status = input_decimal("Local Port", 0, 65535, &dec);
        if (status == MENU_OK)
        {
          get_config_app()->eth_local_port = dec;
          WRITE_CFG(eth_local_port);
        }
        break;
      case 5: // Remote IP
        status = input_ip_address("Remote IP", get_config_app()->eth_remote_server_ip);
        if (status == MENU_OK)
        {
          WRITE_CFG(eth_remote_server_ip);
        }
        break;
      case 6: // Remote Port
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

  return status;
}

int32_t setup_cdma_config(void)
{
  int32_t choice = 0;
  int32_t status;
  int32_t dec;

  const char* cdma_menu[] = {
    "Server IP", "Port", "Model", "VPN Use"
  };

  while (1)
  {
    screen_clear();
    status = print_menu_list(cdma_menu, _countof(cdma_menu), &choice);
    
    if (status != MENU_OK)
      break;

    switch (choice)
    {
      case 0: // Server IP
        status = input_ip_address("Server IP", get_config_app()->cdma_server_ip);
        if (status == MENU_OK)
        {
          WRITE_CFG(cdma_server_ip);
        }
        break;
      case 1: // Port
        dec = get_config_app()->cdma_port;
        status = input_decimal("Port", 0, 65535, &dec);
        if (status == MENU_OK)
        {
          get_config_app()->cdma_port = dec;
          WRITE_CFG(cdma_port);
        }
        break;
      case 2: // Model
        choice = get_config_app()->cdma_model;
        status = print_menu_list(cdma_model_list_eng, _countof(cdma_model_list_eng), &choice);
        if (status == MENU_OK)
        {
          get_config_app()->cdma_model = choice;
          WRITE_CFG(cdma_model);
        }
        break;
      case 3: // VPN Use
        choice = get_config_app()->vpn_use;
        status = print_menu_list(enable_list_eng, _countof(enable_list_eng), &choice);
        if (status == MENU_OK)
        {
          get_config_app()->vpn_use = choice;
          WRITE_CFG(vpn_use);
        }
        break;
    }

    if (status == MENU_ABORT)
      break;
  }

  return status;
}

int32_t setup_direct_config(void)
{
  int32_t status;
  int32_t dec;

  dec = get_config_app()->direct_baud;
  status = input_decimal("Baud Rate", 1200, 115200, &dec);
  if (status == MENU_OK)
  {
    get_config_app()->direct_baud = dec;
    WRITE_CFG(direct_baud);
  }

  return status;
}

int32_t setup_menu_network(void)
{
  int32_t choice = 0;
  int32_t status;
  int32_t key;
  screen_menu_t menu;
  int32_t index;

  screen_menu_create(&menu, 8, 20);

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
          choice = get_config_app()->eth_use;
          status = print_menu_list(enable_list_eng, _countof(enable_list_eng), &choice);
          if (status == MENU_OK)
          {
            get_config_app()->eth_use = choice;
            WRITE_CFG(eth_use);
          }
          break;
        case NETWORK_MENU_CDMA_USE:
          choice = get_config_app()->cdma_use;
          status = print_menu_list(enable_list_eng, _countof(enable_list_eng), &choice);
          if (status == MENU_OK)
          {
            get_config_app()->cdma_use = choice;
            if (choice && get_config_app()->direct_use)
            {
              get_config_app()->direct_use = 0;
              WRITE_CFG(direct_use);
            }
            WRITE_CFG(cdma_use);
          }
          break;
        case NETWORK_MENU_DIRECT_USE:
          choice = get_config_app()->direct_use;
          status = print_menu_list(enable_list_eng, _countof(enable_list_eng), &choice);
          if (status == MENU_OK)
          {
            get_config_app()->direct_use = choice;
            if (choice && get_config_app()->cdma_use)
            {
              get_config_app()->cdma_use = 0;
              WRITE_CFG(cdma_use);
            }
            WRITE_CFG(direct_use);
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
          status = print_menu_list(protocol_list_eng, _countof(protocol_list_eng), &choice);
          if (status == MENU_OK)
          {
            get_config_app()->aws_protocol_type = choice;
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