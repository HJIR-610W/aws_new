#include "IO\dev_io.h"
#include "app_dataLogging.h"
#include "aws_data.h"
#include "cli_input.h"
#include "console_define.h"
#include "console_utile.h"
#include "dev_io.h"
#include "old_aws_define.h"
#include "util_memory.h"
#include "util_time.h"

void menu_data_display(void)
{
  int year;
  int month;
  int day;
  int hour;
  int min;
  int ret;
  int cnt;
  AWS_DATA_STRUCT aws;
  DATE_TIME_BUF nt;
  uint32_t startTime;
  io_printf("시작 시간을 입력해주세요(yyyy-mm-dd hh:mm)\r\n");
  ret = cli_scanf_s("%04d-%02d-%02d %02d:%02d", &year, &month, &day, &hour, &min);

  if (ret == CLI_KEYCODE_CTRL_C)
  {
    return;
  }

  io_printf("읽을 갯수를 입력해주세요요\r\n");
  ret = cli_scanf_s("%d", &cnt);

  if (ret == CLI_KEYCODE_CTRL_C)
  {
    return;
  }

  startTime = SetTime(year, month, day, hour, min, 0);

  time_cvt_secTotime(startTime, &nt);

  for (uint32_t n = 0; n < cnt; n++)
  {
    read_data_month(&nt, &aws, sizeof(aws), LOGGING_AWS, 1);

    io_printf("%04d-%02d-%02d %02d:%02d\r\n", nt.Year, nt.Month, nt.Day, nt.Hour, nt.Min);

    io_printf("온도      :%6.1f 일 최소: %6.1f 일 최대: %6.1f\r\n",
              READ_TEMP(aws.mTemperature.sReal), READ_TEMP(aws.mTemperature.sMin),
              (aws.mTemperature.sMax));

    io_printf("풍향      :%6.1f 1분 최대:%6.1f\r\n", READ_X10(aws.mWind.mDirection.sReal),
              READ_X10(aws.mWind.mDirection.sMax));

    io_printf("풍속      :%6.1f 1분 최대:%6.1f\r\n", READ_X10(aws.mWind.mSpeed.sReal),
              READ_X10(aws.mWind.mSpeed.sMax));

    io_printf("강우량(일): %6.1f 월: %6.1f 시간: %6.1f\r\n", READ_X10(aws.mRainFall.sReal),
              READ_X10(aws.mRainFall.sMin), READ_X10(aws.mRainFall.sMax));

    io_printf("기압      : %6.1f 일 최소: %6.1f 일 최대: %6.1f\r\n",
              READ_X10(aws.mBarometric.sReal), READ_X10(aws.mBarometric.sMin),
              READ_X10(aws.mBarometric.sMax));

    io_printf("강우감지  : %6d\r\n", aws.mRainDetect.sReal);

    io_printf("적설      : %6d\r\n", aws.mSnowFall.sReal);

    io_printf("습도      : %6.1f 일 최소: %6.1f 일 최대: %6.1f\r\n", READ_X10(aws.mHumidity.sReal),
              READ_X10(aws.mHumidity.sMin), READ_X10(aws.mHumidity.sMax));

    io_printf("일사      : %7.2f 하루 총: %.2f\r\n", READ_X100(aws.mSolarRad.sReal),
              READ_X100(aws.mSolarRad.sMax));
    io_printf("일조      : %6d  하루 총: %d\r\n", aws.mSunshine.sReal, aws.mSunshine.sMax);

// 지면온도 / 초상온도 / 지중온도
#define PRINT_RIX(label, obj)                                                           \
  io_printf(label " : %6.1f 일 최소: %6.1f 일 최대: %6.1f\r\n", READ_TEMP((obj).sReal), \
            READ_TEMP((obj).sMin), READ_TEMP((obj).sMax))

    PRINT_RIX("지면온도", aws.mGndTemp);
    PRINT_RIX("초상온도", aws.mGrassTemp);

    PRINT_RIX("지중온도  5cm", aws.mSoilTemp5cm);
    PRINT_RIX("지중온도 10cm", aws.mSoilTemp10cm);
    PRINT_RIX("지중온도 20cm", aws.mSoilTemp20cm);
    PRINT_RIX("지중온도 30cm", aws.mSoilTemp30cm);
    PRINT_RIX("지중온도 50cm", aws.mSoilTemp50cm);
    PRINT_RIX("지중온도 1.0m", aws.mSoilTemp1_0m);
    PRINT_RIX("지중온도 1.5m", aws.mSoilTemp1_5m);
    PRINT_RIX("지중온도 3.0m", aws.mSoilTemp3_0m);
    PRINT_RIX("지중온도 5.0m", aws.mSoilTemp5_0m);

#if 0 
    // 예비값 mSpare01 ~ mSpare15
    const SENSOR_RIX_BUF spareList[] = {
        aws.mSpare01, aws.mSpare02, aws.mSpare03, aws.mSpare04, aws.mSpare05,
        aws.mSpare06, aws.mSpare07, aws.mSpare08, aws.mSpare09, aws.mSpare10,
        aws.mSpare11, aws.mSpare12, aws.mSpare13, aws.mSpare14, aws.mSpare15,
    };

    for (int i = 0; i < 15; i++)
    {
      io_printf("Spare%02d 순간: %d 최소: %d 최대: %d\r\n", i + 1, spareList[i].sReal,
                   spareList[i].sMin, spareList[i].sMax);
    }
#endif
    // 상태값 출력
    io_printf("Status sReal: 0x%04X sMin: 0x%04X sMax: 0x%04X\r\n", aws.mStatus.sReal,
              aws.mStatus.sMin, aws.mStatus.sMax);

#if 0 
    // 예비 데이터 (cDataSpare) 출력
    io_printf("Data Spare:");
    for (int i = 0; i < sizeof(aws.cDataSpare); i++)
    {
      io_printf(" %02X", aws.cDataSpare[i]);
    }
    io_printf("\r\n");
#endif
    startTime += 60;
    time_cvt_secTotime(startTime, &nt);
  }
}

#define AWS_DATA_MENU_WITDH 30
int aws_menu_data(void)
{
  int choice, status;

  char* menu[] = {
      "1분자료 확인", "1분자료 편집(구현 예정) "};

  while (1)
  {
    status = choice_menu(AWS_DATA_MENU_WITDH, "데이터", menu, _countof(menu), &choice);
    if (status != MENU_OK)
      break ;

    switch (choice)
    {
      case 1:
        menu_data_display();
      break;
    }
  }

  return status;
}