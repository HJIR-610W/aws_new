
#ifndef IO_HHH
#define IO_HHH

#include <stdint.h>
#include <stdarg.h>

#include "io_interface.h"

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

void debug_init(void);
int32_t debug_recv(uint8_t *buffer, size_t len, uint32_t timeout_ms);
int32_t debug_get_ch(uint8_t *buffer);
int32_t debug_get_ch_nonblocking(uint8_t *buffer);
void debug_inject(uint8_t *data,size_t len);
int debug_scanf_s(const char *fmt, ...);

int32_t debug_printf(const char * fmt, ...);
void debug_printf_color(int color, const char *pFmt, ...);
void debug_send(const uint8_t *data,size_t len);
void debug_put_ch(uint8_t ch);
void debug_puts(const uint8_t *string);
void debug_dump(uint8_t* data, size_t size, uint32_t start_address,uint32_t col);
int32_t debug_vprintf(const char *fmt, va_list ap);

io_if_t *get_debug_io(void) ;


#endif
