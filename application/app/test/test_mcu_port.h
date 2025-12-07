/**
 * @file test_mcu_port.h
 * @brief STM32F407IG MCU GPIO ¡‹ ®»0¡ ‰T
 */

#ifndef TEST_MCU_PORT_H
#define TEST_MCU_PORT_H

/**
 * @brief STM32F407IG MCU GPIO ¡‹ ®»0¡
 * @details STM32F407IG MCUX ®‡ GPIO Ï∏(PA ~ PI)X Ö% ¡‹| }¥ ú%i»‰.
 *
 *          ı 0•:
 *          1. ®‡ GPIO Ï∏ ¡‹ \‹
 *             - 9 Ï∏ (GPIOA ~ GPIOI) x 16@X ¡‹| Lt ›<\ ú%
 *             - â: Ï∏ tÑ (PA, PB, PC, ...)
 *             - Ù: @ à8 (0 ~ 15)
 *             - : 0 (LOW) î 1 (HIGH)
 *
 *          2. π Ï∏ ¡8 ¡‹ \‹
 *             -  ›\ Ï∏X  @ ¡‹| ¡8à ú%
 *             - PA0: 1 (HIGH), PA1: 0 (LOW) ›
 *
 *          3. GPIO ¡‹ ¿T ®»0¡
 *             - ¨©ê ¿ ¸0(ms)\ GPIO ¡‹ ¿T ¿
 *             - ¡‹ ¿T › ‹ [Ï∏@] t í » ú%
 *             - : [PA5] 0 í 1
 *
 *          ¨© API:
 *          - HAL_GPIO_ReadPin() - STM32 HAL |tÏ¨ h
 *
 *          GPIO Ï∏:
 *          - GPIOA ~ GPIOI ( 9 Ï∏)
 *          -  Ï∏˘ 16 @ (0 ~ 15)
 */
void test_mcu_port(void);

#endif /* TEST_MCU_PORT_H */
