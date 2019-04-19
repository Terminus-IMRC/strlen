# strlen


## Results

- [Intel Core i7-7820X](https://ark.intel.com/products/123767) with DDR4-2133 memory and [glibc 2.29](https://sourceware.org/git/?p=glibc.git;a=blob;f=sysdeps/x86_64/multiarch/strlen-avx2.S;h=3e7f14a84603ad5284c443fb0ae157ac99afb1e4;hb=56c86f5dd516284558e106d04b92875d5b623b7a)
```
String length: 128 MiB
strlen_lib                               : 1.130776e+00 [s], 8.834190e-03 [s], 1.519299e+10 [char/s]
my_strlen_pure                           : 3.112147e-01 [s], 3.890183e-02 [s], 3.450164e+09 [char/s]
my_strlen_rep                            : 5.019957e+00 [s], 3.137473e-01 [s], 4.277893e+08 [char/s]
my_strlen_SSE                            : 1.646279e-01 [s], 1.028924e-02 [s], 1.304447e+10 [char/s]
my_strlen_SSE_unroll_4                   : 1.773449e-01 [s], 1.108406e-02 [s], 1.210908e+10 [char/s]
my_strlen_SSE_unroll_8                   : 1.688924e-01 [s], 1.055577e-02 [s], 1.271510e+10 [char/s]
my_strlen_SSE_unroll_16                  : 1.625445e-01 [s], 1.015903e-02 [s], 1.321167e+10 [char/s]
my_strlen_SSE_unroll_4_separate_load_cmp : 1.776598e-01 [s], 1.110374e-02 [s], 1.208762e+10 [char/s]
my_strlen_SSE_unroll_8_separate_load_cmp : 1.681321e-01 [s], 1.050825e-02 [s], 1.277260e+10 [char/s]
my_strlen_SSE_unroll_16_separate_load_cmp: 1.618483e-01 [s], 1.011552e-02 [s], 1.326850e+10 [char/s]
my_strlen_AVX2_vptest_unroll_2           : 1.435399e-01 [s], 8.971247e-03 [s], 1.496088e+10 [char/s]
my_strlen_AVX2_vptest_unroll_4           : 1.429003e-01 [s], 8.931269e-03 [s], 1.502785e+10 [char/s]
my_strlen_AVX2_vptest_unroll_8           : 1.420971e-01 [s], 8.881069e-03 [s], 1.511279e+10 [char/s]
my_strlen_AVX2_vpmovmskb_unroll_2        : 1.419886e-01 [s], 8.874288e-03 [s], 1.512434e+10 [char/s]
my_strlen_AVX2_vpmovmskb_unroll_4        : 1.405189e-01 [s], 8.782430e-03 [s], 1.528253e+10 [char/s]
my_strlen_AVX2_vpmovmskb_unroll_8        : 1.410792e-01 [s], 8.817447e-03 [s], 1.522184e+10 [char/s]
my_strlen_AVX512_unroll_2                : 1.346073e-01 [s], 8.412954e-03 [s], 1.595370e+10 [char/s]
my_strlen_AVX512_unroll_4                : 1.356002e-01 [s], 8.475010e-03 [s], 1.583688e+10 [char/s]
```
