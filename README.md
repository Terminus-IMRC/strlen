# strlen


## Results


- [Intel Core i7-7820X](https://ark.intel.com/products/123767) with 4-ch DDR4-2133 memory and [glibc 2.29](https://sourceware.org/git/?p=glibc.git;a=blob;f=sysdeps/x86_64/multiarch/strlen-avx2.S;h=3e7f14a84603ad5284c443fb0ae157ac99afb1e4;hb=56c86f5dd516284558e106d04b92875d5b623b7a)

`my_strlen_vpcmpb_unroll_2` / `strlen` = 1.03

```
String length: 128 MiB
strlen                                   : 4.474206e+00 [s], 8.738684e-03 [s], 1.535903e+10 [char/s]
my_strlen_pure                           : 2.888953e+00 [s], 4.513989e-02 [s], 2.973373e+09 [char/s]
my_strlen_rep                            : 9.999871e+00 [s], 3.124960e-01 [s], 4.295022e+08 [char/s]
my_strlen_pcmpistri_unroll_2             : 5.027999e+00 [s], 9.820311e-03 [s], 1.366736e+10 [char/s]
my_strlen_pcmpistri_unroll_4             : 5.112742e+00 [s], 9.985824e-03 [s], 1.344083e+10 [char/s]
my_strlen_pcmpistri_unroll_8             : 4.946037e+00 [s], 9.660228e-03 [s], 1.389385e+10 [char/s]
my_strlen_pcmpistri_unroll_16            : 5.096747e+00 [s], 9.954585e-03 [s], 1.348301e+10 [char/s]
my_strlen_pcmpistri_unroll_2_stream_both : 5.102812e+00 [s], 9.966429e-03 [s], 1.346698e+10 [char/s]
my_strlen_pcmpistri_unroll_4_stream_both : 5.224364e+00 [s], 1.020384e-02 [s], 1.315365e+10 [char/s]
my_strlen_pcmpistri_unroll_8_stream_both : 4.997402e+00 [s], 9.760551e-03 [s], 1.375104e+10 [char/s]
my_strlen_pcmpistri_unroll_16_stream_both: 5.023257e+00 [s], 9.811049e-03 [s], 1.368026e+10 [char/s]
my_strlen_ptest_unroll_2                 : 4.914304e+00 [s], 9.598249e-03 [s], 1.398356e+10 [char/s]
my_strlen_ptest_unroll_4                 : 4.693949e+00 [s], 9.167870e-03 [s], 1.464001e+10 [char/s]
my_strlen_ptest_unroll_8                 : 4.610522e+00 [s], 9.004926e-03 [s], 1.490492e+10 [char/s]
my_strlen_pmovmskb_unroll_2              : 4.792710e+00 [s], 9.360762e-03 [s], 1.433833e+10 [char/s]
my_strlen_pmovmskb_unroll_4              : 4.623361e+00 [s], 9.030001e-03 [s], 1.486353e+10 [char/s]
my_strlen_pmovmskb_unroll_8              : 4.559261e+00 [s], 8.904807e-03 [s], 1.507250e+10 [char/s]
my_strlen_vptest_unroll_2                : 4.513557e+00 [s], 8.815540e-03 [s], 1.522513e+10 [char/s]
my_strlen_vptest_unroll_4                : 4.501541e+00 [s], 8.792073e-03 [s], 1.526577e+10 [char/s]
my_strlen_vptest_unroll_8                : 4.438565e+00 [s], 8.669071e-03 [s], 1.548236e+10 [char/s]
my_strlen_vpmovmskb_unroll_2             : 4.489442e+00 [s], 8.768441e-03 [s], 1.530691e+10 [char/s]
my_strlen_vpmovmskb_unroll_4             : 4.423987e+00 [s], 8.640599e-03 [s], 1.553338e+10 [char/s]
my_strlen_vpmovmskb_unroll_8             : 4.444382e+00 [s], 8.680434e-03 [s], 1.546210e+10 [char/s]
my_strlen_vpcmpb_unroll_2                : 4.344696e+00 [s], 8.485735e-03 [s], 1.581686e+10 [char/s]
my_strlen_vptestnmb_unroll_2             : 4.350837e+00 [s], 8.497728e-03 [s], 1.579454e+10 [char/s]
my_strlen_vptestnmb_unroll_4             : 4.501132e+00 [s], 8.791273e-03 [s], 1.526716e+10 [char/s]
my_strlen_vptestnmb_unroll_8             : 4.415832e+00 [s], 8.624672e-03 [s], 1.556207e+10 [char/s]
my_strlen_vptestnmb_unroll_16            : 4.556426e+00 [s], 8.899270e-03 [s], 1.508188e+10 [char/s]
my_strlen_vptestnmb_unroll_2_stream_both : 4.626661e+00 [s], 9.036447e-03 [s], 1.485293e+10 [char/s]
my_strlen_vptestnmb_unroll_4_stream_both : 4.458587e+00 [s], 8.708178e-03 [s], 1.541284e+10 [char/s]
my_strlen_vptestnmb_unroll_8_stream_both : 4.442508e+00 [s], 8.676773e-03 [s], 1.546862e+10 [char/s]
my_strlen_vptestnmb_unroll_16_stream_both: 4.529558e+00 [s], 8.846794e-03 [s], 1.517134e+10 [char/s]
```


- [Intel Core i5-7500](https://ark.intel.com/products/97123) with 2-ch(?) DDR4-2400 memory and [Apple Libc 1244.50.9](https://opensource.apple.com/source/Libc/Libc-1244.50.9/x86_64/string/strlen.s.auto.html)

`my_strlen_vptest_unroll_2` / `strlen` = 1.27

```
String length: 128 MiB
strlen                                   : 3.654697e+00 [s], 7.138080e-03 [s], 1.880306e+10 [char/s]
my_strlen_pure                           : 2.836927e+00 [s], 4.432699e-02 [s], 3.027901e+09 [char/s]
my_strlen_rep                            : 2.340978e+00 [s], 7.315557e-02 [s], 1.834689e+09 [char/s]
my_strlen_pcmpistri_unroll_2             : 3.885332e+00 [s], 7.588539e-03 [s], 1.768690e+10 [char/s]
my_strlen_pcmpistri_unroll_4             : 4.003785e+00 [s], 7.819892e-03 [s], 1.716363e+10 [char/s]
my_strlen_pcmpistri_unroll_8             : 3.737429e+00 [s], 7.299666e-03 [s], 1.838683e+10 [char/s]
my_strlen_pcmpistri_unroll_16            : 3.671422e+00 [s], 7.170747e-03 [s], 1.871740e+10 [char/s]
my_strlen_pcmpistri_unroll_2_stream_both : 3.922006e+00 [s], 7.660169e-03 [s], 1.752151e+10 [char/s]
my_strlen_pcmpistri_unroll_4_stream_both : 4.023969e+00 [s], 7.859315e-03 [s], 1.707754e+10 [char/s]
my_strlen_pcmpistri_unroll_8_stream_both : 3.872375e+00 [s], 7.563232e-03 [s], 1.774608e+10 [char/s]
my_strlen_pcmpistri_unroll_16_stream_both: 3.637111e+00 [s], 7.103732e-03 [s], 1.889397e+10 [char/s]
my_strlen_ptest_unroll_2                 : 3.429973e+00 [s], 6.699166e-03 [s], 2.003499e+10 [char/s]
my_strlen_ptest_unroll_4                 : 3.187208e+00 [s], 6.225016e-03 [s], 2.156102e+10 [char/s]
my_strlen_ptest_unroll_8                 : 2.975255e+00 [s], 5.811045e-03 [s], 2.309700e+10 [char/s]
my_strlen_pmovmskb_unroll_2              : 3.184009e+00 [s], 6.218768e-03 [s], 2.158269e+10 [char/s]
my_strlen_pmovmskb_unroll_4              : 3.040143e+00 [s], 5.937779e-03 [s], 2.260403e+10 [char/s]
my_strlen_pmovmskb_unroll_8              : 3.001307e+00 [s], 5.861927e-03 [s], 2.289652e+10 [char/s]
my_strlen_vptest_unroll_2                : 2.880370e+00 [s], 5.625723e-03 [s], 2.385786e+10 [char/s]
my_strlen_vptest_unroll_4                : 2.950773e+00 [s], 5.763228e-03 [s], 2.328864e+10 [char/s]
my_strlen_vptest_unroll_8                : 3.042759e+00 [s], 5.942889e-03 [s], 2.258459e+10 [char/s]
my_strlen_vpmovmskb_unroll_2             : 2.929602e+00 [s], 5.721880e-03 [s], 2.345693e+10 [char/s]
my_strlen_vpmovmskb_unroll_4             : 3.031580e+00 [s], 5.921055e-03 [s], 2.266787e+10 [char/s]
my_strlen_vpmovmskb_unroll_8             : 3.051883e+00 [s], 5.960709e-03 [s], 2.251707e+10 [char/s]
```


- [Intel Core i5-7500](https://ark.intel.com/products/97123) with 2-ch(?) DDR4-2400 memory and [glibc 2.27](https://sourceware.org/git/?p=glibc.git;a=blob;f=sysdeps/x86_64/multiarch/strlen-avx2.S;h=85d7259746b993bdccb4d0437590494fad045b09;hb=23158b08a0908f381459f273a984c6fd328363cb)

`my_strlen_vptest_unroll_2` / `strlen` = 1.10

```
String length: 128 MiB
strlen                                   : 3.279000e+00 [s], 6.404296e-03 [s], 2.095745e+10 [char/s]
my_strlen_pure                           : 3.052441e+00 [s], 4.769439e-02 [s], 2.814120e+09 [char/s]
my_strlen_rep                            : 2.372250e+00 [s], 7.413283e-02 [s], 1.810503e+09 [char/s]
my_strlen_pcmpistri_unroll_2             : 4.214566e+00 [s], 8.231575e-03 [s], 1.630523e+10 [char/s]
my_strlen_pcmpistri_unroll_4             : 4.043182e+00 [s], 7.896839e-03 [s], 1.699639e+10 [char/s]
my_strlen_pcmpistri_unroll_8             : 4.235277e+00 [s], 8.272025e-03 [s], 1.622550e+10 [char/s]
my_strlen_pcmpistri_unroll_16            : 4.202137e+00 [s], 8.207298e-03 [s], 1.635346e+10 [char/s]
my_strlen_pcmpistri_unroll_2_stream_both : 4.122020e+00 [s], 8.050821e-03 [s], 1.667131e+10 [char/s]
my_strlen_pcmpistri_unroll_4_stream_both : 4.130576e+00 [s], 8.067530e-03 [s], 1.663678e+10 [char/s]
my_strlen_pcmpistri_unroll_8_stream_both : 4.034746e+00 [s], 7.880363e-03 [s], 1.703192e+10 [char/s]
my_strlen_pcmpistri_unroll_16_stream_both: 4.089115e+00 [s], 7.986552e-03 [s], 1.680547e+10 [char/s]
my_strlen_ptest_unroll_2                 : 3.531875e+00 [s], 6.898194e-03 [s], 1.945694e+10 [char/s]
my_strlen_ptest_unroll_4                 : 3.187790e+00 [s], 6.226152e-03 [s], 2.155709e+10 [char/s]
my_strlen_ptest_unroll_8                 : 3.123773e+00 [s], 6.101119e-03 [s], 2.199887e+10 [char/s]
my_strlen_pmovmskb_unroll_2              : 3.121270e+00 [s], 6.096230e-03 [s], 2.201651e+10 [char/s]
my_strlen_pmovmskb_unroll_4              : 3.254964e+00 [s], 6.357351e-03 [s], 2.111221e+10 [char/s]
my_strlen_pmovmskb_unroll_8              : 3.225231e+00 [s], 6.299279e-03 [s], 2.130684e+10 [char/s]
my_strlen_vptest_unroll_2                : 2.977835e+00 [s], 5.816084e-03 [s], 2.307699e+10 [char/s]
my_strlen_vptest_unroll_4                : 3.066630e+00 [s], 5.989511e-03 [s], 2.240880e+10 [char/s]
my_strlen_vptest_unroll_8                : 3.214581e+00 [s], 6.278478e-03 [s], 2.137743e+10 [char/s]
my_strlen_vpmovmskb_unroll_2             : 3.176613e+00 [s], 6.204322e-03 [s], 2.163294e+10 [char/s]
my_strlen_vpmovmskb_unroll_4             : 3.173563e+00 [s], 6.198365e-03 [s], 2.165373e+10 [char/s]
my_strlen_vpmovmskb_unroll_8             : 3.283198e+00 [s], 6.412496e-03 [s], 2.093065e+10 [char/s]
```


- [Intel Core m3-7Y32](https://ark.intel.com/products/97538) with 2-ch(?) LPDDR3-1866 memory and [Apple Libc 1244.50.9](https://opensource.apple.com/source/Libc/Libc-1244.50.9/x86_64/string/strlen.s.auto.html)

`my_strlen_vptest_unroll_2` / `strlen` = 1.26

```
String length: 128 MiB
strlen                                   : 4.753270e+00 [s], 9.283730e-03 [s], 1.445731e+10 [char/s]
my_strlen_pure                           : 3.594311e+00 [s], 5.616110e-02 [s], 2.389870e+09 [char/s]
my_strlen_rep                            : 2.952424e+00 [s], 9.226325e-02 [s], 1.454726e+09 [char/s]
my_strlen_pcmpistri_unroll_2             : 4.581796e+00 [s], 8.948821e-03 [s], 1.499837e+10 [char/s]
my_strlen_pcmpistri_unroll_4             : 4.869028e+00 [s], 9.509820e-03 [s], 1.411359e+10 [char/s]
my_strlen_pcmpistri_unroll_8             : 4.546045e+00 [s], 8.878994e-03 [s], 1.511632e+10 [char/s]
my_strlen_pcmpistri_unroll_16            : 4.623290e+00 [s], 9.029864e-03 [s], 1.486376e+10 [char/s]
my_strlen_pcmpistri_unroll_2_stream_both : 4.679394e+00 [s], 9.139442e-03 [s], 1.468555e+10 [char/s]
my_strlen_pcmpistri_unroll_4_stream_both : 4.952560e+00 [s], 9.672969e-03 [s], 1.387555e+10 [char/s]
my_strlen_pcmpistri_unroll_8_stream_both : 4.731918e+00 [s], 9.242028e-03 [s], 1.452254e+10 [char/s]
my_strlen_pcmpistri_unroll_16_stream_both: 4.616510e+00 [s], 9.016621e-03 [s], 1.488559e+10 [char/s]
my_strlen_ptest_unroll_2                 : 4.644560e+00 [s], 9.071407e-03 [s], 1.479569e+10 [char/s]
my_strlen_ptest_unroll_4                 : 4.269653e+00 [s], 8.339167e-03 [s], 1.609486e+10 [char/s]
my_strlen_ptest_unroll_8                 : 4.146683e+00 [s], 8.098991e-03 [s], 1.657215e+10 [char/s]
my_strlen_pmovmskb_unroll_2              : 4.222578e+00 [s], 8.247222e-03 [s], 1.627429e+10 [char/s]
my_strlen_pmovmskb_unroll_4              : 4.239722e+00 [s], 8.280707e-03 [s], 1.620849e+10 [char/s]
my_strlen_pmovmskb_unroll_8              : 4.237304e+00 [s], 8.275984e-03 [s], 1.621774e+10 [char/s]
my_strlen_vptest_unroll_2                : 3.776305e+00 [s], 7.375595e-03 [s], 1.819755e+10 [char/s]
my_strlen_vptest_unroll_4                : 4.020965e+00 [s], 7.853448e-03 [s], 1.709029e+10 [char/s]
my_strlen_vptest_unroll_8                : 4.922701e+00 [s], 9.614650e-03 [s], 1.395971e+10 [char/s]
my_strlen_vpmovmskb_unroll_2             : 3.880817e+00 [s], 7.579721e-03 [s], 1.770748e+10 [char/s]
my_strlen_vpmovmskb_unroll_4             : 4.334728e+00 [s], 8.466266e-03 [s], 1.585324e+10 [char/s]
my_strlen_vpmovmskb_unroll_8             : 4.950150e+00 [s], 9.668262e-03 [s], 1.388230e+10 [char/s]
```
