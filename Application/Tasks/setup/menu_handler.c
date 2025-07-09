#include <stdint.h>
#include <stdio.h>

#include "menu_handler.h"
#include "app_button.h"
#include "app_screen.h"
#include "cli_key_code.h"

#include "util_memory.h"
#define MAX_ROWS 8

int32_t print_menu_list(const char* menu_list[], int32_t menu_count, int* choice)
{
  int status = MENU_OK;
  if (menu_count <= 0 || choice == NULL)
  {
    return MENU_BACK;
  }

  int current_selection = 0;
  int scroll_offset = 0;
  int max_display_rows = (menu_count < MAX_ROWS) ? menu_count : MAX_ROWS;

  while (true)
  {
    for (int i = 0; i < max_display_rows; i++)
    {
      int item_index = scroll_offset + i;
      if (item_index >= menu_count)

        break;

      if (item_index == current_selection)
      {
        screen_printf(i, 0, "*%s", menu_list[item_index]);
      }
      else
      {
        screen_printf(i, 0, " %s", menu_list[item_index]);
      }
    }

    screen_refresh();

    int32_t key = get_button_key(100);

    switch (key)
    {
      case '8':  // Up arrow
        if (current_selection > 0)
        {
          current_selection--;
          if (current_selection < scroll_offset)
          {
            scroll_offset--;
          }
        }
        break;

      case '2':  // Down arrow
        if (current_selection < menu_count - 1)
        {
          current_selection++;
          if (current_selection >= scroll_offset + MAX_ROWS)
          {
            scroll_offset++;
          }
        }
        break;

      case KEY_CODE_ENTER:  // Enter
        *choice = current_selection;
        return MENU_OK;

      case KEY_CODE_CTRL_Q:  // ESC
        status = MENU_ABORT;
        break;
      case KEY_CODE_CTRL_C:
        status = MENU_BACK;
        break;
    }

    if (status != MENU_OK)
      break;
  }

  return status;
}
