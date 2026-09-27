# NHWC

A cache algorithms research homework from [Kostantin Vladimirov's course](https://github.com/tilir/cpp-graduate). Co-authored with [Sergey Kovalenko](https://github.com/serhiosmol) and [Dmitry Lominin](https://github.com/LomininD).

This project implements multi-level cache with various algorithms supported.

## Building

You need to have [CMake](https://cmake.org) 3.14+, compatible build tools (see [CMake Generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) for more information) and a compiler with C++20 support installed.

```shell
cmake -B build
cmake --build build
```

## Running

First of all, you need to create a `config.txt` file with following contents:
```
<number_of_levels> <cache_algorithms>
```
For example:
```
2 ARC LRU
```
```
1 ARC
```

Following algorithms are supported:

- ARC

- 2Q

- LRU

- LFU

- LIRS

The program accepts size of cache levels, then data set size and a sequence of requests. For example:
```shell
echo 2 ARC 2Q > config.txt
echo 2 4 6 1 2 1 2 1 2 | ./build/main
```
In this case we have a two-level cache with ARC cache (size 2) and 2Q (size 4) and a sequence with 6 requests.

## Testing

We use [GoogleTest](https://google.github.io/googletest) framework for our tests. You can run them with:
```shell
ctest --test-dir build --output-on-failure
```

## Benchmarking

This project includes various benchmarks. To run them, install [uv](https://docs.astral.sh/uv) and run the following:
```shell
cd bench
uv run main.py
```

The following options are supported:

| Option                       | Description                                                                |  Default |
| ---------------------------- | -------------------------------------------------------------------------- | -------: |
| `-s`, `--cache-size`         | Total cache size                                                           |   `6510` |
| `-l`, `--levels`             | Number of maximum cache levels                                             |      `5` |
| `-r`, `--requests`           | Number of requests to generate                                             | `131072` |
| `-k`, `--keys`               | Number of unique keys                                                      |  `65536` |
| `-g`, `--seed`               | Random seed for reproducible generation                                    |     `42` |
| `-p`, `--pattern`            | Workload generation pattern; can be `all` or one of the available patterns |    `all` |
| `-sp`, `--sharing-policy`    | The capacity sharing policy used for multi-level cache                     |    `all` |

Available patterns:

| Pattern                  | Description                                                           |
| ------------------------ | --------------------------------------------------------------------- |
| `scan`                   | Sequentially scans the entire key space                               |
| `cyclic_working_set`     | Repeatedly accesses a working set that fits in the cache              |
| `just_over_cache`        | Cycles through a working set slightly larger than the cache           |
| `hot_cold`               | Most requests target a small hot set; the rest target cold keys       |
| `zipf`                   | Generates requests with Zipf-distributed key popularity               |
| `strong_zipf`            | Stronger Zipf distribution with higher popularity concentration       |
| `random`                 | Uniformly random accesses across all keys                             |
| `burst`                  | Repeated bursts over randomly selected working sets                   |
| `switching_working_sets` | Periodically switches between several working sets                    |
| `returning_working_sets` | Cycles through several working sets and returns to previous ones      |
| `alternating_regions`    | Alternates between two distant key regions                            |
| `drifting_popularity`    | The hot region gradually moves through the key space                  |
| `flash_crowd`            | A temporary burst of popularity appears in the middle of the workload |
| `recent_history_reuse`   | Frequently reuses keys from recent request history                    |
| `delayed_history_reuse`  | Reuses keys after a fixed delay                                       |
| `periodic_hot_set`       | A hot set becomes active periodically for short intervals             |
| `three_frequency_tiers`  | Splits keys into hot, warm, and cold popularity tiers                 |
| `local_walk`             | Moves between nearby keys, creating spatial locality                  |
| `scan_with_reuse`        | Sequential scan mixed with repeated accesses to a small set           |
| `two_phase`              | Uses one working set in the first half and another in the second      |
| `hot_noise`              | Mostly accesses a hot set with random noise                           |
| `repeated_scan`          | Repeatedly scans a cache-sized working set                            |
| `mixed_scan_random`      | Combines sequential scanning with random accesses                     |
| `working_set_growth`     | Gradually increases the working-set size                              |
| `working_set_shrink`     | Gradually decreases the working-set size                              |
| `rotating_hot_sets`      | Sequentially rotates popularity between different hot regions         |
| `random_bursts`          | Random keys appear in short repeated bursts                           |
| `clustered_random`       | Random accesses are concentrated within temporary key clusters        |
| `looping_working_set`    | Repeatedly accesses one working set in a shuffled order               |

Supported capacity sharing policies are:

| Policy      | Description                                           |
| ------------| ----------------------------------------------------- |
| `equal`     | Each level has the same size                          |
| `geometric` | Each next level is twice as large as the previous one |

## Results

Here are the results got from running:
```shell
uv run main.py \
    --cache-size=13020 \
    --keys=131072 \
    --requests=65536 \
    --seed=42 \
    --levels=5
```

### Policy: equal

| Pattern                | Best Configuration      |   Best Hit (%) |   Best Diff Ideal (%) | Single Algorithm        |   Single Hit (%) |   Single Diff Ideal (%) | Multi Algorithm                                                                 |   Multi Hit (%) |   Multi Diff Ideal (%) |
|:-----------------------|:------------------------|---------------:|----------------------:|:------------------------|-----------------:|------------------------:|:--------------------------------------------------------------------------------|----------------:|-----------------------:|
| scan                   | 2Q, ARC, LFU, LIRS, LRU |           0.00 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |             0.00 |                    0.00 | 320                                                                             |            0.00 |                   0.00 |
| cyclic_working_set     | 2Q, ARC, LFU, LIRS, LRU |          80.13 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            80.13 |                    0.00 | LIRS + 2Q                                                                       |           71.98 |                   8.15 |
| just_over_cache        | LIRS                    |          79.33 |                  0.79 | LIRS                    |            79.33 |                    0.79 | LIRS + 2Q                                                                       |           71.97 |                   8.16 |
| hot_cold               | ARC, LFU                |          70.88 |                  0.33 | ARC, LFU                |            70.88 |                    0.33 | LFU + LIRS + 2Q                                                                 |           69.58 |                   1.63 |
| zipf                   | 2Q, ARC, LFU, LIRS, LRU |          86.84 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            86.84 |                    0.00 | 2Q + LIRS, ARC + 2Q + LIRS, ARC + LIRS, LFU + 2Q + LIRS, LFU + LIRS, LRU + LIRS |           86.84 |                   0.00 |
| strong_zipf            | 2Q, ARC, LFU, LIRS, LRU |          99.44 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            99.44 |                    0.00 | 320                                                                             |           99.44 |                   0.00 |
| random                 | LFU                     |           9.01 |                 12.42 | LFU                     |             9.01 |                   12.42 | LIRS + LRU                                                                      |            8.36 |                  13.07 |
| burst                  | LRU                     |          64.95 |                  0.60 | LRU                     |            64.95 |                    0.60 | LFU + LRU                                                                       |           60.88 |                   4.67 |
| switching_working_sets | LRU                     |          61.83 |                  0.87 | LRU                     |            61.83 |                    0.87 | LIRS + 2Q                                                                       |           54.53 |                   8.18 |
| returning_working_sets | 2Q                      |          26.46 |                 21.61 | 2Q                      |            26.46 |                   21.61 | LFU + LIRS                                                                      |           23.21 |                  24.86 |
| alternating_regions    | LFU                     |          46.57 |                 17.20 | LFU                     |            46.57 |                   17.20 | LFU + 2Q                                                                        |           45.60 |                  18.17 |
| drifting_popularity    | 2Q + LIRS, LRU + LIRS   |          21.12 |                  0.00 | 2Q, LRU                 |            20.69 |                    0.43 | 2Q + LIRS, LRU + LIRS                                                           |           21.12 |                   0.00 |
| flash_crowd            | LFU                     |          15.16 |                 10.46 | LFU                     |            15.16 |                   10.46 | LIRS + 2Q                                                                       |           14.43 |                  11.19 |
| recent_history_reuse   | 2Q, ARC, LFU, LIRS, LRU |          80.95 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            80.95 |                    0.00 | LIRS + 2Q, LIRS + ARC, LIRS + LFU, LIRS + LRU                                   |           80.89 |                   0.06 |
| delayed_history_reuse  | LIRS                    |          32.29 |                 15.06 | LIRS                    |            32.29 |                   15.06 | ARC + LIRS                                                                      |           21.77 |                  25.57 |
| periodic_hot_set       | ARC                     |          24.58 |                 10.80 | ARC                     |            24.58 |                   10.80 | LFU + LIRS                                                                      |           23.02 |                  12.35 |
| three_frequency_tiers  | ARC                     |          34.63 |                 11.80 | ARC                     |            34.63 |                   11.80 | LFU + LIRS                                                                      |           31.96 |                  14.46 |
| local_walk             | 2Q, ARC, LFU, LIRS, LRU |          99.04 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            99.04 |                    0.00 | 320                                                                             |           99.04 |                   0.00 |
| scan_with_reuse        | LIRS                    |          16.73 |                  0.78 | LIRS                    |            16.73 |                    0.78 | LFU + LIRS                                                                      |           12.88 |                   4.64 |
| two_phase              | 2Q, LRU                 |          63.50 |                  0.00 | 2Q, LRU                 |            63.50 |                    0.00 | LIRS + LRU                                                                      |           54.89 |                   8.61 |
| hot_noise              | ARC                     |          47.54 |                  8.23 | ARC                     |            47.54 |                    8.23 | LFU + LIRS                                                                      |           43.57 |                  12.20 |
| repeated_scan          | 2Q, ARC, LFU, LIRS, LRU |          80.13 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            80.13 |                    0.00 | LIRS + 2Q                                                                       |           71.98 |                   8.15 |
| mixed_scan_random      | LRU                     |           4.56 |                  7.46 | LRU                     |             4.56 |                    7.46 | LRU + LIRS                                                                      |            4.32 |                   7.70 |
| working_set_growth     | LRU                     |          53.36 |                  2.00 | LRU                     |            53.36 |                    2.00 | LIRS + 2Q                                                                       |           49.61 |                   5.75 |
| working_set_shrink     | LRU                     |          53.36 |                  2.06 | LRU                     |            53.36 |                    2.06 | LIRS + 2Q                                                                       |           51.30 |                   4.12 |
| rotating_hot_sets      | 2Q, LRU                 |          52.77 |                  0.00 | 2Q, LRU                 |            52.77 |                    0.00 | LIRS + LRU                                                                      |           47.59 |                   5.18 |
| random_bursts          | 2Q, ARC, LFU, LIRS, LRU |          99.99 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            99.99 |                    0.00 | 320                                                                             |           99.99 |                   0.00 |
| clustered_random       | LRU                     |          33.20 |                  3.65 | LRU                     |            33.20 |                    3.65 | LIRS + LRU                                                                      |           26.77 |                  10.07 |
| looping_working_set    | 2Q, ARC, LFU, LIRS, LRU |          80.13 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            80.13 |                    0.00 | LFU + LIRS                                                                      |           44.76 |                  35.38 |

### Policy: geometric

| Pattern                | Best Configuration      |   Best Hit (%) |   Best Diff Ideal (%) | Single Algorithm        |   Single Hit (%) |   Single Diff Ideal (%) | Multi Algorithm      |   Multi Hit (%) |   Multi Diff Ideal (%) |
|:-----------------------|:------------------------|---------------:|----------------------:|:------------------------|-----------------:|------------------------:|:---------------------|----------------:|-----------------------:|
| scan                   | 2Q, ARC, LFU, LIRS, LRU |           0.00 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |             0.00 |                    0.00 | 320                  |            0.00 |                   0.00 |
| cyclic_working_set     | 2Q, ARC, LFU, LIRS, LRU |          80.13 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            80.13 |                    0.00 | LIRS + 2Q            |           69.73 |                  10.40 |
| just_over_cache        | LIRS                    |          79.33 |                  0.79 | LIRS                    |            79.33 |                    0.79 | LIRS + 2Q            |           69.72 |                  10.41 |
| hot_cold               | ARC, LFU                |          70.88 |                  0.33 | ARC, LFU                |            70.88 |                    0.33 | LFU + ARC            |           70.08 |                   1.13 |
| zipf                   | 2Q, ARC, LFU, LIRS, LRU |          86.84 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            86.84 |                    0.00 | 73                   |           86.84 |                   0.00 |
| strong_zipf            | 2Q, ARC, LFU, LIRS, LRU |          99.44 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            99.44 |                    0.00 | 320                  |           99.44 |                   0.00 |
| random                 | LFU                     |           9.01 |                 12.42 | LFU                     |             9.01 |                   12.42 | LRU + LIRS           |            8.37 |                  13.06 |
| burst                  | LRU                     |          64.95 |                  0.60 | LRU                     |            64.95 |                    0.60 | LFU + LRU            |           63.14 |                   2.41 |
| switching_working_sets | LRU                     |          61.83 |                  0.87 | LRU                     |            61.83 |                    0.87 | LFU + LRU            |           56.85 |                   5.85 |
| returning_working_sets | 2Q                      |          26.46 |                 21.61 | 2Q                      |            26.46 |                   21.61 | LFU + LIRS           |           23.72 |                  24.35 |
| alternating_regions    | LFU                     |          46.57 |                 17.20 | LFU                     |            46.57 |                   17.20 | LFU + ARC            |           45.13 |                  18.64 |
| drifting_popularity    | LFU + 2Q, LFU + LRU     |          20.74 |                  0.38 | 2Q, LRU                 |            20.69 |                    0.43 | LFU + 2Q, LFU + LRU  |           20.74 |                   0.38 |
| flash_crowd            | LFU                     |          15.16 |                 10.46 | LFU                     |            15.16 |                   10.46 | LIRS + 2Q            |           14.47 |                  11.16 |
| recent_history_reuse   | 2Q, ARC, LFU, LIRS, LRU |          80.95 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            80.95 |                    0.00 | LFU + ARC, LFU + LRU |           80.92 |                   0.02 |
| delayed_history_reuse  | LIRS                    |          32.29 |                 15.06 | LIRS                    |            32.29 |                   15.06 | ARC + LIRS           |           25.19 |                  22.15 |
| periodic_hot_set       | ARC                     |          24.58 |                 10.80 | ARC                     |            24.58 |                   10.80 | LFU + ARC            |           23.47 |                  11.90 |
| three_frequency_tiers  | ARC                     |          34.63 |                 11.80 | ARC                     |            34.63 |                   11.80 | LFU + LIRS           |           32.49 |                  13.93 |
| local_walk             | 2Q, ARC, LFU, LIRS, LRU |          99.04 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            99.04 |                    0.00 | 320                  |           99.04 |                   0.00 |
| scan_with_reuse        | LIRS                    |          16.73 |                  0.78 | LIRS                    |            16.73 |                    0.78 | LFU + LIRS           |           14.49 |                   3.03 |
| two_phase              | 2Q, LRU                 |          63.50 |                  0.00 | 2Q, LRU                 |            63.50 |                    0.00 | LFU + LRU            |           57.94 |                   5.56 |
| hot_noise              | ARC                     |          47.54 |                  8.23 | ARC                     |            47.54 |                    8.23 | LFU + LIRS           |           44.18 |                  11.59 |
| repeated_scan          | 2Q, ARC, LFU, LIRS, LRU |          80.13 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            80.13 |                    0.00 | LIRS + 2Q            |           69.73 |                  10.40 |
| mixed_scan_random      | LRU                     |           4.56 |                  7.46 | LRU                     |             4.56 |                    7.46 | LRU + LIRS           |            4.38 |                   7.64 |
| working_set_growth     | LRU                     |          53.36 |                  2.00 | LRU                     |            53.36 |                    2.00 | LFU + LRU            |           51.21 |                   4.15 |
| working_set_shrink     | LRU                     |          53.36 |                  2.06 | LRU                     |            53.36 |                    2.06 | LFU + LRU            |           51.08 |                   4.34 |
| rotating_hot_sets      | 2Q, LRU                 |          52.77 |                  0.00 | 2Q, LRU                 |            52.77 |                    0.00 | LFU + LRU            |           50.23 |                   2.54 |
| random_bursts          | 2Q, ARC, LFU, LIRS, LRU |          99.99 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            99.99 |                    0.00 | 320                  |           99.99 |                   0.00 |
| clustered_random       | LRU                     |          33.20 |                  3.65 | LRU                     |            33.20 |                    3.65 | LIRS + 2Q            |           28.40 |                   8.45 |
| looping_working_set    | 2Q, ARC, LFU, LIRS, LRU |          80.13 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            80.13 |                    0.00 | 2Q + LIRS            |           49.66 |                  30.48 |

### Conclusion

In our case, when each level has the same access complexity, there is almost no need to use more than one level with one exception. Multi-level configurations work best with the `drifting_popularity` pattern. There is absolutely no need for more than 2 levels of cache.

For two levels of caching, what you do next depends on your strategy for dividing the available space between the levels. If your first level is smaller than the others, it's better to use LFU for it. If the levels are of equal size, it's worth using LIRS at the second level.

More advanced algorithms like ARC and LIRS are the best in some cases. Still, even though they are made to be adaptive, simple strategies like LRU and LFU can sometimes beat them.

The best choice depends on your data pattern. If you know which patterns are more typical for your purposes, then you can stick with the best algorithm on the specific tests. With more complex environments, it's worth using ARC as a low-overhead alternative to LRU and LFU or LIRS cache.

## References

- Nimrod Megiddo and Dharmendra S. Modha. "[ARC: A Self-Tuning, Low Overhead Replacement Cache](https://dl.acm.org/doi/10.5555/1090694.1090708)." In *2nd USENIX Conference on File and Storage Technologies (FAST 03)*, San Francisco, CA, 2003.

- Theodore Johnson and Dennis E. Shasha. "[2Q: A Low Overhead High Performance Buffer Management Replacement Algorithm](https://dl.acm.org/doi/10.5555/645920.672996)." In *Proceedings of the 20th International Conference on Very Large Data Bases (VLDB '94)*, Santiago de Chile, Chile, 1994, pp. 439–450.

- Song Jiang and Xiaodong (Frank) Zhang. "[LIRS: An Efficient Low Inter-Reference Recency Set Replacement Policy to Improve Buffer Cache Performance](https://dl.acm.org/doi/10.1145/511399.511340)." In *ACM SIGMETRICS Performance Evaluation Review*, 2002, vol. 30, pp. 31–42.

- Song Jiang and Xiaodong Zhang. "[Making LRU Friendly to Weak Locality Workloads: A Novel Replacement Algorithm to Improve Buffer Cache Performance](https://www.computer.org/csdl/journal/tc/2005/08/t0939/13rRUy3xY7k)." In *IEEE Transactions on Computers*, 2005, vol. 54, no. 8, pp. 939-952.

- Arjun Singh Saud. "[Survey Inter-Reference Recency Based Page Replacement Policies to Cope with Weak Locality Workloads](https://www.nepjol.info/index.php/kjem/article/view/22017)." In *Kathford Journal of Engineering and Management*, 2018, vol. 1, no. 1, pp. 23-26.

- L. A. Belady and F. P. Palermo. "[On-Line Measurement of Paging Behavior by the Multivalued MIN Algorithm](https://ieeexplore.ieee.org/document/5391336)." In *IBM Journal of Research and Development*, 1974, vol. 18, no. 1, pp. 2–19.
