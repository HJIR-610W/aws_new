
#include "wind_data.h"

#include <stdio.h>
#include <math.h>
#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void calculate_uv(double theta_deg, double s, double *u, double *v)
{
  double theta = theta_deg * M_PI / 180.0; // 도 -> 라디안
  *u = -s * sin(theta);
  *v = -s * cos(theta);
}

void calculate_wind(double u, double v, double *speed, double *direction_deg)
{
  // 1. 풍속 계산 (이 부분은 올바릅니다)
  *speed = sqrt(u * u + v * v);

  // 2. 수학적 각도 계산 (atan2의 인자 순서는 y, x 입니다)
  // u가 x축(동서), v가 y축(남북)에 해당합니다.
  double angle_rad = atan2(v, u);
  double angle_deg = angle_rad * 180.0 / M_PI;

  // 3. 기상학적 풍향으로 변환
  //    수학 각도(동쪽=0°) -> 풍향(북쪽=0°) 변환 및 바람의 방향(불어가는 쪽) -> 풍향(불어오는 쪽) 변환
  //    공식: 270 - (벡터 방향 각도) + 180 => 450 - (벡터 방향 각도)
  double wind_dir = 270.0 - angle_deg;

  // 4. 결과를 0~360도 범위로 정규화
  //    fmod는 부동소수점의 나머지 연산을 수행합니다.
  *direction_deg = fmod(wind_dir, 360.0);
  if (*direction_deg < 0)
  {
    *direction_deg += 360.0;
  }
}
