
#include "menu_handler.h"
#include "menu_system.h"
#include "util_memory.h"
void setup_root(void)
{
  const char* menu_list[] = {"SYSTEM",
                             "SENSOR",
                             "NETWORK",
                             "DATA",
                             "PANEL",
                             "OFFSET",
                             "CALIBRATION",
                             "MANAGER",
                             "ADMIN"};

  int status;
  int choice;

  do
  {
    status = print_menu_list(menu_list,_countof(menu_list),&choice);

    if(status !=MENU_OK)
      break;

      switch (choice)
      {
      case 0: //SYSTEM
        status =  menu_system(); 
        break;
      case 1:  // SYSTEM
        break;
      case 2:  // SYSTEM
        break;
      case 3:  // SYSTEM
        break;
      case 4:  // SYSTEM
        break;
      default:
        break;
      }

      if(status == MENU_ABORT)
      break;
  }while(1);

}