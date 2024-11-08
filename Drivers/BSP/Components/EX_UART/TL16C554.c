

#include "TL16C554.h"

#include <stdio.h>

#define UART_CLOCK_FREQ 3686400

// DLAB 비트 마스크
#define DLAB_BIT 0x80 // LCR 레지스터의 DLAB 비트

#define LSR_DR 0x01 // Data Ready 비트
// LSR의 비트 마스크
#define LSR_THRE 0x20 // Transmitter Holding Register Empty 비트
  volatile uint8_t *exUartBaseAddress[8] = {(uint8_t*)0x68000000,
                                              (uint8_t*)0x68000010,
                                              (uint8_t*)0x68000020,
                                              (uint8_t*)0x68000030,
                                              (uint8_t*)0x68000040,
                                              (uint8_t*)0x68000050,
                                              (uint8_t*)0x68000060,
                                              (uint8_t*)0x68000070};


#define RBR(BASE) (void *)(BASE + 0x00) // Transmitter Holding Register
#define THR(BASE) (void *)(BASE + 0x00) // Transmitter Holding Register
#define DLL(BASE) (void *)(BASE + 0x00) // Divisor Latch Low
#define DLM(BASE) (void *)(BASE + 0x01) // Divisor Latch High
#define FCR(BASE) (void *)(BASE + 0x02) 
#define LCR(BASE) (void *)(BASE + 0x03) 
#define MCR(BASE) (void *)(BASE + 0x04) 
#define LSR(BASE) (void *)(BASE + 0x05) 

#define MSR(BASE) (void *)(BASE + 0x06) 
#define SCR(BASE) (void *)(BASE + 0x07) 


// 레지스터 오프셋
#define DLL_OFFSET 0x00 // Divisor Latch Low
#define DLM_OFFSET 0x01 // Divisor Latch High
#define LCR_OFFSET 0x03 // Line Control Register
#define FCR_OFFSET 0x02 // FIFO Control Register
#define MCR_OFFSET 0x04 // Modem Control Register



uint8_t read_register(void * addr) 
{
  uint8_t data;

  data =  *((volatile  uint8_t *)addr);
   
   return data;
}


// 레지스터 쓰기 함수
void write_register(void * addr, uint8_t value) 
{
    *((volatile uint8_t *)addr) = value;
}




// 보오드레이트 확인 함수
void check_baud_rate(int uart_num)
{
    uint8_t data;

  
    data = read_register(LCR(exUartBaseAddress[uart_num]));
    // LCR의 DLAB 비트를 1로 설정하여 DLL 및 DLM 접근 허용

    data |= 0x80; // DLAB 비트 설정


    write_register(LCR(exUartBaseAddress[uart_num]),data);

    uint8_t dll_value = read_register(DLL(exUartBaseAddress[uart_num]));
    uint8_t dlm_value = read_register(DLM(exUartBaseAddress[uart_num]));

    data =   read_register(LCR(exUartBaseAddress[uart_num])) ;
                                                           
    // DLAB 비트를 다시 0으로 설정하여 DLL 및 DLM 접근 비허용
    data &= ~0x80;
    
    write_register(LCR(exUartBaseAddress[uart_num]),data);

    // Divisor 계산 (DLM은 상위 바이트, DLL은 하위 바이트)
    uint16_t divisor = (dlm_value << 8) | dll_value;

    if (divisor == 0) {
        printf("Invalid divisor value.\n");
        return;
    }

    // 보오드레이트 계산
    uint32_t baud_rate = UART_CLOCK_FREQ / (16 * divisor);
    printf("Calculated Baud Rate: %u\n", baud_rate);
}



// 보오드레이트 설정 함수
void set_baud_rate(int uart_num,uint32_t baud_rate) 
{
    uint16_t divisor = UART_CLOCK_FREQ / (16 * baud_rate);

    // DLAB 비트를 1로 설정하여 DLL과 DLM에 접근 가능하게 함
    uint8_t lcr_value = read_register(LCR(exUartBaseAddress[uart_num]));
    write_register(LCR(exUartBaseAddress[uart_num]), lcr_value | DLAB_BIT);

    // Divisor 값 설정
    write_register(DLL(exUartBaseAddress[uart_num]), divisor & 0xFF); // 하위 바이트 설정
    write_register(DLM(exUartBaseAddress[uart_num]), (divisor >> 8) & 0xFF); // 상위 바이트 설정

    // DLAB 비트를 0으로 다시 설정하여 DLL과 DLM 접근 비허용
    write_register(LCR(exUartBaseAddress[uart_num]), lcr_value & ~DLAB_BIT);
}



void quad_init(driver_t *tls16c554) {

  int baud_rate = 115200;
  int uart_num = tls16c554->num;
    // 보오드레이트 설정을 위한 Divisor 계산
    uint16_t divisor = UART_CLOCK_FREQ / (16 * baud_rate);

    // DLAB 비트 설정 (LCR의 MSB 비트)
    write_register(LCR(exUartBaseAddress[uart_num]),0x80);
        
    // DLL과 DLM에 divisor 값 설정

    write_register(DLL(exUartBaseAddress[uart_num]),divisor & 0xFF);
    write_register(DLM(exUartBaseAddress[uart_num]),(divisor >> 8) & 0xFF);
        
        


    // DLAB 비트를 0으로 설정하여 LCR 설정
    write_register(LCR(exUartBaseAddress[uart_num]),0x03);
        
    
    // FIFO 설정 (FCR)
        write_register(FCR(exUartBaseAddress[uart_num]),0x07);// FIFO enable, RX/TX FIFO reset


    // MCR 설정 (필요에 따라 추가 설정)
        write_register(MCR(exUartBaseAddress[uart_num]),0x00);
}

// 데이터 전송 함수
void send_data(int uart_num,uint8_t data)
{
    // 송신 버퍼가 비어있을 때까지 대기
    while ((read_register(LSR(exUartBaseAddress[uart_num])) & LSR_THRE) == 0);

    // 데이터를 THR에 씁니다.
    write_register(THR(exUartBaseAddress[uart_num]), data);
}

void send_data_n(int uart_num,uint8_t *p_data,uint16_t dataLen)
{

  
  while(dataLen)
  {
    dataLen--;
    send_data(uart_num,*p_data++);
  }
}


// 데이터 읽기 함수
int read_data(int uart_num, uint8_t *data) {
    // LSR의 DR 비트를 확인하여 수신 버퍼에 데이터가 있는지 확인
    if (read_register(LSR(exUartBaseAddress[uart_num])) & LSR_DR) {
        *data = read_register(RBR(exUartBaseAddress[uart_num])); // RBR에서 데이터 읽기
        return 1; // 데이터 읽기 성공
    } else {
        return 0; // 데이터가 준비되지 않음
    }
}


void quad_send(driver_t *tls16c554,uint8_t *pData,uint16_t dataLen)
{
  while(dataLen)
  {
    dataLen--;
    send_data(tls16c554->num,*pData++);
  }
}

int quad_recv_byte(driver_t *tls16c554,uint8_t *data)
{
    // LSR의 DR 비트를 확인하여 수신 버퍼에 데이터가 있는지 확인
    if (read_register(LSR(exUartBaseAddress[tls16c554->num])) & LSR_DR) {
        *data = read_register(RBR(exUartBaseAddress[tls16c554->num])); // RBR에서 데이터 읽기
        return 1; // 데이터 읽기 성공
    } else {
        return 0; // 데이터가 준비되지 않음
    }  
}

void quad_set(driver_t *tls16c554,eTLS16C554_CMD_t cmd,void *option)
{
  tls16c554_cmd_config_t *config;

  switch(cmd)
  {
    case eUART_SET_CONFIG:
    config = (tls16c554_cmd_config_t*)option;

    set_baud_rate(tls16c554->num,config->baud) ;
    break;
  }

}





typedef struct adc_api_s
{
    void (*send)(driver_t *tls16c554,uint8_t *pData,uint16_t dataLen);
    void (*recv)(driver_t *tls16c554,uint8_t *pBuff,uint16_t rLen);
    int32_t (*recv_byte)(driver_t *tls16c554,uint8_t *pData);
    void (*set)(driver_t *tls16c554,eTLS16C554_CMD_t cmd,void *option);
    void (*init)(driver_t *tls16c554);
}tl16c554_api_t;




tl16c554_api_t g_tl16c554_api={.send =quad_send,
                              .recv_byte = quad_recv_byte,
                              .set= quad_set,
                              .init = quad_init};
driver_t g_quad_uart[8];



driver_t *tls16c554_open(int num)
{

  if(g_quad_uart[num].opened == true)
  {
    return &g_quad_uart[num];
  }

  g_quad_uart[num].num = num;
  g_quad_uart[num].api = &g_tl16c554_api;
  

  
  return &g_quad_uart[num];
}






void tls16c554_send(driver_t *tls16c554,uint8_t *pData,uint16_t dataLen)
{
  tl16c554_api_t *api = (tl16c554_api_t *)tls16c554->api;
  
  api->send(tls16c554,pData,dataLen);
}

void tls16c554_recv(driver_t *tls16c554,uint8_t *pBuff,uint16_t rLen)
{
  tl16c554_api_t *api = (tl16c554_api_t *)tls16c554->api;
  
  api->recv(tls16c554,pBuff,rLen);
}

void tls16c554_set(driver_t *tls16c554,eTLS16C554_CMD_t cmd,void *option)
{
  tl16c554_api_t *api = (tl16c554_api_t *)tls16c554->api;
  
  api->set(tls16c554,cmd,option);
    
}

int tls16c554_recv_byte(driver_t *tls16c554,uint8_t *data)
{
    tl16c554_api_t *api = (tl16c554_api_t *)tls16c554->api;
  
  return api->recv_byte(tls16c554,data);
}

void tls16c554_init(driver_t *tls16c554)
{
  tl16c554_api_t *api = (tl16c554_api_t *)tls16c554->api;
  
  api->init(tls16c554);
    
}


