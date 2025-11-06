# Big Integer 곱셈 (`big_mult.c`)

**작성자:** 김재노  
**과목:** 시스템 프로그래밍 (System Programming)  
**언어:** C (C11)  
**개발 환경:** Visual Studio  



## 프로젝트 개요

이 프로그램은 **기본 정수형(int, long 등)** 으로는 표현할 수 없는 매우 큰 수(예: `100000 × 100000 = 10,000,000,000`)를  
직접 만든 **Big Integer (큰 정수) 연산 라이브러리**로 계산하는 과제용 프로젝트입니다.

> 핵심 아이디어:  
> 큰 수를 32비트 단위 워드 배열로 나누어 저장하고,  
> 각 워드를 이용해 학교식 곱셈 알고리즘으로 직접 계산합니다.  
> 결과는 **16진수(HEX)** 로 출력됩니다.


## 주요 기능

- 임의 크기의 정수 연산 지원 (길이에 제한 없음)  
- 10진수 문자열 입력 → 내부 BigInt 구조로 변환  
- 외부 라이브러리 사용 없이 직접 구현한 곱셈(`big_mul`)  
- 결과를 16진수(0x...) 형태로 출력  
- 인자 없이 실행 시 기본값 `100000 × 100000` 자동 계산  
- Visual Studio에서 **F5 한 번으로 실행 가능**


## 자료구조 설명

```c
typedef struct {
    uint32_t* limb;  // 32비트 워드 배열 (리틀 엔디언)
    size_t    len;   // 현재 사용 중인 워드 수
} BigInt;
```
>각 BigInt는 다음과 같이 표현됩니다:
>값 = limb[0] + limb[1] × 2³² + limb[2] × 2⁶⁴ + …
>즉, limb[0]이 가장 낮은 자리(최하위 워드),
>limb[len-1]이 가장 높은 자리(최상위 워드)를 나타냅니다.


## 주요 함수 설명

```c
// BigInt 구조체와 관련된 핵심 함수 목록

// 1. n개의 워드(32bit)로 구성된 BigInt 생성
BigInt big_create(size_t n);

// 2. BigInt 메모리 해제
void big_free(BigInt* a);

// 3. 10진수 문자열을 BigInt로 변환
BigInt big_from_dec_string(const char* s);

// 4. 두 BigInt의 곱셈 수행
BigInt big_mul(const BigInt* A, const BigInt* B);

// 5. BigInt를 16진수(HEX) 형태로 출력
void big_print_hex(const BigInt* a);
```
## 동작 원리
### 입력 (Input)
- 10진수 문자열(예: "100000")을 받아서 BigInt 구조체로 변환합니다.
- 각 자리수를 하나씩 읽으며 res = res * 10 + digit 연산을 수행합니다.

### 곱셈 (Multiplication)
- 학교식 곱셈(자리 올림 포함)을 사용합니다.
- 시간 복잡도는 O(n × m)입니다.

### 출력 (Output)
- 최종 결과를 16진수(0x...) 형태로 출력합니다.
- 예: 100000 * 100000 = 0x2540BE400

## 실행 결과

콘솔에 다음과 같은 결과 출력:
``` consol
[INFO] No arguments given. Using default: 100000 * 100000
100000 (dec) * 100000 (dec) = 0x2540BE400
```
### 명령줄 인자 테스트
프로젝트 속성 → 디버깅 → 명령 인수(Command Arguments) 항목에 입력:
```
12345 67890
```
실행 결과:
```
12345 (dec) * 67890 (dec) = 0x3CDFD62A
```


## 결론
이 프로젝트는

C 언어의 기본 정수형 한계를 극복하고,

직접 구현한 Big Integer 연산 라이브러리를 통해 큰 수 계산을 수행하며,

운영체제 수준의 메모리 관리와 수학적 로직 설계 능력을 함께 다루는
시스템 프로그래밍 과제용 프로젝트입니다.
