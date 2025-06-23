#ifndef HTTP_API_H
#define HTTP_API_H

#include <stdint.h>
#include <stdbool.h>

void http_api_handle_sensor_get(int client_socket);
void http_api_handle_device_get(int client_socket);
void http_api_handle_device_post(int client_socket, const char* body);
void http_api_handle_sensor_config_get(int client_socket, const char* sensor_type);
void http_api_handle_sensor_config_post(int client_socket, const char* body, const char* sensor_type);

#endif