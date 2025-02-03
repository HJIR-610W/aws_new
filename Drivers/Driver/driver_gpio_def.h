

#ifndef DRIVER_GPIO_DEF_H

#define DRIVER_GPIO_DEF_H

#define GPIO_DIR_IN  1
#define GPIO_DIR_OUT 0


#define GPIO_PIN0 0x0001
#define GPIO_PIN1 0x0002
#define GPIO_PIN2 0x0004
#define GPIO_PIN3 0x0008
#define GPIO_PIN4 0x0010
#define GPIO_PIN5 0x0020
#define GPIO_PIN6 0x0040
#define GPIO_PIN7 0x0080
#define GPIO_PIN8 0x0100
#define GPIO_PIN9 0x0200
#define GPIO_PIN10 0x0400
#define GPIO_PIN11 0x0800
#define GPIO_PIN12 0x1000
#define GPIO_PIN13 0x2000
#define GPIO_PIN14 0x4000
#define GPIO_PIN15 0x8000

#define DIR_IN(x) x
#define DIR_OUT(x) (0)





typedef struct gpio_api_s
{
    void (*close)(driver_t *handle);
    int32_t (*read8)(driver_t *handle,uint16_t *data);
    int32_t (*write8)(driver_t *handle,uint8_t data);
    int (*write_pin)(driver_t *drv,uint16_t pin,uint16_t high);
} gpio_api_t;
#endif
