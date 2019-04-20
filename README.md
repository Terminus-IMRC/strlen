# strlen


## Results

- [Intel Core i7-7820X](https://ark.intel.com/products/123767) with 4-ch DDR4-2133 memory and [glibc 2.29](https://sourceware.org/git/?p=glibc.git;a=blob;f=sysdeps/x86_64/multiarch/strlen-avx2.S;h=3e7f14a84603ad5284c443fb0ae157ac99afb1e4;hb=56c86f5dd516284558e106d04b92875d5b623b7a)
```
String length: 128 MiB
strlen_lib                               : 1.145574e+00 [s], 8.949794e-03 [s], 1.499674e+10 [char/s]
my_strlen_pure                           : 1.245239e+00 [s], 3.891371e-02 [s], 3.449112e+09 [char/s]
my_strlen_rep                            : 2.498686e+00 [s], 3.123358e-01 [s], 4.297225e+08 [char/s]
my_strlen_SSE                            : 1.338464e+00 [s], 1.045675e-02 [s], 1.283551e+10 [char/s]
my_strlen_SSE_unroll_4                   : 1.452506e+00 [s], 1.134770e-02 [s], 1.182774e+10 [char/s]
my_strlen_SSE_unroll_8                   : 1.367692e+00 [s], 1.068509e-02 [s], 1.256122e+10 [char/s]
my_strlen_SSE_unroll_16                  : 1.321457e+00 [s], 1.032388e-02 [s], 1.300071e+10 [char/s]
my_strlen_SSE_unroll_4_separate_load_cmp : 1.461797e+00 [s], 1.142029e-02 [s], 1.175257e+10 [char/s]
my_strlen_SSE_unroll_8_separate_load_cmp : 1.360075e+00 [s], 1.062558e-02 [s], 1.263156e+10 [char/s]
my_strlen_SSE_unroll_16_separate_load_cmp: 1.315851e+00 [s], 1.028008e-02 [s], 1.305609e+10 [char/s]
my_strlen_AVX2_vptest_unroll_2           : 1.165411e+00 [s], 9.104773e-03 [s], 1.474147e+10 [char/s]
my_strlen_AVX2_vptest_unroll_4           : 1.159265e+00 [s], 9.056761e-03 [s], 1.481962e+10 [char/s]
my_strlen_AVX2_vptest_unroll_8           : 1.152748e+00 [s], 9.005847e-03 [s], 1.490340e+10 [char/s]
my_strlen_AVX2_vpmovmskb_unroll_2        : 1.150968e+00 [s], 8.991937e-03 [s], 1.492645e+10 [char/s]
my_strlen_AVX2_vpmovmskb_unroll_4        : 1.137589e+00 [s], 8.887412e-03 [s], 1.510200e+10 [char/s]
my_strlen_AVX2_vpmovmskb_unroll_8        : 1.143150e+00 [s], 8.930856e-03 [s], 1.502854e+10 [char/s]
my_strlen_AVX512_vpcmpeqb_unroll_2       : 1.088736e+00 [s], 8.505747e-03 [s], 1.577965e+10 [char/s]
my_strlen_AVX512_vpcmpeqb_unroll_4       : 1.098182e+00 [s], 8.579549e-03 [s], 1.564391e+10 [char/s]
my_strlen_AVX512_vptestnmb_unroll_2      : 1.089298e+00 [s], 8.510142e-03 [s], 1.577150e+10 [char/s]
my_strlen_AVX512_vptestnmb_unroll_4      : 1.098271e+00 [s], 8.580246e-03 [s], 1.564264e+10 [char/s]
```

- [Intel Core m3-7Y32](https://ark.intel.com/products/97538) with 2-ch(?) LPDDR3-1866 memory and [Apple Libc 1244.50.9](https://opensource.apple.com/source/Libc/Libc-1244.50.9/x86_64/string/strlen.s.auto.html)
```
String length: 128 MiB
strlen_lib                               : 1.038049e+00 [s], 8.109756e-03 [s], 1.655016e+10 [char/s]
my_strlen_pure                           : 1.790262e+00 [s], 5.594569e-02 [s], 2.399072e+09 [char/s]
my_strlen_rep                            : 7.321660e-01 [s], 9.152075e-02 [s], 1.466528e+09 [char/s]
my_strlen_SSE                            : 1.339186e+00 [s], 1.046239e-02 [s], 1.282859e+10 [char/s]
my_strlen_SSE_unroll_4                   : 1.416284e+00 [s], 1.106472e-02 [s], 1.213025e+10 [char/s]
my_strlen_SSE_unroll_8                   : 1.392373e+00 [s], 1.087791e-02 [s], 1.233856e+10 [char/s]
my_strlen_SSE_unroll_16                  : 1.346656e+00 [s], 1.052075e-02 [s], 1.275743e+10 [char/s]
my_strlen_SSE_unroll_4_separate_load_cmp : 1.440230e+00 [s], 1.125180e-02 [s], 1.192856e+10 [char/s]
my_strlen_SSE_unroll_8_separate_load_cmp : 1.356404e+00 [s], 1.059691e-02 [s], 1.266574e+10 [char/s]
my_strlen_SSE_unroll_16_separate_load_cmp: 1.355996e+00 [s], 1.059372e-02 [s], 1.266956e+10 [char/s]
my_strlen_AVX2_vptest_unroll_2           : 8.504778e-01 [s], 6.644358e-03 [s], 2.020026e+10 [char/s]
my_strlen_AVX2_vptest_unroll_4           : 8.911648e-01 [s], 6.962225e-03 [s], 1.927799e+10 [char/s]
my_strlen_AVX2_vptest_unroll_8           : 8.936527e-01 [s], 6.981662e-03 [s], 1.922432e+10 [char/s]
my_strlen_AVX2_vpmovmskb_unroll_2        : 8.679680e-01 [s], 6.781000e-03 [s], 1.979320e+10 [char/s]
my_strlen_AVX2_vpmovmskb_unroll_4        : 8.893412e-01 [s], 6.947978e-03 [s], 1.931752e+10 [char/s]
my_strlen_AVX2_vpmovmskb_unroll_8        : 8.925166e-01 [s], 6.972786e-03 [s], 1.924880e+10 [char/s]
```
