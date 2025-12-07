/**
 * @file test_di.h
 * @brief ¿8 Ö%(DI) L§∏ ‰T
 */

#ifndef TEST_DI_H
#define TEST_DI_H

/**
 * @brief ¿8 Ö%(DI) ¡‹ ®»0¡ L§∏
 * @details BSP_DI_0 ~ BSP_DI_5 (6 Ï∏)X ¡‹| ‰‹<\ ®»0¡i»‰.
 *
 *          Ÿë:
 *          - 100ms ¸0\ ®‡ DI Ï∏X ¡‹| Ux
 *          - 0 ¡‹ ú%
 *          - ¡‹ ¿Ω ¿ ‹ Ï∏Ö¸ ¿Ω ¥© ú% (: LOW í HIGH)
 *          - }0 $X › ‹ $X T‹ ú%
 *          - \Ö ¡‹ ú%
 *          - CTRL+C\ ÖÃ
 *
 *          L§∏ Ï∏:
 *          - DI_0 (BSP_DI_0 = 20)
 *          - DI_1 (BSP_DI_1 = 21)
 *          - DI_2 (BSP_DI_2 = 22)
 *          - DI_3 (BSP_DI_3 = 23)
 *          - DI_4 (BSP_DI_4 = 24)
 *          - DI_5 (BSP_DI_5 = 25)
 */
void test_di(void);

#endif /* TEST_DI_H */
