/*
 * According to agner.org and uops.info:
 *
 * +--------------------------+--------------------+-------------+-----+------+-------+
 * | Intrinsic                | Instruction        | Conformance | lat | th   | port  |
 * +==========================+====================+=============+=====+======+=======+
 * | _mm_cmpistrs             | pcmpistri x, x, i8 | SSE4.2      | 10  | 3    | 3p0   |
 * | _mm_min_epu8             | pminub x, x        | SSE2        | 1   | 0.5  | p01   |
 * | _mm_cmpeq_epi8           | pcmpeqb x, x       | SSE2        | 1   | 0.5  | p01   |
 * | _mm_testz_si128          | ptest x, x         | SSE4.1      | 3   | 1    | p0 p5 |
 * | _mm_movemask_epi8        | pmovmskb r32, x    | SSE2        | 2-3 | 1    | p0    |
 * | _mm_setzero_si128        | pxor x, x          | SSE2        | 1   | 0.33 | p015  |
 * | _mm_stream_load_si128    | movntdqa x, m128   | SSE4.1      |     |      |       |
 * +--------------------------+--------------------+-------------+-----+------+-------+
 * | _mm256_min_epu8          | vpminub y, y, y    | AVX2        | 1   | 0.5  | p01   |
 * | _mm256_cmpeq_epi8        | vpcmpeqb y, y, y   | AVX2        | 1   | 0.5  | p01   |
 * | _mm256_testz_si256       | vptest y, y        | AVX         | 3   | 1    | p0 p5 |
 * | _mm256_movemask_epi8     | vpmovmskb r32, y   | AVX2        | 2-3 | 1    | p0    |
 * | _mm256_setzero_si256     | vpxor y, y, y      | AVX         | 1   | 0.33 | p015  |
 * | _mm256_stream_load_si256 | vmovntdqa y, m256  | AVX2        |     |      |       |
 * +--------------------------+--------------------+-------------+-----+------+-------+
 * | _mm512_min_epu8          | vpminub z, z, z    | AVX512BW    | 1   | 1    | p0    |
 * | _mm512_cmpeq_epi8_mask   | vpcmpb k, z, z, i8 | AVX512BW    | 3   | 1    | p5    |
 * | _mm512_testn_epi8_mask   | vptestnmb k, z, z  | AVX512BW    | 3   | 1    | p5    |
 * | _ktestz_mask64_u8        | ktestq k, k        | AVX512BW    | 2   | 1    | p0    |
 * | _mm512_setzero_si512     | vpxorq z, z, z     | AVX512F     | 1   | 0.5  | p05   |
 * | _mm512_stream_load_si512 | vmovntdqa z, m512  | AVX512F     |     |      |       |
 * +--------------------------+--------------------+-------------+-----+------+-------+
 */


#if defined(__SSE4_2__)
#define DO_PCMPISTRI
#endif

#if defined(__SSE4_1__) && defined(__SSE4_2__)
#define DO_PCMPISTRI_STREAM
#endif

#if defined(__SSE2__) && defined(__SSE4_1__)
#define DO_PTEST
#endif

#if defined(__SSE2__)
#define DO_PMOVMSKB
#endif

#if defined(__AVX__) && defined(__AVX2__)
#define DO_VPTEST
#endif

#if defined(__AVX__) && defined(__AVX2__)
#define DO_VPMOVMSKB
#endif

#if defined(__AVX512F__) && defined(__AVX512BW__)
#define DO_VPCMPB
#endif

#if defined(__AVX512BW__)
#define DO_VPTESTNMB
#endif

#if defined(__AVX512F__) && defined(__AVX512BW__)
#define DO_VPTESTNMB_STREAM
#endif


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
size_t find_null_char(const char * const s, const size_t len)
{
    for (size_t i = 0; i < len; i ++)
        if (s[i] == '\0')
            return i;
    printf_abort("Null character is not found\n");
}

static
size_t my_strlen_pure(const char *s)
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


/*
 * As for UNROLL:
 * For xmm, it must be less than or equal to 4096 / (128/8) = 256.
 * For ymm, it must be less than or equal to 4096 / (256/8) = 128.
 * For zmm, it must be less than or equal to 4096 / (512/8) = 64.
 */

/*
 * _mm*_cmpeq_epi8: Set 0xff if matched, or else 0x00.
 * _mm*_testz_si*: Set ZF if logical AND of two inputs are all zero.
 * _mm*_movemask_epi8: Return r32 which corresponds to MSBs of each epi8.
 * _mm512_cmpeq_epi8_mask: Set bits in mask64 if two epi8 are the same.
 * _mm512_testn_epi8_mask: Same as above.
 * _ktestz_mask64_u8: Set ZF if logical AND of two inputs are all zero.
 */

/*
 * _mm_cmpistr* (pcmpistri xmm1, xmm2/m128, imm8) sets SF if any byte/word of
 * xmm1 is null, and sets ZF if any byte/word of xmm2/mem128 is null.  SF can be
 * obtained with _mm_cmpistrs and ZF can be obtained with _mm_cmpistrz.
 */


#ifdef DO_PCMPISTRI

static
size_t my_strlen_pcmpistri_unroll_2(const char * const s)
{
#define UNROLL 2

    size_t c = 0;
    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        const int res0 = _mm_cmpistrs(p[0], p[1], _SIDD_UBYTE_OPS);
        const int res1 = _mm_cmpistrz(p[0], p[1], _SIDD_UBYTE_OPS);
        if (res0 || res1)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_pcmpistri_unroll_4(const char * const s)
{
#define UNROLL 4

    size_t c = 0;
    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        const int res0 = _mm_cmpistrs(p[0], p[1], _SIDD_UBYTE_OPS);
        const int res1 = _mm_cmpistrz(p[0], p[1], _SIDD_UBYTE_OPS);
        const int res2 = _mm_cmpistrs(p[2], p[3], _SIDD_UBYTE_OPS);
        const int res3 = _mm_cmpistrz(p[2], p[3], _SIDD_UBYTE_OPS);
        if (res0 || res1 || res2 || res3)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_pcmpistri_unroll_8(const char * const s)
{
#define UNROLL 8

    size_t c = 0;
    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        const int res0 = _mm_cmpistrs(p[0], p[1], _SIDD_UBYTE_OPS);
        const int res1 = _mm_cmpistrz(p[0], p[1], _SIDD_UBYTE_OPS);
        const int res2 = _mm_cmpistrs(p[2], p[3], _SIDD_UBYTE_OPS);
        const int res3 = _mm_cmpistrz(p[2], p[3], _SIDD_UBYTE_OPS);
        const int res4 = _mm_cmpistrs(p[4], p[5], _SIDD_UBYTE_OPS);
        const int res5 = _mm_cmpistrz(p[4], p[5], _SIDD_UBYTE_OPS);
        const int res6 = _mm_cmpistrs(p[6], p[7], _SIDD_UBYTE_OPS);
        const int res7 = _mm_cmpistrz(p[6], p[7], _SIDD_UBYTE_OPS);
        if (res0 || res1 || res2 || res3 || res4 || res5 || res6 || res7)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_pcmpistri_unroll_16(const char * const s)
{
#define UNROLL 16

    size_t c = 0;
    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        const int res0 = _mm_cmpistrs(p[0], p[1], _SIDD_UBYTE_OPS);
        const int res1 = _mm_cmpistrz(p[0], p[1], _SIDD_UBYTE_OPS);
        const int res2 = _mm_cmpistrs(p[2], p[3], _SIDD_UBYTE_OPS);
        const int res3 = _mm_cmpistrz(p[2], p[3], _SIDD_UBYTE_OPS);
        const int res4 = _mm_cmpistrs(p[4], p[5], _SIDD_UBYTE_OPS);
        const int res5 = _mm_cmpistrz(p[4], p[5], _SIDD_UBYTE_OPS);
        const int res6 = _mm_cmpistrs(p[6], p[7], _SIDD_UBYTE_OPS);
        const int res7 = _mm_cmpistrz(p[6], p[7], _SIDD_UBYTE_OPS);
        const int res8 = _mm_cmpistrs(p[8], p[9], _SIDD_UBYTE_OPS);
        const int res9 = _mm_cmpistrz(p[8], p[9], _SIDD_UBYTE_OPS);
        const int resa = _mm_cmpistrs(p[10], p[11], _SIDD_UBYTE_OPS);
        const int resb = _mm_cmpistrz(p[10], p[11], _SIDD_UBYTE_OPS);
        const int resc = _mm_cmpistrs(p[12], p[13], _SIDD_UBYTE_OPS);
        const int resd = _mm_cmpistrz(p[12], p[13], _SIDD_UBYTE_OPS);
        const int rese = _mm_cmpistrs(p[14], p[15], _SIDD_UBYTE_OPS);
        const int resf = _mm_cmpistrz(p[14], p[15], _SIDD_UBYTE_OPS);
        if (res0 || res1 || res2 || res3 || res4 || res5 || res6 || res7
                || res8 || res9 || resa || resb || resc || resd || rese || resf)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

#endif /* DO_PCMPISTRI */


#ifdef DO_PCMPISTRI_STREAM

static
size_t my_strlen_pcmpistri_unroll_2_stream_both(const char * const s)
{
#define UNROLL 2

    size_t c = 0;
    __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        const __m128i v0 = _mm_stream_load_si128(p + 0);
        const __m128i v1 = _mm_stream_load_si128(p + 1);
        const int res0 = _mm_cmpistrs(v0, v1, _SIDD_UBYTE_OPS);
        const int res1 = _mm_cmpistrz(v0, v1, _SIDD_UBYTE_OPS);
        if (res0 || res1)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_pcmpistri_unroll_4_stream_both(const char * const s)
{
#define UNROLL 4

    size_t c = 0;
    __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        const __m128i v0 = _mm_stream_load_si128(p + 0);
        const __m128i v1 = _mm_stream_load_si128(p + 1);
        const __m128i v2 = _mm_stream_load_si128(p + 2);
        const __m128i v3 = _mm_stream_load_si128(p + 3);
        const int res0 = _mm_cmpistrs(v0, v1, _SIDD_UBYTE_OPS);
        const int res1 = _mm_cmpistrz(v0, v1, _SIDD_UBYTE_OPS);
        const int res2 = _mm_cmpistrs(v2, v3, _SIDD_UBYTE_OPS);
        const int res3 = _mm_cmpistrz(v2, v3, _SIDD_UBYTE_OPS);
        if (res0 || res1 || res2 || res3)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_pcmpistri_unroll_8_stream_both(const char * const s)
{
#define UNROLL 8

    size_t c = 0;
    __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        const __m128i v0 = _mm_stream_load_si128(p + 0);
        const __m128i v1 = _mm_stream_load_si128(p + 1);
        const __m128i v2 = _mm_stream_load_si128(p + 2);
        const __m128i v3 = _mm_stream_load_si128(p + 3);
        const __m128i v4 = _mm_stream_load_si128(p + 4);
        const __m128i v5 = _mm_stream_load_si128(p + 5);
        const __m128i v6 = _mm_stream_load_si128(p + 6);
        const __m128i v7 = _mm_stream_load_si128(p + 7);
        const int res0 = _mm_cmpistrs(v0, v1, _SIDD_UBYTE_OPS);
        const int res1 = _mm_cmpistrz(v0, v1, _SIDD_UBYTE_OPS);
        const int res2 = _mm_cmpistrs(v2, v3, _SIDD_UBYTE_OPS);
        const int res3 = _mm_cmpistrz(v2, v3, _SIDD_UBYTE_OPS);
        const int res4 = _mm_cmpistrs(v4, v5, _SIDD_UBYTE_OPS);
        const int res5 = _mm_cmpistrz(v4, v5, _SIDD_UBYTE_OPS);
        const int res6 = _mm_cmpistrs(v6, v7, _SIDD_UBYTE_OPS);
        const int res7 = _mm_cmpistrz(v6, v7, _SIDD_UBYTE_OPS);
        if (res0 || res1 || res2 || res3 || res4 || res5 || res6 || res7)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_pcmpistri_unroll_16_stream_both(const char * const s)
{
#define UNROLL 16

    size_t c = 0;
    __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    for (; ; p += UNROLL, c++) {
        const __m128i v0 = _mm_stream_load_si128(p + 0);
        const __m128i v1 = _mm_stream_load_si128(p + 1);
        const __m128i v2 = _mm_stream_load_si128(p + 2);
        const __m128i v3 = _mm_stream_load_si128(p + 3);
        const __m128i v4 = _mm_stream_load_si128(p + 4);
        const __m128i v5 = _mm_stream_load_si128(p + 5);
        const __m128i v6 = _mm_stream_load_si128(p + 6);
        const __m128i v7 = _mm_stream_load_si128(p + 7);
        const __m128i v8 = _mm_stream_load_si128(p + 8);
        const __m128i v9 = _mm_stream_load_si128(p + 9);
        const __m128i va = _mm_stream_load_si128(p + 10);
        const __m128i vb = _mm_stream_load_si128(p + 11);
        const __m128i vc = _mm_stream_load_si128(p + 12);
        const __m128i vd = _mm_stream_load_si128(p + 13);
        const __m128i ve = _mm_stream_load_si128(p + 14);
        const __m128i vf = _mm_stream_load_si128(p + 15);
        const int res0 = _mm_cmpistrs(v0, v1, _SIDD_UBYTE_OPS);
        const int res1 = _mm_cmpistrz(v0, v1, _SIDD_UBYTE_OPS);
        const int res2 = _mm_cmpistrs(v2, v3, _SIDD_UBYTE_OPS);
        const int res3 = _mm_cmpistrz(v2, v3, _SIDD_UBYTE_OPS);
        const int res4 = _mm_cmpistrs(v4, v5, _SIDD_UBYTE_OPS);
        const int res5 = _mm_cmpistrz(v4, v5, _SIDD_UBYTE_OPS);
        const int res6 = _mm_cmpistrs(v6, v7, _SIDD_UBYTE_OPS);
        const int res7 = _mm_cmpistrz(v6, v7, _SIDD_UBYTE_OPS);
        const int res8 = _mm_cmpistrs(v8, v9, _SIDD_UBYTE_OPS);
        const int res9 = _mm_cmpistrz(v8, v9, _SIDD_UBYTE_OPS);
        const int resa = _mm_cmpistrs(va, vb, _SIDD_UBYTE_OPS);
        const int resb = _mm_cmpistrz(va, vb, _SIDD_UBYTE_OPS);
        const int resc = _mm_cmpistrs(vc, vd, _SIDD_UBYTE_OPS);
        const int resd = _mm_cmpistrz(vc, vd, _SIDD_UBYTE_OPS);
        const int rese = _mm_cmpistrs(ve, vf, _SIDD_UBYTE_OPS);
        const int resf = _mm_cmpistrz(ve, vf, _SIDD_UBYTE_OPS);
        if (res0 || res1 || res2 || res3 || res4 || res5 || res6 || res7
                || res8 || res9 || resa || resb || resc || resd || rese || resf)
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

#endif /* DO_PCMPISTRI_STREAM */


#ifdef DO_PTEST

static
size_t my_strlen_ptest_unroll_2(const char * const s)
{
#define UNROLL 2

    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m128i tmp = _mm_min_epu8(p[0], p[1]);
        const __m128i cmp = _mm_cmpeq_epi8(tmp, _mm_setzero_si128());
        if (!_mm_testz_si128(cmp, cmp))
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_ptest_unroll_4(const char * const s)
{
#define UNROLL 4

    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m128i tmp01 = _mm_min_epu8(p[0], p[1]);
        const __m128i tmp23 = _mm_min_epu8(p[2], p[3]);
        const __m128i tmp0123 = _mm_min_epu8(tmp01, tmp23);
        const __m128i cmp = _mm_cmpeq_epi8(tmp0123, _mm_setzero_si128());
        if (!_mm_testz_si128(cmp, cmp))
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_ptest_unroll_8(const char * const s)
{
#define UNROLL 8

    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m128i tmp01 = _mm_min_epu8(p[0], p[1]);
        const __m128i tmp23 = _mm_min_epu8(p[2], p[3]);
        const __m128i tmp0123 = _mm_min_epu8(tmp01, tmp23);
        const __m128i tmp45 = _mm_min_epu8(p[4], p[5]);
        const __m128i tmp67 = _mm_min_epu8(p[6], p[7]);
        const __m128i tmp4567 = _mm_min_epu8(tmp45, tmp67);
        const __m128i tmp01234567 = _mm_min_epu8(tmp0123, tmp4567);
        const __m128i cmp = _mm_cmpeq_epi8(tmp01234567, _mm_setzero_si128());
        if (!_mm_testz_si128(cmp, cmp))
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

#endif /* DO_PTEST */


#ifdef DO_PMOVMSKB

static
size_t my_strlen_pmovmskb_unroll_2(const char * const s)
{
#define UNROLL 2

    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m128i tmp = _mm_min_epu8(p[0], p[1]);
        const __m128i cmp = _mm_cmpeq_epi8(tmp, _mm_setzero_si128());
        if (_mm_movemask_epi8(cmp))
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_pmovmskb_unroll_4(const char * const s)
{
#define UNROLL 4

    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m128i tmp01 = _mm_min_epu8(p[0], p[1]);
        const __m128i tmp23 = _mm_min_epu8(p[2], p[3]);
        const __m128i tmp0123 = _mm_min_epu8(tmp01, tmp23);
        const __m128i cmp = _mm_cmpeq_epi8(tmp0123, _mm_setzero_si128());
        if (_mm_movemask_epi8(cmp))
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_pmovmskb_unroll_8(const char * const s)
{
#define UNROLL 8

    const __m128i *p = (__m128i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m128i tmp01 = _mm_min_epu8(p[0], p[1]);
        const __m128i tmp23 = _mm_min_epu8(p[2], p[3]);
        const __m128i tmp0123 = _mm_min_epu8(tmp01, tmp23);
        const __m128i tmp45 = _mm_min_epu8(p[4], p[5]);
        const __m128i tmp67 = _mm_min_epu8(p[6], p[7]);
        const __m128i tmp4567 = _mm_min_epu8(tmp45, tmp67);
        const __m128i tmp01234567 = _mm_min_epu8(tmp0123, tmp4567);
        const __m128i cmp = _mm_cmpeq_epi8(tmp01234567, _mm_setzero_si128());
        if (_mm_movemask_epi8(cmp))
            break;
    }

    return c * (128/8) * UNROLL
        + find_null_char((const char*) p, (128/8) * UNROLL);

#undef UNROLL
}

#endif /* DO_PMOVMSKB */


#ifdef DO_VPTEST

static
size_t my_strlen_vptest_unroll_2(const char * const s)
{
#define UNROLL 2

    const __m256i *p = (__m256i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m256i tmp = _mm256_min_epu8(p[0], p[1]);
        const __m256i cmp = _mm256_cmpeq_epi8(tmp, _mm256_setzero_si256());
        if (!_mm256_testz_si256(cmp, cmp))
            break;
    }

    return c * (256/8) * UNROLL
        + find_null_char((const char*) p, (256/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_vptest_unroll_4(const char * const s)
{
#define UNROLL 4

    const __m256i *p = (__m256i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m256i tmp01 = _mm256_min_epu8(p[0], p[1]);
        const __m256i tmp23 = _mm256_min_epu8(p[2], p[3]);
        const __m256i tmp0123 = _mm256_min_epu8(tmp01, tmp23);
        const __m256i cmp = _mm256_cmpeq_epi8(tmp0123, _mm256_setzero_si256());
        if (!_mm256_testz_si256(cmp, cmp))
            break;
    }

    return c * (256/8) * UNROLL
        + find_null_char((const char*) p, (256/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_vptest_unroll_8(const char * const s)
{
#define UNROLL 8

    const __m256i *p = (__m256i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m256i tmp01 = _mm256_min_epu8(p[0], p[1]);
        const __m256i tmp23 = _mm256_min_epu8(p[2], p[3]);
        const __m256i tmp0123 = _mm256_min_epu8(tmp01, tmp23);
        const __m256i tmp45 = _mm256_min_epu8(p[4], p[5]);
        const __m256i tmp67 = _mm256_min_epu8(p[6], p[7]);
        const __m256i tmp4567 = _mm256_min_epu8(tmp45, tmp67);
        const __m256i tmp01234567 = _mm256_min_epu8(tmp0123, tmp4567);
        const __m256i cmp = _mm256_cmpeq_epi8(tmp01234567,
                _mm256_setzero_si256());
        if (!_mm256_testz_si256(cmp, cmp))
            break;
    }

    return c * (256/8) * UNROLL
        + find_null_char((const char*) p, (256/8) * UNROLL);

#undef UNROLL
}

#endif /* DO_VPTEST */


#ifdef DO_VPMOVMSKB

static
size_t my_strlen_vpmovmskb_unroll_2(const char * const s)
{
#define UNROLL 2

    const __m256i *p = (__m256i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m256i tmp = _mm256_min_epu8(p[0], p[1]);
        const __m256i cmp = _mm256_cmpeq_epi8(tmp, _mm256_setzero_si256());
        if (_mm256_movemask_epi8(cmp))
            break;
    }

    return c * (256/8) * UNROLL
        + find_null_char((const char*) p, (256/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_vpmovmskb_unroll_4(const char * const s)
{
#define UNROLL 4

    const __m256i *p = (__m256i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m256i tmp01 = _mm256_min_epu8(p[0], p[1]);
        const __m256i tmp23 = _mm256_min_epu8(p[2], p[3]);
        const __m256i tmp0123 = _mm256_min_epu8(tmp01, tmp23);
        const __m256i cmp = _mm256_cmpeq_epi8(tmp0123, _mm256_setzero_si256());
        if (_mm256_movemask_epi8(cmp))
            break;
    }

    return c * (256/8) * UNROLL
        + find_null_char((const char*) p, (256/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_vpmovmskb_unroll_8(const char * const s)
{
#define UNROLL 8

    const __m256i *p = (__m256i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m256i tmp01 = _mm256_min_epu8(p[0], p[1]);
        const __m256i tmp23 = _mm256_min_epu8(p[2], p[3]);
        const __m256i tmp0123 = _mm256_min_epu8(tmp01, tmp23);
        const __m256i tmp45 = _mm256_min_epu8(p[4], p[5]);
        const __m256i tmp67 = _mm256_min_epu8(p[6], p[7]);
        const __m256i tmp4567 = _mm256_min_epu8(tmp45, tmp67);
        const __m256i tmp01234567 = _mm256_min_epu8(tmp0123, tmp4567);
        const __m256i cmp = _mm256_cmpeq_epi8(tmp01234567,
                _mm256_setzero_si256());
        if (_mm256_movemask_epi8(cmp))
            break;
    }

    return c * (256/8) * UNROLL
        + find_null_char((const char*) p, (256/8) * UNROLL);

#undef UNROLL
}

#endif /* DO_VPMOVMSKB */


#ifdef DO_VPCMPB

static
size_t my_strlen_vpcmpb_unroll_2(const char * const s)
{
#define UNROLL 2

    const __m512i *p = (__m512i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m512i tmp = _mm512_min_epu8(p[0], p[1]);
        const __mmask64 cmp = _mm512_cmpeq_epi8_mask(tmp,
                _mm512_setzero_si512());
        if (!_ktestz_mask64_u8(cmp, cmp))
            break;
    }

    return c * (512/8) * UNROLL
        + find_null_char((const char*) p, (512/8) * UNROLL);

#undef UNROLL
}

#endif /* DO_VPCMPB */


#ifdef DO_VPTESTNMB

static
size_t my_strlen_vptestnmb_unroll_2(const char * const s)
{
#define UNROLL 2

    const __m512i *p = (__m512i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m512i tmp = _mm512_min_epu8(p[0], p[1]);
        const __mmask64 cmp = _mm512_testn_epi8_mask(tmp, tmp);
        if (!_ktestz_mask64_u8(cmp, cmp))
            break;
    }

    return c * (512/8) * UNROLL
        + find_null_char((const char*) p, (512/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_vptestnmb_unroll_4(const char * const s)
{
#define UNROLL 4

    const __m512i *p = (__m512i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m512i tmp01 = _mm512_min_epu8(p[0], p[1]);
        const __m512i tmp23 = _mm512_min_epu8(p[2], p[3]);
        const __m512i tmp0123 = _mm512_min_epu8(tmp01, tmp23);
        const __mmask64 cmp = _mm512_testn_epi8_mask(tmp0123, tmp0123);
        if (!_ktestz_mask64_u8(cmp, cmp))
            break;
    }

    return c * (512/8) * UNROLL
        + find_null_char((const char*) p, (512/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_vptestnmb_unroll_8(const char * const s)
{
#define UNROLL 8

    const __m512i *p = (__m512i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m512i tmp01 = _mm512_min_epu8(p[0], p[1]);
        const __m512i tmp23 = _mm512_min_epu8(p[2], p[3]);
        const __m512i tmp45 = _mm512_min_epu8(p[4], p[5]);
        const __m512i tmp67 = _mm512_min_epu8(p[6], p[7]);
        const __m512i tmp0123 = _mm512_min_epu8(tmp01, tmp23);
        const __m512i tmp4567 = _mm512_min_epu8(tmp45, tmp67);
        const __m512i tmp01234567 = _mm512_min_epu8(tmp0123, tmp4567);
        const __mmask64 cmp = _mm512_testn_epi8_mask(tmp01234567, tmp01234567);
        if (!_ktestz_mask64_u8(cmp, cmp))
            break;
    }

    return c * (512/8) * UNROLL
        + find_null_char((const char*) p, (512/8) * UNROLL);

#undef UNROLL
}

/* It's a hell. */

static
size_t my_strlen_vptestnmb_unroll_16(const char * const s)
{
#define UNROLL 16

    const __m512i *p = (__m512i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m512i tmp01 = _mm512_min_epu8(p[0], p[1]);
        const __m512i tmp23 = _mm512_min_epu8(p[2], p[3]);
        const __m512i tmp45 = _mm512_min_epu8(p[4], p[5]);
        const __m512i tmp67 = _mm512_min_epu8(p[6], p[7]);
        const __m512i tmp89 = _mm512_min_epu8(p[8], p[9]);
        const __m512i tmpab = _mm512_min_epu8(p[10], p[11]);
        const __m512i tmpcd = _mm512_min_epu8(p[12], p[13]);
        const __m512i tmpef = _mm512_min_epu8(p[14], p[15]);
        const __m512i tmp0123 = _mm512_min_epu8(tmp01, tmp23);
        const __m512i tmp4567 = _mm512_min_epu8(tmp45, tmp67);
        const __m512i tmp89ab = _mm512_min_epu8(tmp89, tmpab);
        const __m512i tmpcdef = _mm512_min_epu8(tmpcd, tmpef);
        const __m512i tmp01234567 = _mm512_min_epu8(tmp0123, tmp4567);
        const __m512i tmp89abcdef = _mm512_min_epu8(tmp89ab, tmpcdef);
        const __m512i tmp0123456789abcdef = _mm512_min_epu8(tmp01234567,
                tmp89abcdef);
        const __mmask64 cmp = _mm512_testn_epi8_mask(tmp0123456789abcdef,
                tmp0123456789abcdef);
        if (!_ktestz_mask64_u8(cmp, cmp))
            break;
    }

    return c * (512/8) * UNROLL
        + find_null_char((const char*) p, (512/8) * UNROLL);

#undef UNROLL
}

#endif /* DO_VPTESTNMB */


#ifdef DO_VPTESTNMB_STREAM

static
size_t my_strlen_vptestnmb_unroll_2_stream_both(const char * const s)
{
#define UNROLL 2

    const __m512i *p = (__m512i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m512i v0 = _mm512_stream_load_si512(p + 0);
        const __m512i v1 = _mm512_stream_load_si512(p + 1);
        const __m512i tmp = _mm512_min_epu8(v0, v1);
        const __mmask64 cmp = _mm512_testn_epi8_mask(tmp, tmp);
        if (!_ktestz_mask64_u8(cmp, cmp))
            break;
    }

    return c * (512/8) * UNROLL
        + find_null_char((const char*) p, (512/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_vptestnmb_unroll_4_stream_both(const char * const s)
{
#define UNROLL 4

    const __m512i *p = (__m512i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m512i v0 = _mm512_stream_load_si512(p + 0);
        const __m512i v1 = _mm512_stream_load_si512(p + 1);
        const __m512i v2 = _mm512_stream_load_si512(p + 2);
        const __m512i v3 = _mm512_stream_load_si512(p + 3);
        const __m512i tmp01 = _mm512_min_epu8(v0, v1);
        const __m512i tmp23 = _mm512_min_epu8(v2, v3);
        const __m512i tmp0123 = _mm512_min_epu8(tmp01, tmp23);
        const __mmask64 cmp = _mm512_testn_epi8_mask(tmp0123, tmp0123);
        if (!_ktestz_mask64_u8(cmp, cmp))
            break;
    }

    return c * (512/8) * UNROLL
        + find_null_char((const char*) p, (512/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_vptestnmb_unroll_8_stream_both(const char * const s)
{
#define UNROLL 8

    const __m512i *p = (__m512i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m512i v0 = _mm512_stream_load_si512(p + 0);
        const __m512i v1 = _mm512_stream_load_si512(p + 1);
        const __m512i v2 = _mm512_stream_load_si512(p + 2);
        const __m512i v3 = _mm512_stream_load_si512(p + 3);
        const __m512i v4 = _mm512_stream_load_si512(p + 4);
        const __m512i v5 = _mm512_stream_load_si512(p + 5);
        const __m512i v6 = _mm512_stream_load_si512(p + 6);
        const __m512i v7 = _mm512_stream_load_si512(p + 7);
        const __m512i tmp01 = _mm512_min_epu8(v0, v1);
        const __m512i tmp23 = _mm512_min_epu8(v2, v3);
        const __m512i tmp45 = _mm512_min_epu8(v4, v5);
        const __m512i tmp67 = _mm512_min_epu8(v6, v7);
        const __m512i tmp0123 = _mm512_min_epu8(tmp01, tmp23);
        const __m512i tmp4567 = _mm512_min_epu8(tmp45, tmp67);
        const __m512i tmp01234567 = _mm512_min_epu8(tmp0123, tmp4567);
        const __mmask64 cmp = _mm512_testn_epi8_mask(tmp01234567, tmp01234567);
        if (!_ktestz_mask64_u8(cmp, cmp))
            break;
    }

    return c * (512/8) * UNROLL
        + find_null_char((const char*) p, (512/8) * UNROLL);

#undef UNROLL
}

static
size_t my_strlen_vptestnmb_unroll_16_stream_both(const char * const s)
{
#define UNROLL 16

    const __m512i *p = (__m512i*) __builtin_assume_aligned(s, PAGE_SIZE);
    size_t c = 0;
    for (; ; p += UNROLL, c++) {
        const __m512i v0 = _mm512_stream_load_si512(p + 0);
        const __m512i v1 = _mm512_stream_load_si512(p + 1);
        const __m512i v2 = _mm512_stream_load_si512(p + 2);
        const __m512i v3 = _mm512_stream_load_si512(p + 3);
        const __m512i v4 = _mm512_stream_load_si512(p + 4);
        const __m512i v5 = _mm512_stream_load_si512(p + 5);
        const __m512i v6 = _mm512_stream_load_si512(p + 6);
        const __m512i v7 = _mm512_stream_load_si512(p + 7);
        const __m512i v8 = _mm512_stream_load_si512(p + 8);
        const __m512i v9 = _mm512_stream_load_si512(p + 9);
        const __m512i va = _mm512_stream_load_si512(p + 10);
        const __m512i vb = _mm512_stream_load_si512(p + 11);
        const __m512i vc = _mm512_stream_load_si512(p + 12);
        const __m512i vd = _mm512_stream_load_si512(p + 13);
        const __m512i ve = _mm512_stream_load_si512(p + 14);
        const __m512i vf = _mm512_stream_load_si512(p + 15);
        const __m512i tmp01 = _mm512_min_epu8(v0, v1);
        const __m512i tmp23 = _mm512_min_epu8(v2, v3);
        const __m512i tmp45 = _mm512_min_epu8(v4, v5);
        const __m512i tmp67 = _mm512_min_epu8(v6, v7);
        const __m512i tmp89 = _mm512_min_epu8(v8, v9);
        const __m512i tmpab = _mm512_min_epu8(va, vb);
        const __m512i tmpcd = _mm512_min_epu8(vc, vd);
        const __m512i tmpef = _mm512_min_epu8(ve, vf);
        const __m512i tmp0123 = _mm512_min_epu8(tmp01, tmp23);
        const __m512i tmp4567 = _mm512_min_epu8(tmp45, tmp67);
        const __m512i tmp89ab = _mm512_min_epu8(tmp89, tmpab);
        const __m512i tmpcdef = _mm512_min_epu8(tmpcd, tmpef);
        const __m512i tmp01234567 = _mm512_min_epu8(tmp0123, tmp4567);
        const __m512i tmp89abcdef = _mm512_min_epu8(tmp89ab, tmpcdef);
        const __m512i tmp0123456789abcdef = _mm512_min_epu8(tmp01234567,
                tmp89abcdef);
        const __mmask64 cmp = _mm512_testn_epi8_mask(tmp0123456789abcdef,
                tmp0123456789abcdef);
        if (!_ktestz_mask64_u8(cmp, cmp))
            break;
    }

    return c * (512/8) * UNROLL
        + find_null_char((const char*) p, (512/8) * UNROLL);

#undef UNROLL
}

#endif /* DO_VPTESTNMB_STREAM */


static
void bench_strlen(char * const s, const size_t len_expected,
        size_t (*mystrlen)(const char *), const unsigned repetition)
{
    /* Warmup */
    for (unsigned i = 0; i < 4; i ++) {
        barrier_data(s);
        const size_t len = mystrlen(s);
        if (unlikely(len != len_expected))
            printf_abort("len(%zu) is not expected(%zu)\n", len, len_expected);
    }

    barrier();
    const double start = getsec();
    barrier();
    for (unsigned i = 0; i < repetition; i ++) {
        barrier_data(s);
        const size_t len = mystrlen(s);
        if (unlikely(len != len_expected))
            printf_abort("len(%zu) is not expected(%zu)\n", len, len_expected);
    }
    barrier();
    const double end = getsec();
    barrier();

    const double t = (end - start) / repetition;

    /* Need a cooldown here? */

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

    s = __builtin_assume_aligned(s, PAGE_SIZE);
    (void) memset(s, 0x55, len - 1);
    s[len - 1] = '\0';

#define DO(func, repetition) \
    do { \
        printf("%-41s: ", #func); \
        bench_strlen(s, len-1, func, repetition); \
    } while (0)

    DO(strlen, 512);
    DO(my_strlen_pure, 64);
    DO(my_strlen_rep, 32);

#ifdef DO_PCMPISTRI
    DO(my_strlen_pcmpistri_unroll_2, 512);
    DO(my_strlen_pcmpistri_unroll_4, 512);
    DO(my_strlen_pcmpistri_unroll_8, 512);
    DO(my_strlen_pcmpistri_unroll_16, 512);
#endif /* DO_PCMPISTRI */

#ifdef DO_PCMPISTRI_STREAM
    DO(my_strlen_pcmpistri_unroll_2_stream_both, 512);
    DO(my_strlen_pcmpistri_unroll_4_stream_both, 512);
    DO(my_strlen_pcmpistri_unroll_8_stream_both, 512);
    DO(my_strlen_pcmpistri_unroll_16_stream_both, 512);
#endif /* DO_PCMPISTRI_STREAM */

#ifdef DO_PTEST
    DO(my_strlen_ptest_unroll_2, 512);
    DO(my_strlen_ptest_unroll_4, 512);
    DO(my_strlen_ptest_unroll_8, 512);
#endif /* DO_PTEST */

#ifdef DO_PMOVMSKB
    DO(my_strlen_pmovmskb_unroll_2, 512);
    DO(my_strlen_pmovmskb_unroll_4, 512);
    DO(my_strlen_pmovmskb_unroll_8, 512);
#endif /* DO_PMOVMSKB */

#ifdef DO_VPTEST
    DO(my_strlen_vptest_unroll_2, 512);
    DO(my_strlen_vptest_unroll_4, 512);
    DO(my_strlen_vptest_unroll_8, 512);
#endif /* DO_VPTEST */

#ifdef DO_VPMOVMSKB
    DO(my_strlen_vpmovmskb_unroll_2, 512);
    DO(my_strlen_vpmovmskb_unroll_4, 512);
    DO(my_strlen_vpmovmskb_unroll_8, 512);
#endif /* DO_VPMONMSKB */

#ifdef DO_VPCMPB
    DO(my_strlen_vpcmpb_unroll_2, 512);
#endif /* DO_VPCMPB */

#ifdef DO_VPTESTNMB
    DO(my_strlen_vptestnmb_unroll_2, 512);
    DO(my_strlen_vptestnmb_unroll_4, 512);
    DO(my_strlen_vptestnmb_unroll_8, 512);
    DO(my_strlen_vptestnmb_unroll_16, 512);
#endif /* DO_VPTESTNMB */

#ifdef DO_VPTESTNMB_STREAM
    DO(my_strlen_vptestnmb_unroll_2_stream_both, 512);
    DO(my_strlen_vptestnmb_unroll_4_stream_both, 512);
    DO(my_strlen_vptestnmb_unroll_8_stream_both, 512);
    DO(my_strlen_vptestnmb_unroll_16_stream_both, 512);
#endif /* DO_VPTESTNMB_STREAM */

    free(s);
    return 0;
}
