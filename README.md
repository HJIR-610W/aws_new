# ST7920 LCD 드라이버 및 VT100 터미널 구현

## 프로젝트 개요
이 프로젝트는 ST7920 128x64 그래픽 LCD와 VT100 터미널 에뮬레이터를 위한 드라이버를 구현한 것입니다.

## 구현된 파일들

### 1. ST7920 LCD 드라이버
#### 파일 위치
- `Drivers/Components/lcd/st7920.h` - ST7920 드라이버 헤더
- `Drivers/Components/lcd/st7920.c` - ST7920 드라이버 구현

#### 주요 기능
- **인터페이스**: 시리얼 SPI 통신 (CS active high)
- **해상도**: 128x64 픽셀
- **텍스트 모드**: 4행 × 16열 (8×16 픽셀 문자)
- **그래픽 모드**: 픽셀 단위 제어 가능
- **프레임버퍼**: 그래픽 모드용 내장

#### ST7920 특징
- 3바이트 시퀀스 통신 (동기바이트 + 상위4비트 + 하위4비트)
- 그래픽 메모리 상하 2개 영역 분할 (상단 32줄, 하단 32줄)
- 초기화 시 3번 반복으로 안정화
- 특정 명령어는 처리 시간 필요 (clear: 1.6ms)

### 2. VT100 터미널 에뮬레이터
#### 파일 위치
- `Drivers/Components/lcd/vt100_terminal.h` - VT100 터미널 헤더
- `Drivers/Components/lcd/vt100_terminal.c` - VT100 터미널 구현

#### 주요 기능
- **통신**: UART를 통한 VT100 이스케이프 시퀀스 전송
- **크기**: 24행 × 80열 (표준 터미널)
- **제어 명령**: 화면 지우기, 커서 제어, 위치 설정
- **포맷 출력**: printf 스타일 문자열 포맷팅 지원

#### VT100 명령어
- `\033[2J` - 화면 지우기
- `\033[H` - 커서 홈으로
- `\033[y;xH` - 커서 위치 설정
- `\033[?25h/l` - 커서 표시/숨기기

### 3. 드라이버 통합 계층
#### 파일 위치
- `Drivers/Driver/driver_lcd.h` - LCD 드라이버 통합 헤더
- `Drivers/Driver/driver_lcd.c` - LCD 드라이버 통합 구현

#### 드라이버 타입
- `DRIVER_CLCD (0)` - ST7920 하드웨어 LCD
- `DRIVER_LCD_TERMINAL (1)` - VT100 터미널 에뮬레이터

#### 공통 API
- `driver_lcd_open()` - 드라이버 초기화
- `driver_lcd_set_position()` - 커서 위치 설정
- `driver_lcd_write_string()` - 문자열 출력
- `driver_lcd_clear_screen()` - 화면 지우기
- `driver_lcd_home()` - 커서 홈으로
- `driver_lcd_display_on/off()` - 디스플레이 제어

### 4. 애플리케이션 계층
#### 파일 위치
- `Application/App_drivers/app_lcd.h` - 애플리케이션 LCD API 헤더
- `Application/App_drivers/app_lcd.c` - 애플리케이션 LCD API 구현

#### 편의 함수
- `clcd_init()` - LCD 초기화 및 켜기
- `clcd_printf()` - printf 스타일 출력
- `clcd_clear()` - 화면 지우기
- `clcd_set_position()` - 위치 설정
- `clcd_write_string()` - 문자열 출력

### 5. 테스트 코드
#### 파일 위치
- `Application/test/test_lcd.c` - LCD 테스트 함수

#### 테스트 기능
- 사용자가 CLCD 또는 TERMINAL 선택
- 4행 모든 라인에 동일한 숫자 출력 (0~9 순환)
- 1초마다 숫자 증가
- CTRL+Q로 종료

## 사용 방법

### 1. 기본 사용법
```c
// 드라이버 초기화
driver_t *lcd = driver_lcd_open(DRIVER_CLCD);  // 또는 DRIVER_LCD_TERMINAL

// 디스플레이 켜기
driver_lcd_display_on(lcd);

// 텍스트 출력
driver_lcd_set_position(lcd, 0, 0);  // x=0, y=0
driver_lcd_write_string(lcd, "Hello World!");

// 화면 지우기
driver_lcd_clear_screen(lcd);
```

### 2. 애플리케이션 API 사용법
```c
// 초기화
clcd_init();

// printf 스타일 출력
clcd_printf(0, 0, "Count: %d", counter);
clcd_printf(1, 0, "Status: %s", status);
```

### 3. 테스트 실행
```c
// 콘솔에서 실행
test_lcd();

// 사용자 입력:
// CLCD      - ST7920 하드웨어 LCD
// TERMINAL  - VT100 터미널 에뮬레이터
```

## 하드웨어 연결

### ST7920 LCD
- **SPI**: STM_SPI_1
- **CS**: DO_FLASH_CS (active high)
- **RST**: DO_HART_RESET

### VT100 터미널
- **UART**: UART_8_CDMA (115200 baud, 8N1)

## 주요 설정값

### ST7920
- 해상도: 128×64 픽셀
- 텍스트: 4행 × 16열
- 문자 크기: 8×16 픽셀

### VT100
- 터미널 크기: 24행 × 80열
- 통신 속도: 115200 baud
- 버퍼 크기: 266 바이트

## 코드 구조

```
Drivers/
├── Components/lcd/
│   ├── st7920.h/.c           # ST7920 드라이버
│   └── vt100_terminal.h/.c   # VT100 터미널
├── Driver/
│   └── driver_lcd.h/.c       # 드라이버 통합 계층
Application/
├── App_drivers/
│   └── app_lcd.h/.c          # 애플리케이션 API
└── test/
    └── test_lcd.c            # 테스트 코드
```

## 주의사항

1. **ST7920**: CS 신호가 active high임에 주의
2. **VT100**: UART 포트가 올바르게 설정되어야 함
3. **메모리**: 그래픽 모드 사용시 프레임버퍼 메모리 사용
4. **타이밍**: ST7920 명령어 처리 시간 준수 필요

## 향후 개선사항

1. 폰트 추가 지원
2. 그래픽 기능 확장
3. 다른 LCD 컨트롤러 지원
4. 성능 최적화