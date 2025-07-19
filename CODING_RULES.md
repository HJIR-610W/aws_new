# AWS App 코딩 규칙

## 로컬 변수 선언 규칙

### 선언 순서
1. **const 변수 가장 먼저**
2. **크기가 작은 순으로**
3. **가나다 정렬**

### 데이터 타입 크기 순서
1. `uint8_t`, `int8_t`, `bool`
2. `uint16_t`, `int16_t`
3. `uint32_t`, `int32_t`
4. `uint64_t`, `int64_t`
5. `float`
6. `double`
7. 포인터 타입 (`*`)

### 예시
```c
int32_t example_function(void)
{
  const uint8_t MAX_COUNT = 10;
  const char* DEFAULT_NAME = "default";
  
  uint8_t count;
  uint8_t error;
  uint8_t status;
  int32_t choice;
  int32_t index;
  int32_t result;
  float offset;
  float temperature;
  float voltage;
  adc_config_t* config;
  driver_t* driver;
  sensor_t* p_sensor;
  
  // 함수 구현...
}
```

## 전처리기 조건부 컴파일

### PCB 버전 조건부 컴파일
- `#ifndef PCB_0_6 #else #endif` 사용
- PCB_0_6이 정의되지 않은 경우: 현재 코드 그대로 적용
- PCB_0_6이 정의된 경우: 타이머 관련 함수만 수정하되 전체 구조는 동일하게 유지

## 파일 생성 규칙

### 기본 원칙
- 절대 필요한 경우가 아니면 파일 생성하지 않음
- 기존 파일 편집을 우선시
- 문서 파일(*.md)이나 README 파일은 명시적 요청 시에만 생성

### 우선 순위
1. 기존 파일 편집
2. 필요시에만 새 파일 생성
3. 문서화는 명시적 요청 시에만