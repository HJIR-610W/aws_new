
#ifndef IO_HHH
#define IO_HHH

#include <stdint.h>
#include <stdarg.h>

#include "io_interface.h"
#include "cli_key_code.h"
#define IO_COLOR_RED     31
#define IO_COLOR_GREEN   32
#define IO_COLOR_YELLOW  33
#define IO_COLOR_BLUE    34
#define IO_COLOR_MAGENTA 35
#define IO_COLOR_CYAN    36
#define IO_COLOR_WHITE   37

#define ASCII_CODE_ESC 0x1B
#define ASCII_CODE_CTRL_Q 0x11
#define ASCII_CODE_CTRL_C 0x03
#define ASCII_CODE_CR     0x0D

#define ASCII_SPEICIAL    0x5B //   '['   

#include "util_escape_sequence.h"
#include "task_logger.h"

typedef enum {
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37
}color_t;



int32_t debug_init(void);
void debug_deinit(void);

int32_t debug_get_key(uint32_t timeout_ms);
int32_t debug_recv(uint8_t *buffer, size_t len, uint32_t timeout_ms);
int32_t debug_get_ch(uint8_t *buffer);
int32_t debug_get_ch_nonblocking(uint8_t *buffer);
void debug_inject(uint8_t *data,size_t len);
int debug_scanf_s(const char *fmt, ...);


void debug_printf_color(int color, const char *pFmt, ...);
void debug_dump(uint8_t* data, size_t size, uint32_t start_address,uint32_t col);
io_if_t *get_debug_io(void) ;
void debug_set_io(io_if_t *debug_io);
void debug_set_io_default(void);


int32_t debug_vprintf(const char *fmt, va_list ap);
int32_t debug_printf(const char *fmt, ...);
void debug_send(const uint8_t *data, size_t len);
void debug_put_ch(uint8_t ch);
void debug_puts(const uint8_t *string);

int32_t log_printf(const char *fmt, ...);

//#define debug_printf os_printf
//#define debug_send os_debug_send
//#define debug_put_ch os_put_ch
//#define debug_puts os_puts
//#define debug_vprintf os_vprintf


/* 상태창용 (row,col 필요) */
#define STATIC_PRINTF(row, col, fmt, ...) \
  os_printf( \
    "\x1B[s" \
    "\x1B[" row ";" col "H" \
    "\x1B[2K" \
    fmt \
    "\x1B[u", \
    ##__VA_ARGS__)

#define VT100_PRINTF_AT(row, col, fmt, ...) \
  debug_printf("\x1B[s\x1B[%d;%dH" fmt "\x1B[u", (row), (col), ##__VA_ARGS__)

#define VT100_PUT_CH_AT(row, col, ch) \
  debug_printf("\x1B[s\x1B[%d;%dH%c\x1B[u", (row), (col), (ch))

#define VT100_PUTS_AT(row, col, str) \
  debug_printf("\x1B[s\x1B[%d;%dH%s\x1B[u", (row), (col), (str))

  #define VT100_CLEAR_ROWS(row_start, row_end)           \
  do                                                   \
  {                                                    \
    for (int r = (row_start); r <= (row_end); r++)     \
    {                                                  \
      debug_printf("\x1B[%d;1H\x1B[2K", r);             \
    }                                                  \
  } while (0)
#define VT100_CLEAR_TO_COL(row, col) \
  debug_printf("\x1B[s\x1B[%d;%dH\x1B[1K\x1B[u", (row), (col))

  #define VT100_CLEAR()           \
  do                                                   \
  {                                                    \
    for (int r = (0); r <= (8); r++)     \
    {                                                  \
      VT100_CLEAR_TO_COL(r,20);             \
    }                                                  \
  } while (0)

#define VT100_PRINTF(fmt, ...)               debug_printf(fmt, ##__VA_ARGS__)
#define VT100_PUTS(str)                      debug_printf(str)

//#define VT100_CLEAR()               debug_printf("\x1B[2J\x1B[H")
#define VT100_SET_RANGE(top, bottom)   debug_printf("\x1B[%d;%dr", (top), (bottom))
#define VT100_RESET_RANGE()  debug_printf("\x1B[r")    /* 전체 화면을 스크롤 영역으로 복원 */

extern uint8_t g_log_write_enable;

#endif
