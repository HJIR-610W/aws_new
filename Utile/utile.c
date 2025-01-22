

#include <stdint.h>

int getPinNumber(uint16_t pin) {
  if (pin == 0) {
    return -1; // 입력이 0인 경우, 유효하지 않음
  }

  int position = 0;
  while (pin != 0) {
    if (pin & 1) {
      return position; // 첫 번째로 1인 비트의 위치 반환
    }
    pin >>= 1;
    position++;
  }
  return -1; // 비트가 1인 위치가 없으면 -1 반환
}



void hex_to_binary_string(uint16_t hex_value, char *binary_str, int bit_length) {
    // bit_length만큼의 이진수 문자열 생성
    for (int i = bit_length - 1; i >= 0; i--) {
        binary_str[bit_length - 1 - i] = (hex_value & (1 << i)) ? '1' : '0';
    }
    binary_str[bit_length] = '\0';  // 문자열 종료 문자 추가
}





uint16_t swap_uint16(uint16_t value)
{
  uint16_t ret = 0;

  ret = (value >> 8 &0x00FF);
  ret |= (value << 8);

  return ret;
}