#ifndef CELLULAR_HAL_H
#define CELLULAR_HAL_H

#include <stdint.h>
#include <stddef.h>

/* Error codes */
#define CELLULAR_CONN_OK                0
#define CELLULAR_CONN_ERR_INVALID       -1
#define CELLULAR_CONN_ERR_IN_USE        -2
#define CELLULAR_CONN_ERR_NOT_CONN      -3
#define CELLULAR_CONN_ERR_TIMEOUT       -4
#define CELLULAR_CONN_ERR_INTERNAL      -5
#define CELLULAR_CONN_ERR_REFUSED       -6
#define CELLULAR_CONN_ERR_RESET         -7

/**
 * @brief Initialize the connection layer.
 * @return 0 on success, -1 on error.
 */
int cellular_conn_init(void);

/**
 * @brief Open a connection to a remote server.
 * @param ip IP address (4 bytes).
 * @param port Port number.
 * @return 0 on success, negative on error.
 */
int cellular_conn_open(uint8_t ip[4], uint16_t port);





/**
 * @brief Internal: Notify the connection of an error.
 * @param error_code Error code.
 */
void cellular_conn_notify_error(int error_code);

#endif /* CELLULAR_HAL_H */

