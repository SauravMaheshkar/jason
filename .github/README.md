[![tests](https://github.com/SauravMaheshkar/jason/actions/workflows/ci.yml/badge.svg)](https://github.com/SauravMaheshkar/jason/actions/workflows/ci.yml)

### Resources

* [Let's Rebuild a JSON Parser in C++ by @SalarAlo08](https://youtu.be/RUTADqhi3tQ?si=odI5u8RBWmxzmsvG)

## Benchmarks

Benchmarking is done via [nativejson-benchmark](https://github.com/miloyip/nativejson-benchmark).
Run `make benchmark` to generate the full report.

### Conformance

#### jason (C++17)

| Test | Score |
| :---: | :---: |
| Parse Validation | 23/34 (68%) |
| Parse Double | 26/66 (39%) |
| Parse String | 2/9 (22%) |
| Roundtrip | - |
| **Overall** | **51/109 (47%)** |

#### Nlohmann (C++11)

| Test | Score |
| :---: | :---: |
| Parse Validation | 34/34 (100%) |
| Parse Double | 66/66 (100%) |
| Parse String | 9/9 (100%) |
| Roundtrip | 23/27 (85%) |
| **Overall** | **132/136 (97%)** |

#### RapidJSON (C++)

| Test | Score |
| :---: | :---: |
| Parse Validation | 34/34 (100%) |
| Parse Double | 48/66 (73%) |
| Parse String | 9/9 (100%) |
| Roundtrip | 27/27 (100%) |
| **Overall** | **118/136 (87%)** |

### Performance

| Test | File | jason (C++17) | Nlohmann (C++11) | RapidJSON (C++) |
| :---: | :---: | :---: | :---: | :---: |
| Parse | canada.json | N/A | 18.306 ms | 4.178 ms |
| Parse | citm_catalog.json | N/A | 5.982 ms | 2.660 ms |
| Parse | twitter.json | N/A | 4.502 ms | 1.863 ms |
| Stringify | canada.json | N/A | 61.909 ms | 11.182 ms |
| Stringify | citm_catalog.json | N/A | 4.796 ms | 1.385 ms |
| Stringify | twitter.json | N/A | 3.212 ms | 1.592 ms |
| Prettify | canada.json | N/A | 80.476 ms | 12.951 ms |
| Prettify | citm_catalog.json | N/A | 8.432 ms | 1.838 ms |
| Prettify | twitter.json | N/A | 3.797 ms | 1.756 ms |
| Statistics | canada.json | N/A | 0.891 ms | 0.581 ms |
| Statistics | citm_catalog.json | N/A | 0.426 ms | 0.178 ms |
| Statistics | twitter.json | N/A | 0.420 ms | 0.088 ms |
| Code size | jsonstat | 107,464 B | 81,160 B | 56,232 B |

> **Note on jason Performance**
>
> `jason` does not produce parse/stringify/prettify performance numbers because it fails to parse the standard benchmark test files (`canada.json`, `citm_catalog.json`, `twitter.json`). The current implementation is intentionally minimal and lacks support for negative numbers, scientific notation, Unicode escapes, and many backslash escape sequences. Therefore the benchmark framework marks these capabilities as "Not support".
