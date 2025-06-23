#include "http_api.h"
#include "task_http_server.h"
#include "core_json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task_logging.h"
#include "user_heap.h"

#include "aws_data.h"

void http_api_handle_sensor_get(int client_socket)
{
  kma_data_ex_t* p_kma = get_kma_data(eAWS_DATA_AVG);
  if (p_kma == NULL)
  {
    http_send_response(client_socket, 500, "application/json",
                       "{\"status\":\"error\",\"message\":\"Sensor data not available\"}");
    return;
  }

  char* json_response = aws_malloc(2048);
  if (json_response == NULL)
  {
    http_send_response(client_socket, 500, "application/json",
                       "{\"status\":\"error\",\"message\":\"Memory allocation failed\"}");
    return;
  }

  snprintf(
      json_response, 2048,
      "{"
      "\"temperature\":%d,"
      "\"humidity\":%d,"
      "\"wind_direction\":%d,"
      "\"wind_speed\":%d,"
      "\"snow_depth\":%d,"
      "\"pressure\":%d,"
      "\"rainfall\":%d,"
      "\"rain_detected\":%d,"
      "\"gust_direction\":%d,"
      "\"gust_speed\":%d,"
      "\"solar_radiation\":%d,"
      "\"sunshine_hours\":%d,"
      "\"soil_temperature_5cm\":%d,"
      "\"soil_temperature_10cm\":%d," 
      "\"soil_temperature_20cm\":%d,"
      "\"soil_temperature_30cm\":%d," 
      "\"soil_temperature_50cm\":%d,"
      "\"soil_temperature_1m\":%d,"
      "\"soil_temperature_1_5m\":%d}",
      p_kma->temperature.data, p_kma->relative_humidity.data, p_kma->wind_direction_avg.data,
      p_kma->wind_speed_avg.data, p_kma->snowfall.data, p_kma->pressure.data,
      p_kma->precipitation.data, p_kma->precipitation_presence.data,
      p_kma->wind_direction_instant.data, p_kma->wind_speed_instant.data,
      p_kma->solar_radiation.data, p_kma->sunshine_duration.data, p_kma->soil_temperature_5cm.data,
      p_kma->soil_temperature_10cm.data, p_kma->soil_temperature_20cm.data,
      p_kma->soil_temperature_30cm.data, p_kma->soil_temperature_50cm.data,
      p_kma->soil_temperature_1m.data, p_kma->soil_temperature_1_5m.data);

  http_send_response(client_socket, 200, "application/json", json_response);
  aws_free(json_response);
}

void http_api_handle_device_get(int client_socket)
{
    char *json_response = aws_malloc(256);
    if (json_response == NULL) {
        http_send_response(client_socket, 500, "application/json", 
            "{\"status\":\"error\",\"message\":\"Memory allocation failed\"}");
        return;
    }
    
    snprintf(json_response, 256,
        "{\"id\":%d,\"protocol_type\":%d}",
        1001, 0);

    http_send_response(client_socket, 200, "application/json", json_response);
    aws_free(json_response);
}

void http_api_handle_device_post(int client_socket, const char* body)
{
    if (!body) {
        http_send_response(client_socket, 400, "application/json", 
            "{\"status\":\"error\",\"message\":\"No request body\"}");
        return;
    }

    JSONStatus_t result = JSONSuccess;
    char* value = NULL;
    size_t valueLength = 0;
    int new_id = -1;
    int new_protocol = -1;
    
    result = JSON_Search((char*)body, strlen(body), "id", strlen("id"), &value, &valueLength);
    if (result == JSONSuccess && value != NULL && valueLength > 0) {
        char id_str[32];
        if (valueLength < sizeof(id_str)) {
            strncpy(id_str, value, valueLength);
            id_str[valueLength] = '\0';
            new_id = atoi(id_str);
        }
    }
    
    result = JSON_Search((char*)body, strlen(body), "protocol_type", strlen("protocol_type"), &value, &valueLength);
    if (result == JSONSuccess && value != NULL && valueLength > 0) {
        char protocol_str[32];
        if (valueLength < sizeof(protocol_str)) {
            strncpy(protocol_str, value, valueLength);
            protocol_str[valueLength] = '\0';
            new_protocol = atoi(protocol_str);
        }
    }
    
    if (new_id > 0 && (new_protocol == 0 || new_protocol == 1)) {
        task_printf("HTTP API: Device configuration updated - ID: %d, Protocol: %d\r\n", new_id, new_protocol);
        http_send_response(client_socket, 200, "application/json", "{\"status\":\"success\"}");
    } else {
        char *error_msg = aws_malloc(256);
        if (error_msg == NULL) {
            http_send_response(client_socket, 500, "application/json", 
                "{\"status\":\"error\",\"message\":\"Memory allocation failed\"}");
            return;
        }
        
        if (new_id <= 0) {
            snprintf(error_msg, 256, "{\"status\":\"error\",\"message\":\"ID must be positive integer\"}");
        } else if (new_protocol != 0 && new_protocol != 1) {
            snprintf(error_msg, 256, "{\"status\":\"error\",\"message\":\"Protocol type must be 0 (KMA2) or 1 (KMA3)\"}");
        } else {
            snprintf(error_msg, 256, "{\"status\":\"error\",\"message\":\"Missing required fields\"}");
        }
        http_send_response(client_socket, 400, "application/json", error_msg);
        aws_free(error_msg);
    }
}

void http_api_handle_sensor_config_get(int client_socket, const char* sensor_type)
{
    char *json_response = aws_malloc(512);
    if (json_response == NULL) {
        http_send_response(client_socket, 500, "application/json", 
            "{\"status\":\"error\",\"message\":\"Memory allocation failed\"}");
        return;
    }
    
    if (strcmp(sensor_type, "temp") == 0) {
        snprintf(json_response, 512,
            "{"
            "\"sensor\":\"temp\","
            "\"model\":\"adc\","
            "\"mode\":\"single\","
            "\"channel\":0,"
            "\"gain\":1"
            "}");
    } else if (strcmp(sensor_type, "hum") == 0) {
        snprintf(json_response, 512,
            "{"
            "\"sensor\":\"hum\","
            "\"model\":\"serial\","
            "\"physical_port\":\"rs232\","
            "\"port\":\"RS232_1\","
            "\"baud_rate\":9600,"
            "\"address\":1"
            "}");
    } else {
        aws_free(json_response);
        http_send_response(client_socket, 404, "application/json", 
            "{\"status\":\"error\",\"message\":\"Sensor type not found\"}");
        return;
    }
    
    http_send_response(client_socket, 200, "application/json", json_response);
    aws_free(json_response);
}

void http_api_handle_sensor_config_post(int client_socket, const char* body, const char* sensor_type)
{
    if (!body || !sensor_type) {
        http_send_response(client_socket, 400, "application/json", 
            "{\"status\":\"error\",\"message\":\"Missing parameters\"}");
        return;
    }

    if (strcmp(sensor_type, "temp") != 0 && strcmp(sensor_type, "hum") != 0) {
        http_send_response(client_socket, 404, "application/json", 
            "{\"status\":\"error\",\"message\":\"Sensor type not found\"}");
        return;
    }

    task_printf("HTTP API: Sensor configuration updated for %s\r\n", sensor_type);
    http_send_response(client_socket, 200, "application/json", "{\"status\":\"success\"}");
}