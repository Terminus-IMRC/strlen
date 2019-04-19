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

- [Intel Core m3-7Y32](https://ark.intel.com/products/97538) with LPDDR3-1866 memory and [Apple Libc 1244.50.9](https://opensource.apple.com/source/Libc/Libc-1244.50.9/x86_64/string/strlen.s.auto.html)
```
String length: 128 MiB
strlen_lib                               : 1.219696e+00 [s], 9.528874e-03 [s], 1.408537e+10 [char/s]
my_strlen_pure                           : 4.544457e-01 [s], 5.680571e-02 [s], 2.362751e+09 [char/s]
my_strlen_rep                            : 1.488302e+00 [s], 9.301889e-02 [s], 1.442908e+09 [char/s]
my_strlen_SSE                            : 1.707641e-01 [s], 1.067275e-02 [s], 1.257573e+10 [char/s]
my_strlen_SSE_unroll_4                   : 1.837998e-01 [s], 1.148749e-02 [s], 1.168382e+10 [char/s]
my_strlen_SSE_unroll_8                   : 1.741155e-01 [s], 1.088222e-02 [s], 1.233368e+10 [char/s]
my_strlen_SSE_unroll_16                  : 1.683031e-01 [s], 1.051895e-02 [s], 1.275962e+10 [char/s]
my_strlen_SSE_unroll_4_separate_load_cmp : 1.840052e-01 [s], 1.150032e-02 [s], 1.167078e+10 [char/s]
my_strlen_SSE_unroll_8_separate_load_cmp : 1.729099e-01 [s], 1.080687e-02 [s], 1.241967e+10 [char/s]
my_strlen_SSE_unroll_16_separate_load_cmp: 1.742009e-01 [s], 1.088756e-02 [s], 1.232762e+10 [char/s]
my_strlen_AVX2_vptest_unroll_2           : 1.259495e-01 [s], 7.871844e-03 [s], 1.705035e+10 [char/s]
my_strlen_AVX2_vptest_unroll_4           : 1.405648e-01 [s], 8.785298e-03 [s], 1.527754e+10 [char/s]
my_strlen_AVX2_vptest_unroll_8           : 1.609970e-01 [s], 1.006231e-02 [s], 1.333865e+10 [char/s]
my_strlen_AVX2_vpmovmskb_unroll_2        : 1.362941e-01 [s], 8.518383e-03 [s], 1.575624e+10 [char/s]
my_strlen_AVX2_vpmovmskb_unroll_4        : 1.620756e-01 [s], 1.012972e-02 [s], 1.324989e+10 [char/s]
my_strlen_AVX2_vpmovmskb_unroll_8        : 1.767699e-01 [s], 1.104812e-02 [s], 1.214847e+10 [char/s]
```
