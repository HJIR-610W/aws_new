/**
 * @file escape_sequence.h
 * @brief ES/ANSI 이스케이프 시퀀스 정의
 * @date 2025-01-11
 * 
 * 터미널 제어를 위한 ES/ANSI 이스케이프 시퀀스 정의
 * 커서 이동, 화면 제어, 색상, 텍스트 속성 등을 포함
 */

#ifndef ESCAPE_SEQUENCE_H
#define ESCAPE_SEQUENCE_H

/* =================================================================== */
/*                          커서 제어                                   */
/* =================================================================== */

/* 커서 이동 */
#define ES_CURSOR_UP             "\x1B[A"        // 커서 위로 1줄
#define ES_CURSOR_DOWN           "\x1B[B"        // 커서 아래로 1줄  
#define ES_CURSOR_RIGHT          "\x1B[C"        // 커서 오른쪽으로 1칸
#define ES_CURSOR_LEFT           "\x1B[D"        // 커서 왼쪽으로 1칸
#define ES_CURSOR_HOME           "\x1B[H"        // 커서 홈 위치 (1,1)
#define ES_CURSOR_HOME_ALT       "\x1B[f"        // 커서 홈 위치 (대체)

/* 커서 이동 (N칸) - printf와 함께 사용 */
#define ES_CURSOR_UP_N           "\x1B[%dA"      // 커서 위로 N줄
#define ES_CURSOR_DOWN_N         "\x1B[%dB"      // 커서 아래로 N줄
#define ES_CURSOR_RIGHT_N        "\x1B[%dC"      // 커서 오른쪽으로 N칸
#define ES_CURSOR_LEFT_N         "\x1B[%dD"      // 커서 왼쪽으로 N칸

/* 커서 위치 설정 */
#define ES_CURSOR_POSITION       "\x1B[%d;%dH"   // 커서 위치 (행;열)
#define ES_CURSOR_COLUMN         "\x1B[%dG"      // 커서 열 위치

/* 커서 표시/숨김 */
#define ES_CURSOR_SHOW           "\x1B[?25h"     // 커서 표시
#define ES_CURSOR_HIDE           "\x1B[?25l"     // 커서 숨김
#define ES_CURSOR_ON             "\x1B[?25h"     // 커서 표시 (별칭)
#define ES_CURSOR_OFF            "\x1B[?25l"     // 커서 숨김 (별칭)

/* 커서 스타일 */
#define ES_CURSOR_BLINK_BLOCK    "\x1B[1 q"      // 깜빡이는 블록 커서
#define ES_CURSOR_STEADY_BLOCK   "\x1B[2 q"      // 고정 블록 커서
#define ES_CURSOR_BLINK_UNDERLINE "\x1B[3 q"     // 깜빡이는 밑줄 커서
#define ES_CURSOR_STEADY_UNDERLINE "\x1B[4 q"    // 고정 밑줄 커서
#define ES_CURSOR_BLINK_BAR      "\x1B[5 q"      // 깜빡이는 바 커서
#define ES_CURSOR_STEADY_BAR     "\x1B[6 q"      // 고정 바 커서

/* =================================================================== */
/*                          화면 제어                                   */
/* =================================================================== */

/* 화면 지우기 */
#define ES_CLEAR_SCREEN          "\x1B[2J"       // 전체 화면 지우기
#define ES_CLEAR_SCREEN_ABOVE    "\x1B[1J"       // 커서 위쪽 화면 지우기
#define ES_CLEAR_SCREEN_BELOW    "\x1B[0J"       // 커서 아래쪽 화면 지우기

/* 줄 지우기 */
#define ES_ERASE_ENTIRE_LINE     "\x1B[2K"       // 전체 줄 지우기
#define ES_ERASE_LINE_LEFT       "\x1B[1K"       // 커서 왼쪽 줄 지우기
#define ES_ERASE_LINE_RIGHT      "\x1B[0K"       // 커서 오른쪽 줄 지우기

/* 스크롤 */
#define ES_SCROLL_UP             "\x1B[S"        // 한 줄 위로 스크롤
#define ES_SCROLL_DOWN           "\x1B[T"        // 한 줄 아래로 스크롤
#define ES_SCROLL_UP_N           "\x1B[%dS"      // N줄 위로 스크롤
#define ES_SCROLL_DOWN_N         "\x1B[%dT"      // N줄 아래로 스크롤

/* 화면 모드 */
#define ES_SAVE_CURSOR           "\x1B[s"        // 커서 위치 저장
#define ES_RESTORE_CURSOR        "\x1B[u"        // 커서 위치 복원
#define ES_SAVE_SCREEN           "\x1B[?47h"     // 화면 저장
#define ES_RESTORE_SCREEN        "\x1B[?47l"     // 화면 복원

/* =================================================================== */
/*                          색상 제어                                   */
/* =================================================================== */

/* 텍스트 속성 리셋 */
#define ES_RESET_ALL             "\x1B[0m"       // 모든 속성 리셋

/* 텍스트 스타일 */
#define ES_BOLD                  "\x1B[1m"       // 굵게
#define ES_DIM                   "\x1B[2m"       // 흐리게
#define ES_ITALIC                "\x1B[3m"       // 기울임
#define ES_UNDERLINE             "\x1B[4m"       // 밑줄
#define ES_BLINK                 "\x1B[5m"       // 깜빡임
#define ES_REVERSE               "\x1B[7m"       // 반전
#define ES_STRIKETHROUGH         "\x1B[9m"       // 취소선

/* 텍스트 스타일 해제 */
#define ES_BOLD_OFF              "\x1B[22m"      // 굵게 해제
#define ES_DIM_OFF               "\x1B[22m"      // 흐리게 해제
#define ES_ITALIC_OFF            "\x1B[23m"      // 기울임 해제
#define ES_UNDERLINE_OFF         "\x1B[24m"      // 밑줄 해제
#define ES_BLINK_OFF             "\x1B[25m"      // 깜빡임 해제
#define ES_REVERSE_OFF           "\x1B[27m"      // 반전 해제
#define ES_STRIKETHROUGH_OFF     "\x1B[29m"      // 취소선 해제

/* 전경색 (텍스트 색상) */
#define ES_FG_BLACK              "\x1B[30m"      // 검은색
#define ES_FG_RED                "\x1B[31m"      // 빨간색
#define ES_FG_GREEN              "\x1B[32m"      // 초록색
#define ES_FG_YELLOW             "\x1B[33m"      // 노란색
#define ES_FG_BLUE               "\x1B[34m"      // 파란색
#define ES_FG_MAGENTA            "\x1B[35m"      // 자홍색
#define ES_FG_CYAN               "\x1B[36m"      // 청록색
#define ES_FG_WHITE              "\x1B[37m"      // 흰색
#define ES_FG_DEFAULT            "\x1B[39m"      // 기본 전경색

/* 배경색 */
#define ES_BG_BLACK              "\x1B[40m"      // 검은색 배경
#define ES_BG_RED                "\x1B[41m"      // 빨간색 배경
#define ES_BG_GREEN              "\x1B[42m"      // 초록색 배경
#define ES_BG_YELLOW             "\x1B[43m"      // 노란색 배경
#define ES_BG_BLUE               "\x1B[44m"      // 파란색 배경
#define ES_BG_MAGENTA            "\x1B[45m"      // 자홍색 배경
#define ES_BG_CYAN               "\x1B[46m"      // 청록색 배경
#define ES_BG_WHITE              "\x1B[47m"      // 흰색 배경
#define ES_BG_DEFAULT            "\x1B[49m"      // 기본 배경색

/* 밝은 전경색 */
#define ES_FG_BRIGHT_BLACK       "\x1B[90m"      // 밝은 검은색
#define ES_FG_BRIGHT_RED         "\x1B[91m"      // 밝은 빨간색
#define ES_FG_BRIGHT_GREEN       "\x1B[92m"      // 밝은 초록색
#define ES_FG_BRIGHT_YELLOW      "\x1B[93m"      // 밝은 노란색
#define ES_FG_BRIGHT_BLUE        "\x1B[94m"      // 밝은 파란색
#define ES_FG_BRIGHT_MAGENTA     "\x1B[95m"      // 밝은 자홍색
#define ES_FG_BRIGHT_CYAN        "\x1B[96m"      // 밝은 청록색
#define ES_FG_BRIGHT_WHITE       "\x1B[97m"      // 밝은 흰색

/* 밝은 배경색 */
#define ES_BG_BRIGHT_BLACK       "\x1B[100m"     // 밝은 검은색 배경
#define ES_BG_BRIGHT_RED         "\x1B[101m"     // 밝은 빨간색 배경
#define ES_BG_BRIGHT_GREEN       "\x1B[102m"     // 밝은 초록색 배경
#define ES_BG_BRIGHT_YELLOW      "\x1B[103m"     // 밝은 노란색 배경
#define ES_BG_BRIGHT_BLUE        "\x1B[104m"     // 밝은 파란색 배경
#define ES_BG_BRIGHT_MAGENTA     "\x1B[105m"     // 밝은 자홍색 배경
#define ES_BG_BRIGHT_CYAN        "\x1B[106m"     // 밝은 청록색 배경
#define ES_BG_BRIGHT_WHITE       "\x1B[107m"     // 밝은 흰색 배경

/* 256색 색상 (printf와 함께 사용) */
#define ES_FG_256COLOR           "\x1B[38;5;%dm"  // 256색 전경색
#define ES_BG_256COLOR           "\x1B[48;5;%dm"  // 256색 배경색

/* RGB 색상 (printf와 함께 사용) */
#define ES_FG_RGB                "\x1B[38;2;%d;%d;%dm"  // RGB 전경색
#define ES_BG_RGB                "\x1B[48;2;%d;%d;%dm"  // RGB 배경색

/* =================================================================== */
/*                          입력 제어                                   */
/* =================================================================== */

/* 키보드 입력 */
#define ES_ENABLE_MOUSE          "\x1B[?1000h"   // 마우스 활성화
#define ES_DISABLE_MOUSE         "\x1B[?1000l"   // 마우스 비활성화
#define ES_ENABLE_ALT_SCREEN     "\x1B[?1049h"   // 대체 화면 활성화
#define ES_DISABLE_ALT_SCREEN    "\x1B[?1049l"   // 대체 화면 비활성화

/* =================================================================== */
/*                         특수 문자                                    */
/* =================================================================== */

/* 제어 문자 */
#define ES_BELL                  "\x07"          // 벨 소리
#define ES_BACKSPACE             "\x08"          // 백스페이스
#define ES_TAB                   "\x09"          // 탭
#define ES_LINE_FEED             "\x0A"          // 줄 바꿈
#define ES_CARRIAGE_RETURN       "\x0D"          // 캐리지 리턴
#define ES_ESCAPE                "\x1B"          // 이스케이프

/* =================================================================== */
/*                         조합 매크로                                  */
/* =================================================================== */

/* 화면 초기화 */
#define ES_INIT_SCREEN           ES_CLEAR_SCREEN ES_CURSOR_HOME ES_CURSOR_SHOW

/* 색상 조합 (자주 사용되는) */
#define ES_ERROR_COLOR           ES_FG_BRIGHT_RED ES_BOLD
#define ES_WARNING_COLOR         ES_FG_BRIGHT_YELLOW ES_BOLD  
#define ES_SUCCESS_COLOR         ES_FG_BRIGHT_GREEN ES_BOLD
#define ES_INFO_COLOR            ES_FG_BRIGHT_CYAN ES_BOLD
#define ES_HEADER_COLOR          ES_FG_BRIGHT_WHITE ES_BG_BLUE ES_BOLD

/* 선택/포커스 표시 */
#define ES_SELECTED              ES_REVERSE ES_BOLD
#define ES_FOCUSED               ES_FG_BRIGHT_WHITE ES_BG_BLUE

/* 디버그 출력 */
#define ES_DEBUG_COLOR           ES_FG_MAGENTA
#define ES_TRACE_COLOR           ES_FG_CYAN ES_DIM

/* =================================================================== */
/*                         편의 매크로                                  */
/* =================================================================== */

/* 위치 이동 + 출력 */
#define ES_GOTO_XY(x, y)         "\x1B[" #y ";" #x "H"

/* 색상 출력 후 리셋 */
#define ES_COLOR_PRINT(color, text)  color text ES_RESET_ALL

/* 줄 클리어 후 출력 */
#define ES_CLEAR_LINE_PRINT(text)    ES_ERASE_ENTIRE_LINE text

#endif /* ESCAPE_SEQUENCE_H */