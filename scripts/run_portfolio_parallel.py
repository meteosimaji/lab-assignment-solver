#!/usr/bin/env python3
import argparse
import concurrent.futures
import shutil
import subprocess
import time
from pathlib import Path


SCRIPT_ROOT = Path(__file__).resolve().parents[1]

LIGHT_CANDIDATES = [
    ("rubric", ["--objective", "rubric"]),
    ("satisfaction", ["--objective", "satisfaction"]),
    ("fair", ["--objective", "fair"]),
    ("balanced", ["--objective", "balanced"]),
    ("guarded", ["--objective", "guarded"]),
]

DEEP_CANDIDATES = LIGHT_CANDIDATES + [
    ("fill_convex", ["--objective", "fill-convex"]),
    (
        "weighted_exact_rank_only",
        [
            "--objective",
            "weighted-exact",
            "--weights",
            str(SCRIPT_ROOT / "weights" / "rank_only.txt"),
        ],
    ),
]


def parse_metrics(path):
    metrics = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        parts = line.split(maxsplit=1)
        if len(parts) == 2:
            metrics[parts[0]] = parts[1]
    return metrics


def recommendation_score(metrics, lab_count):
    return (
        (float(metrics["average_rank"]) - 1.0) / lab_count
        + float(metrics["rank_stddev"]) / lab_count
        + (float(metrics["rank_max"]) - 1.0) / lab_count
        + (1.0 - float(metrics["average_fill_rate"]))
        + (1.0 - float(metrics["minimum_fill_rate"]))
    )


def run_candidate(args):
    name, candidate_args, binary, lab_file, preference_file, output_base, extra_args = args
    output_path = Path(f"{output_base}.{name}.txt")
    command = [
        str(binary),
        str(lab_file),
        str(preference_file),
        str(output_path),
        *candidate_args,
        "--reports",
        *extra_args,
    ]
    start = time.perf_counter()
    completed = subprocess.run(
        command,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    wall_seconds = time.perf_counter() - start
    if completed.returncode != 0:
        raise RuntimeError(f"{name} failed: {completed.stderr.strip()}")
    metrics = parse_metrics(Path(str(output_path) + ".metrics.txt"))
    return name, output_path, metrics, wall_seconds


def main():
    parser = argparse.ArgumentParser(
        description="Run exact portfolio candidates in parallel processes."
    )
    parser.add_argument("lab_file")
    parser.add_argument("preference_file")
    parser.add_argument("output_file")
    parser.add_argument("--binary", default=str(SCRIPT_ROOT / "assign_labs"))
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--deep", action="store_true")
    parser.add_argument(
        "--extra-arg",
        action="append",
        default=[],
        help="Extra argument passed to every assign_labs invocation.",
    )
    args = parser.parse_args()

    candidates = DEEP_CANDIDATES if args.deep else LIGHT_CANDIDATES
    output_path = Path(args.output_file).resolve()
    lab_file = Path(args.lab_file).resolve()
    preference_file = Path(args.preference_file).resolve()
    lab_count = int(lab_file.read_text(encoding="utf-8").splitlines()[0])
    binary = Path(args.binary).resolve()

    jobs = max(1, min(args.jobs, len(candidates)))
    work_items = [
        (
            name,
            candidate_args,
            binary,
            lab_file,
            preference_file,
            output_path,
            args.extra_arg,
        )
        for name, candidate_args in candidates
    ]
    with concurrent.futures.ProcessPoolExecutor(max_workers=jobs) as executor:
        results = list(executor.map(run_candidate, work_items))

    rows = []
    for candidate_index, (name, candidate_output, metrics, wall_seconds) in enumerate(results):
        score = recommendation_score(metrics, lab_count)
        rows.append((score, candidate_index, wall_seconds, name, candidate_output, metrics))
    rows.sort(key=lambda row: (row[0], row[1]))
    recommended_name = rows[0][3]
    shutil.copyfile(rows[0][4], output_path)

    report_path = Path(str(output_path) + ".portfolio_parallel.tsv")
    with report_path.open("w", encoding="utf-8", newline="\n") as report:
        report.write(
            "candidate\tavg_rank\trank_stddev\tmax_rank\tavg_fill\tmin_fill\t"
            "outside_count\twall_seconds\trecommendation_score\trecommended\n"
        )
        for score, _candidate_index, wall_seconds, name, _candidate_output, metrics in rows:
            report.write(
                f"{name}\t{metrics['average_rank']}\t{metrics['rank_stddev']}\t"
                f"{metrics['rank_max']}\t{metrics['average_fill_rate']}\t"
                f"{metrics['minimum_fill_rate']}\t"
                f"{metrics['outside_preference_count']}\t{wall_seconds:.6f}\t"
                f"{score:.17g}\t"
                f"{'yes' if name == recommended_name else 'no'}\n"
            )
    print(f"recommended={recommended_name}")
    print(f"portfolio={report_path}")


if __name__ == "__main__":
    main()
