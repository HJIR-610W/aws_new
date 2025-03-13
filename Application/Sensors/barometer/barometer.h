
#ifndef BAROMETER_H
#define BAROMETER_H


#define BAROMETER_ERR_VAL 1000


void barometer_init(sensor_t *sensor);

int32_t read_sensor_barometer(sensor_t *sensor,uint8_t *err);
#endif