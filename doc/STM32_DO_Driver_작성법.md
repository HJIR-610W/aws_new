# STM32 DO Driver 작성법

## 개요
STM32 마이크로컨트롤러를 위한 범용 DO (Digital Output) 드라이버 작성 방법을 설명합니다. 이 가이드는 PCB 버전이 바뀌어도 적용할 수 있는 일반적인 방법론을 제시하며, CSV 파일의 하드웨어 설계 정보를 정확히 반영하는 방법을 포함합니다.

## 1. 기본 구조 설계

### 파일 구성
- `bsp_stm32_do.h` : 헤더 파일 (매크로 정의, 함수 선언)  
- `driver_stm32_do.c` : 구현 파일 (실제 동작 코드)

### 명명 규칙
- 매크로: `STM32_DO_[핀이름]` (예: `STM32_DO_LED_RED`)
- 함수: `stm32_do_[동작]` (예: `stm32_do_init`, `stm32_do_high`)
- 구조체: `do_inst_t` (DO instance type)

## 2. 헤더 파일 작성 (bsp_stm32_do.h)

### 기본 템플릿
```c
#ifndef DRIVER_STM32_DO_H
#define DRIVER_STM32_DO_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"  // STM32 시리즈에 맞게 변경

// DO 핀 매크로 정의 (PCB별로 수정 필요)
#define STM32_DO_[핀이름1]    0
#define STM32_DO_[핀이름2]    1
#define STM32_DO_[핀이름3]    2
// ... 추가 핀들

#define STM32_DO_MAX          [총핀개수]

// 함수 선언
void stm32_do_init(void);
void stm32_do_low(int num);
void stm32_do_high(int num);

#endif /* DRIVER_STM32_DO_H */
```

### 매크로 정의 규칙
1. **순차 번호**: 0부터 시작하여 연속된 번호 할당
2. **의미있는 이름**: 핀의 기능을 명확히 표현
3. **일관된 접두어**: 모든 매크로에 `STM32_DO_` 접두어 사용
4. **MAX 정의**: 총 핀 개수를 나타내는 `STM32_DO_MAX` 매크로 필수

## 3. C 파일 작성 (driver_stm32_do.c)

### 기본 구조체 정의
```c
#include "bsp_stm32_do.h"
#include "[pcb_헤더파일].h"  // PCB별 핀 정의 파일

typedef struct do_inst_s
{
    bool opened;                // 초기화 여부
    GPIO_InitTypeDef init;      // GPIO 초기화 구조체
    GPIO_TypeDef *port;         // GPIO 포트 (GPIOA, GPIOB 등)
    GPIO_PinState init_state;   // 초기 상태 (HIGH/LOW)
} do_inst_t;
```

### 핀 설정 배열 템플릿 (CSV 기반)
```c
// CSV 파일의 정보를 정확히 반영한 설정
do_inst_t do_inst[STM32_DO_MAX] = {
    // CSV: PORTA.5, CON_PWR_RAIN, PULL_UP, LOW
    [STM32_DO_CON_PWR_RAIN] = {
        .init = {
            .Pin = DO_CON_PWR_RAIN_Pin,     // PCB 헤더에서 정의된 핀 매크로
            .Mode = GPIO_MODE_OUTPUT_PP,    // Push-Pull 출력 모드
            .Pull = GPIO_PULLUP,            // CSV '풀업' 컬럼 값 반영
            .Speed = GPIO_SPEED_FREQ_LOW    // 속도 설정
        },
        .port = DO_CON_PWR_RAIN_GPIO_Port, // PCB 헤더에서 정의된 포트 매크로  
        .init_state = GPIO_PIN_RESET       // CSV 'DO 초기값' 컬럼 값 반영 (LOW)
    },
    // CSV: PORTA.15, SPI1_NSS, PULL_UP, HIGH  
    [STM32_DO_SPI1_NSS] = {
        .init = {
            .Pin = DO_SPI1_NSS_Pin,
            .Mode = GPIO_MODE_OUTPUT_PP,
            .Pull = GPIO_PULLUP,            // CSV 풀업 설정
            .Speed = GPIO_SPEED_FREQ_LOW
        },
        .port = DO_SPI1_NSS_GPIO_Port,
        .init_state = GPIO_PIN_SET         // CSV 초기값 HIGH 반영
    },
    // ... 추가 핀 설정
};
```

### CSV 기반 핀 설정 가이드라인
- **Mode**: 모든 DO 핀에 `GPIO_MODE_OUTPUT_PP` (Push-Pull) 사용
- **Pull**: CSV '풀업' 컬럼 값을 다음과 같이 매핑
  - `PULL_UP` → `GPIO_PULLUP`
  - `PULL_DOWN` → `GPIO_PULLDOWN`  
  - `NO_PULL` → `GPIO_NOPULL`
- **Speed**: 일반적으로 `GPIO_SPEED_FREQ_LOW` 사용
- **init_state**: CSV 'DO 초기값' 컬럼을 다음과 같이 매핑
  - `HIGH` → `GPIO_PIN_SET`
  - `LOW` → `GPIO_PIN_RESET`
  - `NULL` 또는 빈값 → `GPIO_PIN_RESET` (기본값)

## 4. 핵심 함수 구현

### GPIO 클럭 활성화 함수
```c
static void stm32_do_enable_gpio_clock(GPIO_TypeDef *port)
{
    // 모든 가능한 GPIO 포트에 대해 클럭 활성화
    if (port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (port == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
    else if (port == GPIOG) __HAL_RCC_GPIOG_CLK_ENABLE();
    else if (port == GPIOH) __HAL_RCC_GPIOH_CLK_ENABLE();
    else if (port == GPIOI) __HAL_RCC_GPIOI_CLK_ENABLE();
    // STM32 시리즈에 따라 추가 포트 있을 수 있음
}
```

### 개별 핀 초기화 함수
```c
void stm32_do_gpio_init(int do_number)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 1. GPIO 클럭 활성화
    stm32_do_enable_gpio_clock(do_inst[do_number].port);
    
    // 2. 초기 상태 설정 (GPIO 설정 전에 먼저 설정)
    HAL_GPIO_WritePin(do_inst[do_number].port, 
                      do_inst[do_number].init.Pin, 
                      do_inst[do_number].init_state);
    
    // 3. GPIO 핀 설정 적용
    GPIO_InitStruct.Pin = do_inst[do_number].init.Pin;
    GPIO_InitStruct.Mode = do_inst[do_number].init.Mode;
    GPIO_InitStruct.Pull = do_inst[do_number].init.Pull;
    GPIO_InitStruct.Speed = do_inst[do_number].init.Speed;
    HAL_GPIO_Init(do_inst[do_number].port, &GPIO_InitStruct);
}
```

### 전체 시스템 초기화 함수
```c
void stm32_do_init(void)
{
    // 모든 DO 핀을 순차적으로 초기화
    for (int do_num = 0; do_num < STM32_DO_MAX; do_num++)
    {
        // 이미 초기화된 핀은 건너뛰기
        if (do_inst[do_num].opened) 
            continue;
        
        // 개별 핀 초기화 수행
        stm32_do_gpio_init(do_num);
        
        // 초기화 완료 표시
        do_inst[do_num].opened = true;
    }
}
```

### 핀 제어 함수들
```c
// 핀을 LOW 상태로 설정
void stm32_do_low(int num)
{
    // 범위 검사
    if (num >= 0 && num < STM32_DO_MAX)
    {
        HAL_GPIO_WritePin(do_inst[num].port, 
                         do_inst[num].init.Pin, 
                         GPIO_PIN_RESET);
    }
}

// 핀을 HIGH 상태로 설정
void stm32_do_high(int num)
{
    // 범위 검사
    if (num >= 0 && num < STM32_DO_MAX)
    {
        HAL_GPIO_WritePin(do_inst[num].port, 
                         do_inst[num].init.Pin, 
                         GPIO_PIN_SET);
    }
}
```

## 5. CSV 파일 기반 PCB 적응 방법

### CSV 파일 구조 분석
PCB 핀 정의 CSV 파일은 다음과 같은 구조를 가집니다:
```csv
포트.핀,라벨,기능,풀업,AF,DO 초기값,설명
PORTA.5,CON_PWR_RAIN,DO,PULL_UP,NULL,LOW,
PORTA.15,SPI1_NSS,DO,PULL_UP,NULL,HIGH,FRAM
PORTB.9,SEL_IF_UART,DO,PULL_UP,NULL,LOW,rs232 hart선택
```

### DO 핀 식별 단계
1. **기능 컬럼 확인**: '기능' 컬럼이 'DO'인 행들만 선택
2. **핀 정보 추출**: '포트.핀' 컬럼에서 GPIO 포트와 핀 번호 파싱
3. **라벨 정보**: 매크로 이름 생성에 사용될 핀 라벨 추출
4. **풀업 설정**: '풀업' 컬럼 값으로 GPIO Pull 설정 결정
5. **초기값**: 'DO 초기값' 컬럼으로 시작 상태 결정

### 풀업 설정 매핑
CSV 파일의 풀업 값을 GPIO 설정으로 변환:
```c
// CSV: "PULL_UP" -> GPIO_PULLUP
// CSV: "PULL_DOWN" -> GPIO_PULLDOWN  
// CSV: "NO_PULL" -> GPIO_NOPULL
```

### 초기값 설정 매핑
CSV 파일의 DO 초기값을 GPIO 상태로 변환:
```c
// CSV: "HIGH" -> GPIO_PIN_SET
// CSV: "LOW" -> GPIO_PIN_RESET
// CSV: "NULL" 또는 빈값 -> GPIO_PIN_RESET (기본값)
```

### 자동 생성 Python 스크립트 예시
```python
import csv

def extract_do_pins_from_csv(csv_file):
    """CSV에서 DO 기능 핀들을 추출하는 함수"""
    do_pins = []
    
    with open(csv_file, 'r', encoding='utf-8-sig') as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row['기능'].strip() == 'DO':
                # 포트.핀 파싱
                port_pin = row['포트.핀'].strip()
                if '.' in port_pin:
                    port, pin = port_pin.split('.', 1)
                    port = port.replace('PORT', '')
                    
                    # Pull 설정 변환
                    pull_map = {
                        'PULL_UP': 'GPIO_PULLUP',
                        'PULL_DOWN': 'GPIO_PULLDOWN', 
                        'NO_PULL': 'GPIO_NOPULL'
                    }
                    pull_setting = pull_map.get(row['풀업'].strip(), 'GPIO_NOPULL')
                    
                    # 초기값 변환
                    init_value = 'GPIO_PIN_SET' if row['DO 초기값'].strip() == 'HIGH' else 'GPIO_PIN_RESET'
                    
                    do_pins.append({
                        'label': row['라벨'].strip(),
                        'port': port,
                        'pin': pin,
                        'pull': pull_setting,
                        'init_state': init_value,
                        'description': row.get('설명', '').strip()
                    })
    
    return do_pins

def generate_header_file(do_pins, output_file):
    """헤더 파일 생성"""
    with open(output_file, 'w') as f:
        f.write('#ifndef DRIVER_STM32_DO_H\n')
        f.write('#define DRIVER_STM32_DO_H\n\n')
        f.write('#include <stdint.h>\n')
        f.write('#include <stdbool.h>\n')
        f.write('#include "stm32f4xx_hal.h"\n\n')
        
        # 매크로 정의
        for i, pin in enumerate(do_pins):
            f.write(f'#define STM32_DO_{pin["label"]}    {i}\n')
        
        f.write(f'\n#define STM32_DO_MAX    {len(do_pins)}\n\n')
        
        # 함수 선언
        f.write('void stm32_do_init(void);\n')
        f.write('void stm32_do_low(int num);\n')
        f.write('void stm32_do_high(int num);\n\n')
        f.write('#endif /* DRIVER_STM32_DO_H */\n')

def generate_c_file(do_pins, output_file):
    """C 파일 생성"""
    with open(output_file, 'w') as f:
        # 헤더 포함
        f.write('#include "bsp_stm32_do.h"\n')
        f.write('#include "pcb_5_pin.h"\n')
        f.write('#include "stm32f4xx_hal.h"\n')
        f.write('#include <stddef.h>\n')
        f.write('#include <stdint.h>\n')
        f.write('#include <stdbool.h>\n\n')
        
        # 구조체 정의
        f.write('typedef struct do_inst_s\n{\n')
        f.write('    bool opened;\n')
        f.write('    GPIO_InitTypeDef init;\n')
        f.write('    GPIO_TypeDef *port;\n')
        f.write('    GPIO_PinState init_state;\n')
        f.write('} do_inst_t;\n\n')
        
        # 배열 생성
        f.write('// CSV 파일의 풀업과 DO 초기값을 정확히 반영한 설정\n')
        f.write('do_inst_t do_inst[STM32_DO_MAX] = {\n')
        
        for i, pin in enumerate(do_pins):
            f.write(f'    // {pin["port"]}.{pin["pin"]}, {pin["label"]}, {pin["pull"]}, {pin["init_state"]}\n')
            f.write(f'    [STM32_DO_{pin["label"]}] = {{.init = {{.Pin = DO_{pin["label"]}_Pin,\n')
            f.write(f'                                  .Mode = GPIO_MODE_OUTPUT_PP,\n')
            f.write(f'                                  .Pull = {pin["pull"]},\n')
            f.write(f'                                  .Speed = GPIO_SPEED_FREQ_LOW}},\n')
            f.write(f'                         .port = DO_{pin["label"]}_GPIO_Port,\n')
            f.write(f'                         .init_state = {pin["init_state"]}}},\n')
        
        f.write('};\n\n')
        
        # 나머지 함수들 생성...
```

## 6. 사용 방법

### 기본 사용 패턴
```c
#include "bsp_stm32_do.h"

int main(void)
{
    // STM32 HAL 초기화
    HAL_Init();
    SystemClock_Config();
    
    // DO 드라이버 초기화
    stm32_do_init();
    
    // 사용 예시
    stm32_do_high(STM32_DO_LED_GREEN);    // LED 켜기
    stm32_do_low(STM32_DO_RELAY_1);       // 릴레이 끄기
    
    while (1)
    {
        // 메인 루프
        stm32_do_high(STM32_DO_STATUS_LED);
        HAL_Delay(500);
        stm32_do_low(STM32_DO_STATUS_LED);
        HAL_Delay(500);
    }
}
```

## 7. 고려사항 및 주의점

### 하드웨어 고려사항
1. **전류 용량**: GPIO 핀의 최대 출력 전류 확인 (일반적으로 25mA)
2. **전압 레벨**: 3.3V 또는 5V 호환성 확인
3. **풀업/풀다운**: PCB 설계에 맞는 저항 설정
4. **초기 상태**: 시스템 안전을 위한 올바른 초기값 설정

### 소프트웨어 고려사항
1. **STM32 시리즈 호환성**: HAL 헤더 파일 경로 확인
2. **핀 번호 관리**: 매크로 번호의 일관성 유지
3. **에러 처리**: 범위 검사 및 초기화 상태 확인
4. **메모리 효율성**: 사용하지 않는 핀은 배열에서 제외 고려

### 디버깅 팁
1. **초기화 확인**: `opened` 플래그로 초기화 상태 확인
2. **핀 상태 확인**: 오실로스코프나 멀티미터로 실제 출력 확인
3. **클럭 확인**: GPIO 클럭이 올바르게 활성화되었는지 확인

## 8. 확장 가능성

### 추가 기능 구현 고려사항
```c
// 토글 기능
void stm32_do_toggle(int num);

// 상태 읽기 기능  
bool stm32_do_get_state(int num);

// PWM 기능 (타이머 사용)
void stm32_do_pwm_set(int num, uint8_t duty_cycle);
```

## 9. 템플릿 체크리스트

### 구현 전 확인사항
- [ ] PCB 버전 및 핀 정의 파일 확인
- [ ] STM32 시리즈 및 HAL 버전 확인  
- [ ] CSV 파일에서 DO 기능 핀들 식별
- [ ] CSV의 풀업 설정과 초기값 확인
- [ ] PCB 헤더 파일의 핀 매크로 확인

### CSV 파일 분석 체크리스트
- [ ] '기능' 컬럼이 'DO'인 행들 모두 추출
- [ ] '포트.핀' 정보 정확히 파싱 (예: PORTA.5)
- [ ] '라벨' 정보로 매크로 이름 생성 (예: STM32_DO_CON_PWR_RAIN)
- [ ] '풀업' 설정을 GPIO 설정으로 매핑
- [ ] 'DO 초기값'을 GPIO 상태로 매핑
- [ ] NULL 값 처리 방법 확인

### 구현 후 테스트사항
- [ ] 모든 핀 초기화 정상 동작
- [ ] HIGH/LOW 출력 정상 동작
- [ ] CSV 초기값이 정확히 반영되는지 확인
- [ ] 풀업/풀다운 설정이 올바른지 확인
- [ ] 범위 검사 정상 동작
- [ ] 멀티미터/오실로스코프로 실제 출력 확인

### 실제 적용 예시 (pcb_5_pin.csv 기준)
```c
// 총 28개 DO 핀, 모두 PULL_UP 설정
// 초기값 HIGH: 6개 (SPI1_NSS, STATUS_BTM, BTM_PWRC, CON_PWR_RAIN_DIGITAL, RV8803_EVI, SPI2_NSS)
// 초기값 LOW: 22개 (나머지 모든 핀)

stm32_do_init();                              // 모든 DO 핀 초기화
stm32_do_high(STM32_DO_SYS_RUN);             // 시스템 실행 신호 활성화
stm32_do_low(STM32_DO_CON_PWR_RAIN);         // 비 센서 전원 비활성화
```

이 가이드를 따라 구현하면 PCB 버전이 바뀌어도 CSV 파일만 교체하여 쉽게 적응할 수 있는 범용 STM32 DO 드라이버를 작성할 수 있습니다. CSV 파일의 하드웨어 설계 정보가 정확히 반영되어 안전하고 신뢰할 수 있는 드라이버가 됩니다.