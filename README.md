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

| Option                       | Description                                                                |         Default |
| ---------------------------- | -------------------------------------------------------------------------- | --------------: |
| `-s`, `--cache-size`         | Total cache size                                                           |         `13020` |
| `-l`, `--levels`             | Number of maximum cache levels                                             |             `5` |
| `-r`, `--requests`           | Number of requests to generate                                             |        `131072` |
| `-k`, `--keys`               | Number of unique keys                                                      |         `65536` |
| `-g`, `--seed`               | Random seed for reproducible generation                                    |            `42` |
| `-p`, `--pattern`            | Workload generation pattern; can be `all` or one of the available patterns |           `all` |
| `-sp`, `--sharing-policy`    | The capacity sharing policy used for multi-level cache                     |           `all` |
| `-o`, `--output`             | Output file path                                                           |     `report.md` |

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
    --requests=1048576 \
    --seed=42
```

### Policy: equal

| Pattern                | Best Configuration      |   Best Hit (%) |   Best Diff Ideal (%) | Single Algorithm        |   Single Hit (%) |   Single Diff Ideal (%) | Multi Algorithm                                                                              |   Multi Hit (%) |   Multi Diff Ideal (%) |
|:-----------------------|:------------------------|---------------:|----------------------:|:------------------------|-----------------:|------------------------:|:---------------------------------------------------------------------------------------------|----------------:|-----------------------:|
| scan                   | LIRS                    |           8.61 |                  0.09 | LIRS                    |             8.61 |                    0.09 | 2Q + LIRS, ARC + LIRS, LFU + LIRS, LIRS + 2Q, LIRS + ARC, LIRS + LFU, LIRS + LRU, LRU + LIRS |            4.30 |                   4.39 |
| cyclic_working_set     | 2Q, ARC, LFU, LIRS, LRU |          98.76 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            98.76 |                    0.00 | LIRS + 2Q                                                                                    |           97.31 |                   1.45 |
| just_over_cache        | 2Q                      |          97.80 |                  0.95 | 2Q                      |            97.80 |                    0.95 | LIRS + 2Q                                                                                    |           97.30 |                   1.45 |
| hot_cold               | LIRS                    |          80.44 |                  4.45 | LIRS                    |            80.44 |                    4.45 | LFU + LRU                                                                                    |           80.28 |                   4.61 |
| zipf                   | ARC                     |          92.87 |                  1.68 | ARC                     |            92.87 |                    1.68 | LFU + LIRS                                                                                   |           92.60 |                   1.95 |
| strong_zipf            | 2Q, ARC, LFU, LIRS, LRU |          99.87 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            99.87 |                    0.00 | 320                                                                                          |           99.87 |                   0.00 |
| random                 | LIRS                    |           9.90 |                 29.92 | LIRS                    |             9.90 |                   29.92 | LFU + LIRS                                                                                   |            9.65 |                  30.17 |
| burst                  | LRU                     |          62.84 |                  9.39 | LRU                     |            62.84 |                    9.39 | LIRS + ARC                                                                                   |           58.97 |                  13.26 |
| switching_working_sets | LRU                     |          65.94 |                  7.98 | LRU                     |            65.94 |                    7.98 | LFU + LIRS + ARC                                                                             |           59.93 |                  13.99 |
| returning_working_sets | LRU                     |          51.59 |                 16.22 | LRU                     |            51.59 |                   16.22 | LIRS + ARC                                                                                   |           48.56 |                  19.26 |
| alternating_regions    | LFU + 2Q                |          54.22 |                 25.93 | LIRS                    |            52.15 |                   28.00 | LFU + 2Q                                                                                     |           54.22 |                  25.93 |
| drifting_popularity    | ARC                     |          82.89 |                  4.26 | ARC                     |            82.89 |                    4.26 | LIRS + LRU                                                                                   |           66.34 |                  20.80 |
| flash_crowd            | LRU                     |          26.72 |                 23.33 | LRU                     |            26.72 |                   23.33 | LIRS + LRU                                                                                   |           21.03 |                  29.02 |
| recent_history_reuse   | LRU                     |          81.92 |                  4.31 | LRU                     |            81.92 |                    4.31 | LIRS + LRU                                                                                   |           81.18 |                   5.05 |
| delayed_history_reuse  | LIRS                    |          39.47 |                 18.61 | LIRS                    |            39.47 |                   18.61 | ARC + LIRS                                                                                   |           30.51 |                  27.57 |
| periodic_hot_set       | LFU + ARC               |          25.09 |                 21.56 | LFU                     |            25.03 |                   21.62 | LFU + ARC                                                                                    |           25.09 |                  21.56 |
| three_frequency_tiers  | LFU + ARC               |          49.28 |                 15.62 | LFU                     |            49.12 |                   15.78 | LFU + ARC                                                                                    |           49.28 |                  15.62 |
| local_walk             | 2Q, ARC, LFU, LIRS, LRU |          99.78 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            99.78 |                    0.00 | 320                                                                                          |           99.78 |                   0.00 |
| scan_with_reuse        | LIRS                    |          23.89 |                  3.10 | LIRS                    |            23.89 |                    3.10 | LFU + LIRS                                                                                   |           23.71 |                   3.29 |
| two_phase              | 2Q, LRU                 |          97.52 |                  0.00 | 2Q, LRU                 |            97.52 |                    0.00 | LFU + LRU                                                                                    |           73.03 |                  24.49 |
| hot_noise              | LFU                     |          66.61 |                  7.53 | LFU                     |            66.61 |                    7.53 | LFU + ARC                                                                                    |           65.92 |                   8.22 |
| repeated_scan          | 2Q, ARC, LFU, LIRS, LRU |          98.76 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            98.76 |                    0.00 | LIRS + 2Q                                                                                    |           97.31 |                   1.45 |
| mixed_scan_random      | LFU                     |           8.31 |                 20.80 | LFU                     |             8.31 |                   20.80 | LFU + 2Q                                                                                     |            7.23 |                  21.88 |
| working_set_growth     | LRU                     |          81.24 |                  9.70 | LRU                     |            81.24 |                    9.70 | LIRS + ARC                                                                                   |           70.97 |                  19.96 |
| working_set_shrink     | LRU                     |          81.18 |                  9.74 | LRU                     |            81.18 |                    9.74 | LIRS + ARC                                                                                   |           70.90 |                  20.02 |
| rotating_hot_sets      | 2Q, LRU                 |          59.49 |                  2.55 | 2Q, LRU                 |            59.49 |                    2.55 | LIRS + ARC                                                                                   |           58.26 |                   3.77 |
| random_bursts          | 2Q, ARC, LFU, LIRS, LRU |          99.99 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            99.99 |                    0.00 | 320                                                                                          |           99.99 |                   0.00 |
| clustered_random       | LRU                     |          30.67 |                 20.80 | LRU                     |            30.67 |                   20.80 | LIRS + ARC                                                                                   |           27.73 |                  23.74 |
| looping_working_set    | 2Q, ARC, LFU, LIRS, LRU |          98.76 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            98.76 |                    0.00 | LFU + LIRS                                                                                   |           89.69 |                   9.07 |

### Policy: geometric

| Pattern                | Best Configuration      |   Best Hit (%) |   Best Diff Ideal (%) | Single Algorithm        |   Single Hit (%) |   Single Diff Ideal (%) | Multi Algorithm                               |   Multi Hit (%) |   Multi Diff Ideal (%) |
|:-----------------------|:------------------------|---------------:|----------------------:|:------------------------|-----------------:|------------------------:|:----------------------------------------------|----------------:|-----------------------:|
| scan                   | LIRS                    |           8.61 |                  0.09 | LIRS                    |             8.61 |                    0.09 | 2Q + LIRS, ARC + LIRS, LFU + LIRS, LRU + LIRS |            5.74 |                   2.95 |
| cyclic_working_set     | 2Q, ARC, LFU, LIRS, LRU |          98.76 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            98.76 |                    0.00 | LIRS + 2Q                                     |           97.48 |                   1.28 |
| just_over_cache        | 2Q                      |          97.80 |                  0.95 | 2Q                      |            97.80 |                    0.95 | LIRS + 2Q                                     |           97.46 |                   1.29 |
| hot_cold               | LIRS                    |          80.44 |                  4.45 | LIRS                    |            80.44 |                    4.45 | LFU + ARC                                     |           80.34 |                   4.56 |
| zipf                   | ARC                     |          92.87 |                  1.68 | ARC                     |            92.87 |                    1.68 | LFU + LIRS                                    |           92.66 |                   1.89 |
| strong_zipf            | 2Q, ARC, LFU, LIRS, LRU |          99.87 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            99.87 |                    0.00 | 320                                           |           99.87 |                   0.00 |
| random                 | LIRS                    |           9.90 |                 29.92 | LIRS                    |             9.90 |                   29.92 | LFU + LIRS                                    |            9.73 |                  30.09 |
| burst                  | LRU                     |          62.84 |                  9.39 | LRU                     |            62.84 |                    9.39 | LFU + LRU                                     |           56.93 |                  15.29 |
| switching_working_sets | LRU                     |          65.94 |                  7.98 | LRU                     |            65.94 |                    7.98 | LFU + LRU                                     |           62.97 |                  10.95 |
| returning_working_sets | LRU                     |          51.59 |                 16.22 | LRU                     |            51.59 |                   16.22 | LIRS + ARC                                    |           49.76 |                  18.05 |
| alternating_regions    | LFU + 2Q                |          55.30 |                 24.85 | LIRS                    |            52.15 |                   28.00 | LFU + 2Q                                      |           55.30 |                  24.85 |
| drifting_popularity    | ARC                     |          82.89 |                  4.26 | ARC                     |            82.89 |                    4.26 | 2Q + LRU                                      |           66.79 |                  20.35 |
| flash_crowd            | LRU                     |          26.72 |                 23.33 | LRU                     |            26.72 |                   23.33 | LFU + LRU                                     |           21.29 |                  28.76 |
| recent_history_reuse   | LRU                     |          81.92 |                  4.31 | LRU                     |            81.92 |                    4.31 | LIRS + LRU                                    |           79.46 |                   6.77 |
| delayed_history_reuse  | LIRS                    |          39.47 |                 18.61 | LIRS                    |            39.47 |                   18.61 | ARC + LIRS                                    |           33.42 |                  24.66 |
| periodic_hot_set       | LFU                     |          25.03 |                 21.62 | LFU                     |            25.03 |                   21.62 | LFU + ARC                                     |           24.40 |                  22.25 |
| three_frequency_tiers  | LFU                     |          49.12 |                 15.78 | LFU                     |            49.12 |                   15.78 | LFU + ARC                                     |           48.83 |                  16.07 |
| local_walk             | 2Q, ARC, LFU, LIRS, LRU |          99.78 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            99.78 |                    0.00 | 320                                           |           99.78 |                   0.00 |
| scan_with_reuse        | LFU + LIRS              |          24.47 |                  2.52 | LIRS                    |            23.89 |                    3.10 | LFU + LIRS                                    |           24.47 |                   2.52 |
| two_phase              | 2Q, LRU                 |          97.52 |                  0.00 | 2Q, LRU                 |            97.52 |                    0.00 | LFU + LRU                                     |           81.34 |                  16.18 |
| hot_noise              | LFU                     |          66.61 |                  7.53 | LFU                     |            66.61 |                    7.53 | LFU + ARC                                     |           65.35 |                   8.79 |
| repeated_scan          | 2Q, ARC, LFU, LIRS, LRU |          98.76 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            98.76 |                    0.00 | LIRS + 2Q                                     |           97.48 |                   1.28 |
| mixed_scan_random      | LFU                     |           8.31 |                 20.80 | LFU                     |             8.31 |                   20.80 | LFU + 2Q                                      |            7.30 |                  21.81 |
| working_set_growth     | LRU                     |          81.24 |                  9.70 | LRU                     |            81.24 |                    9.70 | 2Q + LRU                                      |           70.07 |                  20.86 |
| working_set_shrink     | LRU                     |          81.18 |                  9.74 | LRU                     |            81.18 |                    9.74 | LFU + LRU                                     |           71.67 |                  19.25 |
| rotating_hot_sets      | 2Q, LRU                 |          59.49 |                  2.55 | 2Q, LRU                 |            59.49 |                    2.55 | 2Q + ARC                                      |           56.96 |                   5.08 |
| random_bursts          | 2Q, ARC, LFU, LIRS, LRU |          99.99 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            99.99 |                    0.00 | 320                                           |           99.99 |                   0.00 |
| clustered_random       | LRU                     |          30.67 |                 20.80 | LRU                     |            30.67 |                   20.80 | LIRS + ARC                                    |           29.05 |                  22.42 |
| looping_working_set    | 2Q, ARC, LFU, LIRS, LRU |          98.76 |                  0.00 | 2Q, ARC, LFU, LIRS, LRU |            98.76 |                    0.00 | LFU + LIRS                                    |           88.39 |                  10.37 |

### Conclusion

In our case, when each level has the same access complexity, there is absolutely no need to use more than 2 cache levels.

For two levels of caching, LFU is the best option for the first level. For the second level, however, it depends on your strategy for dividing the available space between the levels.

More advanced algorithms like ARC, LIRS, and 2Q are the best for some specific scenarios. Still, simple strategies like LRU and LFU outperform them in most tests.

The best choice depends on your data pattern. If you know which patterns are more typical for your purposes, then you can stick with the best configuration on the specific tests. With more complex environments, it's worth using ARC or LIRS since they are made to be adaptive for various data patterns.

## References

- Nimrod Megiddo and Dharmendra S. Modha. "[ARC: A Self-Tuning, Low Overhead Replacement Cache](https://dl.acm.org/doi/10.5555/1090694.1090708)." In *2nd USENIX Conference on File and Storage Technologies (FAST 03)*, San Francisco, CA, 2003.

- Theodore Johnson and Dennis E. Shasha. "[2Q: A Low Overhead High Performance Buffer Management Replacement Algorithm](https://dl.acm.org/doi/10.5555/645920.672996)." In *Proceedings of the 20th International Conference on Very Large Data Bases (VLDB '94)*, Santiago de Chile, Chile, 1994, pp. 439–450.

- Song Jiang and Xiaodong (Frank) Zhang. "[LIRS: An Efficient Low Inter-Reference Recency Set Replacement Policy to Improve Buffer Cache Performance](https://dl.acm.org/doi/10.1145/511399.511340)." In *ACM SIGMETRICS Performance Evaluation Review*, 2002, vol. 30, pp. 31–42.

- Song Jiang and Xiaodong Zhang. "[Making LRU Friendly to Weak Locality Workloads: A Novel Replacement Algorithm to Improve Buffer Cache Performance](https://www.computer.org/csdl/journal/tc/2005/08/t0939/13rRUy3xY7k)." In *IEEE Transactions on Computers*, 2005, vol. 54, no. 8, pp. 939-952.

- Arjun Singh Saud. "[Survey Inter-Reference Recency Based Page Replacement Policies to Cope with Weak Locality Workloads](https://www.nepjol.info/index.php/kjem/article/view/22017)." In *Kathford Journal of Engineering and Management*, 2018, vol. 1, no. 1, pp. 23-26.

- L. A. Belady and F. P. Palermo. "[On-Line Measurement of Paging Behavior by the Multivalued MIN Algorithm](https://ieeexplore.ieee.org/document/5391336)." In *IBM Journal of Research and Development*, 1974, vol. 18, no. 1, pp. 2–19.
