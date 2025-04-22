/**
 * @file adc_calibration_menu.c
 * @brief TeraTerm/VT100 기반 ADC 캘리브레이션 메뉴 (최종 통합 버전)
 * @date 2025-04-22
 *
 * 설명:
 * - 제공된 debug_printf, console_scanf, recv_key 함수를 사용하여 구현.
 * - console_scanf는 Ctrl+C 또는 Ctrl+Q 입력 시 -1 반환.
 * - 계층적 메뉴 구조 사용.
 * - 공장 캘리브레이션, 온도 보상 설정(계수/LUT), 동적 오프셋 조정 기능 포함.
 * - 채널 상태 보기, 설정 저장/로드(스텁) 기능 포함.
 * - VT100 이스케이프 시퀀스를 사용하여 화면 제어.
 */

#include <math.h>    // For NAN, isnan, fabsf
#include <stdarg.h>  // For va_list in debug_printf stub
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>  // For atoi, atof (대안 입력 파싱 시)
#include <string.h>  // For memcpy, strcmp (필요시)

#include "IO\dev_io.h"
#include "console_scanf.h"
#include "fsl_shell.h"
#include "config_adc.h"
#include "app_adc.h"
#include "adc_calibration.h"

extern char recv_key(void);
extern config_adc_adv_t g_adc_config;
extern float g_current_temp;


// 메뉴 반환 상태
#define MENU_OK 0
#define MENU_ABORT -1  // Ctrl+C/Q
#define MENU_BACK -2
#define MENU_ERROR -3



// --- 5. 입력 헬퍼 함수 ---

/** @brief Enter 키를 기다리고, Ctrl+C/Q 감지 */
int wait_for_enter()
{
  int key = recv_key();
  return (key == -1) ? MENU_ABORT : MENU_OK;
}

/** @brief 정수 입력을 받고 유효성 검사 및 종료(-1) 처리 */
int get_int_input(const char* prompt, int* value, int min_val, int max_val)
{
  int ret_scan;
  int attempts = 0;
  while (attempts < 3)
  {
    debug_printf("%s (%d ~ %d): ", prompt, min_val, max_val);
    ret_scan = console_scanf("%d", value);
    if (ret_scan == -1)
      return MENU_ABORT;  // Ctrl+C/Q 종료
    if (ret_scan == 1 && *value >= min_val && *value <= max_val)
    {
      return MENU_OK;  // 성공
    }
    debug_printf("오류: 잘못된 입력입니다. 다시 시도하세요.\r\n");
    // 입력 버퍼 비우기 (간단한 방식) - console_scanf 구현에 따라 불필요할 수 있음
    // while(recv_key() != '\r\r\n'); // 실제 구현 필요
    attempts++;
  }
  debug_printf("오류: 입력 시도 횟수 초과.\r\n");
  return MENU_ERROR;
}

/** @brief 실수 입력을 받고 유효성 검사 및 종료(-1) 처리 */
int get_float_input(const char* prompt, float* value)
{
  int ret_scan;
  int attempts = 0;
  while (attempts < 3)
  {
    debug_printf("%s: ", prompt);
    ret_scan = console_scanf("%f", value);
    if (ret_scan == -1)
      return MENU_ABORT;  // 종료
    if (ret_scan == 1)
    {
      return MENU_OK;  // 성공 (범위 검사 추가 가능)
    }
    debug_printf("오류: 잘못된 실수 입력입니다. 다시 시도하세요.\r\n");
    attempts++;
  }
  debug_printf("오류: 입력 시도 횟수 초과.\r\n");
  return MENU_ERROR;
}



// --- 6. 메뉴 처리 함수 ---

/** @brief 채널 선택 (공통 로직) */
int select_channel(adc_channel_type_t type, int* channel_index)
{
  int max_ch = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED) ? (NUM_SINGLE_ENDED_CHANNELS - 1)
                                                       : (NUM_DIFFERENTIAL_CHANNELS - 1);
  const char* type_str = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED) ? "싱글 엔드" : "차동";
  char prompt[100];
  snprintf(prompt, sizeof(prompt), "채널 번호 입력 (%s: 0 ~ %d)", type_str, max_ch);
  return get_int_input(prompt, channel_index, 0, max_ch);
}

/** @brief 공장 캘리브레이션 메뉴 처리 */
int handle_factory_calibration()
{
  int choice, channel_index, status;
  adc_channel_type_t type;
  adc_cal_params_t* cal_params_ptr;
  adc_cal_point_t p1, p2;
  float cal_temp;

  while (1)
  {
    debug_printf("\x1b[2J\x1b[H");  // 화면 지우기 & 커서 홈
    debug_printf("+---------------------------------------+\r\n");
    debug_printf("|       --- 공장 캘리브레이션 ---       |\r\n");
    debug_printf("+---------------------------------------+\r\n");
    debug_printf("|  1. 싱글 엔드 채널 캘리브레이션       |\r\n");
    debug_printf("|  2. 차동 채널 캘리브레이션            |\r\n");
    debug_printf("|  b. 이전 메뉴로 ('b' 또는 숫자 0)     |\r\n");
    debug_printf("+---------------------------------------+\r\n");

    status = get_int_input("선택", &choice, 0, 2);
    if (status == MENU_ABORT)
      return MENU_ABORT;
    if (status != MENU_OK)
      continue;
    if (choice == 0)
      choice = 'b';  // 숫자 0도 뒤로가기로 처리

    if (choice == 1 || choice == 2)
    {
      type = (choice == 1) ? ADC_CHANNEL_TYPE_SINGLE_ENDED : ADC_CHANNEL_TYPE_DIFFERENTIAL;
      status = select_channel(type, &channel_index);
      if (status == MENU_ABORT)
        return MENU_ABORT;
      if (status != MENU_OK)
        continue;

      cal_params_ptr = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
                           ? &g_adc_config.single_ended_cal[channel_index]
                           : &g_adc_config.differential_cal[channel_index];

      debug_printf("\r\n--- %s 채널 %d 캘리브레이션 시작 ---\r\n", (type == 0 ? "SE" : "Diff"),
                   channel_index);

      // Point 1 입력
      debug_printf("1. 낮은 기준점(Low Reference)을 연결하고 Enter를 누르세요.\r\n");

      while(1)
      {
        uint8_t err;
 
        if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
        {
          p1.raw_value = adc_read_single_raw(channel_index, &err);
        }
        else if (type == ADC_CHANNEL_TYPE_DIFFERENTIAL)
        {
          p1.raw_value = adc_read_diff_raw(channel_index, &err);
        }
        debug_printf("raw:%10d\r\n", p1.raw_value);
        if(recv_key()==0x11) 
        break;
      }


      status = get_int_input("   측정된 RAW 값 입력", (int*)&p1.raw_value, 0,
                             g_adc_config.max_raw_value);
      if (status != MENU_OK)
        return status;  // ABORT 또는 ERROR
      status = get_float_input("   낮은 기준점의 실제 값(단위 포함) 입력", &p1.reference_value);
      if (status != MENU_OK)
        return status;

      // Point 2 입력
      debug_printf("2. 높은 기준점(High Reference)을 연결하고 Enter를 누르세요.\r\n");
      while (1)
      {
        uint8_t err;

        if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
        {
          p2.raw_value = adc_read_single_raw(channel_index, &err);
        }
        else if (type == ADC_CHANNEL_TYPE_DIFFERENTIAL)
        {
          p2.raw_value = adc_read_diff_raw(channel_index, &err);
        }
        debug_printf("raw:%10d\r\n", p2.raw_value);
        if (recv_key() == 0x11)
          break;
      }
      status = get_int_input("   측정된 RAW 값 입력", (int*)&p2.raw_value, 0,
                             g_adc_config.max_raw_value);
      if (status != MENU_OK)
        return status;
      status = get_float_input("   높은 기준점의 실제 값(단위 포함) 입력", &p2.reference_value);
      if (status != MENU_OK)
        return status;

      // 캘리브레이션 온도 입력
      g_current_temp = read_current_temperature();  // 현재 온도 읽기
      debug_printf("현재 측정된 온도: %.1f °C\r\n", g_current_temp);
      status =
          get_float_input("캘리브레이션 수행 온도를 입력하세요 (기본값: 현재 온도)", &cal_temp);
      if (status == MENU_ABORT)
        return MENU_ABORT;
      if (status != MENU_OK)
        cal_temp = g_current_temp;  // 입력 실패 시 현재 온도 사용

      // 캘리브레이션 수행
      if (adc_perform_factory_calibration(&g_adc_config, cal_params_ptr, p1, p2, cal_temp))
      {
        debug_printf("캘리브레이션 성공! 설정이 RAM에 업데이트되었습니다.\r\n");
      }
      else
      {
        debug_printf("오류: 캘리브레이션 실패.\r\n");
      }
      if (wait_for_enter() == MENU_ABORT)
        return MENU_ABORT;  // 결과 확인 시간
    }
    else if (choice == 'b')
    {
      return MENU_BACK;
    }
    else
    {
      debug_printf("잘못된 선택입니다.\r\n");
      if (wait_for_enter() == MENU_ABORT)
        return MENU_ABORT;
    }
  }
  return MENU_OK;  // Normal exit (should not be reached in while(1))
}

/** @brief 온도 보상 설정 메뉴 처리 */
int handle_temp_comp_setup()
{
  int choice, channel_index, status, method_choice;
  adc_channel_type_t type;
  adc_cal_params_t* params;

  while (1)
  {
    debug_printf("\x1b[2J\x1b[H");
    debug_printf("+---------------------------------------+\\rn");
    debug_printf("|       --- 온도 보상 설정 ---          |\r\n");
    debug_printf("+---------------------------------------+\r\n");
    debug_printf("|  1. 싱글 엔드 채널 설정               |\r\n");
    debug_printf("|  2. 차동 채널 설정                    |\r\n");
    debug_printf("|  b. 이전 메뉴로 ('b' 또는 숫자 0)     |\r\n");
    debug_printf("+---------------------------------------+\r\n");

    status = get_int_input("선택", &choice, 0, 2);
    if (status == MENU_ABORT)
      return MENU_ABORT;
    if (status != MENU_OK)
      continue;
    if (choice == 0)
      choice = 'b';

    if (choice == 1 || choice == 2)
    {
      type = (choice == 1) ? ADC_CHANNEL_TYPE_SINGLE_ENDED : ADC_CHANNEL_TYPE_DIFFERENTIAL;
      status = select_channel(type, &channel_index);
      if (status == MENU_ABORT)
        return MENU_ABORT;
      if (status != MENU_OK)
        continue;

      params = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
                   ? &g_adc_config.single_ended_cal[channel_index]
                   : &g_adc_config.differential_cal[channel_index];

      // 채널별 상세 설정 루프
      while (1)
      {
        debug_printf("\x1b[2J\x1b[H");
        debug_printf("+--- 채널 %s[%d] 온도 보상 설정 ---+\r\n", (type == 0 ? "SE" : "Diff"),
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
        debug_printf("| 현재 방식: %s\r\n", method_str);
        debug_printf("+---------------------------------------+\r\n");
        debug_printf("|  1. 보상 방식 변경                    |\r\n");
        debug_printf("|  2. 온도 계수 설정 (방식=계수)        |\r\n");
        debug_printf("|  3. LUT 데이터 설정/보기 (방식=LUT)   |\r\n");
        debug_printf("|  b. 이전 메뉴로 ('b' 또는 숫자 0)     |\r\n");
        debug_printf("+---------------------------------------+\r\n");

        status = get_int_input("선택", &choice, 0, 3);
        if (status == MENU_ABORT)
          return MENU_ABORT;  // 중첩 메뉴 종료 처리
        if (status != MENU_OK)
          continue;
        if (choice == 0)
          choice = 'b';

        switch (choice)
        {
          case 1:  // 방식 변경
            status = get_int_input("새 방식 선택 (0:없음, 1:계수, 2:LUT)", &method_choice, 0, 2);
            if (status == MENU_OK)
            {
              params->comp_method = (temp_comp_method_t)method_choice;
              debug_printf("보상 방식이 변경되었습니다.\r\n");
              save_adc_cali(); // NVM 저장 필요
            }
            else if (status == MENU_ABORT)
              return MENU_ABORT;
            if (wait_for_enter() == MENU_ABORT)
              return MENU_ABORT;
            break;
          case 2:  // 계수 설정
            if (params->comp_method == TEMP_COMP_COEFF)
            {
              debug_printf("현재 SlopeTC=%.6f, OffsetTC=%.6f\r\n", params->slope_temp_coeff,
                           params->offset_temp_coeff);
              status = get_float_input("새 Slope TempCo 입력", &params->slope_temp_coeff);
              if (status == MENU_ABORT)
                return MENU_ABORT;
              if (status == MENU_OK)
              {
                status = get_float_input("새 Offset TempCo 입력", &params->offset_temp_coeff);
                if (status == MENU_ABORT)
                  return MENU_ABORT;
                if (status == MENU_OK)
                {
                  debug_printf("온도 계수가 업데이트되었습니다.\r\n");
                   save_adc_cali(); // NVM 저장 필요
                }
              }
            }
            else
            {
              debug_printf("오류: 현재 보상 방식이 '계수 사용'이 아닙니다.\r\n");
            }
            if (wait_for_enter() == MENU_ABORT)
              return MENU_ABORT;
            break;
          case 3:  // LUT 설정/보기
#ifdef ADC_LUT
            if (params->comp_method == TEMP_COMP_LUT)
            {
              // TODO: LUT 보기 및 편집 기능 구현 (복잡함)
              // 예시: 현재 설정된 LUT 보기
              debug_printf("현재 LUT 데이터 (최대 %d개):\r\n", MAX_LUT_SIZE);
              if (params->lut_size == 0)
              {
                debug_printf("  (설정된 데이터 없음)\r\n");
              }
              else
              {
                debug_printf("  Idx | Temp(C) | SlopeMult | OffsetCorr\r\n");
                debug_printf("  --------------------------------------\r\n");
                for (uint8_t i = 0; i < params->lut_size; ++i)
                {
                  debug_printf("  %2u | %7.1f | %9.6f | %9.6f\r\n", i,
                               params->temp_comp_lut[i].temperature,
                               params->temp_comp_lut[i].slope_multiplier,
                               params->temp_comp_lut[i].offset_correction);
                }
              }
              debug_printf("\r\nLUT 데이터 편집 기능은 이 예제에 포함되지 않았습니다.\r\n");
              // 예시 LUT 채우기 호출 (디버그용)
              // populate_lut_example(params);
              // save_adc_cali(&g_adc_config);
            }
            else
            {
              debug_printf("오류: 현재 보상 방식이 'LUT 사용'이 아닙니다.\r\n");
            }
            if (wait_for_enter() == MENU_ABORT)
              return MENU_ABORT;
            break;
#endif
          case 'b':
            goto channel_setup_exit;  // 채널 설정 루프 탈출
          default:
            debug_printf("잘못된 선택입니다.\r\n");
            if (wait_for_enter() == MENU_ABORT)
              return MENU_ABORT;
            break;
        }
      }  // end channel setup loop
    channel_setup_exit:;
    }
    else if (choice == 'b')
    {
      return MENU_BACK;
    }
    else
    {
      debug_printf("잘못된 선택입니다.\r\n");
      if (wait_for_enter() == MENU_ABORT)
        return MENU_ABORT;
    }
  }  // end main loop
  return MENU_OK;
}

/** @brief 오프셋 조정 메뉴 처리 */
int handle_offset_adjustment()
{
  int choice, channel_index, status;
  adc_channel_type_t type;
  adc_cal_params_t* params;
  float target_ref;
  uint8_t err;
  int32_t raw_now;
  int mode;
  
  while (1)
  {
    debug_printf("\x1b[2J\x1b[H");
    debug_printf("+---------------------------------------+\r\n");
    debug_printf("|         --- 오프셋 조정 ---           |\r\n");
    debug_printf("+---------------------------------------+\r\n");
    debug_printf("|  1. 싱글 엔드 채널 조정               |\r\n");
    debug_printf("|  2. 차동 채널 조정                    |\r\n");
    debug_printf("|  b. 이전 메뉴로 ('b' 또는 숫자 0)     |\r\n");
    debug_printf("+---------------------------------------+\r\n");

    status = get_int_input("선택", &choice, 0, 2);
    if (status == MENU_ABORT)
      return MENU_ABORT;
    if (status != MENU_OK)
      continue;
    if (choice == 0)
      choice = 'b';

    if (choice == 1 || choice == 2)
    {
      
      type = (choice == 1) ? ADC_CHANNEL_TYPE_SINGLE_ENDED : ADC_CHANNEL_TYPE_DIFFERENTIAL;
      status = select_channel(type, &channel_index);
      if (status == MENU_ABORT)
        return MENU_ABORT;
      if (status != MENU_OK)
        continue;

      params = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
                   ? &g_adc_config.single_ended_cal[channel_index]
                   : &g_adc_config.differential_cal[channel_index];

      if (!params->is_calibrated)
      {
        debug_printf("오류: 이 채널은 공장 캘리브레이션되지 않아 오프셋 조정 불가.\r\n");
        if (wait_for_enter() == MENU_ABORT)
          return MENU_ABORT;
        continue;
      }

      // 상세 조정 메뉴
      while (1)
      {
        debug_printf("\x1b[2J\x1b[H");
        debug_printf("+--- 채널 %s[%d] 오프셋 조정 ---+\r\n", (type == 0 ? "SE" : "Diff"),
                     channel_index);
        g_current_temp = read_current_temperature();
        float current_val =
            adc_get_compensated_value((type == 0 ? adc_read_single_raw(channel_index,&err)
                                                 : adc_read_diff_raw(channel_index,&err)),
                                      params, g_current_temp);
        debug_printf("| 현재 온도: %.1f°C\r\n", g_current_temp);
        debug_printf("| 현재 측정값: ");
        if (isnan(current_val))
          debug_printf("N/A\r\n");
        else
          debug_printf("%.4f\r\n", current_val);
        debug_printf("+---------------------------------------+\r\n");
        debug_printf("|  1. 동적 영점 조절 (현재 값을 0.0으로)|\r\n");
        debug_printf("|  2. 단일 지점 오프셋 조정 (다른 값)   |\r\n");
        debug_printf("|  b. 이전 메뉴로 ('b' 또는 숫자 0)     |\r\n");
        debug_printf("+---------------------------------------+\r\n");

        status = get_int_input("선택", &choice, 0, 2);
        if (status == MENU_ABORT)
          return MENU_ABORT;  // Abort propagates up
        if (status != MENU_OK)
          continue;
        if (choice == 0)
          choice = 'b';

        
           if(type == ADC_CHANNEL_TYPE_SINGLE_ENDED)//싱글
          {
            uint8_t err;
            raw_now = adc_read_single_raw(channel_index,&err);
          }
          else if(type ==ADC_CHANNEL_TYPE_DIFFERENTIAL)
          {
                        uint8_t err;
            raw_now = adc_read_diff_raw(channel_index,&err);
          }
          
          
        if (choice == 1)
        {  // 영점 조절
          debug_printf("현재 측정값을 0.0으로 조정합니다.\r\n");

          adc_perform_offset_adjustment(&g_adc_config, params, type, channel_index, g_current_temp,
                                        0.0f,raw_now);
          if (wait_for_enter() == MENU_ABORT)
            return MENU_ABORT;
        }
        else if (choice == 2)
        {  // 단일 지점 조정
          status = get_float_input("목표 기준값 입력", &target_ref);
          if (status == MENU_ABORT)
            return MENU_ABORT;
          
          
          if (status == MENU_OK)
          {
            adc_perform_offset_adjustment(&g_adc_config, params, type, channel_index,
                                          g_current_temp, target_ref,raw_now);
          }
          if (wait_for_enter() == MENU_ABORT)
            return MENU_ABORT;
        }
        else if (choice == 'b')
        {
          goto offset_adjust_exit;  // 조정 루프 탈출
        }
        else
        {
          debug_printf("잘못된 선택입니다.\r\n");
          if (wait_for_enter() == MENU_ABORT)
            return MENU_ABORT;
        }
      }  // end adjustment loop
    offset_adjust_exit:;
    }
    else if (choice == 'b')
    {
      return MENU_BACK;
    }
    else
    {
      debug_printf("잘못된 선택입니다.\r\n");
      if (wait_for_enter() == MENU_ABORT)
        return MENU_ABORT;
    }
  }  // end main loop
  return MENU_OK;
}

/** @brief 채널 상태 보기 메뉴 처리 */
int handle_view_status()
{
  int choice, channel_index, status;
  adc_channel_type_t type;
  const adc_cal_params_t* params;
uint8_t err;
  while (1)
  {
    debug_printf("\x1b[2J\x1b[H");
    debug_printf("+---------------------------------------+\r\n");
    debug_printf("|       --- 채널 상태 보기 ---          |\r\n");
    debug_printf("+---------------------------------------+\r\n");
    debug_printf("|  1. 싱글 엔드 채널 상태 보기 (0-15)   |\r\n");
    debug_printf("|  2. 차동 채널 상태 보기 (0-7)         |\r\n");
    debug_printf("|  3. 모든 채널 요약 보기 (구현 안됨)   |\r\n");  // TODO
    debug_printf("|  4. 시스템 정보 보기 (Res, Vref)      |\r\n");
    debug_printf("|  b. 이전 메뉴로 ('b' 또는 숫자 0)     |\r\n");
    debug_printf("+---------------------------------------+\r\n");

    status = get_int_input("선택", &choice, 0, 4);
    if (status == MENU_ABORT)
      return MENU_ABORT;
    if (status != MENU_OK)
      continue;
    if (choice == 0)
      choice = 'b';

    if (choice == 1 || choice == 2)
    {
      type = (choice == 1) ? ADC_CHANNEL_TYPE_SINGLE_ENDED : ADC_CHANNEL_TYPE_DIFFERENTIAL;
      status = select_channel(type, &channel_index);
      if (status == MENU_ABORT)
        return MENU_ABORT;
      if (status != MENU_OK)
        continue;

      params = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
                   ? &g_adc_config.single_ended_cal[channel_index]
                   : &g_adc_config.differential_cal[channel_index];

      debug_printf("\r\n--- 채널 %s[%d] 상세 정보 ---\r\n", (type == 0 ? "SE" : "Diff"), channel_index);
      debug_printf("  공장 캘리브레이션됨: %s\r\n", params->is_calibrated ? "예" : "아니오");
      if (params->is_calibrated)
      {
        debug_printf("  공장 Slope: %.6f\r\n", params->factory_slope);
        debug_printf("  공장 Offset: %.6f\r\n", params->factory_offset);
        debug_printf("  공장 캘리 온도: %.1f C\r\n", params->factory_cal_temp);
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
      debug_printf("  온도 보상 방식: %s\r\n", method_str);
      if (params->comp_method == TEMP_COMP_COEFF)
      {
        debug_printf("    Slope TC: %.6f\r\n", params->slope_temp_coeff);
        debug_printf("    Offset TC: %.6f\r\n", params->offset_temp_coeff);
      }
      else if (params->comp_method == TEMP_COMP_LUT)
      {
        debug_printf("    LUT 크기: %d / %d\r\n", params->lut_size, MAX_LUT_SIZE);
        // LUT 내용 표시 로직 추가 가능
      }
      g_current_temp = read_current_temperature();
      float current_val =
          adc_get_compensated_value((type == 0 ? adc_read_single_raw(channel_index,&err)
                                               : adc_read_diff_raw(channel_index,&err)),
                                    params, g_current_temp);
      debug_printf("  현재 측정 값 (%.1f C): ", g_current_temp);
      if (isnan(current_val))
        debug_printf("N/A (캘리 안됨?)\r\n");
      else
        debug_printf("%.4f\r\n", current_val);

      if (wait_for_enter() == MENU_ABORT)
        return MENU_ABORT;
    }
    else if (choice == 3)
    {
      debug_printf("모든 채널 요약 보기는 아직 구현되지 않았습니다.\r\n");
      if (wait_for_enter() == MENU_ABORT)
        return MENU_ABORT;
    }
    else if (choice == 4)
    {
      debug_printf("\r\n--- 시스템 정보 ---\r\n");
      debug_printf("  ADC 해상도: %u 비트\r\n", g_adc_config.resolution_bits);
      debug_printf("  기준 전압 (Vref): %.3f V\r\n", g_adc_config.reference_voltage);
      debug_printf("  최대 Raw 값: %u\r\n", g_adc_config.max_raw_value);
      if (wait_for_enter() == MENU_ABORT)
        return MENU_ABORT;
    }
    else if (choice == 'b')
    {
      return MENU_BACK;
    }
    else
    {
      debug_printf("잘못된 선택입니다.\r\n");
      if (wait_for_enter() == MENU_ABORT)
        return MENU_ABORT;
    }
  }
  return MENU_OK;
}

/** @brief 설정 저장/로드 메뉴 처리 */
int handle_save_load()
{
  int choice, status;
  while (1)
  {
    debug_printf("\x1b[2J\x1b[H");
    debug_printf("+---------------------------------------+\r\n");
    debug_printf("|       --- 설정 저장/로드 (NVM) ---    |\r\n");
    debug_printf("+---------------------------------------+\r\n");
    debug_printf("|  1. 현재 설정 NVM에 저장              |\r\n");
    debug_printf("|  2. NVM에서 설정 로드 (재시작 권장)   |\r\n");
    debug_printf("|  3. 공장 초기값 복원 (재시작 권장)    |\r\n");
    debug_printf("|  b. 이전 메뉴로 ('b' 또는 숫자 0)     |\r\n");
    debug_printf("+---------------------------------------+\r\n");

    status = get_int_input("선택", &choice, 0, 3);
    if (status == MENU_ABORT)
      return MENU_ABORT;
    if (status != MENU_OK)
      continue;
    if (choice == 0)
      choice = 'b';

    switch (choice)
    {
      case 1:
        debug_printf("현재 설정을 NVM에 저장 시도...\r\n");
        save_adc_cali();

          debug_printf("NVM 저장 성공 (스텁).\r\n");

        break;
      case 2:
        debug_printf("NVM에서 설정을 로드 시도...\r\n");
        load_adc_cali();
         break;
      case 3:
        debug_printf("공장 초기값 복원 시도...\r\n");
        // 실제로는 NVM 영역을 지우거나 기본값을 쓰는 동작
        // 여기서는 adc_config_init을 다시 호출하여 RAM을 초기화
        adc_config_init(&g_adc_config, 24, 5.0f);  // 예시 기본값으로 RAM 리셋
        debug_printf("공장 초기값 복원 완료 (RAM). NVM 초기화 및 재시작이 필요할 수 있습니다.\r\n");
         save_adc_cali(); // 초기화된 값을 저장할 수도 있음
        break;
      case 'b':
        return MENU_BACK;
      default:
        debug_printf("잘못된 선택입니다.\r\n");
        break;
    }
    if (wait_for_enter() == MENU_ABORT)
      return MENU_ABORT;
  }
  return MENU_OK;
}


void run_calibration_menu()
{
  int choice, status;
  bool exit_menu = false;

  while (!exit_menu)
  {
    debug_printf("\x1b[2J\x1b[H");  // 화면 지우기 & 커서 홈
    debug_printf("\x1b[1m");        // Bold
    debug_printf("+---------------------------------------+\r\n");
    debug_printf("|       *** ADC Calibration Menu ***    |\r\n");
    debug_printf("+---------------------------------------+\x1b[0m\r\n");  // Reset Bold
    debug_printf("|  1. 공장 캘리브레이션 수행            |\r\n");
    debug_printf("|  2. 온도 보상 설정                    |\r\n");
    debug_printf("|  3. 오프셋 조정 (영점/단일지점)       |\r\n");
    debug_printf("|  4. 채널 상태 보기                    |\r\n");
    debug_printf("|  5. 설정 저장/로드 (NVM)              |\r\n");
    debug_printf("|  q. 메뉴 종료 ('q' 또는 숫자 0)       |\r\n");
    debug_printf("+---------------------------------------+\r\n");

    status = get_int_input("선택", &choice, 0, 5);  // 0 입력 시 종료로 처리

    if (status == MENU_ABORT)
    {  // Ctrl+C/Q
      exit_menu = true;
      debug_printf("\r\n사용자 요청으로 메뉴를 종료합니다.\r\n");
      continue;
    }
    else if (status != MENU_OK)
    {
      continue;  // 잘못된 입력 시 다시 시도
    }

    if (choice == 0)
      choice = 'q';  // 숫자 0 입력 시 종료

    switch (choice)
    {
      case 1:
        status = handle_factory_calibration();
        break;
      case 2:
        status = handle_temp_comp_setup();
        break;
      case 3:
        status = handle_offset_adjustment();
        break;
      case 4:
        status = handle_view_status();
        break;
      case 5:
        status = handle_save_load();
        break;
      case 'q':
        exit_menu = true;
        debug_printf("메뉴를 종료합니다.\r\n");
        break;
      default:
        debug_printf("알 수 없는 선택입니다: %d\r\n", choice);
        status = MENU_ERROR;
        break;
    }

    // 하위 메뉴에서 종료(-1) 신호가 올라오면 메인 루프도 종료
    if (status == MENU_ABORT)
    {
      exit_menu = true;
      debug_printf("\r\n사용자 요청으로 메뉴를 종료합니다.\r\n");
    }
  }
}



