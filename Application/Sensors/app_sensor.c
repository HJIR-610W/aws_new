
#include "app_sensor.h"
#include "utile.h"

#include "config.h"
const char *sensorTypeList[14]={"미사용",         /* 0 S_T_UNSUED */
                                "ADC",            /* 1 S_T_ADC */
                                "RS232",          /* 2 S_T_RS232 */
                                "RS485",          /* 3 S_T_RS485 */
                                "MODBUS",         /* 4 S_T_MODBUS */
                                "HART",           /* 5 S_T_HART */
                                "FREQ_0",         /* 6 S_T_FREQ_0*/
                                "REED 0.5mm",     /* 7 S_T_RAIN_REED_05MM */
                                "REED 1mm",       /* 8 S_T_RAIN_REED_1MM */
                                "HALL 0.5mm",     /* 9 S_T_RAIN_HALL_05MM */
                                "HALL 1mm",       /* 10 S_T_RAIN_HALL_1MM */
                                "DI_0",           /* 11 S_T_DI_0 */
                                "SNOW_HJ_RS485", /* 12 S_T_HJ_SNOW_RS485 */
                                "RAIN_SERIAL"};  /* 13 S_T_RAIN_SERIAL */





config_manager_t s_config;




adc_config_t * get_adc_config(sensor_t *sensor)
{
  for (int i = 0; i < sensor->configCnt; i++)
  {
    if (sensor->config[i][0] == S_T_ADC)
    {
      return &s_config.adc[sensor->config[i][1]];
    }
  }
  return 0;
}

void set_adc_config(sensor_t *sensor,adc_config_t *adc)
{
  for (int i = 0; i < sensor->configCnt; i++)
  {
    if (sensor->config[i][0] == S_T_ADC)
    {
      s_config.adc[sensor->config[i][1]] = *adc;
      break;
    }
  }
}


void set_sensor_type(sensor_t *sensor,uint8_t type)
{
  sensor->type = type;
}




/**
 * @brief 센서에 ADC 설정값 추가
 * 
 */
void add_adc_sensor_config(sensor_t *sensor,adc_config_t *adc)
{
  uint8_t cnt=0;

  for(int i = 0 ; i< _countof(sensor->config);i++)
  {
    if(sensor->config[i][0] == S_T_ADC)
    {
      s_config.adc[sensor->config[i][1]] = *adc;
    }
  }

  if(cnt==0)
  {
    sensor->config[sensor->configCnt++][0] = S_T_ADC;
    s_config.adc[s_config.adc_cnt++] = *adc;
  }
}


void write_config_mem(void *mem)
{
  
}



void * sensor_add(sensor_t *sensor,uint8_t sensorType)
{
  uint8_t index=0;

  switch (sensorType)
    {
      case S_T_ADC:
      if(s_config.adc_cnt < _countof(s_config.adc))//할당 가능한지 판단
      {
        sensor->config[sensor->configCnt][0] = sensorType;//해당 타입을 추가

        WRITE_CFG_MEM(&sensor->config[sensor->configCnt][0],sizeof(sensor->config[sensor->configCnt][0]));
        sensor->configCnt++;
        WRITE_CFG_MEM(&sensor->configCnt,sizeof(sensor->configCnt));

        index = s_config.adc_cnt;
        s_config.adc_cnt++;
        WRITE_S_CFG(adc_cnt);
        return &s_config.adc[index];
      }
      return 0;
      case S_T_RS232:
      case S_T_RAIN_SERIAL:
      if(s_config.rs232_cnt < _countof(s_config.rs232))
      {
        sensor->config[sensor->configCnt][0] = sensorType;//해당 타입을 추가

        WRITE_CFG_MEM(&sensor->config[sensor->configCnt][0],sizeof(sensor->config[sensor->configCnt][0]));
        sensor->configCnt++;
        WRITE_CFG_MEM(&sensor->configCnt,sizeof(sensor->configCnt));

        index = s_config.rs232_cnt;
        s_config.rs232_cnt++;
        WRITE_S_CFG(rs232_cnt);
        return &s_config.rs232[index];
      }
      return 0;
      break;
      case S_T_RS485:
      if(s_config.rs485_cnt < _countof(s_config.rs485))
      {
        sensor->config[sensor->configCnt][0] = sensorType;//해당 타입을 추가

        WRITE_CFG_MEM(&sensor->config[sensor->configCnt][0],sizeof(sensor->config[sensor->configCnt][0]));
        sensor->configCnt++;
        WRITE_CFG_MEM(&sensor->configCnt,sizeof(sensor->configCnt));

        index = s_config.rs485_cnt;
        s_config.rs485_cnt++;
        WRITE_S_CFG(rs485_cnt);
        return &s_config.rs485[index];
      }
      return 0;
      break;
      break;
    
    default:
      break;
    }

    return 0;
}

void * get_sensor_config(sensor_t *sensor,uint8_t sensorType)
{
  int index;
  int configCnt;

  //저장된 config정보가 없으면 생성성
  if(sensor->configCnt==0)
  {
    return sensor_add(sensor,sensorType);
  }

  for (int i = 0; i < sensor->configCnt; i++)
  {
    if (sensor->config[i][0] == sensorType)
    {
      switch(sensorType)
      {
        case S_T_ADC:
        return &s_config.adc[sensor->config[i][1]];
        break;
        case S_T_RS232:
        case S_T_RAIN_SERIAL:
        return &s_config.rs232[sensor->config[i][1]];
        break;
        case S_T_RS485:
        return &s_config.rs485[sensor->config[i][1]];
        break;
        case S_T_MODBUS:
        return &s_config.modbus[sensor->config[i][1]];
        break;
      }
    }
  }
  //해당 센서 타입 config가 설정되어 있지 않으면 추가 
  return sensor_add(sensor,sensorType);
}


void add_sensor_config(sensor_t *sensor,void *config,uint8_t sensorType)
{
  uint8_t cnt=0;

  for(int i = 0 ; i< _countof(sensor->config);i++)
  {
    if(sensor->config[i][0] == sensorType)
    {
      switch(sensorType)
      {
        case S_T_ADC:
        s_config.adc[sensor->config[i][1]] = *(adc_config_t *)config;
        break;
      }

    }
  }

  if(cnt==0)
  {
    switch (sensorType)
    {
      case S_T_ADC:
      sensor->config[sensor->configCnt++][0] = S_T_ADC;
      s_config.adc[s_config.adc_cnt++] = *(adc_config_t *)config;
      break;
    
    default:
      break;
    }
  }
}

