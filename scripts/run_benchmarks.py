#!/usr/bin/env python3
import argparse
import csv
import statistics
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


@dataclass(frozen=True)
class DatasetSpec:
    name: str
    labs: int
    students: int
    preferences: int
    name_bytes: int
    capacity_mode: str
    distribution: str
    seed: int
    hot_labs: int = 0
    hot_weight: int = 24


@dataclass(frozen=True)
class BenchmarkCase:
    name: str
    dataset: str
    args: tuple[str, ...]


DATASETS = {
    "uniform_256x1024_r8": DatasetSpec(
        "uniform_256x1024_r8", 256, 1024, 8, 16, "varied", "uniform", 11
    ),
    "popular_256x1024_r8": DatasetSpec(
        "popular_256x1024_r8", 256, 1024, 8, 16, "varied", "popular", 13
    ),
    "popular_128x512_r8": DatasetSpec(
        "popular_128x512_r8", 128, 512, 8, 16, "varied", "popular", 17
    ),
    "dense_128x512_full": DatasetSpec(
        "dense_128x512_full", 128, 512, 128, 16, "varied", "uniform", 19
    ),
    "long_name_256x1024_full": DatasetSpec(
        "long_name_256x1024_full", 256, 1024, 256, 250, "varied", "uniform", 7
    ),
}


CASES = [
    BenchmarkCase("uniform_256x1024_rubric", "uniform_256x1024_r8", ("--objective", "rubric")),
    BenchmarkCase("popular_256x1024_rubric", "popular_256x1024_r8", ("--objective", "rubric")),
    BenchmarkCase(
        "popular_256x1024_satisfaction",
        "popular_256x1024_r8",
        ("--objective", "satisfaction", "--rank-costs", str(ROOT / "rank_costs" / "student_friendly.txt")),
    ),
    BenchmarkCase("popular_256x1024_fair", "popular_256x1024_r8", ("--objective", "fair")),
    BenchmarkCase("dense_128x512_rubric", "dense_128x512_full", ("--objective", "rubric")),
    BenchmarkCase(
        "dense_128x512_weighted_evaluation",
        "dense_128x512_full",
        ("--objective", "weighted-exact", "--weights", str(ROOT / "weights" / "evaluation_balance.txt")),
    ),
    BenchmarkCase(
        "long_name_256x1024_full_rubric",
        "long_name_256x1024_full",
        ("--objective", "rubric"),
    ),
    BenchmarkCase(
        "popular_256x1024_weighted_rank_only",
        "popular_256x1024_r8",
        ("--objective", "weighted-exact", "--weights", str(ROOT / "weights" / "rank_only.txt")),
    ),
    BenchmarkCase(
        "popular_256x1024_weighted_evaluation",
        "popular_256x1024_r8",
        ("--objective", "weighted-exact", "--weights", str(ROOT / "weights" / "evaluation_balance.txt")),
    ),
    BenchmarkCase(
        "popular_128x512_portfolio_serial",
        "popular_128x512_r8",
        ("--portfolio", "--jobs", "1"),
    ),
    BenchmarkCase(
        "popular_128x512_portfolio_jobs4",
        "popular_128x512_r8",
        ("--portfolio", "--jobs", "4"),
    ),
    BenchmarkCase(
        "popular_128x512_portfolio_jobs4_summary",
        "popular_128x512_r8",
        ("--portfolio", "--jobs", "4", "--portfolio-summary-only"),
    ),
    BenchmarkCase(
        "popular_128x512_deep_serial",
        "popular_128x512_r8",
        ("--portfolio-deep", "--jobs", "1"),
    ),
    BenchmarkCase(
        "popular_128x512_deep_jobs4",
        "popular_128x512_r8",
        ("--portfolio-deep", "--jobs", "4"),
    ),
    BenchmarkCase(
        "popular_128x512_counterfactual",
        "popular_128x512_r8",
        ("--objective", "rubric", "__COUNTERFACTUAL__"),
    ),
]


FIELDNAMES = [
    "case",
    "status",
    "repeat",
    "wall",
    "input_bytes",
    "avg_rank",
    "stddev",
    "max_rank",
    "avg_fill",
    "min_fill",
    "outside",
    "read_cpu",
    "solver_cpu",
    "counterfactual_cpu",
    "report_cpu",
    "total_cpu",
    "mcf",
    "dinic",
    "student_group_builds",
    "q_tested",
    "u_tested",
    "bb_prunes",
    "corner_hits",
    "corner_misses",
    "exact_path_cost_comparisons",
    "biguint_score_comparisons",
]


def run_command(command, cwd=ROOT):
    completed = subprocess.run(
        command,
        cwd=cwd,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if completed.returncode != 0:
        raise RuntimeError(
            f"command failed ({completed.returncode}): {' '.join(map(str, command))}\n"
            f"stdout:\n{completed.stdout}\n"
            f"stderr:\n{completed.stderr}"
        )
    return completed


def strict_build(binary):
    run_command(["make", "clean"])
    run_command(
        [
            "make",
            "CFLAGS=-std=c11 -O2 -Wall -Wextra -Wshadow -Wconversion -pedantic -Werror",
        ]
    )
    if binary != ROOT / "assign_labs":
        run_command(["cp", str(ROOT / "assign_labs"), str(binary)])


def generate_dataset(spec, bench_dir):
    lab_path = bench_dir / "inputs" / f"{spec.name}_labs.txt"
    pref_path = bench_dir / "inputs" / f"{spec.name}_prefs.txt"
    if lab_path.exists() and pref_path.exists():
        return lab_path, pref_path
    command = [
        sys.executable,
        str(ROOT / "scripts" / "generate_stress.py"),
        "--labs",
        str(spec.labs),
        "--students",
        str(spec.students),
        "--preferences",
        str(spec.preferences),
        "--name-bytes",
        str(spec.name_bytes),
        "--capacity-mode",
        spec.capacity_mode,
        "--distribution",
        spec.distribution,
        "--hot-labs",
        str(spec.hot_labs),
        "--hot-weight",
        str(spec.hot_weight),
        "--seed",
        str(spec.seed),
        "--lab-file",
        str(lab_path),
        "--preference-file",
        str(pref_path),
    ]
    run_command(command)
    return lab_path, pref_path


def parse_key_value_file(path, separator=None):
    values = {}
    if not path.exists():
        return values
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line or line == "metric\tvalue" or line == "lab_counts":
            continue
        if separator is None:
            parts = line.split(maxsplit=1)
        else:
            parts = line.split(separator, maxsplit=1)
        if len(parts) == 2:
            values[parts[0]] = parts[1]
    return values


def first_counterfactual_args(preference_path):
    for line in preference_path.read_text(encoding="utf-8").splitlines()[1:]:
        parts = line.split()
        if len(parts) >= 2:
            student_id = parts[0]
            lab_name = parts[1]
            return ("--explain-student", student_id, "--try-lock", f"{student_id}:{lab_name}")
    raise RuntimeError(f"no student row in {preference_path}")


def run_case_once(case, repeat_index, binary, bench_dir):
    spec = DATASETS[case.dataset]
    lab_path, preference_path = generate_dataset(spec, bench_dir)
    output_path = bench_dir / "outputs" / f"{case.name}.{repeat_index}.txt"
    output_path.parent.mkdir(parents=True, exist_ok=True)
    case_args = []
    for item in case.args:
        if item == "__COUNTERFACTUAL__":
            case_args.extend(first_counterfactual_args(preference_path))
        else:
            case_args.append(item)
    command = [
        str(binary),
        str(lab_path),
        str(preference_path),
        str(output_path),
        *case_args,
        "--reports",
        "--profile",
    ]
    start = time.perf_counter()
    completed = subprocess.run(
        command,
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    wall = time.perf_counter() - start
    if completed.returncode != 0:
        return {
            "case": case.name,
            "status": f"failed:{completed.returncode}",
            "repeat": str(repeat_index),
            "wall": f"{wall:.6f}",
            "input_bytes": str(lab_path.stat().st_size + preference_path.stat().st_size),
        }
    metrics = parse_key_value_file(Path(str(output_path) + ".metrics.txt"))
    profile = parse_key_value_file(Path(str(output_path) + ".profile.tsv"), "\t")
    return {
        "case": case.name,
        "status": "ok",
        "repeat": str(repeat_index),
        "wall": f"{wall:.6f}",
        "input_bytes": str(lab_path.stat().st_size + preference_path.stat().st_size),
        "avg_rank": metrics.get("average_rank", ""),
        "stddev": metrics.get("rank_stddev", ""),
        "max_rank": metrics.get("rank_max", ""),
        "avg_fill": metrics.get("average_fill_rate", ""),
        "min_fill": metrics.get("minimum_fill_rate", ""),
        "outside": metrics.get("outside_preference_count", ""),
        "read_cpu": profile.get("read_cpu_seconds", ""),
        "solver_cpu": profile.get("solver_cpu_seconds", ""),
        "counterfactual_cpu": profile.get("counterfactual_cpu_seconds", ""),
        "report_cpu": profile.get("report_cpu_seconds", ""),
        "total_cpu": profile.get("total_cpu_seconds", ""),
        "mcf": profile.get("min_cost_flow_calls", ""),
        "dinic": profile.get("dinic_calls", ""),
        "student_group_builds": profile.get("student_group_builds", ""),
        "q_tested": profile.get("q_candidates_tested", ""),
        "u_tested": profile.get("minimum_candidates_tested", ""),
        "bb_prunes": profile.get("weighted_bound_prunes", ""),
        "corner_hits": profile.get("weighted_corner_cache_hits", ""),
        "corner_misses": profile.get("weighted_corner_cache_misses", ""),
        "exact_path_cost_comparisons": profile.get("exact_path_cost_comparisons", ""),
        "biguint_score_comparisons": profile.get("biguint_score_comparisons", ""),
    }


def choose_median_record(records):
    successful = [record for record in records if record.get("status") == "ok"]
    if not successful:
        return records[0]
    median_wall = statistics.median(float(record["wall"]) for record in successful)
    return min(successful, key=lambda record: abs(float(record["wall"]) - median_wall))


def main():
    parser = argparse.ArgumentParser(
        description="Regenerate the benchmark TSV from representative exact solver runs."
    )
    parser.add_argument("--output", default=str(ROOT / "tmp" / "bench_final" / "benchmark_results.tsv"))
    parser.add_argument("--bench-dir", default=str(ROOT / "tmp" / "bench_final"))
    parser.add_argument("--binary", default=str(ROOT / "assign_labs"))
    parser.add_argument("--repeat", type=int, default=1)
    parser.add_argument("--case", action="append", choices=[case.name for case in CASES])
    parser.add_argument("--no-build", action="store_true")
    args = parser.parse_args()

    output_path = Path(args.output)
    bench_dir = Path(args.bench_dir)
    binary = Path(args.binary).resolve()
    bench_dir.mkdir(parents=True, exist_ok=True)
    if not args.no_build:
        strict_build(binary)

    selected_cases = [case for case in CASES if args.case is None or case.name in args.case]
    rows = []
    for case in selected_cases:
        records = [
            run_case_once(case, repeat_index + 1, binary, bench_dir)
            for repeat_index in range(max(1, args.repeat))
        ]
        rows.append(choose_median_record(records))

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", encoding="utf-8", newline="") as output_file:
        writer = csv.DictWriter(
            output_file,
            fieldnames=FIELDNAMES,
            delimiter="\t",
            lineterminator="\n",
        )
        writer.writeheader()
        for row in rows:
            writer.writerow({field: row.get(field, "") for field in FIELDNAMES})
    print(output_path)


if __name__ == "__main__":
    main()
