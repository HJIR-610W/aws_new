

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