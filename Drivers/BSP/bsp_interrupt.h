


#ifndef MCU_INTERRUPT_H
#define MCU_INTERRUPT_H

#include <stdint.h>

typedef struct exti_isr_cfg_s
{
  const char *name;
  uint16_t prio;
  uint16_t gpio_pin;
  uint16_t irq;
  int32_t handle;
  void (*call)(int32_t );
}exti_isr_cfg_t;

void exti_register(exti_isr_cfg_t *cfg);
void bsp_interrupt_init(void);
#endif
