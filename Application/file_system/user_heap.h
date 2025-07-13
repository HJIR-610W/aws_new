
#ifndef _USER_HEAP_H
#define _USER_HEAP_H

#include <stdio.h>

#define POOL_SIZE (1024*1024 * 3)  

void user_tlsf_init(size_t size);
void *user_malloc(size_t size);
void user_free(void *ptr);


#endif