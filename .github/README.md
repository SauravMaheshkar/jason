[![tests](https://github.com/SauravMaheshkar/jason/actions/workflows/ci.yml/badge.svg)](https://github.com/SauravMaheshkar/jason/actions/workflows/ci.yml)

## Benchmarks

Benchmarking is done via [nativejson-benchmark](https://github.com/miloyip/nativejson-benchmark).
Run `make benchmark` to generate the full report.

### Conformance

#### jason (C++17)

| Test | Score |
| :---: | :---: |
| Parse Validation | 26/34 (76%) |
| Parse Double | 57/66 (86%) |
| Parse String | 4/9 (44%) |
| Roundtrip | - |
| **Overall** | **87/109 (80%)** |

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

## Performance

| Test | File | jason (C++17) | Nlohmann (C++11) | RapidJSON (C++) |
| :---: | :---: | :---: | :---: | :---: |
| Parse | canada.json | 17.332 ms | 17.451 ms | 4.854 ms |
| Parse | citm_catalog.json | 8.478 ms | 6.018 ms | 2.749 ms |
| Parse | twitter.json | 4.227 ms | 4.523 ms | 1.867 ms |
| Stringify | canada.json | N/A | 62.003 ms | 11.175 ms |
| Stringify | citm_catalog.json | N/A | 4.937 ms | 1.389 ms |
| Stringify | twitter.json | N/A | 3.309 ms | 1.596 ms |
| Prettify | canada.json | N/A | 80.563 ms | 13.328 ms |
| Prettify | citm_catalog.json | N/A | 8.578 ms | 1.846 ms |
| Prettify | twitter.json | N/A | 3.861 ms | 1.742 ms |
| Statistics | canada.json | 0.468 ms | 1.009 ms | 0.582 ms |
| Statistics | citm_catalog.json | 0.185 ms | 0.468 ms | 0.178 ms |
| Statistics | twitter.json | 0.083 ms | 0.470 ms | 0.088 ms |
| Code size | jsonstat | 106,376 B | 81,160 B | 56,232 B |

> **Note on jason Performance**
>
> `jason` does not produce stringify/prettify performance numbers because the current implementation is intentionally minimal and only supports parsing.

## Resources

* [Let's Rebuild a JSON Parser in C++ by @SalarAlo08](https://youtu.be/RUTADqhi3tQ?si=odI5u8RBWmxzmsvG)
