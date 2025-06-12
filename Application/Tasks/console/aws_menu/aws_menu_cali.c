/**
 * @file console_cali.c
 * @brief ADC 캘리브레이션 메뉴 (최종 통합 버전)
 * @date 2025-04-22
 *
 */

#include <math.h>    // For NAN, isnan, fabsf
#include <stdarg.h>  // For va_list in io_printf stub
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>  // For atoi, atof (대안 입력 파싱 시)
#include <string.h>  // For memcpy, strcmp (필요시)

#include "IO\dev_io.h"
#include "adc_calibration.h"
#include "app_adc.h"
#include "app_file.h"
#include "cli_input.h"
#include "config_adc.h"
#include "console_define.h"
#include "console_scanf.h"
#include "console_utile.h"
#include "driver_adc.h"
#include "fsl_shell.h"
#include "mcu_utile.h"
#include "usDelay.h"
#include "util_filter.h"
#include "util_time.h"
#include "vt100_command.h"
#include "cli_input.h"
extern float g_current_temp;

extern bool wait_break(uint32_t timeoutms);

int wait_for_enter()
{
  int key = recv_key(osWaitForever);
  return (key == -1) ? MENU_ABORT : MENU_OK;
}




// --- 6. 메뉴 처리 함수 ---

/** @brief 채널 선택 (공통 로직) */
int select_channel(adc_channel_type_t type, int* channel_index)
{
  int max_ch = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED) ? (ADS1220_NUM_SINGLE_ENDED_CHANNELS - 1)
                                                       : (ADS1220_NUM_DIFFERENTIAL_CHANNELS - 1);
  const char* type_str = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED) ? "싱글 엔드" : "차동";
  char prompt[100];
  snprintf(prompt, sizeof(prompt), "채널 번호 입력 (%s: 0 ~ %d)", type_str, max_ch);
  return input_decimal_prompt(prompt, channel_index, 0, max_ch);
}

#define MENU_CALI_SINGLE 1
#define MENU_CALI_DIFF 2
/** @brief 공장 캘리브레이션 메뉴 처리 */
int handle_factory_calibration(int adc_num)
{
  char ch;
  int choice, channel_index, status;
  float cal_temp;
  adc_channel_type_t type;
  adc_cal_params_t* cal_params_ptr;
  adc_cal_point_t p1, p2;
  config_adc_adv_t* p_adc = get_adc_config(adc_num);

  while (1)
  {
    io_printf("+---------------------------------------+\r\n");
    io_printf("|           공장 캘리브레이션           |\r\n");
    io_printf("+---------------------------------------+\r\n");
    io_printf("|  1. 싱글 엔드 채널 캘리브레이션       |\r\n");
    io_printf("|  2. 차동 채널 캘리브레이션            |\r\n");
    io_printf("|     CTRL+C 이전,CTRL+Q 종료           |\r\n");
    io_printf("+---------------------------------------+\r\n");

    status = input_decimal_prompt("선택", &choice, 1, 2);

    if (status != MENU_OK)
    {
      break;
    }

    if (choice == 1 || choice == 2)
    {
      type = (choice == 1) ? ADC_CHANNEL_TYPE_SINGLE_ENDED : ADC_CHANNEL_TYPE_DIFFERENTIAL;
      status = select_channel(type, &channel_index);
      if (status != MENU_OK)
      {
        break;
      }

      cal_params_ptr = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
                           ? &p_adc->single_ended_cal[channel_index]
                           : &p_adc->differential_cal[channel_index];

      io_printf("\r\n--- %s 채널 %d 캘리브레이션 시작 ---\r\n", (type == 0 ? "SE" : "Diff"),
                channel_index);

      // Point 1 입력
      io_printf("1. 낮은 기준점(Low Reference)을 연결하고 엔터를 입력해주세요요\r\n");
      io_recv(&ch, 1, 60000);
      float avg = 0;
      int32_t adc_raw;
      int32_t avg_cnt = 0;
      while (1)
      {
        uint8_t err;

        if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
        {
          adc_raw = (int32_t)adc_read_single_raw(channel_index, &err);
        }
        else if (type == ADC_CHANNEL_TYPE_DIFFERENTIAL)
        {
          adc_raw = (int32_t)adc_read_diff_raw(channel_index, &err);
        }
        avg_cnt++;
        avg = recursive_avg_i(avg, adc_raw, avg_cnt);

        io_printf("RAW:%10d AVG:%10.0f\r\n", adc_raw, avg);
        if (!wait_break(10))
          break;
      }

      status = input_decimal_prompt("   측정된 RAW 값 입력", (int*)&p1.raw_value,
                             p_adc->bits->min_raw_value, p_adc->bits->max_raw_value);
      if (status != MENU_OK)
      {
        break;
      }

      status =
      input_float_prompt("   낮은 기준점의 실제 값(전압)을 입력하세요.", &p1.reference_value);
      if (status != MENU_OK)
      {
        return status;
      }

      // Point 2 입력
      io_printf("2. 높은 기준점(High Reference)을 연결하고 엔터를 입력해주세요\r\n");
      io_recv(&ch, 1, 60000);
      avg_cnt = 0;
      avg = 0;
      while (1)
      {
        uint8_t err;

        if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
        {
          adc_raw = (int32_t)adc_read_single_raw(channel_index, &err);
        }
        else if (type == ADC_CHANNEL_TYPE_DIFFERENTIAL)
        {
          adc_raw = (int32_t)adc_read_diff_raw(channel_index, &err);
        }
        avg_cnt++;
        avg = recursive_avg_i(avg, adc_raw, avg_cnt);

        io_printf("RAW:%10d AVG:%10.0f\r\n", adc_raw, avg);
        if (!wait_break(10))
          break;
      }
      status = input_decimal_prompt("측정된 RAW 값 입력", (int*)&p2.raw_value,
                             p_adc->bits->min_raw_value, p_adc->bits->max_raw_value);
      if (status != MENU_OK)
      {
        break;
      }
      status = input_float_prompt("높은 기준점의 실제 값(전압)을 입력하세요.", &p2.reference_value);
      if (status != MENU_OK)
      {
        break;;
      }

      // 캘리브레이션 온도 입력
      g_current_temp = read_current_temperature();  // 현재 온도 읽기
      io_printf("현재 측정된 온도: %.1f °C\r\n", g_current_temp);
      #if 0
      status =
          input_float_prompt("캘리브레이션 수행 온도를 입력하세요 (기본값: 현재 온도)", &cal_temp);
      if (status == MENU_ABORT || status == MENU_BACK)
      {
        return status;
      }
      if (status != MENU_OK)
        cal_temp = g_current_temp;  // 입력 실패 시 현재 온도 사용
#endif

      cal_temp = 25;
      if (adc_perform_factory_calibration(p_adc, cal_params_ptr, p1, p2, cal_temp))
      {
        save_adc_cali();
        io_printf("Slope:%e Offset:%e\r\n", cal_params_ptr->factory_offset,
                  cal_params_ptr->factory_offset);
        io_printf("캘리브레이션 성공! 설정이 NVM에 저장되었습니다.\r\n");
      }
      else
      {
        io_printf("오류: 캘리브레이션 실패.\r\n");
      }
    }
  }

  return status;
}

/** @brief 온도 보상 설정 메뉴 처리 */
int handle_temp_comp_setup(int adc_num)
{
  int choice, channel_index, status, method_choice;
  adc_channel_type_t type;
  adc_cal_params_t* params;

  config_adc_adv_t* p_adc = get_adc_config(adc_num);

  while (1)
  {
    io_printf("+---------------------------------------+\\rn");
    io_printf("|       --- 온도 보상 설정 ---          |\r\n");
    io_printf("+---------------------------------------+\r\n");
    io_printf("|  1. 싱글 엔드 채널 설정               |\r\n");
    io_printf("|  2. 차동 채널 설정                    |\r\n");
    io_printf("|     CTRL+C 이전,CTRL+Q 종료           |\r\n");
    io_printf("+---------------------------------------+\r\n");

    status = input_decimal_prompt("선택", &choice, 1, 2);
    if (status == MENU_ABORT || status == MENU_BACK)
    {
      return status;
    }

    if (status != MENU_OK)
    {
      continue;
    }

    if (choice == 1 || choice == 2)
    {
      type = (choice == 1) ? ADC_CHANNEL_TYPE_SINGLE_ENDED : ADC_CHANNEL_TYPE_DIFFERENTIAL;
      status = select_channel(type, &channel_index);
      if (status == MENU_ABORT || status == MENU_BACK)
      {
        return status;
      }

      if (status != MENU_OK)
      {
        continue;
      }

      params = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED) ? &p_adc->single_ended_cal[channel_index]
                                                       : &p_adc->differential_cal[channel_index];

      // 채널별 상세 설정 루프
      while (1)
      {
        io_printf("+--- 채널 %s[%d] 온도 보상 설정 ---+\r\n", (type == 0 ? "SE" : "Diff"),
                  channel_index);
        const char* method_str;
        switch (params->comp_method)
        {
          case TEMP_COMP_COEFF:
            method_str = "계수 사용";
            break;
          case TEMP_COMP_LUT:
            method_str = "LUT 사용";
            break;
          default:
            method_str = "사용 안함";
            break;
        }
        io_printf("| 현재 방식: %s\r\n", method_str);
        io_printf("+---------------------------------------+\r\n");
        io_printf("|  1. 보상 방식 변경                    |\r\n");
        io_printf("|  2. 온도 계수 설정 (방식=계수)        |\r\n");
        io_printf("|  3. LUT 데이터 설정/보기 (방식=LUT)   |\r\n");
        io_printf("|     CTRL+C 이전,CTRL+Q 종료           |\r\n");
        io_printf("+---------------------------------------+\r\n");

        status = input_decimal_prompt("선택", &choice, 1, 3);
        if (status == MENU_ABORT || status == MENU_BACK)
        {
          return status;
        }

        if (status != MENU_OK)
        {
          continue;
        }

        switch (choice)
        {
          case 1:  // 방식 변경
            status = input_decimal_prompt("새 방식 선택 (0:없음, 1:계수, 2:LUT)", &method_choice, 0, 2);
            if (status == MENU_ABORT || status == MENU_BACK)
              return status;
            params->comp_method = (temp_comp_method_t)method_choice;
            io_printf("보상 방식이 변경되었습니다.\r\n");
            save_adc_cali();  // NVM 저장 필요

            break;
          case 2:  // 계수 설정
            if (params->comp_method == TEMP_COMP_COEFF)
            {
              io_printf("현재 SlopeTC=%.6f, OffsetTC=%.6f\r\n", params->slope_temp_coeff,
                        params->offset_temp_coeff);
              status = input_float_prompt("새 Slope TempCo 입력", &params->slope_temp_coeff);
              if (status == MENU_ABORT || status == MENU_BACK)
              {
                return status;
              }

              if (status == MENU_OK)
              {
                status = input_float_prompt("새 Offset TempCo 입력", &params->offset_temp_coeff);
                if (status == MENU_ABORT || status == MENU_BACK)
                {
                  return status;
                }

                if (status == MENU_OK)
                {
                  io_printf("온도 계수가 업데이트되었습니다.\r\n");
                  save_adc_cali();  // NVM 저장 필요
                }
              }
            }
            else
            {
              io_printf("오류: 현재 보상 방식이 '계수 사용'이 아닙니다.\r\n");
            }

            break;
          case 3:  // LUT 설정/보기
#ifdef ADC_LUT
            if (params->comp_method == TEMP_COMP_LUT)
            {
              // TODO: LUT 보기 및 편집 기능 구현 (복잡함)
              // 예시: 현재 설정된 LUT 보기
              io_printf("현재 LUT 데이터 (최대 %d개):\r\n", MAX_LUT_SIZE);
              if (params->lut_size == 0)
              {
                io_printf("  (설정된 데이터 없음)\r\n");
              }
              else
              {
                io_printf("  Idx | Temp(C) | SlopeMult | OffsetCorr\r\n");
                io_printf("  --------------------------------------\r\n");
                for (uint8_t i = 0; i < params->lut_size; ++i)
                {
                  io_printf("  %2u | %7.1f | %9.6f | %9.6f\r\n", i,
                            params->temp_comp_lut[i].temperature,
                            params->temp_comp_lut[i].slope_multiplier,
                            params->temp_comp_lut[i].offset_correction);
                }
              }
              io_printf("\r\nLUT 데이터 편집 기능은 이 예제에 포함되지 않았습니다.\r\n");
              // 예시 LUT 채우기 호출 (디버그용)
              // populate_lut_example(params);
              // save_adc_cali(&g_adc_config_nvm);
            }
            else
            {
              io_printf("오류: 현재 보상 방식이 'LUT 사용'이 아닙니다.\r\n");
            }
            if (wait_for_enter() == MENU_ABORT)
              return MENU_ABORT;
            break;
#endif
          case 'b':
            goto channel_setup_exit;  // 채널 설정 루프 탈출
          default:
            io_printf("잘못된 선택입니다.\r\n");

            break;
        }
      }  // end channel setup loop
    channel_setup_exit:;
    }

  }  // end main loop
  // return MENU_OK;
}

/** @brief 오프셋 조정 메뉴 처리 */
int handle_offset_adjustment(int adc_num)
{
  uint8_t err;
  int choice, channel_index, status;
  adc_channel_type_t type;
  adc_cal_params_t* params;
  float target_ref;
  int32_t raw_now;

  config_adc_adv_t* p_adc = get_adc_config(adc_num);
  while (1)
  {
    io_printf("+---------------------------------------+\r\n");
    io_printf("|         --- 오프셋 조정 ---           |\r\n");
    io_printf("+---------------------------------------+\r\n");
    io_printf("|  1. 싱글 엔드 채널 조정               |\r\n");
    io_printf("|  2. 차동 채널 조정                    |\r\n");
    io_printf("|     CTRL+C 이전,CTRL+Q 종료           |\r\n");
    io_printf("+---------------------------------------+\r\n");

    status = input_decimal_prompt("선택", &choice, 0, 2);
    if (status == MENU_ABORT || status == MENU_BACK)
    {
      return status;
    }

    if (status != MENU_OK)
    {
      continue;
    }

    if (choice == 1 || choice == 2)
    {
      type = (choice == 1) ? ADC_CHANNEL_TYPE_SINGLE_ENDED : ADC_CHANNEL_TYPE_DIFFERENTIAL;
      status = select_channel(type, &channel_index);
      if (status == MENU_ABORT || status == MENU_BACK)
      {
        return status;
      }

      if (status != MENU_OK)
      {
        continue;
      }

      params = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED) ? &p_adc->single_ended_cal[channel_index]
                                                       : &p_adc->differential_cal[channel_index];

      if (!params->is_calibrated)
      {
        io_printf("오류: 이 채널은 공장 캘리브레이션되지 않아 오프셋 조정 불가.\r\n");
        continue;
      }

      // 상세 조정 메뉴
      while (1)
      {
        io_printf("+--- 채널 %s[%d] 오프셋 조정 ---+\r\n", (type == 0 ? "SE" : "Diff"),
                  channel_index);
        g_current_temp = read_current_temperature();
        float current_val =
            adc_get_compensated_value((type == 0 ? (int32_t)adc_read_single_raw(channel_index, &err)
                                                 : (int32_t)adc_read_diff_raw(channel_index, &err)),
                                      params, g_current_temp);
        io_printf("| 현재 온도: %.1f°C\r\n", g_current_temp);
        io_printf("| 현재 측정값: ");
        if (isnan(current_val))
          io_printf("N/A\r\n");
        else
          io_printf("%.4f\r\n", current_val);

        io_printf("+---------------------------------------+\r\n");
        io_printf("|  1. 오프셋 조정                       |\r\n");
        io_printf("|     CTRL+C 이전,CTRL+Q 종료           |\r\n");
        io_printf("+---------------------------------------+\r\n");

        status = input_decimal_prompt("선택", &choice, 1, 1);
        if (status == MENU_ABORT || status == MENU_BACK)
        {
          return status;
        }

        if (status != MENU_OK)
        {
          continue;
        }

        if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)  // 싱글
        {
          uint8_t err;
          raw_now = (int32_t)adc_read_single_raw(channel_index, &err);
        }
        else if (type == ADC_CHANNEL_TYPE_DIFFERENTIAL)
        {
          uint8_t err;
          raw_now = adc_read_diff_raw(channel_index, &err);
        }

        if (choice == 1)
        {
          status = input_float_prompt("목표 기준값 입력", &target_ref);
          if (status == MENU_ABORT || status == MENU_BACK)
          {
            return status;
          }
          if (status == MENU_OK)
          {
            adc_perform_offset_adjustment(p_adc, params, type, channel_index, g_current_temp,
                                          target_ref, raw_now);
          }
        }
      }  // end adjustment loop
    }

  }  // end main loop
  // return MENU_OK;
}

typedef enum
{
  MENU_VIEW_SINGLE_CHANNEL = 1,  // 1. 싱글 엔드 채널 상태 보기 (0-16)
  MENU_VIEW_DIFFERENTIAL = 2,    // 2. 차동 채널 상태 보기 (0-7)
  MENU_VIEW_SINGLE_SUMMARY = 3,  // 3. 모든 채널 요약 보기
  MENU_VIEW_DIFF_SUMMARY = 4,    // 3. 모든 채널 요약 보기
  MENU_VIEW_SYSINFO = 5          // 4. 시스템 정보 보기 (Res, Vref)
} menu_view_t;



/**
 * @brief 
 * @param adc_num  ADC IC 번호
 */
int handle_view_status(int adc_num)
{
  char user_input[10];
  char buffer[100];
  const adc_cal_params_t* params;
  uint8_t err;
  uint8_t osc_use;
  uint8_t file_save_use = 0;
  int32_t scan_ms;
  int choice, channel_index, status;
  adc_channel_type_t type;

  config_adc_adv_t* p_adc = get_adc_config(adc_num);

  while (1)
  {
    io_printf("+---------------------------------------+\r\n");
    io_printf("|           채널 상태 보기              |\r\n");
    io_printf("+---------------------------------------+\r\n");
    io_printf("|  1. 싱글 엔드 채널 상태 보기 (0-18)   |\r\n");
    io_printf("|  2. 차동 채널 상태 보기 (0-7)         |\r\n");
    io_printf("|  3. 싱글 채널 모두 보기               |\r\n");
    io_printf("|  4. 차동 채널 모두 보기               |\r\n");
    io_printf("|  5. 시스템 정보 보기                  |\r\n");
    io_printf("|     CTRL+C 이전,CTRL+Q 종료           |\r\n");
    io_printf("+---------------------------------------+\r\n");

    status = input_decimal_prompt("선택", &choice, 1, 5);
    if (status != MENU_OK)
      break;

    switch (choice)
    {
      case MENU_VIEW_SINGLE_CHANNEL:
      case MENU_VIEW_DIFFERENTIAL:

        type = (choice == 1) ? ADC_CHANNEL_TYPE_SINGLE_ENDED : ADC_CHANNEL_TYPE_DIFFERENTIAL;
        status = select_channel(type, &channel_index);
        if (status != MENU_OK)
          break;

        io_printf("시리얼 오실로스코프 사용하려면 yes입력\r\n");
        user_input[0] = 0;
        if (cli_scanf_s("%6s", user_input) == CLI_KEYCODE_CTRL_C)
        {
          return 0;
        }
        osc_use = 0;
        if (strcasecmp("yes", user_input) == 0)
        {
          osc_use = 1;
        }
        io_printf("파일로 저장하려면 yes입력\r\n");
        user_input[0] = 0;
        if (cli_scanf_s("%6s", user_input) == CLI_KEYCODE_CTRL_C)
        {
          return 0;
        }
        file_save_use = 0;
        if (strcasecmp("yes", user_input) == 0)
        {
          file_save_use = 1;
          delete_file("adc_sample.txt");
        }

        io_printf("스캔 주기를 ms 단위로 입력하세요\r\n");
        if (cli_scanf_s("%d", &scan_ms) == CLI_KEYCODE_CTRL_C)
        {
          return 0;
        }

        params = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED) ? &p_adc->single_ended_cal[channel_index]
                                                         : &p_adc->differential_cal[channel_index];

        io_printf("\r\n--- 채널 %s[%d] 상세 정보 ---\r\n", (type == 0 ? "SE" : "Diff"),
                  channel_index);
        io_printf("  공장 캘리브레이션됨: %s\r\n", params->is_calibrated ? "예" : "아니오");
        if (params->is_calibrated)
        {
          io_printf("  공장 Slope: %.6f\r\n", params->factory_slope);
          io_printf("  공장 Offset: %.6f\r\n", params->factory_offset);
          io_printf("  공장 캘리 온도: %.1f C\r\n", params->factory_cal_temp);
        }

        const char* method_str;
        switch (params->comp_method)
        {
          case TEMP_COMP_COEFF:
            method_str = "계수 사용";
            break;
          case TEMP_COMP_LUT:
            method_str = "LUT 사용";
            break;
          default:
            method_str = "사용 안함";
            break;
        }
        io_printf("  온도 보상 방식: %s\r\n", method_str);
        switch (params->comp_method)
        {
          case TEMP_COMP_COEFF:

            io_printf("    Slope TC: %.6f\r\n", params->slope_temp_coeff);
            io_printf("    Offset TC: %.6f\r\n", params->offset_temp_coeff);
            break;
          case TEMP_COMP_LUT:
          {
            io_printf("    LUT 크기: %d / %d\r\n", params->lut_size, MAX_LUT_SIZE);
            // LUT 내용 표시 로직 추가 가능
          }
          break;
        }
        g_current_temp = read_current_temperature();
        io_printf("  현재 측정 값 (%.1f C): \r\n", g_current_temp);

        uint32_t start_time;
        uint32_t elased_time;
        char buff[20];
        do
        {
          start_time = mcu_get_clk();
          int32_t raw_adc = (type == 0 ? (int32_t)adc_read_single_raw(channel_index, &err)
                                       : adc_read_diff_raw(channel_index, &err));
          elased_time = cal_elapsed_us(start_time);
          float current_val = adc_get_compensated_value(raw_adc, params, g_current_temp);

          if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
          {
            if (osc_use)
            {
              io_printf("%d\r", (int32_t)(current_val * 1000000));
            }
            else
            {
              make_timeToStr(&Date_Time, buff, sizeof(buff));
              snprintf(buffer, sizeof(buffer), "%s SE CH:%d ADC:%8d VOLTAGE:%8.6f %.3fms\r\n", buff,
                       channel_index, raw_adc, current_val, elased_time / 1000.0f);
              io_printf("%s", buffer);
              if (file_save_use)
              {
                append_file("0:adc_sample.txt", (uint8_t*)buffer, strlen(buffer));
              }
            }
          }
          else
          {
            if (osc_use)
            {
              io_printf("%d\r", (int32_t)(current_val * 1000000));
            }
            else
            {
              make_timeToStr(&Date_Time, buff, sizeof(buff));
              snprintf(buffer, sizeof(buffer), "%s DI CH:%d ADC:%8d VOLTAGE:%8.6f %.3fms\r\n", buff,
                       channel_index, raw_adc, current_val, elased_time / 1000.0f);
              io_printf("%s", buffer);
              if (file_save_use)
              {
                append_file("0:adc_sample.txt", (uint8_t*)buffer, strlen(buffer));
              }
            }
          }

        } while (wait_break(scan_ms));

        break;
      case MENU_VIEW_SINGLE_SUMMARY:
      {
        //    float slope;
        //   float offset;
        int32_t raw;
        float voltage;

        io_printf(VT100_CLEAR_SCREEN);
        io_printf(VT100_CURSOR_OFF);

        type = ADC_CHANNEL_TYPE_SINGLE_ENDED;

        do
        {
          io_printf(VT100_CURSOR_HOME);
          for (int channel = 0; channel < 18; channel++)
          {
            params = &p_adc->single_ended_cal[channel];

            raw = (int32_t)adc_read_single_raw(channel, &err);

            voltage = adc_get_compensated_value(raw, params, g_current_temp);

            if (isnan(voltage))
            {
              io_printf("SE %2d slope:%e offset:%e raw:%10d %s\r\n", channel, params->factory_slope,
                        params->factory_offset, raw, "켈리브레이션 필요");
            }
            else
            {
              io_printf("SE %2d slope:%e offset:%e raw:%10d voltage:%8.7f\r\n", channel,
                        params->factory_slope, params->factory_offset, raw, voltage);
            }
          }
        } while (wait_break(100));
      }
        io_printf(VT100_CURSOR_ON);

        break;
      case MENU_VIEW_DIFF_SUMMARY:
      {
        int32_t raw;
        float voltage;

        io_printf(VT100_CLEAR_SCREEN);
        io_printf(VT100_CURSOR_OFF);

        type = ADC_CHANNEL_TYPE_SINGLE_ENDED;

        do
        {
          io_printf(VT100_CURSOR_HOME);
          for (int channel = 0; channel < 8; channel++)
          {
            params = &p_adc->differential_cal[channel];

            raw = adc_read_diff_raw(channel, &err);

            voltage = adc_get_compensated_value(raw, params, g_current_temp);
            if (isnan(voltage))
            {
              io_printf("DIFF %2d slope:%e offset:%e raw:%10d %s\r\n", channel,
                        params->factory_slope, params->factory_offset, raw, "켈리브레이션 필요");
            }
            else
            {
              io_printf("DIFF %2d slope:%e offset:%e raw:%10d voltage:%8.7f\r\n", channel,
                        params->factory_slope, params->factory_offset, raw, voltage);
            }
          }
        } while (wait_break(500));
      }
        io_printf(VT100_CURSOR_ON);
        break;
      case MENU_VIEW_SYSINFO:
        io_printf("\r\n    시스템 정보\r\n");
        io_printf("  ADC 해상도: %u 비트\r\n", p_adc->bits->resolution_bits);
        io_printf("  기준 전압 (Vref): %.3f V\r\n", p_adc->bits->reference_voltage);
        io_printf("  최소 Raw 값: %d\r\n", p_adc->bits->min_raw_value);
        io_printf("  최대 Raw 값: %d\r\n", p_adc->bits->max_raw_value);
        break;
      default:
        io_printf("잘못된 선택입니다.\r\n");
        break;
    }
  }
  return status;
}

/** @brief 설정 저장/로드 메뉴 처리 */
int handle_save_load(int adc_num)
{
  int choice, status;
  int ok;
  while (1)
  {
    io_printf("+---------------------------------------+\r\n");
    io_printf("|           설정 저장(NVM)              |\r\n");
    io_printf("+---------------------------------------+\r\n");
    io_printf("|  1. 켈리브레이션 기본값 적용          |\r\n");
    io_printf("|  2. 켈리브레이션 초기화               |\r\n");
    io_printf("|     CTRL+C 이전,CTRL+Q 종료           |\r\n");
    io_printf("+---------------------------------------+\r\n");

    status = input_decimal_prompt("선택", &choice, 1, 2);
    if (status != MENU_OK)
      break;

    switch (choice)
    {
      case 1:
        status = confirm_continue(&ok);
        if(status != MENU_OK)
        {
          return status;
        }
        if(ok==0)
        {
          continue;
        }

        adc_config_init(&g_adc_config_ads1220, 24, 5.0f);

        // ADS1220 18개
        for (int channel = 0; channel < g_adc_config_ads1220.params_se_cnt; channel++)
        {
          g_adc_config_ads1220.single_ended_cal[channel].comp_method = TEMP_COMP_NONE;
          g_adc_config_ads1220.single_ended_cal[channel].factory_cal_temp =
              DEFAULT_FACTORY_CAL_TEMP;
          g_adc_config_ads1220.single_ended_cal[channel].is_calibrated = true;
          g_adc_config_ads1220.single_ended_cal[channel].factory_offset = 4.928633e-03;
          g_adc_config_ads1220.single_ended_cal[channel].factory_slope = 5.958932e-07;
          g_adc_config_ads1220.single_ended_cal[channel].offset_temp_coeff = 1;
          g_adc_config_ads1220.single_ended_cal[channel].slope_temp_coeff = 1;
        }
        save_adc_cali();
        // stm32 2개
        for (int channel = 0; channel < g_adc_config_ads1220.params_di_cnt; channel++)
        {
          g_adc_config_ads1220.differential_cal[channel].comp_method = TEMP_COMP_NONE;
          g_adc_config_ads1220.differential_cal[channel].factory_cal_temp =
              DEFAULT_FACTORY_CAL_TEMP;
          g_adc_config_ads1220.differential_cal[channel].is_calibrated = true;
          g_adc_config_ads1220.differential_cal[channel].factory_offset = 4.928633e-03;
          g_adc_config_ads1220.differential_cal[channel].factory_slope = 5.958932e-07;
          g_adc_config_ads1220.differential_cal[channel].factory_offset_trim = 0.0f;
          g_adc_config_ads1220.differential_cal[channel].offset_temp_coeff = 1;
          g_adc_config_ads1220.differential_cal[channel].slope_temp_coeff = 1;
        }

        adc_config_init(&g_adc_config_stm32, 12, 3.3f);

        for (int channel = 0; channel < STM32_NUM_SINGLE_ENDED_CHANNELS; channel++)
        {
          g_adc_config_stm32.single_ended_cal[channel].comp_method = TEMP_COMP_NONE;
          g_adc_config_stm32.single_ended_cal[channel].factory_cal_temp = DEFAULT_FACTORY_CAL_TEMP;
          g_adc_config_stm32.single_ended_cal[channel].is_calibrated = true;
          g_adc_config_stm32.single_ended_cal[channel].factory_offset = 0.0f;
          g_adc_config_stm32.single_ended_cal[channel].factory_slope = 8.05e-04;
          g_adc_config_stm32.single_ended_cal[channel].factory_offset_trim = 0.0f;
          g_adc_config_stm32.single_ended_cal[channel].offset_temp_coeff = 1;
          g_adc_config_stm32.single_ended_cal[channel].slope_temp_coeff = 1;
        }
        save_adc_cali();

        io_printf("NVM 저장 성공\r\n");

        break;
      case 2:
        status = confirm_continue(&ok);
        if (status != MENU_OK)
        {
          return status;
        }
        if (ok == 0)
        {
          continue;
        }

        adc_config_init(&g_adc_config_ads1220, 24, 5.0f);  // 예시 기본값으로 RAM 리셋

        adc_config_init(&g_adc_config_stm32, 12, 3.3f);  // 예시 기본값으로 RAM 리셋
        for (int channel = 0; channel < STM32_NUM_SINGLE_ENDED_CHANNELS; channel++)
        {
          g_adc_config_stm32.single_ended_cal[channel].comp_method = TEMP_COMP_NONE;
          g_adc_config_stm32.single_ended_cal[channel].factory_cal_temp = DEFAULT_FACTORY_CAL_TEMP;
          g_adc_config_stm32.single_ended_cal[channel].is_calibrated = true;
          g_adc_config_stm32.single_ended_cal[channel].factory_offset = 0.0f;
          g_adc_config_stm32.single_ended_cal[channel].factory_slope = 8.05e-04;
          g_adc_config_stm32.single_ended_cal[channel].offset_temp_coeff = 1;
          g_adc_config_stm32.single_ended_cal[channel].slope_temp_coeff = 1;
        }
        io_printf("켈리브레이션 값 초기화 완료\r\n");
        io_printf("켈리브레이션을 다시 진행해주세요\r\n");
        save_adc_cali();
        break;
        // io_printf("잘못된 선택입니다.\r\n");
        break;
    }
  }
  return status;
}

typedef enum
{
  MENU_FACTORY_CALIBRATION = 1,  // 1. 공장 캘리브레이션 수행
  MENU_TEMP_COMPENSATION,        // 2. 온도 보상 설정
  MENU_OFFSET_ADJUST,            // 3. 오프셋 조정 (영점/단일지점)
  MENU_CHANNEL_STATUS,           // 4. 채널 상태 보기
  MENU_NVM_SAVE_LOAD             // 5. 설정 저장/로드 (NVM)
} menu_item_t;

typedef struct
{
  int menu_id;                      // 실제 내부 처리용 ID (enum)
  const char* label;                // 메뉴 문자열
  int (*handler)(int32_t adc_num);  // 처리 함수
  bool enabled;                     // 사용 여부
  int display_idx;                  // 사용자에게 보여줄 번호
} menu_entry_t;

menu_entry_t menu_table[] = {
    {MENU_FACTORY_CALIBRATION, "공장 캘리브레이션 수행", handle_factory_calibration, true},
    {MENU_TEMP_COMPENSATION, "온도 보상 설정", handle_temp_comp_setup, false},  // 이장비 미사용
    {MENU_OFFSET_ADJUST, "오프셋 조정", handle_offset_adjustment, true},
    {MENU_CHANNEL_STATUS, "채널 상태 보기", handle_view_status, true},
    {MENU_NVM_SAVE_LOAD, "초기화", handle_save_load, true}};

void run_calibration_menu(int adc_num)
{
  int choice = 0, status = 0;
  bool exit_menu = false;
  int display_idx = 1;

  while (!exit_menu)
  {
    io_printf("+---------------------------------------+\r\n");
    io_printf("|       *** ADC 켈리브레이션  ***       |\r\n");
    io_printf("+---------------------------------------+\r\n");

    // 메뉴 출력
    display_idx = 1;
    for (int i = 0; i < sizeof(menu_table) / sizeof(menu_table[0]); i++)
    {
      if (menu_table[i].enabled)
      {
        menu_table[i].display_idx = display_idx;  // 동적으로 표시 인덱스 지정
        io_printf("|  %d. %-33s |\r\n", display_idx, menu_table[i].label);

        display_idx++;
      }
    }

    io_printf("|     CTRL+C 이전, CTRL+Q 종료          |\r\n");
    io_printf("+---------------------------------------+\r\n");

    status = input_decimal_prompt("선택", &choice, 1, display_idx - 1);
    if (status == MENU_ABORT || status == MENU_BACK)
      return;

    for (int i = 0; i < sizeof(menu_table) / sizeof(menu_table[0]); i++)
    {
      if (menu_table[i].enabled && menu_table[i].display_idx == choice)
      {
        status = menu_table[i].handler(adc_num);

        break;
      }
    }
  }
}

int aws_menu_calibration()
{
  int choice, status;
  char* menu[] = {"공장 캘리브레이션",
                  "채널 상태 보기",
                  "초기화"};

  driver_adc_open(ADC_ADS1220, 0);
  do
  {
    status = choice_menu(24, "ADC 켈리브레이션", menu, _countof(menu), &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
    case 1:
      status = handle_factory_calibration(0);
       break;
    case 2:
     status = handle_view_status(0);
       break;
    case 3:
     status = handle_save_load(0);
       break;
    }

    if(status==MENU_ABORT)
    break;
  }while(1);

  return status;
}