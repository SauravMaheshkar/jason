[![tests](https://github.com/SauravMaheshkar/jason/actions/workflows/ci.yml/badge.svg)](https://github.com/SauravMaheshkar/jason/actions/workflows/ci.yml)
Can we make a minimal, readable yet feature rich JSON parser?

## Benchmarks

Benchmarking is done via [nativejson-benchmark](https://github.com/miloyip/nativejson-benchmark).
Run `make benchmark` to generate the full report.

<!-- benchmark-table-start -->
### Conformance

| Test | jason (C++17) | Nlohmann (C++11) | RapidJSON (C++) |
|:---:|:---:|:---:|:---:|
| Parse Validation | 34/34 (100%) | 34/34 (100%) | 34/34 (100%) |
| Parse Double | 57/66 (86%) | 66/66 (100%) | 48/66 (73%) |
| Parse String | 9/9 (100%) | 9/9 (100%) | 9/9 (100%) |
| Roundtrip | - | 23/27 (85%) | 27/27 (100%) |
| **Overall** | **100/109 (92%)** | **132/136 (97%)** | **118/136 (87%)** |

### Performance

| Test | File | jason (C++17) | Nlohmann (C++11) | RapidJSON (C++) |
|:---:|:---:|:---:|:---:|:---:|
| Parse | canada.json | 17.332 ms | 17.451 ms | 4.854 ms |
| Parse | citm_catalog.json | 8.478 ms | 6.018 ms | 2.749 ms |
| Parse | twitter.json | 4.227 ms | 4.523 ms | 1.867 ms |
| Statistics | canada.json | 0.468 ms | 1.009 ms | 0.582 ms |
| Statistics | citm_catalog.json | 0.185 ms | 0.468 ms | 0.178 ms |
| Statistics | twitter.json | 0.083 ms | 0.470 ms | 0.088 ms |
| Code size | jsonstat | 106,376 B | 81,160 B | 56,232 B |

<!-- benchmark-table-end -->

> **Note on jason Performance**
>
> `jason` does not produce stringify/prettify performance numbers because the current implementation is intentionally minimal and only supports parsing.

## Resources

* [Let's Rebuild a JSON Parser in C++ by @SalarAlo08](https://youtu.be/RUTADqhi3tQ?si=odI5u8RBWmxzmsvG)
