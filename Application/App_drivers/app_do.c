
#include "driver_do.h"
#include "app_do.h"
#include "utile.h"

typedef struct app_do_s
{
  uint8_t num;
  const char *name;
}app_do_t;

const app_do_t do_define[]={{.num = DO_CON_PWR_232_A,   .name ="DO_CON_PWR_232_A"},
                            {.num = DO_CON_PWR_232_B,  .name ="D-SUB_EX1_232"}};

driver_t *do_drivers[eDO_MAX];




void do_open(eDO_PORT_t port)
{
  do_drivers[(int)port] = driver_do_open(do_define[(int)port].num,0);
}


void do_close(eDO_PORT_t port)
{
  //driver 해제 구현
  do_drivers[(int)port] = 0;
}

void do_low(eDO_PORT_t port)
{
  if(do_drivers[(int)port])
  {
    driver_do_low(do_drivers[(int)port]);
  }
}

void do_high(eDO_PORT_t port)
{
  if(do_drivers[(int)port])
  {
    driver_do_high(do_drivers[(int)port]);
  }
}


uint16_t do_get_portList(const char **list,uint16_t listMax)
{
  int i=0;
  for( i = 0; i <_countof(do_define);i++)
  {
    if(i<listMax)
    {
      list[i] = do_define[i].name;
    }
  }
  return i;
}

bool do_is_opened(eDO_PORT_t port)
{
  if(do_drivers[(int)port] != 0)
  {
    return true;
  }
  return false;
}