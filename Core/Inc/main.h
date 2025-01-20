
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx_hal.h"


void Error_Handler(const char *file,int32_t line);
void *aws_malloc(size_t size);
void aws_free(void *ptr);


#define AWS_PCB_VER 2

#if (AWS_PCB_VER==1)
#define NOT_USED_PE2_Pin GPIO_PIN_2
#define NOT_USED_PE2_GPIO_Port GPIOE
#define OUT_SPI1_CS_RTC_Pin GPIO_PIN_5
#define OUT_SPI1_CS_RTC_GPIO_Port GPIOE
#define NOT_USED_PE6_Pin GPIO_PIN_6
#define NOT_USED_PE6_GPIO_Port GPIOE

#define NOT_USED_PC13_Pin GPIO_PIN_13
#define NOT_USED_PC13_GPIO_Port GPIOC

#define OUT_CON_PWR_ASEN_C_Pin GPIO_PIN_9
#define OUT_CON_PWR_ASEN_C_GPIO_Port GPIOF
#define OUT_CON_PWR_ASEN_D_Pin GPIO_PIN_10
#define OUT_CON_PWR_ASEN_D_GPIO_Port GPIOF
#define OUT_CON_PWR_ASEN_Pin GPIO_PIN_0
#define OUT_CON_PWR_ASEN_GPIO_Port GPIOC
#define OUT_CON_PWR_ASEN_A_Pin GPIO_PIN_2
#define OUT_CON_PWR_ASEN_A_GPIO_Port GPIOC
#define OUT_CON_PWR_ASEN_B_Pin GPIO_PIN_3
#define OUT_CON_PWR_ASEN_B_GPIO_Port GPIOC
#define OUT_ETH_RST_PHY_Pin GPIO_PIN_2
#define OUT_ETH_RST_PHY_GPIO_Port GPIOH
#define NOT_USED_PA5_Pin GPIO_PIN_5
#define NOT_USED_PA5_GPIO_Port GPIOA
#define INT_D_IO_Pin GPIO_PIN_0
#define INT_D_IO_GPIO_Port GPIOB
#define INT_D_IO_EXTI_IRQn EXTI0_IRQn


#define IN_RAIN_PULSE_H_EXTI_IRQn EXTI1_IRQn

#define IN_BOOT1_Pin GPIO_PIN_2
#define IN_BOOT1_GPIO_Port GPIOB
#define OUT_DO_PWR_CDMA_Pin GPIO_PIN_6
#define OUT_DO_PWR_CDMA_GPIO_Port GPIOH
#define OUT_SYS_RUN_Pin GPIO_PIN_9
#define OUT_SYS_RUN_GPIO_Port GPIOH
#define OUT_ADC_EN_RTD_Pin GPIO_PIN_10
#define OUT_ADC_EN_RTD_GPIO_Port GPIOH
#define OUT_ADC_EN_ODD_Pin GPIO_PIN_11
#define OUT_ADC_EN_ODD_GPIO_Port GPIOH
#define OUT_ADC_EN_EVEN_Pin GPIO_PIN_12
#define OUT_ADC_EN_EVEN_GPIO_Port GPIOH
#define IN_STATUS_BTM_Pin GPIO_PIN_14
#define IN_STATUS_BTM_GPIO_Port GPIOB
#define BTM_PWRC_Pin GPIO_PIN_15
#define BTM_PWRC_GPIO_Port GPIOB
#define OUT_DIR_RS485_A_Pin GPIO_PIN_6
#define OUT_DIR_RS485_A_GPIO_Port GPIOG
#define OUT_DIR_RS485_B_Pin GPIO_PIN_7
#define OUT_DIR_RS485_B_GPIO_Port GPIOG
#define OUT_DIR_SDI_Pin GPIO_PIN_8
#define OUT_DIR_SDI_GPIO_Port GPIOG
#define NOT_USED_PA11_Pin GPIO_PIN_11
#define NOT_USED_PA11_GPIO_Port GPIOA
#define IN_SPI2_DRDY_Pin GPIO_PIN_12
#define IN_SPI2_DRDY_GPIO_Port GPIOA

#define OUT_ADC_SEL_A0_Pin GPIO_PIN_13
#define OUT_ADC_SEL_A0_GPIO_Port GPIOH

#define OUT_ADC_SEL_A1_Pin GPIO_PIN_14
#define OUT_ADC_SEL_A1_GPIO_Port GPIOH
#define OUT_ADC_SEL_A2_Pin GPIO_PIN_15
#define OUT_ADC_SEL_A2_GPIO_Port GPIOH
#define OUT_SPI2_NSS_Pin GPIO_PIN_0
#define OUT_SPI2_NSS_GPIO_Port GPIOI
#define OUT_SPI1_NSS_Pin GPIO_PIN_15
#define OUT_SPI1_NSS_GPIO_Port GPIOA
#define IN_SDIO_DETECT_Pin GPIO_PIN_3
#define IN_SDIO_DETECT_GPIO_Port GPIOD
#define OUT_CON_PWR_DSEN_Pin GPIO_PIN_6
#define OUT_CON_PWR_DSEN_GPIO_Port GPIOD


#define OUT_NOR_RESET_Pin GPIO_PIN_12
#define OUT_NOR_RESET_GPIO_Port GPIOG
#define OUT_CON_PWR_232_A_Pin GPIO_PIN_13
#define OUT_CON_PWR_232_A_GPIO_Port GPIOG
#define OUT_CON_PWR_232_B_Pin GPIO_PIN_14
#define OUT_CON_PWR_232_B_GPIO_Port GPIOG
#define INT_RTC_Pin GPIO_PIN_15
#define INT_RTC_GPIO_Port GPIOG
#define INT_RTC_EXTI_IRQn EXTI15_10_IRQn
#define OUT_CON_PWR_485_Pin GPIO_PIN_8
#define OUT_CON_PWR_485_GPIO_Port GPIOB
#define CON_PWR_TC_Pin GPIO_PIN_9
#define CON_PWR_TC_GPIO_Port GPIOB
#define OUT_EX_UART_RST_A_Pin GPIO_PIN_0
#define OUT_EX_UART_RST_A_GPIO_Port GPIOE
#define OUT_EX_UART_RST_B_Pin GPIO_PIN_1
#define OUT_EX_UART_RST_B_GPIO_Port GPIOE


#define IN_EX_UART_INT_1_Pin GPIO_PIN_4
#define IN_EX_UART_INT_1_GPIO_Port GPIOI

#define IN_EX_UART_INT_2_Pin GPIO_PIN_5
#define IN_EX_UART_INT_2_GPIO_Port GPIOI

#define IN_EX_UART_INT_3_Pin GPIO_PIN_6
#define IN_EX_UART_INT_3_GPIO_Port GPIOI

#define IN_EX_UART_INT_4_Pin GPIO_PIN_7
#define IN_EX_UART_INT_4_GPIO_Port GPIOI

#define IN_EX_UART_INT_5_Pin GPIO_PIN_8
#define IN_EX_UART_INT_5_GPIO_Port GPIOI

#define IN_EX_UART_INT_6_Pin GPIO_PIN_9
#define IN_EX_UART_INT_6_GPIO_Port GPIOI

#define IN_EX_UART_INT_7_Pin GPIO_PIN_10
#define IN_EX_UART_INT_7_GPIO_Port GPIOI

#define IN_EX_UART_INT_8_Pin GPIO_PIN_11
#define IN_EX_UART_INT_8_GPIO_Port GPIOI




#define OUT_FLASH_CS_Pin GPIO_PIN_7
#define OUT_FLASH_CS_GPIO_Port GPIOD

#define I2C2_CLK_Pin GPIO_PIN_4
#define I2C2_CLK_GPIO_Port GPIOH

#define I2C2_SDA_Pin GPIO_PIN_5
#define I2C2_SDA_GPIO_Port GPIOH

#define IN_TIM10_CH1_Pin GPIO_PIN_6
#define IN_TIM10_CH1_GPIO_Port GPIOF


#define IN_TIM11_CH1_Pin GPIO_PIN_7
#define IN_TIM11_CH1_GPIO_Port GPIOF

#define IN_TIM13_CH1_Pin GPIO_PIN_8
#define IN_TIM13_CH1_GPIO_Port GPIOF


#define IN_RAIN_REED_Pin GPIO_PIN_11
#define IN_RAIN_REED_GPIO_Port GPIOF

#define IN_RAIN_HALL_Pin GPIO_PIN_1
#define IN_RAIN_HALL_GPIO_Port GPIOB

#define IN_RAIN_ERR_Pin GPIO_PIN_6
#define IN_RAIN_ERR_GPIO_Port GPIOA

#endif


#if (AWS_PCB_VER==2)

#define IN_WAKE_UP_PIN            GPIO_PIN_0 
#define IN_WAKE_UP_GPIO_Port      GPIOA
#define IN_ETH_REF_CLK_PIN        GPIO_PIN_1
#define IN_ETH_REF_CLK_GPIO_Port  GPIOA
#define ETH_MDIO_PIN              GPIO_PIN_2
#define ETH_MDIO_GPIO_Port        GPIOA
#define IN_ADC1_IN3_PIN           GPIO_PIN_3
#define IN_ADC1_IN3_GPIO_Port     GPIOA
#define IN_ADC1_IN4_PIN           GPIO_PIN_4
#define IN_ADC1_IN3_GPIO_Port     GPIOA
#define UNUSED_A5_PIN             GPIO_PIN_5
#define UNSUED_A5_GPIO_Port       GPIOA      //#미사용
#define IN_RAIN_INT_H_PIN         GPIO_PIN_6 //홀 센서 에러 감지
#define IN_RAIN_INT_H_GPIO_Port   GPIOA
#define ETH_CRS_DV_PIN            GPIO_PIN_7
#define ETH_CRS_DV_GPIO_Port      GPIOA
#define USB_OTG_FS_SOF_PIN        GPIO_PIN_8
#define USB_OTG_FS_SOF_GPIO_Port  GPIOA
#define USB_OTG_FS_VBUS_PIN       GPIO_PIN_9
#define USB_OTG_FS_VBUS_GPIO_Port GPIOA
#define USB_OTG_FS_ID_PIN         GPIO_PIN_10
#define USB_OTG_FS_ID_GPIO_Port   GPIOA
#define USB_OTG_FS_DM_PIN         GPIO_PIN_11
#define USB_OTG_FS_DM_GPIO_Port   GPIOA
#define USB_OTG_FS_DP_PIN         GPIO_PIN_12
#define USB_OTG_FS_DP_GPIO_Port   GPIOA
#define SYS_JTMS_SWDIO_PIN       GPIO_PIN_13
#define SYS_JTMS_SWDIO_GPIO_Port GPIOA
#define SYS_JTCK_SWCLK_PIN       GPIO_PIN_14
#define SYS_JTCK_SWCLK_GPIO_Port GPIOA
#define OUT_SPI1_NSS_PIN         GPIO_PIN_15
#define OUT_SPI1_NSS_GPIO_Port   GPIOA


#define INT_D_IO_PIN           GPIO_PIN_0 
#define INT_D_IO_GPIO_Port     GPIOB
#define RAIN_PULSE_H_PIN       GPIO_PIN_1
#define RAIN_PULSE_H_GPIO_Port GPIOB
#define BOOT1_PIN              GPIO_PIN_2
#define BOOT1_GPIO_Port        GPIOB
#define SPI1_SCK_PIN           GPIO_PIN_3
#define SPI1_SCK_GPIO_Port     GPIOB
#define SPI1_MISO_PIN          GPIO_PIN_4
#define SPI1_MISO_GPIO_Port    GPIOB
#define SPI1_MOSI_PIN          GPIO_PIN_5
#define SPI1_MOSI_GPIO_Port    GPIOB     
#define I2C1_SCL_PIN           GPIO_PIN_6 
#define I2C1_SCL_GPIO_Port     GPIOB
#define I2C1_SDA_PIN                GPIO_PIN_7
#define I2C1_SDA_GPIO_Port          GPIOB
#define USB_OTG_PWR_FAIL_PIN        GPIO_PIN_8
#define USB_OTG_PWR_FAIL_GPIO_Port  GPIOB
#define SEL_IF_UART_PIN             GPIO_PIN_9
#define SEL_IF_UART_GPIO_Port       GPIOB
#define UART3_TX_PIN                GPIO_PIN_10
#define UART3_TX_GPIO_Port          GPIOB
#define UART3_RX_PIN                GPIO_PIN_11
#define UART3_RX_GPIO_Port          GPIOB
#define ETH_TXD0_PIN                GPIO_PIN_12
#define ETH_TXD0_GPIO_Port          GPIOB
#define ETH_TXD1_PIN                GPIO_PIN_13
#define ETH_TXD1_GPIO_Port          GPIOB
#define STATUS_BTM_PIN              GPIO_PIN_14
#define STATUS_BTM_GPIO_Port        GPIOB
#define BTM_PWRC_PIN                GPIO_PIN_15
#define BTM_PWRC_GPIO_Port          GPIOB
#define CON_PWR_S24                 GPIO_PIN_0 
#define CON_PWR_S24_GPIO_Port       GPIOC
#define CON_PWR_S24_PIN             GPIO_PIN_1
#define CON_PWR_S24_GPIO_Port       GPIOC
#define SPI2_DRDY_PIN               GPIO_PIN_2
#define SPI2_DRDY_GPIO_Port         GPIOC
#define UNUSED_C2_PIN               GPIO_PIN_3
#define UNUSED_C2_GPIO_Port         GPIOC
#define ETH_RXD0_PIN                GPIO_PIN_4
#define ETH_RXD0_GPIO_Port          GPIOC
#define ETH_RXD1_PIN                GPIO_PIN_5
#define ETH_RXD1_GPIO_Port          GPIOC
#define UART6_TX_PIN                GPIO_PIN_6
#define UART6_TX_GPIO_Port          GPIOC     
#define UART6_RX_PIN                GPIO_PIN_7
#define UART6_RX_GPIO_Port          GPIOC
#define SDIO_D0_PIN                 GPIO_PIN_8
#define SDIO_D0_GPIO_Port           GPIOC
#define SDIO_D1_PIN                 GPIO_PIN_9
#define SDIO_D1_GPIO_Port           GPIOC
#define SDIO_D2_PIN                 GPIO_PIN_10
#define SDIO_D2_GPIO_Port           GPIOC
#define SDIO_D3_PIN                 GPIO_PIN_11
#define SDIO_D3_GPIO_Port           GPIOC
#define SDIO_CK_PIN                 GPIO_PIN_12
#define SDIO_CK_GPIO_Port           GPIOC
#define RAIN_IN_PIN                 GPIO_PIN_13
#define RAIN_IN_GPIO_Port           GPIOC
#define OSC32_IN_PIN                GPIO_PIN_14
#define OSC32_IN_GPIO_Port          GPIOC
#define OSC32_OUT_PIN               GPIO_PIN_15
#define OSC32_OUT_GPIO_Port         GPIOC

#define FSMC_D2                 GPIO_PIN_0 
#define FSMC_D2_GPIO_Port       GPIOD
#define FSMC_D3_PIN             GPIO_PIN_1
#define FSMC_D3_GPIO_Port       GPIOD
#define SDIO_CMD_PIN            GPIO_PIN_2
#define SDIO_CMD_GPIO_Port      GPIOD
#define SDIO_DETECT_PIN         GPIO_PIN_3
#define SDIO_DETECT_GPIO_Port   GPIOD
#define FSMC_NOE_PIN            GPIO_PIN_4
#define FSMC_NOE_GPIO_Port      GPIOD
#define FSMC_NWE_PIN            GPIO_PIN_5
#define FSMC_NWE_GPIO_Port      GPIOD   
#define UNUSED_PIN_D6           GPIO_PIN_6 
#define UNUSED_PIN_D6_GPIO_Port GPIOD
#define FSMC_NE1_PIN            GPIO_PIN_7
#define FSMC_NE1_GPIO_Port      GPIOD
#define FSMC_D13_PIN            GPIO_PIN_8
#define FSMC_D13_GPIO_Port      GPIOD
#define FSMC_D14_PIN            GPIO_PIN_9
#define FSMC_D14_GPIO_Port      GPIOD
#define FSMC_D15X_PIN           GPIO_PIN_10
#define FSMC_D15_GPIO_Port      GPIOD
#define FSMC_A16_PIN            GPIO_PIN_11
#define FSMC_A16_GPIO_Port      GPIOD
#define FSMC_A17_PIN            GPIO_PIN_12
#define FSMC_A17_GPIO_Port      GPIOD
#define FSMC_A18_PIN            GPIO_PIN_13
#define FSMC_A18_GPIO_Port      GPIOD
#define FSMC_D0_PIN             GPIO_PIN_14
#define FSMC_D0_GPIO_Port       GPIOD
#define FSMC_D1_PIN             GPIO_PIN_15
#define FSMC_D1_GPIO_Port       GPIOD


#define FSMC_NBL0                GPIO_PIN_0 
#define FSMC_NBL0_GPIO_Port      GPIOE
#define FSMC_NBL1_PIN            GPIO_PIN_1
#define FSMC_NBL1_GPIO_Port      GPIOE
#define EX_UART_RST_A_PIN        GPIO_PIN_2
#define EX_UART_RST_A_GPIO_Port  GPIOE
#define FSMC_A19_PIN             GPIO_PIN_3
#define FSMC_A19_GPIO_Port       GPIOE
#define FSMC_A20_PIN             GPIO_PIN_4
#define FSMC_A20_GPIO_Port       GPIOE
#define SPI1_CS_RTC_PIN          GPIO_PIN_5
#define SPI1_CS_RTC_GPIO_Port    GPIOE     
#define EX_UART_RST_B_PIN        GPIO_PIN_6 
#define EX_UART_RST_B_GPIO_Port  GPIOE
#define FSMC_D4_PIN              GPIO_PIN_7
#define FSMC_D4_GPIO_Port        GPIOE
#define FSMC_D5_PIN              GPIO_PIN_8
#define FSMC_D5_GPIO_Port        GPIOE
#define FSMC_D6_PIN              GPIO_PIN_9
#define FSMC_D6_GPIO_Port        GPIOE
#define FSMC_D7_PIN              GPIO_PIN_10
#define FSMC_D7_GPIO_Port        GPIOE
#define FSMC_D8_PIN              GPIO_PIN_11
#define FSMC_D8_GPIO_Port        GPIOE
#define FSMC_D9_PIN              GPIO_PIN_12
#define FSMC_D9_GPIO_Port        GPIOE
#define FSMC_D10_PIN             GPIO_PIN_13
#define FSMC_D10_GPIO_Port       GPIOE
#define FSMC_D11_PIN             GPIO_PIN_14
#define FSMC_D11_GPIO_Port       GPIOE
#define FSMC_D12_PIN             GPIO_PIN_15
#define FSMC_D12_GPIO_Port       GPIOE



#define FSMC_A0               GPIO_PIN_0 
#define FSMC_A0_GPIO_Port     GPIOF
#define FSMC_A1_PIN           GPIO_PIN_1
#define FSMC_A1_GPIO_Port     GPIOF
#define FSMC_A2_PIN           GPIO_PIN_2
#define FSMC_A2_GPIO_Port     GPIOF
#define FSMC_A3_PIN           GPIO_PIN_3
#define FSMC_A3_GPIO_Port     GPIOF
#define FSMC_A4_PIN           GPIO_PIN_4
#define FSMC_A4_GPIO_Port     GPIOF
#define FSMC_A5_PIN           GPIO_PIN_5
#define FSMC_A5_GPIO_Port     GPIOF
#define TIM10_CH1_PIN         GPIO_PIN_6 
#define TIM10_CH1_GPIO_Port   GPIOF
#define TIM11_CH1_PIN         GPIO_PIN_7
#define TIM11_CH1_GPIO_Port   GPIOF
#define RESET_H_PIN           GPIO_PIN_8
#define RESET_H_GPIO_Port     GPIOF
#define RTS_H_PIN             GPIO_PIN_9
#define RTS_H_GPIO_Port       GPIOF
#define CD_H_PIN              GPIO_PIN_10
#define CD_H_GPIO_Port        GPIOF
#define UNSUED_F11_PIN        GPIO_PIN_11
#define UNSUED_F11_GPIO_Port  GPIOF
#define FSMC_A6_PIN           GPIO_PIN_12
#define FSMC_A6_GPIO_Port     GPIOF
#define FSMC_A7_PIN           GPIO_PIN_13
#define FSMC_A7_GPIO_Port     GPIOF
#define FSMC_A8_PIN           GPIO_PIN_14
#define FSMC_A8_GPIO_Port     GPIOF
#define FSMC_A9_PIN           GPIO_PIN_15
#define FSMC_A9_GPIO_Port     GPIOF


#define FSMC_A10                GPIO_PIN_0 
#define FSMC_A10_GPIO_Port      GPIOG
#define FSMC_A11_PIN            GPIO_PIN_1
#define FSMC_A11_GPIO_Port      GPIOG
#define FSMC_A12_PIN            GPIO_PIN_2
#define FSMC_A12_GPIO_Port      GPIOG
#define FSMC_A13_PIN            GPIO_PIN_3
#define FSMC_A13_GPIO_Port      GPIOG
#define FSMC_A14_PIN            GPIO_PIN_4
#define FSMC_A14_GPIO_Port      GPIOG
#define FSMC_A15_PIN            GPIO_PIN_5
#define FSMC_A15_GPIO_Port      GPIOG     
#define DIR_RS485_A_PIN         GPIO_PIN_6 
#define DIR_RS485_A_GPIO_Port   GPIOG
#define DIR_RS485_B_PIN         GPIO_PIN_7
#define DIR_RS485_B_GPIO_Port   GPIOG
#define DIR_SDI_PIN             GPIO_PIN_8
#define DIR_SDI_GPIO_Port       GPIOG
#define FSMC_NE2_PIN            GPIO_PIN_9
#define FSMC_NE2_GPIO_Port      GPIOG
#define FSMC_NE3_PIN            GPIO_PIN_10
#define FSMC_NE3_GPIO_Port      GPIOG
#define ETH_TX_EN_PIN           GPIO_PIN_11
#define ETH_TX_EN_GPIO_Port     GPIOG
#define SW_SYS_PIN              GPIO_PIN_12
#define SW_SYS_GPIO_Port        GPIOG
#define NOR_RESET_PIN           GPIO_PIN_13
#define NOR_RESET_GPIO_Port     GPIOG
#define INT_RTC_PIN             GPIO_PIN_14
#define INT_RTC_GPIO_Port       GPIOG
#define RAIN_DETECT_PIN         GPIO_PIN_15
#define RAIN_DETECT_GPIO_Port   GPIOG

#define OSC_IN_PIN              GPIO_PIN_0
#define OSC_IN_GPIO_Port        GPIOH
#define OSC_OUT_H1_PIN          GPIO_PIN_1
#define OSC_OUT_H1_GPIO_Port    GPIOH    
#define ETH_RST_PHY             GPIO_PIN_02
#define ETH_RST_PHY_GPIO_Port   GPIOH
#define ETH_RX_ER_PIN           GPIO_PIN_3
#define ETH_RX_ER_GPIO_Port     GPIOH
#define I2C2_SCL_PIN            GPIO_PIN_4
#define I2C2_SCL_GPIO_Port      GPIOH
#define I2C2_SDA_PIN            GPIO_PIN_5
#define I2C2_SDA_GPIO_Port      GPIOH
#define CON_PWR_CDMA_PIN        GPIO_PIN_6 
#define ICON_PWR_CDMA_GPIO_Port GPIOH
#define I2C3_SCL_PIN            GPIO_PIN_7
#define I2C3_SCL_GPIO_Port      GPIOH
#define I2C3_SDA_PIN            GPIO_PIN_8
#define I2C3_SDA_GPIO_Port      GPIOH
#define SYS_RUN_PIN             GPIO_PIN_9
#define SYS_RUN_GPIO_Port       GPIOH
#define ADC_EN_RTD_PIN          GPIO_PIN_10
#define ADC_EN_RTD_GPIO_Port    GPIOH
#define ADC_EN_ODD_PIN          GPIO_PIN_11
#define ADC_EN_ODD_GPIO_Port    GPIOH
#define ADC_EN_EVEN_PIN         GPIO_PIN_12
#define ADC_EN_EVEN_GPIO_Port   GPIOH
#define ADC_SEL_A0_PIN          GPIO_PIN_13
#define ADC_SEL_A0_GPIO_Port    GPIOH
#define ADC_SEL_A1_PIN          GPIO_PIN_14
#define ADC_SEL_A1_GPIO_Port    GPIOH
#define ADC_SEL_A2_PIN          GPIO_PIN_15
#define ADC_SEL_A2_GPIO_Port    GPIOH


#define SPI2_NSS                GPIO_PIN_0 
#define SPI2_NSS_GPIO_Port      GPIOI
#define SPI2_SCK_PIN            GPIO_PIN_1
#define SPI2_SCK_GPIO_Port      GPIOI
#define SPI2_MISO_PIN           GPIO_PIN_2
#define SPI2_MISO_GPIO_Port     GPIOI
#define SPI2_MOSI_PIN           GPIO_PIN_3
#define SPI2_MOSI_GPIO_Port     GPIOI
#define EX_UART_INT1_PIN        GPIO_PIN_4
#define EX_UART_INT1_GPIO_Port  GPIOI
#define EX_UART_INT2_PIN        GPIO_PIN_5
#define EX_UART_INT2_GPIO_Port  GPIOI     
#define EX_UART_INT3_PIN        GPIO_PIN_6 
#define EX_UART_INT3_GPIO_Port  GPIOI
#define EX_UART_INT4_PIN        GPIO_PIN_7
#define EX_UART_INT4_GPIO_Port  GPIOI
#define EX_UART_INT5_PIN        GPIO_PIN_8
#define EX_UART_INT5_GPIO_Port  GPIOI
#define EX_UART_INT6_PIN        GPIO_PIN_9
#define EX_UART_INT6_GPIO_Port  GPIOI
#define EX_UART_INT7_PIN        GPIO_PIN_10
#define EX_UART_INT7_GPIO_Port  GPIOI
#define EX_UART_INT8_PIN        GPIO_PIN_11
#define EX_UART_INT8_GPIO_Port  GPIOI


#ifdef __cplusplus
}
#endif

#endif
