

#ifndef APP_DI_H
#define APP_DI_H


#include <stdint.h>
#include <stdbool.h>



#define APP_DI_0 0
#define APP_DI_1 1
#define APP_DI_2 2
#define APP_DI_3 3
#define APP_DI_4 4
#define APP_DI_5 5


#define IS_DOOR_OPENED() read_di(APP_DI_0)
#define IS_DI_PRESSED(n)  is_di_pressed(n)

void di_init(void);
int32_t read_di(int32_t num);
bool is_di_pressed(int32_t num);

#endif
