
#include "menu_handler.h"
#include "menu_network.h"
#include "menu_sensor.h"
#include "menu_system.h"
#include "util_memory.h"
#include "menu_offset.h"
#include "menu_panel.h"
#include "menu_calibration.h"
#include "menu_manager.h"
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
  int choice=0;

  do
  {
    status = print_menu_list(menu_list,_countof(menu_list),&choice);

    if(status !=MENU_OK)
      break;

      switch (choice)
      {
      case 0: //SYSTEM
        status =  setup_menu_system(); 
        break;
      case 1:
        status = setup_menu_sensor();
         break;
      case 2:
        status = setup_menu_network();
        break;
      case 3:  // SYSTEM
        break;
      case 4:  
        status = setup_menu_panel();
         break;
      case 5:
       status = setup_menu_offset();
       break;
      case 6:
      status =setup_menu_calibration();
      break;
      case 7:
        status = setup_menu_manager();
        break;
        default : break;
      }

      if(status == MENU_ABORT)
      break;
  }while(1);

}