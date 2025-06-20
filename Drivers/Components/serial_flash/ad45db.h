
#ifndef AT45DB_H
#define AT45DB_H

#include <stdint.h>

#include "cmsis_os.h"

#include "driver_interface.h"

driver_t *at45db_open(int32_t num);

#endif