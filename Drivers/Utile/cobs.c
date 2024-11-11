


#include <stdint.h>

uint16_t cobs_decode(const uint8_t *input, uint16_t length, uint8_t *output) {
    uint16_t read_index = 0;  // 입력 버퍼 읽기 인덱스
    uint16_t write_index = 0; // 출력 버퍼 쓰기 인덱스
    uint8_t code;             // COBS 코드 바이트

    while (read_index < length) {
        code = input[read_index++];  // 코드 바이트 읽기

        // 유효하지 않은 코드 바이트는 에러 처리
        if (code == 0 || read_index + code - 1 > length) {
            return 0;  // 오류를 나타내기 위해 0을 반환
        }

        // 코드 바이트에 의해 지정된 바이트 복사
        for (uint8_t i = 1; i < code; i++) {
            output[write_index++] = input[read_index++];
        }

        // 코드 바이트가 최대 값이 아닌 경우 `0x00` 추가
        if (code != 0xFF && read_index < length) {
            output[write_index++] = 0;
        }
    }

    return write_index;  // 디코딩된 데이터의 길이를 반환
}


uint16_t cobs_encode(const uint8_t *input, uint16_t length, uint8_t *output) {
    uint16_t read_index = 0;       // 입력 버퍼에서 읽을 위치
    uint16_t write_index = 1;      // 출력 버퍼에서 쓸 위치 (코드 바이트 공간을 위해 1부터 시작)
    uint16_t code_index = 0;       // 코드 바이트의 위치
    uint8_t code = 1;              // 코드 바이트 (기본값은 1)

    // 입력 버퍼의 모든 바이트를 처리
    while (read_index < length) {
        if (input[read_index] == 0) {
            // `0x00` 바이트를 만나면, 현재 code 값을 설정하고 새로운 블록을 시작
            output[code_index] = code;
            code_index = write_index++; // 새로운 코드 바이트 위치
            code = 1;                   // 새로운 블록의 초기값
        } else {
            // `0x00`이 아닌 바이트를 출력 버퍼에 복사
            output[write_index++] = input[read_index];
            code++;

            // code가 최대값(255)이 되면, 강제로 새로운 블록을 시작
            if (code == 0xFF) {
                output[code_index] = code;
                code_index = write_index++;
                code = 1;
            }
        }
        read_index++;
    }

    // 마지막 코드 바이트 설정
    output[code_index] = code;

    // 패킷의 끝을 표시하기 위해 `0x00` 추가
    output[write_index++] = 0;

    return write_index; // 인코딩된 데이터의 길이를 반환
}
