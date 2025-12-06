/**
 * @file vt100_terminal.h
 * @brief VT100 터미널 에뮬레이터 헤더 (dbg_printf/dbg_puts 사용)
 */

#ifndef VT100_TERMINAL_H
#define VT100_TERMINAL_H

#include <stdint.h>
#include <stdbool.h>
#include "driver_interface.h"


driver_t* vt100_terminal_open(void);

#endif