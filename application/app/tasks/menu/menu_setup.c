
#include "app_key.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "menu_handler.h"
#include "menu_network.h"
#include "menu_sensor.h"
#include "menu_system.h"
#include "menu_offset.h"
#include "menu_panel.h"
#include "menu_calibration.h"
#include "menu_manager.h"
#include "menu_data.h"
#include "menu_admin.h"
#include "util_memory.h"


#define AWS_SETUP_SYSTEM    0
#define AWS_SETUP_SENSOR    1
#define AWS_SETUP_NETWORK   2
#define AWS_SETUP_DATA      3
#define AWS_SETUP_PANEL     4
#define AWS_SETUP_OFFSET    5
#define AWS_SETUP_CALI      6
#define AWS_SETUP_MANAGER   7
#define AWS_SETUP_DEVELOPER 8


static bool g_admin_menu_active=false; //필요에 의해서만 developer 메뉴 활성화 목적

void draw_aws_setup_page(screen_menu_t* p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, AWS_SETUP_SYSTEM, "System");
  screen_menu_printf(p_win, AWS_SETUP_SENSOR, "Sensor");
  screen_menu_printf(p_win, AWS_SETUP_NETWORK,"Network");
  screen_menu_printf(p_win, AWS_SETUP_DATA,   "Data");
  screen_menu_printf(p_win, AWS_SETUP_PANEL,  "Panel");
  screen_menu_printf(p_win, AWS_SETUP_OFFSET, "Offset");
  screen_menu_printf(p_win, AWS_SETUP_CALI,   "Calibraion");
  screen_menu_printf(p_win, AWS_SETUP_MANAGER,"Manager");
  if(g_admin_menu_active)
  {
    screen_menu_printf(p_win, AWS_SETUP_DEVELOPER, "Developer");
  }
  screen_menu_clear(p_win);
}

void setting_menu_auto_close_callback(void *argument)
{
  button_put_key(KEY_CODE_ESC_LONG); // 설정화면 종료 버튼을 대신 눌러준다
  button_put_key(KEY_CODE_ESC_LONG); // 설정화면 종료 버튼을 대신 눌러준다
}


#define SCREEN_EXIT_TIMEOUT_SEC 3600
static osTimerId_t g_screen_timer_id;
void auto_close_screen_timer_init(uint32_t delay_seconds)
{

  osTimerAttr_t timer_attr = {.name = "close screen", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0};

  // 원샷 타이머 생성 (한 번만 실행)
  g_screen_timer_id = osTimerNew(setting_menu_auto_close_callback, osTimerOnce, NULL, &timer_attr);

  if (g_screen_timer_id != NULL)
  {
    osStatus_t status = osTimerStart(g_screen_timer_id, delay_seconds * osKernelGetTickFreq());
  }
}

void auto_close_screen_timer_reset(void)
{
  if (g_screen_timer_id != NULL )
  {
    // 기존 타이머 정지
    osTimerStop(g_screen_timer_id);

    // 타이머 재시작
    osTimerStart(g_screen_timer_id, SCREEN_EXIT_TIMEOUT_SEC * osKernelGetTickFreq());
  }
}

void auto_close_timer_delete(void)
{
    if (g_screen_timer_id != NULL)
    {
        osStatus_t status = osTimerDelete(g_screen_timer_id);
        g_screen_timer_id = NULL;  // 안전하게 포인터 초기화

    }
}


#define MENU_PASSWORD 7777
void setup_menu(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  int32_t admin_menu_active_count=0;
  screen_menu_t menu;
  int32_t password=0;

  auto_close_screen_timer_init(SCREEN_EXIT_TIMEOUT_SEC);

  register_key_callback(auto_close_screen_timer_reset);

  while(1)
  {
    status = input_password("PASS WORD",&password);

    if(status != MENU_OK)
    {
        unregister_key_callback();
  auto_close_timer_delete();
    return;
    }

    if (password == MENU_PASSWORD)
    {
      break;
    }
    else
    {
      show_popup("Error", "Incorrect password");
    }
  
  }

  screen_menu_create(&menu, "AWS Setup");

  while(1)
  {
    draw_aws_setup_page(&menu);
    screen_refresh();

    key = get_menu_key(WAIT_FOREVER);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }
    else if(key== KEY_CODE_RIGHT)
    {
      if (menu.index_list[menu.selected_index] == AWS_SETUP_MANAGER)
      {
         admin_menu_active_count++;
        if (admin_menu_active_count == 5)
        {
          g_admin_menu_active = true;
        }

      }
    }



    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
        case AWS_SETUP_SYSTEM:
          status = setup_menu_system();
          break;
        case AWS_SETUP_SENSOR:
          status = setup_menu_sensor();
          break;
        case AWS_SETUP_NETWORK:
          status = setup_menu_network();
          break;
        case AWS_SETUP_DATA:
          status  = setup_menu_data();
          break;
        case AWS_SETUP_PANEL:
          status = setup_menu_panel();
          break;
        case AWS_SETUP_OFFSET:
          status = setup_menu_offset();
          break;
        case AWS_SETUP_CALI:
          status = setup_menu_calibration();
          break;
        case AWS_SETUP_MANAGER:
          status = setup_menu_manager();
          break;
        case AWS_SETUP_DEVELOPER:
          status = setup_menu_developer();
          break;
        default:
          break;

      }
      if (status == MENU_ABORT)
        return ;
    }
    else if (key != KEY_CODE_UNKNOWN)
    {
      screen_menu_handle(&menu, key);

    }
  }

  unregister_key_callback();
  auto_close_timer_delete();
}