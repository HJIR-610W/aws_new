### device driver 작성규칙
정의:디바이스 드라이버(장치 드라이버)

- drivers
  - bsp(각종 드라이버 라우팅 역활)
    - bsp_uart.c,bsp_uart.h(tl16c554.h와 driver_stm32_uart.h를 포함)
      보드가 제공 할 수 있는 모든 uart포트 노출
    - bsp_접두어
  - components(내장형 장치 드라이버)
    - flash(flash 관련 ic 드라이버)
      - at45db.c,at45db.h      
    - ex_uart(확장 uart 관련 드라이버)
      - tl16c554.c,tl16c554.h
  - devices(외장형 장치 드라이버)
    - 애플리케이션 구현에 따라 bsp_,drv_ 드라이버 사용가능
      일반적으로 drv 사용
      charger_a.c,charger_a.h
      charger_b.c,charger_b.h
      dev_charger.c
      dev_ 접두어 사용
  - driver(애플리케이션에서 호출하여 사용,단순 bsp호출역활)
    - bsp_uart.h를 사용하여 애플리케이션에서 필요한 포트만 노출
    - drv_접두어
  - driver_stm32(mcu 기반 드라이버)
    - driver_stm32_uart.h
    - stm32_접두어
     보드에 rs232,rs485,hart,sdi와 와 같은 외부 장치와 통신할 수 있는 포트가 있고


예) uart 기반 충전기 데이터 수집
    rs232포트는 다목적으로 애플리케이션에 따라 사용하므로 drv_uart를 사용
    int charger_init(int num,void *opt);
    int charger_read_solar(int num,uint8_t *err);
    매개변수 num은 충전기 모델종류
    #define CHARGER_A 0 //제조사 A
    #define CHARGER_B 1 //제조사 B


예)charger_a.c 작성 
충전기가 요구하는 통신 방식이 uart인경우
``` c
#include "drv_uart.h"
#include "dev_charger_a.h"

typedef struct charger_a_s
{
  bool opened;//드라이버가 초기화 되었는지 판단 여부 
  void *sem;//os 필요한 경우 자원 보호 방법 적용
  int rs232_port;//만약 고정포트 사용이 아니라 설정하여 사용하려면 변수 적용
}charger_a_instance_t;

charger_a_instance_t charger_a_inst;


int dev_charger_init(int num,void *opt)
{
  charger_config_t *cfg = (charger_config_t *)opt;
  uart_config_t uart_config;
  int status;
  if(charger_a_inst.opened)
  {
    return 1;
  }

  charger_a_inst.opened = true;
  charger_a_inst.rs232_port = cfg->port;//만약 고정된 포트 사용이라면 포트 번호 하드코딩
  uart_config.paraty = UART_PARITY_NONE;
  uart_config.baudrate = 57600; //이미 고정되어 사용되기 때문에 하드코딩 상관없음
  uart_config.stopbit = UART_STOPBIT_1;

  status = drv_uart_init(charger_a_inst.rs232_port,&uart_config);//uart를 초기화 한다.

  if(status )
  {
    return -1;
  }

  return 1;
}


int dev_charger_read_solar(int num,uint8_t *err)
{
  int len;
  char buff[10];
  int solar;

  if(charger_a_inst.opened == false)
  {
    return -1;
  }

  len = drv_uart_read(charger_a_inst.rs232_port,buff,10);

  solar = atoi(buff);
  return solar;
}
```
