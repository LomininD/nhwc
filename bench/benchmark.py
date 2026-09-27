import os
import tempfile
from concurrent.futures import ThreadPoolExecutor, as_completed
from functools import partial
from itertools import permutations

import pandas as pd

import nhwc
from capacity_policy import calc_cache_levels_capacities, CapacitySharingPolicy
from generator import Generator


MAX_WINNERS_TO_DISPLAY = 10


def dict_sorted_by_value(dictionary):
    return dict(sorted(dictionary.items(), key=lambda item: item[1], reverse=True))


class Benchmarker:
    def __init__(self, max_cache_levels, cache_algorithms, capacity_sharing_policies,
                 generator_patterns):
        if max_cache_levels <= 0:
            raise ValueError("max_cache_levels must be > 0")

        if any(policy not in list(CapacitySharingPolicy) for policy in capacity_sharing_policies):
            raise ValueError("unknown cache policy")

        if any(algorithm not in nhwc.SUPPORTED_CACHE_ALGORITHMS for algorithm in cache_algorithms):
            raise ValueError("unknown cache algorithm")

        self.max_cache_levels = max_cache_levels
        self.cache_algorithms = cache_algorithms
        self.generator_patterns = generator_patterns
        self.capacity_sharing_policies = capacity_sharing_policies

    def generate_workloads(self, cache_size, requests_count, key_count, seed):
        generator = Generator(seed, requests_count, key_count)

        return {
            pattern: generator.generate(pattern, cache_size)
            for pattern in self.generator_patterns
        }

    def _run_nhwc(self, capacities, algorithms, data):
        fd, config_path = tempfile.mkstemp(suffix=".txt", prefix="nhwc_cfg_")
        try:
            with os.fdopen(fd, "w") as f:
                f.write(f"{len(algorithms)} {' '.join(algorithms)}\n")

            return nhwc.run_nhwc_cache(capacities, data, config_path)
        finally:
            try:
                os.unlink(config_path)
            except OSError:
                pass

    def _build_tasks(self, workloads, cache_size, run_ideal):
        tasks = []

        for pattern, data in workloads.items():
            for policy in self.capacity_sharing_policies:
                if run_ideal:
                    tasks.append((policy, pattern, "Ideal Cache",
                                  partial(nhwc.run_ideal_cache, cache_size, data)))

                for cache_levels in range(1, self.max_cache_levels + 1):
                    capacities = calc_cache_levels_capacities(cache_size, cache_levels, policy)
                    for algorithms in permutations(self.cache_algorithms, cache_levels):
                        label = " + ".join(algorithms)
                        tasks.append((policy, pattern, label,
                            partial(self._run_nhwc, capacities, algorithms, data)))

        return tasks

    def run(self, cache_size, requests_count, key_count, seed, run_ideal=True, max_workers=None):
        workloads = self.generate_workloads(cache_size, requests_count, key_count, seed)
        tasks = self._build_tasks(workloads, cache_size, run_ideal)

        result = {
            policy: {pattern: {} for pattern in self.generator_patterns}
            for policy in self.capacity_sharing_policies
        }

        if max_workers is None:
            max_workers = os.cpu_count() or 4

        with ThreadPoolExecutor(max_workers=max_workers) as executor:
            future_to_key = {
                executor.submit(fn): (policy, pattern, label)
                for policy, pattern, label, fn in tasks
            }

            for future in as_completed(future_to_key):
                policy, pattern, label = future_to_key[future]
                try:
                    hits = future.result()
                except Exception as e:
                    print(f"[!] {policy}, {pattern}, {label} failed: {e}")
                    continue

                result[policy][pattern][label] = hits / requests_count
                print(f"[+] {policy}, {pattern}, {label}")

        return result

    def analyze(self, data):
        best = {}
        for policy, patterns in data.items():
            best[policy] = {}
            for pattern, metrics in patterns.items():
                ideal = metrics.get("Ideal Cache")
                configs = {k: v for k, v in metrics.items() if k != "Ideal Cache"}

                single = {k: v for k, v in configs.items() if " + " not in k}
                multi = {k: v for k, v in configs.items() if " + " in k}

                def pick(d):
                    if not d:
                        return None

                    max_ratio = max(d.values())
                    winners = sorted([k for k, v in d.items() if v == max_ratio])

                    if len(winners) < MAX_WINNERS_TO_DISPLAY:
                        display = ", ".join(winners)
                    else:
                        display = len(winners)
                    return display, max_ratio, len(winners)

                best_single = pick(single)
                best_multi = pick(multi)

                candidates = [c for c in (best_single, best_multi) if c is not None]
                best_configuration = max(candidates, key=lambda c: c[1]) if candidates else None

                best[policy][pattern] = {
                    "Best Single": best_single,
                    "Best Multi": best_multi,
                    "Best Configuration": best_configuration,
                    "Ideal": ideal,
                }
        return best

    def save_report(self, data, filename):
        chunks = []
        for policy, patterns in data.items():
            rows = []
            for pattern, info in patterns.items():
                ideal = info["Ideal"]

                def unpack(entry):
                    if entry is None:
                        return "-", None, None
                    display, ratio, _ = entry
                    diff = (ideal - ratio) * 100 if ideal is not None else None
                    return display, ratio * 100, diff

                single_display, single_ratio, single_diff = unpack(info["Best Single"])
                multi_display, multi_ratio, multi_diff = unpack(info["Best Multi"])
                overall_display, overall_ratio, overall_diff = unpack(info["Best Configuration"])

                rows.append({
                    "Pattern": pattern,
                    "Best Configuration": overall_display,
                    "Best Hit (%)": overall_ratio,
                    "Best Diff Ideal (%)": overall_diff,
                    "Single Algorithm": single_display,
                    "Single Hit (%)": single_ratio,
                    "Single Diff Ideal (%)": single_diff,
                    "Multi Algorithm": multi_display,
                    "Multi Hit (%)": multi_ratio,
                    "Multi Diff Ideal (%)": multi_diff,
                })

            df = pd.DataFrame(rows)
            table = df.to_markdown(index=False, floatfmt=".2f")
            chunks.append(f"# Policy: {policy}\n\n{table}\n\n")

        with open(filename, "w") as f:
            f.write("".join(chunks))

        with open(filename, "w") as f:
            f.write("".join(chunks))
