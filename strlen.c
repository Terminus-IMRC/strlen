/*
 * According to agner.org and uops.info:
 * |------------------------+--------------------+-------------+-----+-----+-------|
 * | Intrin                 | Insn               | Conformance | lat | th  | port  |
 * |------------------------+--------------------+-------------+-----+-----+-------|
 * | _mm_cmpistrs           | pcmpistri x, x, i8 | SSE4.2      | 10  | 3   | 3p0   |
 * | _mm256_min_epu8        | vpminub y, y, y    | AVX2        | 1   | 0.5 | p01   |
 * | _mm256_cmpeq_epi8      | vpcmpeqb y, y, y   | AVX2        | 1   | 0.5 | p01   |
 * | _mm256_testz_si256     | vptest y, y        | AVX         | 3   | 1   | p0 p5 |
 * | _mm256_movemask_epi8   | vpmovmskb r32, y   | AVX2        | 2-3 | 1   | p0    |
 * | _mm512_min_epu8        | vpminub z, z, z    | AVX512BW    | 1   | 1   | p0    |
 * | _mm512_cmpeq_epi8      | vpcmpeqb k, z, z   | AVX512BW    | 3   | 1   | p5    |
 * | _mm512_cmpeq_epi8_mask | vpcmpb k, z, z, i8 | AVX512BW    | 3   | 1   | p5    |
 * | _mm512_testn_epi8_mask | vptestnmb k, z, z  | AVX512BW    | 3   | 1   | p5    |
 * | _ktestz_mask64_u8      | ktestq k, k        | AVX512BW    | 2   | 1   | p0    |
 * |------------------------+--------------------+-------------+-----+-----+-------|
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/user.h> /* For PAGE_SIZE */
#include <string.h>
#include <errno.h>
#include <time.h>

#define barrier() __asm__ volatile ("" : : : "memory")
/*
 * Avoid code elimination of read-only access on ptr.
 * c.f. https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/compiler-gcc.h
 */
#define barrier_data(ptr) __asm__ volatile ("" : : "r" (ptr) : "memory")
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define __maybe_unused __attribute__((__unused__))

#define pprintf_abort(str, ...) \
    do { \
        fprintf(stderr, "%s:%u (%s): ", __FILE__, __LINE__, __func__); \
        fprintf(stderr, str, ##__VA_ARGS__); \
        fprintf(stderr, ": %s\n", strerror(errno)); \
        exit(EXIT_FAILURE); \
    } while (0)

#define printf_abort(str, ...) \
    do { \
        fprintf(stderr, "%s:%u (%s): ", __FILE__, __LINE__, __func__); \
        fprintf(stderr, str, ##__VA_ARGS__); \
        exit(EXIT_FAILURE); \
    } while (0)

static inline
double getsec(void)
{
    struct timespec t;

    const int err = clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    if (err)
        pprintf_abort("clock_gettime(CLOCK_MONOTONIC_RAW)");

    return (double) t.tv_sec + t.tv_nsec * 1e-9;
}

static inline
__maybe_unused
size_t find_null_char(const char *s, const size_t len)
{
    for (size_t i = 0; i < len; i ++)
        if (s[i] == '\0')
            return i;
    printf_abort("Null character is not found\n");
}

static
size_t my_strlen_pure(const char* s)
{
    s = __builtin_assume_aligned(s, PAGE_SIZE);

    size_t c = 0;

    while (*s++)
        c++;

    return c;
}

static
size_t my_strlen_rep(const char * const s)
{
    size_t c;

    __asm__ (
            "mov %[s], %%rdi\n\t"
            "mov $-1, %%rcx\n\t"
            "xor %%al, %%al\n\t"
            "repne scasb\n\t"
            "mov %%rcx, %[c]\n\t"
            : [c] "=r" (c)
            : [s] "r" (s)
            : "al", "rcx", "rdi", "cc", "memory"
    );

    return ~c - 1;
}

#ifdef __SSE4_2__

static
size_t my_strlen_SSE(const char *s)
{
    size_t c = 0;
    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p++, c++)
        if (_mm_cmpistrs(*p, *p, _SIDD_UBYTE_OPS))
            break;

    return c * (128/8) + find_null_char((const char*) p, 128/8);
}

static
size_t my_strlen_SSE_unroll_4(const char *s)
{
    /* Must be less than or equal to 4096 / (128/8) = 256. */
#define UNROLL 4

    size_t c = 0;
    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        int res0 = _mm_cmpistrs(p[0], p[0], _SIDD_UBYTE_OPS);
        int res1 = _mm_cmpistrs(p[1], p[1], _SIDD_UBYTE_OPS);
        int res2 = _mm_cmpistrs(p[2], p[2], _SIDD_UBYTE_OPS);
        int res3 = _mm_cmpistrs(p[3], p[3], _SIDD_UBYTE_OPS);
        if (res0 || res1 || res2 || res3)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_SSE_unroll_8(const char *s)
{
    /* Must be less than or equal to 4096 / (128/8) = 256. */
#define UNROLL 8

    size_t c = 0;
    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        int res0 = _mm_cmpistrs(p[0], p[0], _SIDD_UBYTE_OPS);
        int res1 = _mm_cmpistrs(p[1], p[1], _SIDD_UBYTE_OPS);
        int res2 = _mm_cmpistrs(p[2], p[2], _SIDD_UBYTE_OPS);
        int res3 = _mm_cmpistrs(p[3], p[3], _SIDD_UBYTE_OPS);
        int res4 = _mm_cmpistrs(p[4], p[4], _SIDD_UBYTE_OPS);
        int res5 = _mm_cmpistrs(p[5], p[5], _SIDD_UBYTE_OPS);
        int res6 = _mm_cmpistrs(p[6], p[6], _SIDD_UBYTE_OPS);
        int res7 = _mm_cmpistrs(p[7], p[7], _SIDD_UBYTE_OPS);
        if (res0 || res1 || res2 || res3 || res4 || res5 || res6 || res7)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_SSE_unroll_16(const char *s)
{
    /* Must be less than or equal to 4096 / (128/8) = 256. */
#define UNROLL 16

    size_t c = 0;
    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        int res0 = _mm_cmpistrs(p[0], p[0], _SIDD_UBYTE_OPS);
        int res1 = _mm_cmpistrs(p[1], p[1], _SIDD_UBYTE_OPS);
        int res2 = _mm_cmpistrs(p[2], p[2], _SIDD_UBYTE_OPS);
        int res3 = _mm_cmpistrs(p[3], p[3], _SIDD_UBYTE_OPS);
        int res4 = _mm_cmpistrs(p[4], p[4], _SIDD_UBYTE_OPS);
        int res5 = _mm_cmpistrs(p[5], p[5], _SIDD_UBYTE_OPS);
        int res6 = _mm_cmpistrs(p[6], p[6], _SIDD_UBYTE_OPS);
        int res7 = _mm_cmpistrs(p[7], p[7], _SIDD_UBYTE_OPS);
        int res8 = _mm_cmpistrs(p[8], p[8], _SIDD_UBYTE_OPS);
        int res9 = _mm_cmpistrs(p[9], p[9], _SIDD_UBYTE_OPS);
        int resa = _mm_cmpistrs(p[10], p[10], _SIDD_UBYTE_OPS);
        int resb = _mm_cmpistrs(p[11], p[11], _SIDD_UBYTE_OPS);
        int resc = _mm_cmpistrs(p[12], p[12], _SIDD_UBYTE_OPS);
        int resd = _mm_cmpistrs(p[13], p[13], _SIDD_UBYTE_OPS);
        int rese = _mm_cmpistrs(p[14], p[14], _SIDD_UBYTE_OPS);
        int resf = _mm_cmpistrs(p[15], p[15], _SIDD_UBYTE_OPS);
        if (res0 || res1 || res2 || res3 || res4 || res5 || res6 || res7
                || res8 || res9 || resa || resb || resc || resd || rese || resf)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_SSE_unroll_4_separate_load_cmp(const char *s)
{
    /* Must be less than or equal to 4096 / (128/8) = 256. */
#define UNROLL 4

    size_t c = 0;
    __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        register __m128i v0 = _mm_stream_load_si128(p + 0);
        register __m128i v1 = _mm_stream_load_si128(p + 1);
        register __m128i v2 = _mm_stream_load_si128(p + 2);
        register __m128i v3 = _mm_stream_load_si128(p + 3);
        int res0 = _mm_cmpistrs(v0, v0, _SIDD_UBYTE_OPS);
        int res1 = _mm_cmpistrs(v1, v1, _SIDD_UBYTE_OPS);
        int res2 = _mm_cmpistrs(v2, v2, _SIDD_UBYTE_OPS);
        int res3 = _mm_cmpistrs(v3, v3, _SIDD_UBYTE_OPS);
        if (res0 || res1 || res2 || res3)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_SSE_unroll_8_separate_load_cmp(const char *s)
{
    /* Must be less than or equal to 4096 / (128/8) = 256. */
#define UNROLL 8

    size_t c = 0;
    __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        register __m128i v0 = _mm_stream_load_si128(p + 0);
        register __m128i v1 = _mm_stream_load_si128(p + 1);
        register __m128i v2 = _mm_stream_load_si128(p + 2);
        register __m128i v3 = _mm_stream_load_si128(p + 3);
        register __m128i v4 = _mm_stream_load_si128(p + 4);
        register __m128i v5 = _mm_stream_load_si128(p + 5);
        register __m128i v6 = _mm_stream_load_si128(p + 6);
        register __m128i v7 = _mm_stream_load_si128(p + 7);
        int res0 = _mm_cmpistrs(v0, v0, _SIDD_UBYTE_OPS);
        int res1 = _mm_cmpistrs(v1, v1, _SIDD_UBYTE_OPS);
        int res2 = _mm_cmpistrs(v2, v2, _SIDD_UBYTE_OPS);
        int res3 = _mm_cmpistrs(v3, v3, _SIDD_UBYTE_OPS);
        int res4 = _mm_cmpistrs(v4, v4, _SIDD_UBYTE_OPS);
        int res5 = _mm_cmpistrs(v5, v5, _SIDD_UBYTE_OPS);
        int res6 = _mm_cmpistrs(v6, v6, _SIDD_UBYTE_OPS);
        int res7 = _mm_cmpistrs(v7, v7, _SIDD_UBYTE_OPS);
        if (res0 || res1 || res2 || res3 || res4 || res5 || res6 || res7)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_SSE_unroll_16_separate_load_cmp(const char *s)
{
    /* Must be less than or equal to 4096 / (128/8) = 256. */
#define UNROLL 16

    size_t c = 0;
    __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        register __m128i v0 = _mm_stream_load_si128(p + 0);
        register __m128i v1 = _mm_stream_load_si128(p + 1);
        register __m128i v2 = _mm_stream_load_si128(p + 2);
        register __m128i v3 = _mm_stream_load_si128(p + 3);
        register __m128i v4 = _mm_stream_load_si128(p + 4);
        register __m128i v5 = _mm_stream_load_si128(p + 5);
        register __m128i v6 = _mm_stream_load_si128(p + 6);
        register __m128i v7 = _mm_stream_load_si128(p + 7);
        register __m128i v8 = _mm_stream_load_si128(p + 8);
        register __m128i v9 = _mm_stream_load_si128(p + 9);
        register __m128i va = _mm_stream_load_si128(p + 10);
        register __m128i vb = _mm_stream_load_si128(p + 11);
        register __m128i vc = _mm_stream_load_si128(p + 12);
        register __m128i vd = _mm_stream_load_si128(p + 13);
        register __m128i ve = _mm_stream_load_si128(p + 14);
        register __m128i vf = _mm_stream_load_si128(p + 15);
        int res0 = _mm_cmpistrs(v0, v0, _SIDD_UBYTE_OPS);
        int res1 = _mm_cmpistrs(v1, v1, _SIDD_UBYTE_OPS);
        int res2 = _mm_cmpistrs(v2, v2, _SIDD_UBYTE_OPS);
        int res3 = _mm_cmpistrs(v3, v3, _SIDD_UBYTE_OPS);
        int res4 = _mm_cmpistrs(v4, v4, _SIDD_UBYTE_OPS);
        int res5 = _mm_cmpistrs(v5, v5, _SIDD_UBYTE_OPS);
        int res6 = _mm_cmpistrs(v6, v6, _SIDD_UBYTE_OPS);
        int res7 = _mm_cmpistrs(v7, v7, _SIDD_UBYTE_OPS);
        int res8 = _mm_cmpistrs(v8, v8, _SIDD_UBYTE_OPS);
        int res9 = _mm_cmpistrs(v9, v9, _SIDD_UBYTE_OPS);
        int resa = _mm_cmpistrs(va, va, _SIDD_UBYTE_OPS);
        int resb = _mm_cmpistrs(vb, vb, _SIDD_UBYTE_OPS);
        int resc = _mm_cmpistrs(vc, vc, _SIDD_UBYTE_OPS);
        int resd = _mm_cmpistrs(vd, vd, _SIDD_UBYTE_OPS);
        int rese = _mm_cmpistrs(ve, ve, _SIDD_UBYTE_OPS);
        int resf = _mm_cmpistrs(vf, vf, _SIDD_UBYTE_OPS);
        if (res0 || res1 || res2 || res3 || res4 || res5 || res6 || res7
                || res8 || res9 || resa || resb || resc || resd || rese || resf)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

#endif /* __SSE4_2__ */

#ifdef __AVX2__

static
size_t my_strlen_AVX2_vptest_unroll_2(const char *s)
{
    /* Must be less than or equal to 4096 / (256/8) = 128. */
#define UNROLL 2

    __m256i *p = (__m256i*) __builtin_assume_aligned(s, PAGE_SIZE);
    register const __m256i zero = _mm256_setzero_si256();
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        __m256i tmp = _mm256_min_epu8(p[0], p[1]);
        /* Match=0xff Unmatch=0x00 */
        __m256i cmp = _mm256_cmpeq_epi8(tmp, zero);
        if (!_mm256_testz_si256(cmp, cmp))
            break;
    }

    return c * (256/8) * UNROLL
        + find_null_char((const char*) p, (256/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_AVX2_vptest_unroll_4(const char *s)
{
    /* Must be less than or equal to 4096 / (256/8) = 128. */
#define UNROLL 4

    __m256i *p = (__m256i*) __builtin_assume_aligned(s, PAGE_SIZE);
    register const __m256i zero = _mm256_setzero_si256();
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        register __m256i tmp01 = _mm256_min_epu8(p[0], p[1]);
        register __m256i tmp23 = _mm256_min_epu8(p[2], p[3]);
        register __m256i tmp0123 = _mm256_min_epu8(tmp01, tmp23);
        /* Match=0xff Unmatch=0x00 */
        __m256i cmp = _mm256_cmpeq_epi8(tmp0123, zero);
        if (!_mm256_testz_si256(cmp, cmp))
            break;
    }

    return c * (256/8) * UNROLL
        + find_null_char((const char*) p, (256/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_AVX2_vptest_unroll_8(const char *s)
{
    /* Must be less than or equal to 4096 / (256/8) = 128. */
#define UNROLL 8

    __m256i *p = (__m256i*) __builtin_assume_aligned(s, PAGE_SIZE);
    register const __m256i zero = _mm256_setzero_si256();
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        register __m256i tmp01 = _mm256_min_epu8(p[0], p[1]);
        register __m256i tmp23 = _mm256_min_epu8(p[2], p[3]);
        register __m256i tmp0123 = _mm256_min_epu8(tmp01, tmp23);
        register __m256i tmp45 = _mm256_min_epu8(p[4], p[5]);
        register __m256i tmp67 = _mm256_min_epu8(p[6], p[7]);
        register __m256i tmp4567 = _mm256_min_epu8(tmp45, tmp67);
        register __m256i tmp01234567 = _mm256_min_epu8(tmp0123, tmp4567);
        /* Match=0xff Unmatch=0x00 */
        __m256i cmp = _mm256_cmpeq_epi8(tmp01234567, zero);
        if (!_mm256_testz_si256(cmp, cmp))
            break;
    }

    return c * (256/8) * UNROLL
        + find_null_char((const char*) p, (256/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_AVX2_vpmovmskb_unroll_2(const char *s)
{
    /* Must be less than or equal to 4096 / (256/8) = 128. */
#define UNROLL 2

    __m256i *p = (__m256i*) __builtin_assume_aligned(s, PAGE_SIZE);
    register const __m256i zero = _mm256_setzero_si256();
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        __m256i tmp = _mm256_min_epu8(p[0], p[1]);
        /* Match=0xff Unmatch=0x00 */
        __m256i cmp = _mm256_cmpeq_epi8(tmp, zero);
        if (_mm256_movemask_epi8(cmp))
            break;
    }

    return c * (256/8) * UNROLL
        + find_null_char((const char*) p, (256/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_AVX2_vpmovmskb_unroll_4(const char *s)
{
    /* Must be less than or equal to 4096 / (256/8) = 128. */
#define UNROLL 4

    __m256i *p = (__m256i*) __builtin_assume_aligned(s, PAGE_SIZE);
    register const __m256i zero = _mm256_setzero_si256();
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        register __m256i tmp01 = _mm256_min_epu8(p[0], p[1]);
        register __m256i tmp23 = _mm256_min_epu8(p[2], p[3]);
        register __m256i tmp0123 = _mm256_min_epu8(tmp01, tmp23);
        /* Match=0xff Unmatch=0x00 */
        __m256i cmp = _mm256_cmpeq_epi8(tmp0123, zero);
        if (_mm256_movemask_epi8(cmp))
            break;
    }

    return c * (256/8) * UNROLL
        + find_null_char((const char*) p, (256/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_AVX2_vpmovmskb_unroll_8(const char *s)
{
    /* Must be less than or equal to 4096 / (256/8) = 128. */
#define UNROLL 8

    __m256i *p = (__m256i*) __builtin_assume_aligned(s, PAGE_SIZE);
    register const __m256i zero = _mm256_setzero_si256();
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        register __m256i tmp01 = _mm256_min_epu8(p[0], p[1]);
        register __m256i tmp23 = _mm256_min_epu8(p[2], p[3]);
        register __m256i tmp0123 = _mm256_min_epu8(tmp01, tmp23);
        register __m256i tmp45 = _mm256_min_epu8(p[4], p[5]);
        register __m256i tmp67 = _mm256_min_epu8(p[6], p[7]);
        register __m256i tmp4567 = _mm256_min_epu8(tmp45, tmp67);
        register __m256i tmp01234567 = _mm256_min_epu8(tmp0123, tmp4567);
        /* Match=0xff Unmatch=0x00 */
        __m256i cmp = _mm256_cmpeq_epi8(tmp01234567, zero);
        if (_mm256_movemask_epi8(cmp))
            break;
    }

    return c * (256/8) * UNROLL
        + find_null_char((const char*) p, (256/8) * UNROLL);

#undef UNROLL
}

#endif /* __AVX2__ */

#ifdef __AVX512BW__

static
size_t my_strlen_AVX512_vpcmpeqb_unroll_2(const char *s)
{
    /* Must be less than or equal to 4096 / (512/8) = 64. */
#define UNROLL 2

    __m512i *p = (__m512i*) __builtin_assume_aligned(s, PAGE_SIZE);
    register const __m512i zero = _mm512_setzero_si512();
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        __m512i tmp = _mm512_min_epu8(p[0], p[1]);
        /* vpcmpeqb: equal => 1, not equal => 0 */
        const __mmask64 cmp = _mm512_cmpeq_epi8_mask(tmp, zero);
        if (!_ktestz_mask64_u8(cmp, cmp))
            break;
    }

    return c * (512/8) * UNROLL
        + find_null_char((const char*) p, (512/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_AVX512_vpcmpeqb_unroll_4(const char *s)
{
    /* Must be less than or equal to 4096 / (512/8) = 64. */
#define UNROLL 4

    __m512i *p = (__m512i*) __builtin_assume_aligned(s, PAGE_SIZE);
    register const __m512i zero = _mm512_setzero_si512();
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        register const __m512i tmp01 = _mm512_min_epu8(p[0], p[1]);
        register const __m512i tmp23 = _mm512_min_epu8(p[2], p[3]);
        register const __m512i tmp0123 = _mm512_min_epu8(tmp01, tmp23);
        const __mmask64 cmp = _mm512_cmpeq_epi8_mask(zero, tmp0123);
        if (!_ktestz_mask64_u8(cmp, cmp))
            break;
    }

    return c * (512/8) * UNROLL
        + find_null_char((const char*) p, (512/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_AVX512_vptestnmb_unroll_2(const char *s)
{
    /* Must be less than or equal to 4096 / (512/8) = 64. */
#define UNROLL 2

    __m512i *p = (__m512i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        __m512i tmp = _mm512_min_epu8(p[0], p[1]);
        /* 0x00 => 1, others => 0 */
        const __mmask64 cmp = _mm512_testn_epi8_mask(tmp, tmp);
        if (!_ktestz_mask64_u8(cmp, cmp))
            break;
    }

    return c * (512/8) * UNROLL
        + find_null_char((const char*) p, (512/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_AVX512_vptestnmb_unroll_4(const char *s)
{
    /* Must be less than or equal to 4096 / (512/8) = 64. */
#define UNROLL 4

    __m512i *p = (__m512i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        register const __m512i tmp01 = _mm512_min_epu8(p[0], p[1]);
        register const __m512i tmp23 = _mm512_min_epu8(p[2], p[3]);
        register const __m512i tmp0123 = _mm512_min_epu8(tmp01, tmp23);
        const __mmask64 cmp = _mm512_testn_epi8_mask(tmp0123, tmp0123);
        if (!_ktestz_mask64_u8(cmp, cmp))
            break;
    }

    return c * (512/8) * UNROLL
        + find_null_char((const char*) p, (512/8) * UNROLL);

#undef UNROLL
}

#endif /* __AVX512BW__ */

static
void bench_strlen(char * const s, const size_t len_expected,
        size_t (*mystrlen)(const char *s), const unsigned repetition)
{
    /* Warmup */
    for (unsigned i = 0; i < 4; i ++) {
        const size_t len = mystrlen(s);
        if (unlikely(len != len_expected))
            printf_abort("len(%zu) is not expected(%zu)\n", len, len_expected);
    }

    barrier();
    double start = getsec();
    barrier();
    for (unsigned i = 0; i < repetition; i ++) {
        barrier_data(s);
        const size_t len = mystrlen(s);
        if (unlikely(len != len_expected))
            printf_abort("len(%zu) is not expected(%zu)\n", len, len_expected);
    }
    barrier();
    double end = getsec();
    barrier();

    double t = (end - start) / repetition;

    printf("%e [s], %e [s], %e [char/s]\n", end - start, t, len_expected / t);
}

int main(void)
{
    const size_t len = 1ULL << 27;

    printf("String length: %zu MiB\n", len >> 20);

    char *s;

    const int err = posix_memalign((void**) &s, PAGE_SIZE, len);
    if (err)
        pprintf_abort("posix_memalign");

    (void) memset(s, 0x55, len - 1);
    s[len - 1] = '\0';

#define DO(func, repetition) \
    do { \
        printf("%-41s: ", #func); \
        bench_strlen(s, len-1, func, repetition); \
    } while (0)

    DO(strlen, 128);
    DO(my_strlen_pure, 32);
    DO(my_strlen_rep, 8);

#ifdef __SSE4_2__
    DO(my_strlen_SSE, 128);
    DO(my_strlen_SSE_unroll_4, 128);
    DO(my_strlen_SSE_unroll_8, 128);
    DO(my_strlen_SSE_unroll_16, 128);
    DO(my_strlen_SSE_unroll_4_separate_load_cmp, 128);
    DO(my_strlen_SSE_unroll_8_separate_load_cmp, 128);
    DO(my_strlen_SSE_unroll_16_separate_load_cmp, 128);
#endif /* __SSE4_2__ */

#ifdef __AVX2__
    DO(my_strlen_AVX2_vptest_unroll_2, 128);
    DO(my_strlen_AVX2_vptest_unroll_4, 128);
    DO(my_strlen_AVX2_vptest_unroll_8, 128);
    DO(my_strlen_AVX2_vpmovmskb_unroll_2, 128);
    DO(my_strlen_AVX2_vpmovmskb_unroll_4, 128);
    DO(my_strlen_AVX2_vpmovmskb_unroll_8, 128);
#endif /* __AVX2__ */

#ifdef __AVX512BW__
    DO(my_strlen_AVX512_vpcmpeqb_unroll_2, 128);
    DO(my_strlen_AVX512_vpcmpeqb_unroll_4, 128);
    DO(my_strlen_AVX512_vptestnmb_unroll_2, 128);
    DO(my_strlen_AVX512_vptestnmb_unroll_4, 128);
#endif /* __AVX512BW__ */

    free(s);
    return 0;
}
