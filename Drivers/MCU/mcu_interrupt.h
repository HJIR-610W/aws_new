


#ifndef MCU_INTERRUPT_H
#define MCU_INTERRUPT_H

#include <stdint.h>


void exti_register(uint16_t pin,void *handle,void (*call)(void *));

#endif
