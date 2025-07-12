


#ifndef MCU_INTERRUPT_H
#define MCU_INTERRUPT_H

#include <stdint.h>

typedef struct exti_isr_cfg_s
{
  const char *name;
  uint16_t prio;
  uint16_t gpio_pin;
  uint16_t irq;
  void *handle;
  void (*call)(void *);
}exti_isr_cfg_t;

void exti_register(exti_isr_cfg_t *cfg);
void mcu_interrupt_init(void);
#endif
