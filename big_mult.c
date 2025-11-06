#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

  /*
   * BigInt 구조체
   * - limb: 32비트 워드 배열 (리틀 엔디언, limb[0]가 최하위 워드)
   * - len : 현재 사용 중인 워드 개수
   *
   * 예) 값 = limb[0] + limb[1]*2^32 + limb[2]*2^64 + ...
   */
typedef struct {
    uint32_t* limb;
    size_t    len;
} BigInt;


 /* 메모리 할당 실패 시 프로그램 종료 */
static void die(const char* msg) {
    fprintf(stderr, "Fatal error: %s\n", msg);
    exit(EXIT_FAILURE);
}

/* BigInt 생성 (워드 개수 n, 초기값 0) */
static BigInt big_create(size_t n) {
    BigInt a;
    a.len = n;
    a.limb = (uint32_t*)calloc(n, sizeof(uint32_t));  // 0으로 초기화
    if (!a.limb) die("memory allocation failed");
    return a;
}

/* BigInt 해제 */
static void big_free(BigInt* a) {
    if (a->limb) {
        free(a->limb);
        a->limb = NULL;
    }
    a->len = 0;
}

/* 앞쪽(상위) 불필요한 0 워드 제거 (len 축소) */
static void big_normalize(BigInt* a) {
    while (a->len > 1 && a->limb[a->len - 1] == 0) {
        a->len--;
    }
}

/* 0인지 확인 */
static int big_is_zero(const BigInt* a) {
    return (a->len == 0) || (a->len == 1 && a->limb[0] == 0);
}

 /*
  * 10진수 문자열이 유효한지 체크 (선행 공백은 허용 X, +, - 도 허용 X)
  * 양수만 취급한다고 가정.
  */
static int is_valid_dec_string(const char* s) {
    if (*s == '\0') return 0;
    while (*s) {
        if (!isdigit((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

/*
 * 내부 보조 함수: BigInt res = 0으로 초기화 후,
 * res = res * 10 + digit 를 수행하는 연산
 * (digit 은 0~9)
 */

 /* res = res * 10 */
static void big_mul_small(BigInt* res, uint32_t k) {
    uint64_t carry = 0;
    for (size_t i = 0; i < res->len; i++) {
        uint64_t cur = (uint64_t)res->limb[i] * k + carry;
        res->limb[i] = (uint32_t)(cur & 0xFFFFFFFFu);
        carry = cur >> 32;
    }
    if (carry != 0) {
        // limb 하나 더 필요
        size_t new_len = res->len + 1;
        uint32_t* new_limb = (uint32_t*)realloc(res->limb, new_len * sizeof(uint32_t));
        if (!new_limb) die("realloc failed");
        res->limb = new_limb;
        res->limb[res->len] = (uint32_t)carry;
        res->len = new_len;
    }
}

/* res = res + k (0 <= k < 10) */
static void big_add_small(BigInt* res, uint32_t k) {
    uint64_t carry = k;
    size_t i = 0;
    while (carry != 0 && i < res->len) {
        uint64_t cur = (uint64_t)res->limb[i] + carry;
        res->limb[i] = (uint32_t)(cur & 0xFFFFFFFFu);
        carry = cur >> 32;
        i++;
    }
    if (carry != 0) {
        size_t new_len = res->len + 1;
        uint32_t* new_limb = (uint32_t*)realloc(res->limb, new_len * sizeof(uint32_t));
        if (!new_limb) die("realloc failed");
        res->limb = new_limb;
        res->limb[res->len] = (uint32_t)carry;
        res->len = new_len;
    }
}

/*
 * 10진수 문자열 → BigInt
 * - 알고리즘: result = 0 에서 시작
 *   각 자리 digit 에 대해: result = result * 10 + digit
 * - 예: "123" -> ((0*10+1)*10+2)*10+3
 */
static BigInt big_from_dec_string(const char* s) {
    if (!is_valid_dec_string(s)) {
        die("Invalid decimal string (only non-negative integers allowed)");
    }

    // 처음엔 워드 1개짜리(0)로 시작
    BigInt res = big_create(1);
    res.limb[0] = 0;

    while (*s) {
        uint32_t digit = (uint32_t)(*s - '0');  // 0~9
        big_mul_small(&res, 10);                // res *= 10
        big_add_small(&res, digit);             // res += digit
        s++;
    }

    big_normalize(&res);
    return res;
}


 /*
  * C = A * B  (학교식 곱셈)
  *
  * - A, B: 32비트 워드 리틀 엔디언
  * - C: 새로 할당해서 반환 (호출자가 big_free()로 해제)
  */
static BigInt big_mul(const BigInt* A, const BigInt* B) {
    size_t n = A->len;
    size_t m = B->len;

    // 결과는 최대 n+m 워드까지 필요
    BigInt C = big_create(n + m);

    for (size_t i = 0; i < n; i++) {
        uint64_t carry = 0;
        uint64_t a_i = A->limb[i];

        for (size_t j = 0; j < m; j++) {
            uint64_t cur = (uint64_t)C.limb[i + j] +
                a_i * (uint64_t)B->limb[j] +
                carry;
            C.limb[i + j] = (uint32_t)(cur & 0xFFFFFFFFu);
            carry = cur >> 32;
        }

        C.limb[i + m] = (uint32_t)((uint64_t)C.limb[i + m] + carry);
    }

    big_normalize(&C);
    return C;
}

 /*
  * BigInt를 16진수로 출력
  * - 예: 0이면 "0x0"
  * - 그 외: "0x" + 상위 워드(앞에 0 제거) + 나머지는 8자리씩 0 채워서 출력
  */
static void big_print_hex(const BigInt* a) {
    if (big_is_zero(a)) {
        printf("0x0");
        return;
    }

    printf("0x");
    size_t i = a->len;
    i--;

    // 최상위 워드는 앞쪽 0 제거하고 출력
    printf("%X", a->limb[i]);

    // 나머지 워드들은 항상 8자리 16진수(앞 0 포함)로 출력
    while (i > 0) {
        i--;
        printf("%08X", a->limb[i]);
    }
}


 /*
  * 사용법:
  *   big_mult           -> 100000 x 100000 수행
  *   big_mult A B       -> 10진수 문자열 A, B 곱셈
  *
  * 출력:
  *   A (dec) * B (dec) = (hex)
  */
int main(int argc, char* argv[]) {
    const char* s1;
    const char* s2;

    if (argc == 1) {
        // 인자 없으면 기본 테스트: 100000 x 100000
        s1 = "100000";
        s2 = "100000";
        printf("[INFO] No arguments given. Using default: %s * %s\n", s1, s2);
    }
    else if (argc == 3) {
        s1 = argv[1];
        s2 = argv[2];
    }
    else {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s           (default: 100000 * 100000)\n", argv[0]);
        fprintf(stderr, "  %s <A> <B>   (A, B are non-negative decimal strings)\n", argv[0]);
        return EXIT_FAILURE;
    }

    // 10진수 문자열 → BigInt 변환
    BigInt A = big_from_dec_string(s1);
    BigInt B = big_from_dec_string(s2);

    // 곱셈
    BigInt C = big_mul(&A, &B);

    // 결과 출력
    printf("%s (dec) * %s (dec) = ", s1, s2);
    big_print_hex(&C);
    printf("\n");

    // 메모리 해제
    big_free(&A);
    big_free(&B);
    big_free(&C);

    return 0;
}
