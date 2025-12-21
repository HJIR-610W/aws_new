#ifndef UTIL_MACRO_H
#define UTIL_MACRO_H

/*
 * util_macro.h
 * 자주 쓰는 전처리기 유틸 매크로 모음
 *
 * - C99 기준
 * - 임베디드(STM32, CMSIS, FreeRTOS) 친화
 */

/* ============================================================
 * 1. 문자열화 (Stringify)
 * ============================================================ */

/*
 * STR(x)
 * - 매크로/리터럴을 문자열로 변환
 * - 2단계 매크로 필수
 */
#define STR_(x)   #x
#define STR(x)    STR_(x)

/* ============================================================
 * 2. 배열 관련
 * ============================================================ */

/*
 * COUNTOF(arr)
 * - 배열 요소 개수
 * - 포인터에 쓰면 컴파일은 되지만 논리 오류 → 사용 주의
 */
#define COUNTOF(arr)   (sizeof(arr) / sizeof((arr)[0]))

/*
 * ARRAY_END(arr)
 * - 배열 끝 포인터
 */
#define ARRAY_END(arr) ((arr) + COUNTOF(arr))

/* ============================================================
 * 3. MIN / MAX / CLAMP
 * ============================================================ */

/*
 * 타입 안전하지 않음 (부작용 있는 표현식 주의)
 */
#ifndef MIN
#define MIN(a, b)      (( (a) < (b) ) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)      (( (a) > (b) ) ? (a) : (b))
#endif

/*
 * CLAMP(x, low, high)
 * - 범위 제한
 */
#define CLAMP(x, low, high) \
  ( ((x) < (low)) ? (low) : (((x) > (high)) ? (high) : (x)) )

/* ============================================================
 * 4. UNUSED / LIKELY / UNLIKELY
 * ============================================================ */

/*
 * UTIL_UNUSED(x)
 * - 미사용 파라미터 경고 제거
 */
#ifndef UTIL_UNUSED
#define UTIL_UNUSED(X) (void)X  
#endif


/* ============================================================
 * 5. BIT 매크로
 * ============================================================ */

/*
 * BIT(n)
 * - n번째 비트
 */
#define BIT(n)      (1UL << (n))

/*
 * BIT_MASK(msb, lsb)
 * - 비트 마스크 생성
 * - 예: BIT_MASK(7,4) → 0xF0
 */
#define BIT_MASK(msb, lsb) \
  ( ((1UL << ((msb) - (lsb) + 1)) - 1UL) << (lsb) )

/* ============================================================
 * 6. ALIGN / ROUND
 * ============================================================ */

/*
 * ALIGN_UP(x, a)
 * - a는 2의 거듭제곱
 */
/* a는 반드시 2의 거듭제곱(2^n) */
#define ALIGN_UP_P2(x, a)     (((x) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN_P2(x, a)   ((x) & ~((a) - 1))

    
/*
 * DIV_ROUND_UP
 */
#define DIV_ROUND_UP(n, d)   (((n) + (d) - 1) / (d))

/* ============================================================
 * 7. 토큰 결합 (Token paste)
 * ============================================================ */

/*
 * CONCAT(a, b)
 * - 토큰 결합
 */
#define CONCAT_(a, b)   a##b
#define CONCAT(a, b)    CONCAT_(a, b)

/*
 * CONCAT3(a, b, c)
 */
#define CONCAT3(a, b, c)  CONCAT(CONCAT(a, b), c)

/* ============================================================
 * 8. 컴파일 타임 체크
 * ============================================================ */

/*
 * STATIC_ASSERT(cond)
 * - 컴파일 타임 조건 체크
 */
#define STATIC_ASSERT(cond) \
  typedef char CONCAT(static_assert_, __LINE__)[(cond) ? 1 : -1]

/* ============================================================
 * 9. 주소/포인터 유틸
 * ============================================================ */

#define PTR_ADD(ptr, offset)   ((void *)((uintptr_t)(ptr) + (offset)))
#define PTR_SUB(ptr, offset)   ((void *)((uintptr_t)(ptr) - (offset)))

/* ============================================================
 * 10. 범용 SWAP
 * ============================================================ */

/*
 * SWAP(type, a, b)
 * - 타입 명시 필요
 */
#define SWAP(type, a, b) \
  do \
  { \
    type _tmp = (a); \
    (a) = (b); \
    (b) = _tmp; \
  } while (0)

/* ============================================================
 * 11. sizeof 안전 매크로
 * ============================================================ */

/*
 * SIZEOF_MEMBER
 * - 구조체 멤버 크기
 */
#define SIZEOF_MEMBER(type, member)  sizeof(((type *)0)->member)

/*
 * OFFSET_OF
 * - 구조체 멤버 오프셋
 */
#define OFFSET_OF(type, member)  ((size_t)&(((type *)0)->member))
    
#endif
