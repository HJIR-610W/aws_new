## 신규 AWS

### 설명
- 기상청 프로토콜 기준으로 센서 데이터 수집 후 전송

### 메모리 구성
- SRAM(4MB)
SRAM 0x64000000 1MB:코드 변수용으로 사용 ,stm32f407xx_flash.icf파일에서 1MB 섹션 정의하여 사용
TLSF 0x64100000 3MB:tlsf 사용 

- FSMC  
NE1 (FSMC_NORSRAM_BANK1): 0x60000000 - 0x63FFFFFF (64MB) ST7920     8bit
NE2 (FSMC_NORSRAM_BANK2): 0x64000000 - 0x67FFFFFF (64MB) SRAM      16bit 
NE3 (FSMC_NORSRAM_BANK3): 0x68000000 - 0x6BFFFFFF (64MB) QUAD UART 16bit



