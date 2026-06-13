#!/usr/bin/env python3
import argparse
import random
from pathlib import Path


def make_lab_name(index, name_bytes):
    base = f"L{index:03d}_"
    if len(base.encode("utf-8")) > name_bytes:
        raise ValueError("name_bytes is too small")
    return base + ("X" * (name_bytes - len(base.encode("utf-8"))))


def build_capacities(lab_count, student_count, mode):
    if mode == "exact":
        capacities = [student_count // lab_count] * lab_count
        for index in range(student_count % lab_count):
            capacities[index] += 1
        return capacities

    if mode == "varied":
        capacities = [1 + (index % 9) for index in range(lab_count)]
        total = sum(capacities)
        index = 0
        while total < student_count:
            capacities[index % lab_count] += 1
            total += 1
            index += 1
        return capacities

    raise ValueError(f"unknown capacity mode: {mode}")


def choose_weighted_without_replacement(rng, labs, count, hot_count, hot_weight):
    remaining = list(labs)
    selected = []
    while remaining and len(selected) < count:
        weights = [
            hot_weight if int(lab_name[1:4]) < hot_count else 1
            for lab_name in remaining
        ]
        picked = rng.choices(remaining, weights=weights, k=1)[0]
        selected.append(picked)
        remaining.remove(picked)
    return selected


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--labs", type=int, required=True)
    parser.add_argument("--students", type=int, required=True)
    parser.add_argument("--preferences", type=int, required=True)
    parser.add_argument("--name-bytes", type=int, default=16)
    parser.add_argument("--capacity-mode", choices=["exact", "varied"], default="varied")
    parser.add_argument("--distribution", choices=["uniform", "popular"], default="uniform")
    parser.add_argument("--hot-labs", type=int, default=0)
    parser.add_argument("--hot-weight", type=int, default=24)
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--lab-file", required=True)
    parser.add_argument("--preference-file", required=True)
    args = parser.parse_args()

    rng = random.Random(args.seed)
    labs = [make_lab_name(index, args.name_bytes) for index in range(args.labs)]
    capacities = build_capacities(args.labs, args.students, args.capacity_mode)

    lab_path = Path(args.lab_file)
    preference_path = Path(args.preference_file)
    lab_path.parent.mkdir(parents=True, exist_ok=True)
    preference_path.parent.mkdir(parents=True, exist_ok=True)

    with lab_path.open("w", encoding="utf-8", newline="\n") as file:
        file.write(f"{args.labs}\n")
        for lab_name, capacity in zip(labs, capacities):
            file.write(f"{lab_name} {capacity}\n")

    with preference_path.open("w", encoding="utf-8", newline="\n") as file:
        file.write(f"{args.students} {args.preferences}\n")
        hot_count = args.hot_labs if args.hot_labs > 0 else max(1, args.labs // 12)
        for student_index in range(args.students):
            if args.distribution == "popular":
                selected = choose_weighted_without_replacement(
                    rng,
                    labs,
                    args.preferences,
                    hot_count,
                    args.hot_weight,
                )
            else:
                preference_labs = labs[:]
                rng.shuffle(preference_labs)
                selected = preference_labs[: args.preferences]
            file.write(f"{student_index + 1:05d} {' '.join(selected)}\n")


if __name__ == "__main__":
    main()
