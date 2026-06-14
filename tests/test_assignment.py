import os
import random
import shlex
import subprocess
from fractions import Fraction
from itertools import product
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[1]
BINARY = ROOT / "assign_labs"
DATASETS = ROOT / "datasets"


def build_binary():
    cc = os.environ.get("ASSIGN_LABS_CC", "gcc")
    cflags = os.environ.get(
        "ASSIGN_LABS_CFLAGS",
        "-std=c11 -O2 -Wall -Wextra -Wshadow -Wconversion -pedantic -Werror",
    )
    subprocess.run(
        [cc, *shlex.split(cflags), "-o", str(BINARY), str(ROOT / "assignment.c")],
        check=True,
        cwd=ROOT,
    )


@pytest.fixture(scope="session", autouse=True)
def compiled_binary():
    build_binary()


def write_text(path, text):
    path.write_text(text, encoding="utf-8")


def read_output(path):
    lines = path.read_text(encoding="utf-8").strip().splitlines()
    count = int(lines[0])
    assignments = {}
    for line in lines[1:]:
        student_id, lab_name = line.split()
        assignments[student_id] = lab_name
    assert count == len(assignments)
    return assignments


def metrics_path_for(output_path):
    return Path(str(output_path) + ".metrics.txt")


def target_status_path_for(output_path):
    return Path(str(output_path) + ".target_status.tsv")


def lab_report_path_for(output_path):
    return Path(str(output_path) + ".by_lab.txt")


def student_report_path_for(output_path):
    return Path(str(output_path) + ".by_student.tsv")


def outside_report_path_for(output_path):
    return Path(str(output_path) + ".outside_preferences.tsv")


def read_metrics(output_path):
    metrics = {}
    lab_counts = {}
    in_lab_counts = False
    for line in metrics_path_for(output_path).read_text(encoding="utf-8").splitlines():
        if line == "lab_counts":
            in_lab_counts = True
            continue
        parts = line.split()
        if in_lab_counts:
            lab_counts[parts[0]] = {
                "assigned": int(parts[1]),
                "capacity": int(parts[2]),
                "fill_rate": None if parts[3] == "NA" else float(parts[3]),
            }
        else:
            metrics[parts[0]] = parts[1]
    metrics["lab_counts"] = lab_counts
    return metrics


def read_profile(output_path):
    profile = {}
    path = Path(str(output_path) + ".profile.tsv")
    for line in path.read_text(encoding="utf-8").splitlines()[1:]:
        key, value = line.split("\t")
        profile[key] = value
    return profile


def read_lab_report(output_path):
    lab_rows = {}
    in_lab_rows = False
    for line in lab_report_path_for(output_path).read_text(encoding="utf-8").splitlines():
        if line == "lab_name\tassigned\tcapacity\tfill_rate\tfill_percent\tstudents":
            in_lab_rows = True
            continue
        if in_lab_rows:
            parts = line.split("\t")
            student_tokens = parts[5].split(",") if len(parts) > 5 and parts[5] else []
            lab_rows[parts[0]] = {
                "assigned": int(parts[1]),
                "capacity": int(parts[2]),
                "fill_rate": None if parts[3] == "NA" else float(parts[3]),
                "fill_rate_percent": parts[4],
                "student_ids": [
                    student_token.split("(", 1)[0] for student_token in student_tokens
                ],
                "student_tokens": student_tokens,
            }
    return lab_rows


def read_tsv(path):
    lines = path.read_text(encoding="utf-8").splitlines()
    header = lines[0].split("\t")
    rows = []
    for line in lines[1:]:
        values = line.split("\t")
        rows.append(dict(zip(header, values)))
    return rows


def validate_assignment(lab_text, preference_text, output_path):
    labs = {}
    lab_lines = lab_text.strip().splitlines()
    for line in lab_lines[1:]:
        name, capacity = line.split()
        labs[name] = int(capacity)

    pref_lines = preference_text.strip().splitlines()
    expected_count, max_preferences = map(int, pref_lines[0].split())
    preferences = {}
    for line in pref_lines[1:]:
        parts = line.split()
        preferences[parts[0]] = parts[1 : 1 + max_preferences]

    assignments = read_output(output_path)
    assert len(assignments) == expected_count
    assert set(assignments) == set(preferences)

    lab_counts = {name: 0 for name in labs}
    for lab_name in assignments.values():
        assert lab_name in labs
        lab_counts[lab_name] += 1

    for lab_name, capacity in labs.items():
        assert lab_counts[lab_name] <= capacity
        if capacity > 0:
            assert lab_counts[lab_name] >= 1

    return assignments


def assert_metrics_match_assignment(lab_text, preference_text, output_path, assignments):
    labs, capacities, student_ids, ranks = parse_instance(lab_text, preference_text)
    objective = objective_from_solver_output(labs, capacities, student_ids, ranks, assignments)
    metrics = read_metrics(output_path)

    assert int(metrics["students"]) == len(student_ids)
    assert int(metrics["labs"]) == len(labs)
    assert float(metrics["rank_average"]) == pytest.approx(objective[1] / len(student_ids))
    variance = objective[2] / len(student_ids) - (objective[1] / len(student_ids)) ** 2
    assert float(metrics["rank_stddev"]) == pytest.approx(variance ** 0.5)
    assert int(metrics["rank_max"]) == objective[0]
    assert int(metrics["rank_sum"]) == objective[1]
    assert int(metrics["rank_square_sum"]) == objective[2]
    assigned_costs = [
        dissatisfaction_cost(
            ranks[student_index][
                labs.index(assignments[student_ids[student_index]])
            ],
            len(labs),
        )
        for student_index in range(len(student_ids))
    ]
    assert float(metrics["average_dissatisfaction"]) == pytest.approx(
        sum(assigned_costs) / len(student_ids)
    )
    dissatisfaction_variance = (
        sum(cost * cost for cost in assigned_costs) / len(student_ids)
        - (sum(assigned_costs) / len(student_ids)) ** 2
    )
    assert float(metrics["dissatisfaction_stddev"]) == pytest.approx(
        dissatisfaction_variance ** 0.5
    )
    assert int(metrics["max_dissatisfaction"]) == max(assigned_costs)
    assert int(metrics["dissatisfaction_sum"]) == sum(assigned_costs)
    assert int(metrics["dissatisfaction_square_sum"]) == sum(
        cost * cost for cost in assigned_costs
    )
    assert float(metrics["fill_rate_minimum"]) == pytest.approx(float(objective[3]))
    assert float(metrics["fill_rate_average"]) == pytest.approx(float(objective[4]))

    lab_counts = {lab: 0 for lab in labs}
    for lab_name in assignments.values():
        lab_counts[lab_name] += 1
    assert set(metrics["lab_counts"]) == set(labs)
    for lab_name, capacity in zip(labs, capacities):
        assert metrics["lab_counts"][lab_name]["assigned"] == lab_counts[lab_name]
        assert metrics["lab_counts"][lab_name]["capacity"] == capacity
    assert_lab_report_matches_assignment(lab_text, output_path, assignments)


def assert_lab_report_matches_assignment(lab_text, output_path, assignments):
    labs = {}
    for line in lab_text.strip().splitlines()[1:]:
        lab_name, capacity = line.split()
        labs[lab_name] = int(capacity)

    expected_students_by_lab = {lab_name: [] for lab_name in labs}
    for student_id, lab_name in assignments.items():
        expected_students_by_lab[lab_name].append(student_id)

    report = read_lab_report(output_path)
    assert set(report) == set(labs)
    for lab_name, capacity in labs.items():
        assert report[lab_name]["capacity"] == capacity
        assert report[lab_name]["assigned"] == len(expected_students_by_lab[lab_name])
        assert report[lab_name]["student_ids"] == expected_students_by_lab[lab_name]


def parse_instance(lab_text, preference_text):
    lab_lines = lab_text.strip().splitlines()
    labs = []
    capacities = []
    for line in lab_lines[1:]:
        name, capacity = line.split()
        labs.append(name)
        capacities.append(int(capacity))

    pref_lines = preference_text.strip().splitlines()
    _, max_preferences = map(int, pref_lines[0].split())
    student_ids = []
    ranks = []
    for line in pref_lines[1:]:
        parts = line.split()
        student_ids.append(parts[0])
        rank_by_lab = {lab: len(labs) + 1 for lab in labs}
        for rank, lab_name in enumerate(parts[1 : 1 + max_preferences], start=1):
            rank_by_lab[lab_name] = rank
        ranks.append([rank_by_lab[lab_name] for lab_name in labs])
    return labs, capacities, student_ids, ranks


def exact_objective_for_assignment(capacities, ranks, assignment_indices):
    lab_counts = [0] * len(capacities)
    assigned_ranks = []
    for student_index, lab_index in enumerate(assignment_indices):
        lab_counts[lab_index] += 1
        assigned_ranks.append(ranks[student_index][lab_index])

    for lab_index, capacity in enumerate(capacities):
        if lab_counts[lab_index] > capacity:
            return None
        if capacity > 0 and lab_counts[lab_index] < 1:
            return None

    positive_labs = [idx for idx, capacity in enumerate(capacities) if capacity > 0]
    minimum_fill = min(
        Fraction(lab_counts[lab_index], capacities[lab_index])
        for lab_index in positive_labs
    )
    average_fill = sum(
        Fraction(lab_counts[lab_index], capacities[lab_index])
        for lab_index in positive_labs
    ) / len(positive_labs)
    return (
        max(assigned_ranks),
        sum(assigned_ranks),
        sum(rank * rank for rank in assigned_ranks),
        minimum_fill,
        average_fill,
    )


def dissatisfaction_cost(rank, lab_count, gap=100, linear=30, quadratic=5, outside=10000):
    if rank <= 1:
        return 0
    if rank == lab_count + 1:
        return outside
    offset = rank - 2
    return gap + linear * offset + quadratic * offset * offset


def exact_best_objective(capacities, ranks):
    best = None
    for assignment_indices in product(range(len(capacities)), repeat=len(ranks)):
        objective = exact_objective_for_assignment(capacities, ranks, assignment_indices)
        if objective is None:
            continue
        comparable = objective[:3] + (-objective[3], -objective[4])
        if best is None or comparable < best[0]:
            best = (comparable, objective)
    assert best is not None
    return best[1]


def exact_best_objective_by_key(capacities, ranks, key_function):
    best = None
    for assignment_indices in product(range(len(capacities)), repeat=len(ranks)):
        objective = exact_objective_for_assignment(capacities, ranks, assignment_indices)
        if objective is None:
            continue
        comparable = key_function(objective)
        if best is None or comparable < best[0]:
            best = (comparable, objective)
    assert best is not None
    return best[1]


def rubric_key(objective):
    max_rank, rank_sum, rank_square_sum, minimum_fill, average_fill = objective
    return (rank_sum, rank_square_sum, max_rank, -average_fill, -minimum_fill)


def balanced_key(objective):
    max_rank, rank_sum, rank_square_sum, minimum_fill, average_fill = objective
    return (rank_sum, rank_square_sum, max_rank, -minimum_fill, -average_fill)


def satisfaction_key_for_assignment(capacities, ranks, assignment_indices, **cost_kwargs):
    objective = exact_objective_for_assignment(capacities, ranks, assignment_indices)
    if objective is None:
        return None
    lab_count = len(capacities)
    assigned_costs = [
        dissatisfaction_cost(ranks[student_index][lab_index], lab_count, **cost_kwargs)
        for student_index, lab_index in enumerate(assignment_indices)
    ]
    max_rank, _, _, minimum_fill, average_fill = objective
    return (
        sum(assigned_costs),
        sum(cost * cost for cost in assigned_costs),
        max_rank,
        -average_fill,
        -minimum_fill,
    )


def exact_best_satisfaction_key(capacities, ranks, **cost_kwargs):
    best = None
    for assignment_indices in product(range(len(capacities)), repeat=len(ranks)):
        comparable = satisfaction_key_for_assignment(
            capacities,
            ranks,
            assignment_indices,
            **cost_kwargs,
        )
        if comparable is None:
            continue
        if best is None or comparable < best:
            best = comparable
    assert best is not None
    return best


def exact_best_rubric_objective(capacities, ranks):
    return exact_best_objective_by_key(capacities, ranks, rubric_key)


def exact_best_balanced_objective(capacities, ranks):
    return exact_best_objective_by_key(capacities, ranks, balanced_key)


def exact_best_guarded_objective(capacities, ranks, slack):
    fair_objective = exact_best_objective(capacities, ranks)
    max_allowed_rank = fair_objective[0] + slack
    return exact_best_objective_by_key(
        capacities,
        ranks,
        lambda objective: (
            0 if objective[0] <= max_allowed_rank else 1,
            *rubric_key(objective),
        ),
    )


def weighted_score_for_objective(objective, outside_count, weights, changed_count=0):
    max_rank, rank_sum, rank_square_sum, minimum_fill, average_fill = objective
    return (
        weights["rank_sum"] * rank_sum
        + weights["rank_square"] * rank_square_sum
        + weights["max_rank"] * max_rank
        - weights["average_fill"] * average_fill
        - weights["minimum_fill"] * minimum_fill
        + weights["outside"] * outside_count
        + weights.get("change", 0) * changed_count
    )


def exact_best_weighted_score(capacities, ranks, weights, base_assignment_indices=None):
    best = None
    lab_count = len(capacities)
    for assignment_indices in product(range(lab_count), repeat=len(ranks)):
        objective = exact_objective_for_assignment(capacities, ranks, assignment_indices)
        if objective is None:
            continue
        assigned_ranks = [
            ranks[student_index][lab_index]
            for student_index, lab_index in enumerate(assignment_indices)
        ]
        outside_count = sum(rank == lab_count + 1 for rank in assigned_ranks)
        changed_count = 0
        if base_assignment_indices is not None:
            changed_count = sum(
                lab_index != base_assignment_indices[student_index]
                for student_index, lab_index in enumerate(assignment_indices)
            )
        score = weighted_score_for_objective(
            objective,
            outside_count,
            weights,
            changed_count,
        )
        if best is None or score < best:
            best = score
    assert best is not None
    return best


def weighted_score_from_solver_output(
    labs,
    capacities,
    student_ids,
    ranks,
    assignments,
    weights,
    base_assignments=None,
):
    lab_index_by_name = {name: index for index, name in enumerate(labs)}
    assignment_indices = [
        lab_index_by_name[assignments[student_id]]
        for student_id in student_ids
    ]
    objective = exact_objective_for_assignment(capacities, ranks, assignment_indices)
    assigned_ranks = [
        ranks[student_index][lab_index]
        for student_index, lab_index in enumerate(assignment_indices)
    ]
    outside_count = sum(rank == len(labs) + 1 for rank in assigned_ranks)
    changed_count = 0
    if base_assignments is not None:
        changed_count = sum(
            assignments[student_id] != base_assignments[student_id]
            for student_id in student_ids
        )
    return weighted_score_for_objective(
        objective,
        outside_count,
        weights,
        changed_count,
    )


def objective_from_solver_output(labs, capacities, student_ids, ranks, assignments):
    lab_index_by_name = {name: index for index, name in enumerate(labs)}
    assignment_indices = [
        lab_index_by_name[assignments[student_id]]
        for student_id in student_ids
    ]
    return exact_objective_for_assignment(capacities, ranks, assignment_indices)


def satisfaction_key_from_solver_output(
    labs,
    capacities,
    student_ids,
    ranks,
    assignments,
    **cost_kwargs,
):
    lab_index_by_name = {name: index for index, name in enumerate(labs)}
    assignment_indices = [
        lab_index_by_name[assignments[student_id]]
        for student_id in student_ids
    ]
    return satisfaction_key_for_assignment(
        capacities,
        ranks,
        assignment_indices,
        **cost_kwargs,
    )


def run_dataset(tmp_path, name, extra_args=None):
    lab_text = (DATASETS / f"{name}_labs.txt").read_text(encoding="utf-8")
    preference_text = (DATASETS / f"{name}_prefs.txt").read_text(encoding="utf-8")
    completed, output_path = run_solver(
        tmp_path / name,
        lab_text,
        preference_text,
        extra_args=extra_args,
    )
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(lab_text, preference_text, output_path)
    return lab_text, preference_text, assignments


def run_solver(tmp_path, lab_text, preference_text, include_reports=True, extra_args=None):
    return run_solver_built(tmp_path, lab_text, preference_text, include_reports, extra_args)


def run_solver_built(tmp_path, lab_text, preference_text, include_reports=True, extra_args=None):
    tmp_path.mkdir(parents=True, exist_ok=True)
    lab_path = tmp_path / "labs.txt"
    preference_path = tmp_path / "preferences.txt"
    output_path = tmp_path / "out.txt"
    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)
    command = [str(BINARY), str(lab_path), str(preference_path), str(output_path)]
    if extra_args:
        command.extend(extra_args)
    if include_reports:
        command.append("--reports")
    completed = subprocess.run(
        command,
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    return completed, output_path


def test_synthetic_sample_is_valid(tmp_path):
    lab_text = """\
8
LabA 5
LabB 4
LabC 2
LabD 3
LabE 0
LabF 5
LabG 4
LabH 1
"""
    preference_text = """\
10 3
00001 LabA LabB LabC
00002 LabB LabC LabD
00003 LabD LabF LabG
00004 LabF LabG LabH
00005 LabA LabC LabE
00006 LabB LabD LabF
00007 LabG LabH LabA
00008 LabH LabA LabB
00009 LabC LabF LabH
00010 LabD LabG LabB
"""
    completed, output_path = run_solver(tmp_path, lab_text, preference_text)
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(lab_text, preference_text, output_path)
    assert metrics_path_for(output_path).exists()
    assert_metrics_match_assignment(lab_text, preference_text, output_path, assignments)


def test_default_mode_writes_only_assignment_file(tmp_path):
    lab_text = """\
2
A 1
B 1
"""
    preference_text = """\
2 1
00001 A
00002 A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        include_reports=False,
    )
    assert completed.returncode == 0, completed.stderr
    assert completed.stdout == ""
    validate_assignment(lab_text, preference_text, output_path)
    assert output_path.exists()
    assert not metrics_path_for(output_path).exists()
    assert not lab_report_path_for(output_path).exists()
    assert not student_report_path_for(output_path).exists()
    assert not outside_report_path_for(output_path).exists()


def test_reports_print_success_summary_and_quiet_suppresses_it(tmp_path):
    lab_text = """\
2
A 1
B 1
"""
    preference_text = """\
2 1
00001 A
00002 A
"""
    completed, output_path = run_solver(
        tmp_path / "loud",
        lab_text,
        preference_text,
        include_reports=True,
    )
    assert completed.returncode == 0, completed.stderr
    assert f"output={output_path}" in completed.stdout
    assert f"reports={metrics_path_for(output_path)}" in completed.stdout

    quiet_completed, quiet_output_path = run_solver(
        tmp_path / "quiet",
        lab_text,
        preference_text,
        include_reports=True,
        extra_args=["--quiet"],
    )
    assert quiet_completed.returncode == 0, quiet_completed.stderr
    assert quiet_completed.stdout == ""
    assert metrics_path_for(quiet_output_path).exists()


def test_forced_outside_preference_when_required(tmp_path):
    lab_text = """\
3
A 1
B 1
C 1
"""
    preference_text = """\
3 1
00001 A
00002 A
00003 A
"""
    completed, output_path = run_solver(tmp_path, lab_text, preference_text)
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(lab_text, preference_text, output_path)
    assert metrics_path_for(output_path).exists()
    assert_metrics_match_assignment(lab_text, preference_text, output_path, assignments)
    assert set(assignments.values()) == {"A", "B", "C"}
    assert "00001" in assignments
    report_tokens = [
        token
        for row in read_lab_report(output_path).values()
        for token in row["student_tokens"]
    ]
    assert any("outside" in token for token in report_tokens)
    student_rows = read_tsv(student_report_path_for(output_path))
    assert any(row["outside_preference"] == "yes" for row in student_rows)
    outside_rows = read_tsv(outside_report_path_for(output_path))
    assert len(outside_rows) == 2
    assert all(row["submitted_preferences"] == "A" for row in outside_rows)


def test_duplicate_preference_is_rejected_without_output(tmp_path):
    lab_text = """\
2
A 1
B 1
"""
    preference_text = """\
1 2
12345 A A
"""
    completed, output_path = run_solver(tmp_path, lab_text, preference_text)
    assert completed.returncode != 0
    assert "duplicate lab name" in completed.stderr
    assert not output_path.exists()
    assert not metrics_path_for(output_path).exists()
    assert not lab_report_path_for(output_path).exists()
    assert not student_report_path_for(output_path).exists()
    assert not outside_report_path_for(output_path).exists()


def test_unknown_lab_is_rejected_without_output(tmp_path):
    lab_text = """\
2
A 1
B 1
"""
    preference_text = """\
1 1
12345 Z
"""
    completed, output_path = run_solver(tmp_path, lab_text, preference_text)
    assert completed.returncode != 0
    assert "unknown lab name" in completed.stderr
    assert not output_path.exists()
    assert not metrics_path_for(output_path).exists()
    assert not lab_report_path_for(output_path).exists()
    assert not student_report_path_for(output_path).exists()
    assert not outside_report_path_for(output_path).exists()


@pytest.mark.parametrize(
    ("lab_text", "preference_text", "expected_error"),
    [
        (
            "2\nA 1\n\n",
            "1 1\n00001 A\n",
            "blank lines are not valid data lines",
        ),
        (
            "1\nA\n",
            "1 1\n00001 A\n",
            "expected lab name and capacity",
        ),
        (
            "1\nA 1 extra\n",
            "1 1\n00001 A\n",
            "too many fields",
        ),
        (
            "1\nA 1\nextra 1\n",
            "1 1\n00001 A\n",
            "unexpected extra data after lab definitions",
        ),
        (
            "1\nA 1\n",
            "1 1\n00001\n",
            "not enough preference fields",
        ),
    ],
)
def test_line_level_parser_errors_are_preserved(
    tmp_path,
    lab_text,
    preference_text,
    expected_error,
):
    completed, output_path = run_solver(tmp_path, lab_text, preference_text)
    assert completed.returncode != 0
    assert expected_error in completed.stderr
    assert not output_path.exists()


def test_assignment5_id_policy_zero_pads_and_detects_normalized_duplicates(tmp_path):
    lab_text = """\
2
A 2
B 1
"""
    preference_text = """\
3 2
1 A B
2 A B
3 B A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--id-policy", "assignment5"],
    )
    assert completed.returncode == 0, completed.stderr
    assignments = read_output(output_path)
    assert set(assignments) == {"00001", "00002", "00003"}

    duplicate_preferences = """\
2 1
1 A
00001 A
"""
    completed, duplicate_output_path = run_solver(
        tmp_path / "duplicate",
        "1\nA 2\n",
        duplicate_preferences,
        extra_args=["--id-policy", "assignment5"],
    )
    assert completed.returncode != 0
    assert "duplicate student id" in completed.stderr
    assert not duplicate_output_path.exists()


def test_default_auto_id_policy_never_prompts_when_stdin_is_not_tty(tmp_path):
    lab_text = """\
1
A 2
"""
    preference_text = """\
2 1
1 A
2 A
"""
    completed, output_path = run_solver(tmp_path, lab_text, preference_text)
    assert completed.returncode != 0
    assert "cannot ask for confirmation" in completed.stderr
    assert "--id-policy assignment5" in completed.stderr
    assert not output_path.exists()


def test_numeric_and_token_id_policies_accept_oss_identifiers(tmp_path):
    lab_text = """\
2
A 2
B 1
"""
    numeric_preferences = """\
3 2
100000 A B
000002 A B
3 B A
"""
    completed, numeric_output_path = run_solver(
        tmp_path / "numeric",
        lab_text,
        numeric_preferences,
        extra_args=["--id-policy", "numeric"],
    )
    assert completed.returncode == 0, completed.stderr
    numeric_assignments = read_output(numeric_output_path)
    assert set(numeric_assignments) == {"100000", "000002", "000003"}

    token_preferences = """\
3 2
e23213 A B
e23214@example.ac.jp A B
学生_甲 B A
"""
    completed, token_output_path = run_solver(
        tmp_path / "token",
        lab_text,
        token_preferences,
        extra_args=["--id-policy", "token"],
    )
    assert completed.returncode == 0, completed.stderr
    token_assignments = read_output(token_output_path)
    assert set(token_assignments) == {
        "e23213",
        "e23214@example.ac.jp",
        "学生_甲",
    }

    completed, auto_output_path = run_solver(
        tmp_path / "auto",
        lab_text,
        token_preferences,
        extra_args=["--id-policy", "auto", "--assume-yes"],
    )
    assert completed.returncode == 0, completed.stderr
    assert read_output(auto_output_path) == token_assignments


def test_total_capacity_smaller_than_students_is_rejected_without_deleting_existing_output(tmp_path):
    lab_text = """\
2
A 1
B 1
"""
    preference_text = """\
3 1
00001 A
00002 A
00003 B
"""

    tmp_path.mkdir(parents=True, exist_ok=True)
    lab_path = tmp_path / "labs.txt"
    preference_path = tmp_path / "preferences.txt"
    output_path = tmp_path / "out.txt"

    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)
    write_text(output_path, "sentinel\n")

    completed = subprocess.run(
        [str(BINARY), str(lab_path), str(preference_path), str(output_path)],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )

    assert completed.returncode != 0
    assert "total lab capacity" in completed.stderr
    assert "smaller than student count" in completed.stderr
    assert output_path.exists()
    assert output_path.read_text(encoding="utf-8") == "sentinel\n"
    assert not metrics_path_for(output_path).exists()
    assert not lab_report_path_for(output_path).exists()
    assert not student_report_path_for(output_path).exists()
    assert not outside_report_path_for(output_path).exists()


def test_output_path_equal_to_input_path_is_rejected_without_overwriting_input(tmp_path):
    lab_text = """\
1
A 1
"""
    preference_text = """\
1 1
00001 A
"""

    tmp_path.mkdir(parents=True, exist_ok=True)
    lab_path = tmp_path / "labs.txt"
    preference_path = tmp_path / "preferences.txt"
    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)

    completed = subprocess.run(
        [str(BINARY), str(lab_path), str(preference_path), str(lab_path)],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )

    assert completed.returncode != 0
    assert "output/report path must be different from lab input file path" in completed.stderr
    assert lab_path.read_text(encoding="utf-8") == lab_text


def test_output_path_same_input_file_with_different_spelling_is_rejected(tmp_path):
    lab_text = """\
2
A 1
B 1
"""
    preference_text = """\
2 1
00001 A
00002 B
"""

    tmp_path.mkdir(parents=True, exist_ok=True)
    write_text(tmp_path / "labs.txt", lab_text)
    write_text(tmp_path / "preferences.txt", preference_text)

    completed = subprocess.run(
        [str(BINARY), "labs.txt", "preferences.txt", "./labs.txt"],
        cwd=tmp_path,
        text=True,
        capture_output=True,
    )

    assert completed.returncode != 0
    assert "output/report path must be different from lab input file path" in completed.stderr
    assert (tmp_path / "labs.txt").read_text(encoding="utf-8") == lab_text


def test_report_sidecar_path_equal_to_input_path_is_rejected_without_overwriting_input(tmp_path):
    lab_text = """\
1
A 1
"""
    preference_text = """\
1 1
00001 A
"""

    tmp_path.mkdir(parents=True, exist_ok=True)
    output_path = tmp_path / "out.txt"
    lab_path = metrics_path_for(output_path)
    preference_path = tmp_path / "preferences.txt"
    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)

    completed = subprocess.run(
        [
            str(BINARY),
            str(lab_path),
            str(preference_path),
            str(output_path),
            "--reports",
        ],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )

    assert completed.returncode != 0
    assert "metrics output path" in completed.stderr
    assert "output/report path must be different from lab input file path" in completed.stderr
    assert lab_path.read_text(encoding="utf-8") == lab_text
    assert not output_path.exists()


def test_report_sidecar_same_input_file_with_different_spelling_is_rejected(tmp_path):
    lab_text = """\
1
A 1
"""
    preference_text = """\
1 1
00001 A
"""

    tmp_path.mkdir(parents=True, exist_ok=True)
    output_path_text = str(tmp_path) + "/./out.txt"
    lab_path = metrics_path_for(tmp_path / "out.txt")
    preference_path = tmp_path / "preferences.txt"
    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)

    completed = subprocess.run(
        [
            str(BINARY),
            str(lab_path),
            str(preference_path),
            output_path_text,
            "--reports",
        ],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )

    assert completed.returncode != 0
    assert "metrics output path" in completed.stderr
    assert "output/report path must be different from lab input file path" in completed.stderr
    assert lab_path.read_text(encoding="utf-8") == lab_text
    assert not Path(str(tmp_path / "out.txt")).exists()


def test_invalid_input_with_reports_does_not_delete_existing_sidecars(tmp_path):
    lab_text = """\
2
A 1
B 1
"""
    preference_text = """\
3 1
00001 A
00002 A
00003 B
"""

    tmp_path.mkdir(parents=True, exist_ok=True)
    lab_path = tmp_path / "labs.txt"
    preference_path = tmp_path / "preferences.txt"
    output_path = tmp_path / "out.txt"

    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)
    write_text(output_path, "assignment sentinel\n")
    write_text(metrics_path_for(output_path), "metrics sentinel\n")
    write_text(lab_report_path_for(output_path), "report sentinel\n")
    write_text(student_report_path_for(output_path), "student sentinel\n")
    write_text(outside_report_path_for(output_path), "outside sentinel\n")

    completed = subprocess.run(
        [
            str(BINARY),
            str(lab_path),
            str(preference_path),
            str(output_path),
            "--reports",
        ],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )

    assert completed.returncode != 0
    assert "total lab capacity" in completed.stderr
    assert "smaller than student count" in completed.stderr
    assert output_path.read_text(encoding="utf-8") == "assignment sentinel\n"
    assert metrics_path_for(output_path).read_text(encoding="utf-8") == "metrics sentinel\n"
    assert lab_report_path_for(output_path).read_text(encoding="utf-8") == "report sentinel\n"
    assert student_report_path_for(output_path).read_text(encoding="utf-8") == "student sentinel\n"
    assert outside_report_path_for(output_path).read_text(encoding="utf-8") == "outside sentinel\n"


def test_too_many_positive_labs_is_rejected(tmp_path):
    lab_text = """\
3
A 1
B 1
C 1
"""
    preference_text = """\
2 1
00001 A
00002 B
"""
    completed, output_path = run_solver(tmp_path, lab_text, preference_text)
    assert completed.returncode != 0
    assert "minimum occupancy is impossible" in completed.stderr
    assert not output_path.exists()
    assert not metrics_path_for(output_path).exists()
    assert not lab_report_path_for(output_path).exists()
    assert not student_report_path_for(output_path).exists()
    assert not outside_report_path_for(output_path).exists()


def test_minimum_occupancy_uses_unpopular_lab(tmp_path):
    lab_text = """\
3
A 3
B 1
C 1
"""
    preference_text = """\
5 1
00001 A
00002 A
00003 A
00004 A
00005 A
"""
    completed, output_path = run_solver(tmp_path, lab_text, preference_text)
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(lab_text, preference_text, output_path)
    assert metrics_path_for(output_path).exists()
    assert_metrics_match_assignment(lab_text, preference_text, output_path, assignments)
    assert set(assignments.values()) == {"A", "B", "C"}


def test_matches_bruteforce_exact_objective_on_small_instances(tmp_path):
    instances = [
        (
            """\
3
A 2
B 2
C 2
""",
            """\
4 2
00001 A B
00002 A C
00003 B C
00004 B A
""",
        ),
        (
            """\
3
A 3
B 1
C 2
""",
            """\
5 2
00001 A B
00002 A C
00003 A B
00004 B C
00005 C A
""",
        ),
    ]

    for index, (lab_text, preference_text) in enumerate(instances):
        completed, output_path = run_solver(
            tmp_path / f"case_{index}",
            lab_text,
            preference_text,
            extra_args=["--objective", "fair"],
        )
        assert completed.returncode == 0, completed.stderr
        assignments = validate_assignment(lab_text, preference_text, output_path)
        assert metrics_path_for(output_path).exists()
        assert_metrics_match_assignment(lab_text, preference_text, output_path, assignments)
        labs, capacities, student_ids, ranks = parse_instance(lab_text, preference_text)
        expected = exact_best_objective(capacities, ranks)
        actual = objective_from_solver_output(labs, capacities, student_ids, ranks, assignments)
        assert actual == expected


def test_weighted_exact_matches_bruteforce_on_small_instance(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
00001 A B
00002 A C
00003 B A
00004 B C
"""
    weights = {
        "rank_sum": 7,
        "rank_square": 3,
        "max_rank": 11,
        "average_fill": 5,
        "minimum_fill": 13,
        "outside": 17,
    }
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--objective",
            "weighted-exact",
            "--weight-rank-sum",
            str(weights["rank_sum"]),
            "--weight-rank-square",
            str(weights["rank_square"]),
            "--weight-max-rank",
            str(weights["max_rank"]),
            "--weight-average-fill",
            str(weights["average_fill"]),
            "--weight-minimum-fill",
            str(weights["minimum_fill"]),
            "--weight-outside",
            str(weights["outside"]),
        ],
    )
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(lab_text, preference_text, output_path)
    assert_metrics_match_assignment(lab_text, preference_text, output_path, assignments)
    labs, capacities, student_ids, ranks = parse_instance(lab_text, preference_text)
    expected = exact_best_weighted_score(capacities, ranks, weights)
    actual = weighted_score_from_solver_output(
        labs,
        capacities,
        student_ids,
        ranks,
        assignments,
        weights,
    )
    assert actual == expected


def test_weighted_exact_matches_bruteforce_with_average_minimum_and_change(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
00001 A B
00002 A C
00003 B A
00004 C B
"""
    base_text = """\
4
00001 A
00002 B
00003 C
00004 B
"""
    weights = {
        "rank_sum": 5,
        "rank_square": 2,
        "max_rank": 7,
        "average_fill": 3,
        "minimum_fill": 11,
        "outside": 13,
        "change": 17,
    }
    tmp_path.mkdir(parents=True, exist_ok=True)
    lab_path = tmp_path / "labs.txt"
    preference_path = tmp_path / "preferences.txt"
    base_path = tmp_path / "base.txt"
    output_path = tmp_path / "out.txt"
    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)
    write_text(base_path, base_text)
    completed = subprocess.run(
        [
            str(BINARY),
            str(lab_path),
            str(preference_path),
            str(output_path),
            "--objective",
            "weighted-exact",
            "--base-assignment",
            str(base_path),
            "--weight-rank-sum",
            str(weights["rank_sum"]),
            "--weight-rank-square",
            str(weights["rank_square"]),
            "--weight-max-rank",
            str(weights["max_rank"]),
            "--weight-average-fill",
            str(weights["average_fill"]),
            "--weight-minimum-fill",
            str(weights["minimum_fill"]),
            "--weight-outside",
            str(weights["outside"]),
            "--weight-change",
            str(weights["change"]),
            "--reports",
        ],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(lab_text, preference_text, output_path)
    labs, capacities, student_ids, ranks = parse_instance(lab_text, preference_text)
    base_assignments = read_output(base_path)
    lab_index_by_name = {name: index for index, name in enumerate(labs)}
    base_assignment_indices = [
        lab_index_by_name[base_assignments[student_id]]
        for student_id in student_ids
    ]
    expected = exact_best_weighted_score(
        capacities,
        ranks,
        weights,
        base_assignment_indices=base_assignment_indices,
    )
    actual = weighted_score_from_solver_output(
        labs,
        capacities,
        student_ids,
        ranks,
        assignments,
        weights,
        base_assignments=base_assignments,
    )
    assert actual == expected


def test_rubric_balanced_and_guarded_match_bruteforce_on_small_instance(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
00001 A B
00002 A C
00003 B C
00004 C A
"""
    labs, capacities, student_ids, ranks = parse_instance(lab_text, preference_text)
    cases = [
        (["--objective", "rubric"], exact_best_rubric_objective(capacities, ranks)),
        (["--objective", "balanced"], exact_best_balanced_objective(capacities, ranks)),
        (
            ["--objective", "guarded", "--max-rank-slack", "0"],
            exact_best_guarded_objective(capacities, ranks, 0),
        ),
    ]
    for index, (extra_args, expected) in enumerate(cases):
        completed, output_path = run_solver(
            tmp_path / f"objective_{index}",
            lab_text,
            preference_text,
            extra_args=extra_args,
        )
        assert completed.returncode == 0, completed.stderr
        assignments = validate_assignment(lab_text, preference_text, output_path)
        actual = objective_from_solver_output(labs, capacities, student_ids, ranks, assignments)
        assert actual == expected


def test_satisfaction_matches_bruteforce_on_small_instance(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
00001 A B
00002 A C
00003 B C
00004 C A
"""
    labs, capacities, student_ids, ranks = parse_instance(lab_text, preference_text)
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--objective", "satisfaction"],
    )
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(lab_text, preference_text, output_path)
    actual = satisfaction_key_from_solver_output(
        labs,
        capacities,
        student_ids,
        ranks,
        assignments,
    )
    expected = exact_best_satisfaction_key(capacities, ranks)
    assert actual == expected


def test_satisfaction_rank_cost_file_and_reports(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
00001 A B
00002 A C
00003 B C
00004 C A
"""
    tmp_path.mkdir(parents=True, exist_ok=True)
    rank_costs_path = tmp_path / "rank_costs.txt"
    write_text(
        rank_costs_path,
        """\
rank 1 0
rank 2 300
outside 9000
""",
    )
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--objective",
            "satisfaction",
            "--rank-costs",
            str(rank_costs_path),
            "--reports",
        ],
    )
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(lab_text, preference_text, output_path)
    labs, capacities, student_ids, ranks = parse_instance(lab_text, preference_text)
    actual = satisfaction_key_from_solver_output(
        labs,
        capacities,
        student_ids,
        ranks,
        assignments,
        gap=300,
        linear=30,
        quadratic=5,
        outside=9000,
    )
    expected = exact_best_satisfaction_key(
        capacities,
        ranks,
        gap=300,
        linear=30,
        quadratic=5,
        outside=9000,
    )
    assert actual == expected
    metrics = read_metrics(output_path)
    assert "average_dissatisfaction" in metrics
    rows = read_tsv(student_report_path_for(output_path))
    assert "dissatisfaction" in rows[0]


def test_profile_and_print_helpers(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
00001 A B
00002 A C
00003 B C
00004 C A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--objective", "satisfaction", "--profile"],
    )
    assert completed.returncode == 0, completed.stderr
    profile = read_profile(output_path)
    assert int(profile["min_cost_flow_calls"]) > 0
    assert int(profile["student_group_builds"]) > 0
    assert profile["exact_path_cost_comparisons"] == profile["weighted_score_comparisons"]
    assert "counterfactual_cpu_seconds" in profile
    assert float(profile["solver_cpu_seconds"]) >= 0.0

    objectives = subprocess.run(
        [str(BINARY), "--print-objectives"],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    assert objectives.returncode == 0
    assert "satisfaction" in objectives.stdout
    assert "fill-convex" in objectives.stdout
    assert "weighted-exact" in objectives.stdout

    rank_costs = subprocess.run(
        [str(BINARY), "--print-rank-costs"],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    assert rank_costs.returncode == 0
    assert "rank 2 100" in rank_costs.stdout
    assert "outside 10000" in rank_costs.stdout

    presets = subprocess.run(
        [str(BINARY), "--print-presets"],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    assert presets.returncode == 0
    assert "rank_costs/student_friendly.txt" in presets.stdout
    assert "weights/evaluation_balance.txt" in presets.stdout
    assert "inspection commands" in presets.stdout

    explain_weights = subprocess.run(
        [str(BINARY), "--explain-weights", str(ROOT / "weights" / "evaluation_balance.txt")],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    assert explain_weights.returncode == 0
    assert "weighted-exact score" in explain_weights.stdout
    assert "max_rank" in explain_weights.stdout
    assert "Global terms" in explain_weights.stdout
    assert "complexity notes" in explain_weights.stdout
    assert "heavy exact weighted search" in explain_weights.stdout

    explain_rank_only = subprocess.run(
        [str(BINARY), "--explain-weights", str(ROOT / "weights" / "rank_only.txt")],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    assert explain_rank_only.returncode == 0
    assert "integer-only weighted min-cost flow" in explain_rank_only.stdout


def test_ordinary_average_scalar_fast_path_is_used(tmp_path):
    lab_text = """\
3
A 4
B 5
C 6
"""
    preference_text = """\
6 3
00001 A B C
00002 A B C
00003 A C B
00004 B A C
00005 C B A
00006 C A B
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--objective", "rubric", "--profile"],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    profile = read_profile(output_path)
    assert int(profile["ordinary_average_scalar_attempts"]) > 0
    assert int(profile["ordinary_average_scalar_used"]) > 0
    assert int(profile["ordinary_average_scalar_fallback_lcm"]) == 0
    assert int(profile["exact_min_cost_flow_calls"]) == 0
    assert int(profile["exact_path_cost_comparisons"]) == 0


def test_ordinary_average_scalar_fast_path_falls_back_on_large_lcm(tmp_path):
    lab_text = """\
4
A 1000000007
B 1000000009
C 1000000033
D 1000000087
"""
    preference_text = """\
8 4
00001 A B C D
00002 A B C D
00003 B A C D
00004 B C A D
00005 C A B D
00006 C D A B
00007 D A B C
00008 D C B A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--objective", "rubric", "--profile"],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    profile = read_profile(output_path)
    assert int(profile["ordinary_average_scalar_attempts"]) > 0
    assert int(profile["ordinary_average_scalar_used"]) == 0
    assert int(profile["ordinary_average_scalar_fallback_lcm"]) > 0
    assert int(profile["exact_min_cost_flow_calls"]) > 0
    assert int(profile["exact_path_cost_comparisons"]) > 0


def test_ordinary_average_scalar_fast_path_uses_active_rank_bound_for_satisfaction(tmp_path):
    lab_text = """\
8
A 4
B 5
C 6
D 7
E 8
F 9
G 10
H 11
"""
    preference_text = """\
16 8
00001 A B C D E F G H
00002 A C B D E F G H
00003 B A C D E F G H
00004 B C D A E F G H
00005 C B A D E F G H
00006 C D E F G H A B
00007 D C B A E F G H
00008 D E F G H A B C
00009 E D C B A F G H
00010 E F G H A B C D
00011 F E D C B A G H
00012 F G H A B C D E
00013 G F E D C B A H
00014 G H A B C D E F
00015 H G F E D C B A
00016 H A B C D E F G
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--objective",
            "satisfaction",
            "--rank-costs",
            str(ROOT / "rank_costs" / "student_friendly.txt"),
            "--profile",
        ],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    profile = read_profile(output_path)
    assert int(profile["ordinary_average_scalar_attempts"]) > 0
    assert int(profile["ordinary_average_scalar_used"]) > 0
    assert int(profile["ordinary_average_scalar_fallback_overflow"]) == 0
    assert int(profile["exact_path_cost_comparisons"]) == 0


def test_ordinary_average_scalar_fast_path_uses_capacity_aware_reward_bound(tmp_path):
    student_count = 500
    lab_text = """\
2
A 1
B 1000000000000
"""
    preference_rows = [
        f"{index:05d} B A"
        for index in range(1, student_count + 1)
    ]
    preference_text = (
        f"{student_count} 2\n" + "\n".join(preference_rows) + "\n"
    )
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--objective", "rubric", "--profile"],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    profile = read_profile(output_path)
    assert int(profile["ordinary_average_scalar_attempts"]) > 0
    assert int(profile["ordinary_average_scalar_used"]) > 0
    assert int(profile["ordinary_average_scalar_fallback_overflow"]) == 0
    assert int(profile["exact_path_cost_comparisons"]) == 0


def test_weight_file_preset_is_accepted(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
00001 A B
00002 A C
00003 B C
00004 C A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--objective",
            "weighted-exact",
            "--weights",
            str(ROOT / "weights" / "rank_only.txt"),
            "--profile",
        ],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    profile = read_profile(output_path)
    assert int(profile["exact_min_cost_flow_calls"]) == 0
    assert int(profile["biguint_score_comparisons"]) == 0


def test_fill_convex_objective_is_valid(tmp_path):
    lab_text = """\
4
A 2
B 2
C 2
D 2
"""
    preference_text = """\
6 2
00001 A B
00002 A C
00003 A D
00004 B C
00005 C D
00006 D B
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--objective", "fill-convex", "--profile"],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    profile = read_profile(output_path)
    assert int(profile["min_cost_flow_calls"]) > 0


def test_constraints_lock_forbid_allow_and_capacity_override(tmp_path):
    lab_text = """\
3
A 1
B 1
C 1
"""
    preference_text = """\
3 2
00001 A B
00002 A B
00003 A C
"""
    tmp_path.mkdir(parents=True, exist_ok=True)
    lab_path = tmp_path / "labs.txt"
    preference_path = tmp_path / "preferences.txt"
    output_path = tmp_path / "out.txt"
    constraints_path = tmp_path / "constraints.txt"
    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)
    write_text(
        constraints_path,
        """\
capacity A 2
lock 00003 C
forbid 00002 A
allow 00001 A C
""",
    )
    completed = subprocess.run(
        [
            str(BINARY),
            str(lab_path),
            str(preference_path),
            str(output_path),
            "--objective",
            "rubric",
            "--constraints",
            str(constraints_path),
            "--reports",
        ],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(
        "3\nA 2\nB 1\nC 1\n",
        preference_text,
        output_path,
    )
    assert assignments["00003"] == "C"
    assert assignments["00002"] != "A"
    reasons = read_tsv(Path(str(output_path) + ".reasons.tsv"))
    assert any(row["student_id"] == "00003" and row["reason"] == "manual lock" for row in reasons)
    locked_reason = next(row for row in reasons if row["student_id"] == "00003")
    assert locked_reason["first_choice_demand"] == "3"
    assert locked_reason["first_choice_capacity"] == "2"
    assert locked_reason["first_choice_assigned"] == "1"
    metrics = read_metrics(output_path)
    assert int(metrics["reason_manual_lock"]) == 1


def test_base_assignment_change_penalty_and_adjustment_report(tmp_path):
    lab_text = """\
2
A 2
B 2
"""
    preference_text = """\
3 2
00001 B A
00002 A B
00003 A B
"""
    constraints_text = "lock 00001 B\n"
    base_text = """\
3
00001 A
00002 A
00003 B
"""
    tmp_path.mkdir(parents=True, exist_ok=True)
    lab_path = tmp_path / "labs.txt"
    preference_path = tmp_path / "preferences.txt"
    constraints_path = tmp_path / "constraints.txt"
    base_path = tmp_path / "base.txt"
    output_path = tmp_path / "adjusted.txt"
    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)
    write_text(constraints_path, constraints_text)
    write_text(base_path, base_text)
    completed = subprocess.run(
        [
            str(BINARY),
            str(lab_path),
            str(preference_path),
            str(output_path),
            "--objective",
            "rubric",
            "--constraints",
            str(constraints_path),
            "--base-assignment",
            str(base_path),
            "--change-penalty",
            "100",
            "--reports",
        ],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(lab_text, preference_text, output_path)
    assert assignments["00001"] == "B"
    adjustment_text = Path(str(output_path) + ".adjustment_delta.tsv").read_text(encoding="utf-8")
    assert "changed_students" in adjustment_text
    assert "00001\tA\tB" in adjustment_text


def test_rank_sum_target_rejects_positive_change_penalty(tmp_path):
    lab_text = """\
2
A 1
B 1
"""
    preference_text = """\
2 2
00001 A B
00002 B A
"""
    base_text = """\
2
00001 B
00002 A
"""
    tmp_path.mkdir(parents=True, exist_ok=True)
    lab_path = tmp_path / "labs.txt"
    preference_path = tmp_path / "preferences.txt"
    base_path = tmp_path / "base.txt"
    output_path = tmp_path / "out.txt"
    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)
    write_text(base_path, base_text)
    completed = subprocess.run(
        [
            str(BINARY),
            str(lab_path),
            str(preference_path),
            str(output_path),
            "--objective",
            "rubric",
            "--base-assignment",
            str(base_path),
            "--change-penalty",
            "100",
            "--require-average-rank-at-most",
            "1",
        ],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    assert completed.returncode != 0
    assert "not currently exact with --change-penalty" in completed.stderr
    assert not output_path.exists()


def test_normalized_student_ids_work_in_constraints_base_and_explain(tmp_path):
    lab_text = """\
2
A 2
B 2
"""
    preference_text = """\
3 2
00001 B A
00002 A B
00003 A B
"""
    constraints_text = "lock 1 B\n"
    base_text = """\
3
1 A
2 A
3 B
"""
    tmp_path.mkdir(parents=True, exist_ok=True)
    lab_path = tmp_path / "labs.txt"
    preference_path = tmp_path / "preferences.txt"
    constraints_path = tmp_path / "constraints.txt"
    base_path = tmp_path / "base.txt"
    output_path = tmp_path / "adjusted.txt"
    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)
    write_text(constraints_path, constraints_text)
    write_text(base_path, base_text)
    completed = subprocess.run(
        [
            str(BINARY),
            str(lab_path),
            str(preference_path),
            str(output_path),
            "--constraints",
            str(constraints_path),
            "--base-assignment",
            str(base_path),
            "--change-penalty",
            "100",
            "--explain-student",
            "1",
            "--try-lock",
            "00001:B",
            "--reports",
        ],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(lab_text, preference_text, output_path)
    assert assignments["00001"] == "B"
    explain_rows = read_tsv(Path(str(output_path) + ".explain.tsv"))
    assert explain_rows[0]["student_id"] == "00001"
    assert explain_rows[0]["try_lab"] == "B"


def test_exact_counterfactual_explanation_reports_reoptimized_impact(tmp_path):
    lab_text = """\
3
A 2
B 2
C 1
"""
    preference_text = """\
4 3
00001 A B C
00002 A B C
00003 B A C
00004 C A B
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--explain-student", "00003", "--try-lock", "00003:A"],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    rows = read_tsv(Path(str(output_path) + ".explain.tsv"))
    assert len(rows) == 1
    assert rows[0]["student_id"] == "00003"
    assert rows[0]["try_lab"] == "A"
    assert rows[0]["feasible"] == "yes"
    assert rows[0]["try_rank"] == "2"
    assert rows[0]["reason"] == "exact re-solve succeeded"
    assert int(rows[0]["changed_students"]) >= 1


def test_exact_counterfactual_explanation_respects_existing_constraints(tmp_path):
    lab_text = """\
2
A 2
B 2
"""
    preference_text = """\
3 2
00001 A B
00002 A B
00003 B A
"""
    constraints_path = tmp_path / "constraints.txt"
    write_text(constraints_path, "allow 00003 B\n")
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--constraints",
            str(constraints_path),
            "--explain-student",
            "00003",
            "--try-lock",
            "00003:A",
        ],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    rows = read_tsv(Path(str(output_path) + ".explain.tsv"))
    assert len(rows) == 1
    assert rows[0]["feasible"] == "no"
    assert rows[0]["reason"] == "target lab is outside allowed set"


def test_counterfactual_explanation_rejects_portfolio_mode(tmp_path):
    lab_text = """\
2
A 2
B 2
"""
    preference_text = """\
3 2
00001 A B
00002 A B
00003 B A
"""
    completed, _ = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--portfolio",
            "--explain-student",
            "00003",
            "--try-lock",
            "00003:A",
        ],
    )
    assert completed.returncode != 0
    assert "requires a single --objective mode" in completed.stderr


def test_invalid_try_lock_is_rejected_before_output_is_written(tmp_path):
    lab_text = """\
2
A 2
B 2
"""
    preference_text = """\
3 2
00001 A B
00002 A B
00003 B A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--explain-student",
            "00003",
            "--try-lock",
            "00003:MissingLab",
        ],
    )
    assert completed.returncode != 0
    assert "unknown lab name" in completed.stderr
    assert not output_path.exists()
    assert not Path(str(output_path) + ".explain.tsv").exists()


def test_weighted_exact_change_weight_from_file_is_honored(tmp_path):
    lab_text = """\
2
A 2
B 2
"""
    preference_text = """\
3 2
00001 B A
00002 A B
00003 A B
"""
    base_text = """\
3
00001 A
00002 A
00003 B
"""
    weights_text = """\
rank_sum 1
rank_square 0
max_rank 0
average_fill 0
minimum_fill 0
outside 0
change 1000
"""
    tmp_path.mkdir(parents=True, exist_ok=True)
    lab_path = tmp_path / "labs.txt"
    preference_path = tmp_path / "preferences.txt"
    base_path = tmp_path / "base.txt"
    weights_path = tmp_path / "weights.txt"
    output_path = tmp_path / "weighted_change.txt"
    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)
    write_text(base_path, base_text)
    write_text(weights_path, weights_text)
    completed = subprocess.run(
        [
            str(BINARY),
            str(lab_path),
            str(preference_path),
            str(output_path),
            "--objective",
            "weighted-exact",
            "--weights",
            str(weights_path),
            "--base-assignment",
            str(base_path),
        ],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    assert completed.returncode == 0, completed.stderr
    assert read_output(output_path) == {"00001": "A", "00002": "A", "00003": "B"}


def test_portfolio_outputs_candidates_and_comparison(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
00001 A B
00002 A C
00003 B C
00004 C A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--portfolio"],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    portfolio_rows = read_tsv(Path(str(output_path) + ".portfolio.tsv"))
    candidate_names = {row["candidate"] for row in portfolio_rows}
    assert {"rubric", "satisfaction", "fair", "balanced", "guarded"} <= candidate_names
    assert "fill_focused" not in candidate_names
    assert "weighted_exact" not in candidate_names
    assert sum(row["recommended"] == "yes" for row in portfolio_rows) == 1
    assert {
        "avg_rank_component",
        "stddev_component",
        "max_rank_component",
        "avg_fill_deficit",
        "min_fill_deficit",
        "strengths",
        "weaknesses",
    } <= set(portfolio_rows[0])
    assert all(row["strengths"] for row in portfolio_rows)
    assert all(row["weaknesses"] for row in portfolio_rows)
    for candidate_name in candidate_names:
        candidate_path = Path(str(output_path) + f".{candidate_name}.txt")
        assert candidate_path.exists()
        validate_assignment(lab_text, preference_text, candidate_path)

    completed, deep_output_path = run_solver(
        tmp_path / "deep",
        lab_text,
        preference_text,
        extra_args=["--portfolio-deep"],
    )
    assert completed.returncode == 0, completed.stderr
    deep_rows = read_tsv(Path(str(deep_output_path) + ".portfolio.tsv"))
    deep_candidate_names = {row["candidate"] for row in deep_rows}
    assert {
        "rubric",
        "satisfaction",
        "fair",
        "balanced",
        "guarded",
        "fill_focused",
        "weighted_exact",
    } <= deep_candidate_names


def test_portfolio_recommendation_tie_break_is_deterministic(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
6 3
00001 A B C
00002 A B C
00003 A B C
00004 A B C
00005 A B C
00006 A B C
"""
    recommended = []
    for index in range(2):
        completed, output_path = run_solver(
            tmp_path / f"run{index}",
            lab_text,
            preference_text,
            extra_args=["--portfolio"],
        )
        assert completed.returncode == 0, completed.stderr
        rows = read_tsv(Path(str(output_path) + ".portfolio.tsv"))
        recommended.append(next(row["candidate"] for row in rows if row["recommended"] == "yes"))
    assert recommended == ["rubric", "rubric"]


def test_c_portfolio_jobs_outputs_parallel_candidates(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
00001 A B
00002 A C
00003 B C
00004 C A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--portfolio", "--jobs", "2"],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    portfolio_rows = read_tsv(Path(str(output_path) + ".portfolio.tsv"))
    candidate_names = {row["candidate"] for row in portfolio_rows}
    assert {"rubric", "satisfaction", "fair", "balanced", "guarded"} <= candidate_names
    assert sum(row["recommended"] == "yes" for row in portfolio_rows) == 1
    for candidate_name in candidate_names:
        candidate_path = Path(str(output_path) + f".{candidate_name}.txt")
        assert candidate_path.exists()
        assert metrics_path_for(candidate_path).exists()
        validate_assignment(lab_text, preference_text, candidate_path)


def test_c_portfolio_jobs_propagates_token_id_policy_to_children(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
e23213 A B
e23214@example.ac.jp A C
学生_甲 B C
KAWADA-23215 C A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--id-policy",
            "token",
            "--portfolio",
            "--jobs",
            "2",
            "--portfolio-summary-only",
        ],
    )
    assert completed.returncode == 0, completed.stderr
    assignments = read_output(output_path)
    assert set(assignments) == {
        "e23213",
        "e23214@example.ac.jp",
        "学生_甲",
        "KAWADA-23215",
    }
    assert Path(str(output_path) + ".portfolio.tsv").exists()
    assert not Path(str(output_path) + ".rubric.txt").exists()


def test_require_average_rank_at_most_succeeds_and_reports(tmp_path):
    lab_text = """\
2
A 2
B 2
"""
    preference_text = """\
3 1
00001 A
00002 A
00003 A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--objective",
            "fair",
            "--require-average-rank-at-most",
            "2.0",
        ],
    )
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(lab_text, preference_text, output_path)
    metrics = read_metrics(output_path)
    rows = read_tsv(target_status_path_for(output_path))
    assert float(metrics["rank_average"]) <= 2.0
    assert metrics["target_count"] == "1"
    assert metrics["target_pass_count"] == "1"
    assert metrics["target_all_passed"] == "1"
    assert len(assignments) == 3
    assert rows == [
        {
            "target": "average_rank",
            "operator": "<=",
            "required": "2",
            "actual": metrics["rank_average"],
            "status": "pass",
            "margin": rows[0]["margin"],
        }
    ]


def test_require_average_rank_at_most_reports_no_solution_without_output(tmp_path):
    lab_text = """\
2
A 1
B 1
"""
    preference_text = """\
2 1
00001 A
00002 A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--objective",
            "fair",
            "--require-average-rank-at-most",
            "1.0",
        ],
    )
    assert completed.returncode != 0
    assert "No feasible solution" in completed.stderr
    assert not output_path.exists()
    assert not target_status_path_for(output_path).exists()


def test_require_average_rank_unsupported_objective_is_rejected(tmp_path):
    lab_text = """\
2
A 1
B 1
"""
    preference_text = """\
2 2
00001 A B
00002 B A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--objective",
            "satisfaction",
            "--require-average-rank-at-most",
            "2.0",
        ],
    )
    assert completed.returncode != 0
    assert "average-rank/rank-sum hard targets are exact only" in completed.stderr
    assert not output_path.exists()


def test_require_average_fill_hard_target_succeeds_for_uniform_capacities(tmp_path):
    lab_text = """\
2
A 2
B 2
"""
    preference_text = """\
3 2
00001 A B
00002 B A
00003 A B
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--require-average-fill-at-least",
            "50%",
        ],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    metrics = read_metrics(output_path)
    rows = read_tsv(target_status_path_for(output_path))
    assert float(metrics["average_fill_rate"]) >= 0.5
    assert {row["target"]: row["status"] for row in rows} == {
        "average_fill_rate": "pass",
    }


def test_require_average_fill_hard_target_is_supported_when_min_fill_implies_it(tmp_path):
    lab_text = """\
2
A 2
B 4
"""
    preference_text = """\
4 2
00001 A B
00002 A B
00003 B A
00004 B A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--require-minimum-fill-at-least",
            "50%",
            "--require-average-fill-at-least",
            "50%",
        ],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    rows = read_tsv(target_status_path_for(output_path))
    assert {row["target"]: row["status"] for row in rows} == {
        "average_fill_rate": "pass",
        "minimum_fill_rate": "pass",
    }


def test_require_average_fill_hard_target_rejects_unsupported_resource_case(tmp_path):
    lab_text = """\
2
A 2
B 4
"""
    preference_text = """\
4 2
00001 A B
00002 A B
00003 B A
00004 B A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--require-average-fill-at-least",
            "75%",
        ],
    )
    assert completed.returncode != 0
    assert "average_fill_rate hard targets need a bounded exact resource case" in completed.stderr
    assert not output_path.exists()


def test_targets_file_minimum_fill_and_outside_report(tmp_path):
    lab_text = """\
2
A 3
B 3
"""
    preference_text = """\
4 2
00001 A B
00002 A B
00003 A B
00004 A B
"""
    targets_path = tmp_path / "winning_line.txt"
    write_text(
        targets_path,
        """\
minimum_fill_rate >= 50%
average_rank <= 2.0
outside_preference_count <= 0
""",
    )
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--objective",
            "fair",
            "--targets",
            str(targets_path),
        ],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    rows = read_tsv(target_status_path_for(output_path))
    statuses = {row["target"]: row["status"] for row in rows}
    assert statuses == {
        "average_rank": "pass",
        "minimum_fill_rate": "pass",
        "outside_preference_count": "pass",
    }
    metrics = read_metrics(output_path)
    assert metrics["target_count"] == "3"
    assert metrics["target_all_passed"] == "1"


def test_structural_targets_work_with_portfolio_summary_only(tmp_path):
    lab_text = """\
2
A 3
B 3
"""
    preference_text = """\
4 2
00001 A B
00002 A B
00003 A B
00004 A B
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--portfolio",
            "--portfolio-summary-only",
            "--require-minimum-fill-at-least",
            "50%",
            "--require-outside-at-most",
            "0",
        ],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    rows = read_tsv(target_status_path_for(output_path))
    assert {row["target"]: row["status"] for row in rows} == {
        "minimum_fill_rate": "pass",
        "outside_preference_count": "pass",
    }
    assert Path(str(output_path) + ".portfolio.tsv").exists()
    assert not Path(str(output_path) + ".rubric.txt").exists()


def test_require_no_outside_alias(tmp_path):
    lab_text = """\
2
A 2
B 2
"""
    preference_text = """\
3 2
00001 A B
00002 A B
00003 B A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=[
            "--objective",
            "fair",
            "--require-no-outside",
        ],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    rows = read_tsv(target_status_path_for(output_path))
    assert rows == [
        {
            "target": "outside_preference_count",
            "operator": "<=",
            "required": "0",
            "actual": "0",
            "status": "pass",
            "margin": "0",
        }
    ]


def test_portfolio_summary_only_removes_candidate_outputs(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
00001 A B
00002 A C
00003 B C
00004 C A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--portfolio"],
    )
    assert completed.returncode == 0, completed.stderr
    stale_candidate_path = Path(str(output_path) + ".rubric.txt")
    assert stale_candidate_path.exists()

    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--portfolio", "--portfolio-summary-only"],
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    portfolio_rows = read_tsv(Path(str(output_path) + ".portfolio.tsv"))
    for row in portfolio_rows:
        candidate_path = Path(str(output_path) + f".{row['candidate']}.txt")
        assert not candidate_path.exists()
        assert not metrics_path_for(candidate_path).exists()

    completed, parallel_output_path = run_solver(
        tmp_path / "parallel",
        lab_text,
        preference_text,
        extra_args=["--portfolio", "--jobs", "2", "--portfolio-summary-only"],
    )
    assert completed.returncode == 0, completed.stderr
    parallel_rows = read_tsv(Path(str(parallel_output_path) + ".portfolio.tsv"))
    for row in parallel_rows:
        candidate_path = Path(str(parallel_output_path) + f".{row['candidate']}.txt")
        assert not candidate_path.exists()
        assert not metrics_path_for(candidate_path).exists()

    completed, kept_output_path = run_solver(
        tmp_path / "keep",
        lab_text,
        preference_text,
        extra_args=[
            "--portfolio",
            "--portfolio-summary-only",
            "--keep-candidate-files",
        ],
    )
    assert completed.returncode == 0, completed.stderr
    kept_rows = read_tsv(Path(str(kept_output_path) + ".portfolio.tsv"))
    for row in kept_rows:
        candidate_path = Path(str(kept_output_path) + f".{row['candidate']}.txt")
        assert candidate_path.exists()
        validate_assignment(lab_text, preference_text, candidate_path)


def test_c_portfolio_jobs_works_when_parent_is_found_by_path(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
00001 A B
00002 A C
00003 B C
00004 C A
"""
    tmp_path.mkdir(parents=True, exist_ok=True)
    lab_path = tmp_path / "labs.txt"
    preference_path = tmp_path / "preferences.txt"
    output_path = tmp_path / "path_out.txt"
    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)
    env = os.environ.copy()
    env["PATH"] = f"{ROOT}:{env.get('PATH', '')}"
    completed = subprocess.run(
        [
            "assign_labs",
            str(lab_path),
            str(preference_path),
            str(output_path),
            "--portfolio",
            "--jobs",
            "2",
            "--reports",
        ],
        cwd=tmp_path,
        env=env,
        text=True,
        capture_output=True,
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    assert Path(str(output_path) + ".portfolio.tsv").exists()


def test_c_portfolio_jobs_profile_aggregates_child_counters(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
00001 A B
00002 A C
00003 B C
00004 C A
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--portfolio-deep", "--jobs", "2", "--profile"],
    )
    assert completed.returncode == 0, completed.stderr
    profile = read_profile(output_path)
    assert int(profile["min_cost_flow_calls"]) > 0
    assert int(profile["dinic_calls"]) > 0


def test_parallel_portfolio_wrapper(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
4 2
00001 A B
00002 A C
00003 B C
00004 C A
"""
    tmp_path.mkdir(parents=True, exist_ok=True)
    lab_path = tmp_path / "labs.txt"
    preference_path = tmp_path / "preferences.txt"
    output_path = tmp_path / "parallel_out.txt"
    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)
    completed = subprocess.run(
        [
            "python3",
            str(ROOT / "scripts" / "run_portfolio_parallel.py"),
            str(lab_path),
            str(preference_path),
            str(output_path),
            "--binary",
            str(BINARY),
            "--jobs",
            "2",
            "--deep",
        ],
        cwd=tmp_path,
        text=True,
        capture_output=True,
    )
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)
    rows = read_tsv(Path(str(output_path) + ".portfolio_parallel.tsv"))
    assert {
        "rubric",
        "satisfaction",
        "fair",
        "balanced",
        "guarded",
        "fill_convex",
        "weighted_exact_rank_only",
    } <= {
        row["candidate"] for row in rows
    }
    assert sum(row["recommended"] == "yes" for row in rows) == 1


def test_parallel_portfolio_wrapper_uses_candidate_order_tie_break(tmp_path):
    lab_text = """\
3
A 2
B 2
C 2
"""
    preference_text = """\
6 3
00001 A B C
00002 A B C
00003 A B C
00004 A B C
00005 A B C
00006 A B C
"""
    tmp_path.mkdir(parents=True, exist_ok=True)
    lab_path = tmp_path / "labs.txt"
    preference_path = tmp_path / "preferences.txt"
    output_path = tmp_path / "parallel_tie.txt"
    write_text(lab_path, lab_text)
    write_text(preference_path, preference_text)
    completed = subprocess.run(
        [
            "python3",
            str(ROOT / "scripts" / "run_portfolio_parallel.py"),
            str(lab_path),
            str(preference_path),
            str(output_path),
            "--binary",
            str(BINARY),
            "--jobs",
            "2",
        ],
        cwd=tmp_path,
        text=True,
        capture_output=True,
    )
    assert completed.returncode == 0, completed.stderr
    rows = read_tsv(Path(str(output_path) + ".portfolio_parallel.tsv"))
    assert next(row["candidate"] for row in rows if row["recommended"] == "yes") == "rubric"


def test_benchmark_script_help():
    completed = subprocess.run(
        ["python3", str(ROOT / "scripts" / "run_benchmarks.py"), "--help"],
        cwd=ROOT,
        text=True,
        capture_output=True,
    )
    assert completed.returncode == 0
    assert "Regenerate the benchmark TSV" in completed.stdout
    assert "popular_256x1024_weighted_evaluation" in completed.stdout


def test_uavg_capacity_bucket_compression_matches_bruteforce(tmp_path):
    lab_text = """\
4
A 2
B 2
C 3
D 3
"""
    preference_text = """\
5 2
00001 A C
00002 B C
00003 C A
00004 D B
00005 A D
"""

    completed, output_path = run_solver(tmp_path, lab_text, preference_text)
    assert completed.returncode == 0, completed.stderr

    assignments = validate_assignment(lab_text, preference_text, output_path)
    assert_metrics_match_assignment(lab_text, preference_text, output_path, assignments)

    labs, capacities, student_ids, ranks = parse_instance(lab_text, preference_text)
    expected = exact_best_objective(capacities, ranks)
    actual = objective_from_solver_output(labs, capacities, student_ids, ranks, assignments)

    assert actual == expected


def test_crlf_and_trailing_blank_lines_are_accepted(tmp_path):
    lab_text = "2\r\nA 1\r\nB 1\r\n\r\n"
    preference_text = "2 1\r\n00001 A\r\n00002 B\r\n\r\n"
    completed, output_path = run_solver(tmp_path, lab_text, preference_text)
    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)


def test_above_benchmark_size_limits_are_accepted_when_feasible(tmp_path):
    lab_count = 300
    student_count = 1050
    lab_lines = [str(lab_count)] + [
        f"L{lab_index:03d} 4" for lab_index in range(lab_count)
    ]
    preference_lines = [f"{student_count} 1"] + [
        f"{student_index + 1:05d} L000" for student_index in range(student_count)
    ]
    lab_text = "\n".join(lab_lines) + "\n"
    preference_text = "\n".join(preference_lines) + "\n"

    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        include_reports=False,
    )

    assert completed.returncode == 0, completed.stderr
    validate_assignment(lab_text, preference_text, output_path)


def test_all_dataset_files_are_valid(tmp_path):
    for name in [
        "synthetic_sample",
        "forced_outside",
        "zero_capacity",
        "rank_variance",
        "fill_rate",
    ]:
        run_dataset(tmp_path, name)


def test_small_dataset_files_match_bruteforce_optimum(tmp_path):
    for name in [
        "forced_outside",
        "zero_capacity",
        "rank_variance",
        "fill_rate",
    ]:
        lab_text, preference_text, assignments = run_dataset(
            tmp_path,
            name,
            extra_args=["--objective", "fair"],
        )
        labs, capacities, student_ids, ranks = parse_instance(lab_text, preference_text)
        expected = exact_best_objective(capacities, ranks)
        actual = objective_from_solver_output(labs, capacities, student_ids, ranks, assignments)
        assert actual == expected, name


def test_random_small_instances_match_bruteforce_optimum(tmp_path):
    rng = random.Random(20260611)

    for case_index in range(12):
        lab_count = rng.randint(1, 4)
        student_count = rng.randint(lab_count, 5)
        labs = [chr(ord("A") + index) for index in range(lab_count)]

        while True:
            capacities = [rng.randint(0, 4) for _ in labs]
            if sum(capacity > 0 for capacity in capacities) <= student_count and (
                sum(capacities) >= student_count
            ):
                break

        max_preferences = rng.randint(1, lab_count)
        lab_lines = [str(lab_count)] + [
            f"{lab_name} {capacity}"
            for lab_name, capacity in zip(labs, capacities)
        ]
        preference_lines = [f"{student_count} {max_preferences}"]
        for student_index in range(student_count):
            preference_labs = rng.sample(labs, max_preferences)
            preference_lines.append(
                f"{student_index + 1:05d} " + " ".join(preference_labs)
            )

        lab_text = "\n".join(lab_lines) + "\n"
        preference_text = "\n".join(preference_lines) + "\n"
        completed, output_path = run_solver_built(
            tmp_path / f"random_{case_index}",
            lab_text,
            preference_text,
            extra_args=["--objective", "fair"],
        )
        assert completed.returncode == 0, completed.stderr
        assignments = validate_assignment(lab_text, preference_text, output_path)
        assert_metrics_match_assignment(lab_text, preference_text, output_path, assignments)
        _, parsed_capacities, student_ids, ranks = parse_instance(lab_text, preference_text)
        expected = exact_best_objective(parsed_capacities, ranks)
        actual = objective_from_solver_output(labs, parsed_capacities, student_ids, ranks, assignments)
        assert actual == expected, case_index


def test_identical_rank_shortcut_matches_bruteforce_optimum(tmp_path):
    lab_text = """\
4
A 3
B 2
C 4
D 2
"""
    preference_text = """\
6 2
00001 A B
00002 A B
00003 A B
00004 A B
00005 A B
00006 A B
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--objective", "fair"],
    )
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(lab_text, preference_text, output_path)
    assert_metrics_match_assignment(lab_text, preference_text, output_path, assignments)
    labs, capacities, student_ids, ranks = parse_instance(lab_text, preference_text)
    expected = exact_best_objective(capacities, ranks)
    actual = objective_from_solver_output(labs, capacities, student_ids, ranks, assignments)
    assert actual == expected


def test_large_capacity_tie_breaker_matches_bruteforce_optimum(tmp_path):
    rng = random.Random(20260612)
    capacities_pool = [1000003, 1500000, 1785031, 2000000, 3000000]

    for case_index in range(8):
        lab_count = rng.randint(2, 4)
        student_count = rng.randint(lab_count, 5)
        labs = [chr(ord("A") + index) for index in range(lab_count)]
        capacities = rng.sample(capacities_pool, lab_count)
        max_preferences = rng.randint(1, lab_count)
        lab_lines = [str(lab_count)] + [
            f"{lab_name} {capacity}"
            for lab_name, capacity in zip(labs, capacities)
        ]
        preference_lines = [f"{student_count} {max_preferences}"]
        for student_index in range(student_count):
            preference_labs = rng.sample(labs, max_preferences)
            preference_lines.append(
                f"{student_index + 1:05d} " + " ".join(preference_labs)
            )

        lab_text = "\n".join(lab_lines) + "\n"
        preference_text = "\n".join(preference_lines) + "\n"
        completed, output_path = run_solver_built(
            tmp_path / f"large_capacity_{case_index}",
            lab_text,
            preference_text,
            extra_args=["--objective", "fair"],
        )
        assert completed.returncode == 0, completed.stderr
        assignments = validate_assignment(lab_text, preference_text, output_path)
        assert_metrics_match_assignment(lab_text, preference_text, output_path, assignments)
        _, parsed_capacities, student_ids, ranks = parse_instance(lab_text, preference_text)
        expected = exact_best_objective(parsed_capacities, ranks)
        actual = objective_from_solver_output(labs, parsed_capacities, student_ids, ranks, assignments)
        assert actual == expected, case_index


def test_huge_capacity_uavg_tie_breaker_is_exact(tmp_path):
    lab_text = """\
3
A 1000000000000003
B 1000000000000033
C 1000000000001009
"""
    preference_text = """\
4 3
00001 A B C
00002 A B C
00003 B A C
00004 B A C
"""
    completed, output_path = run_solver(
        tmp_path,
        lab_text,
        preference_text,
        extra_args=["--objective", "fair"],
    )
    assert completed.returncode == 0, completed.stderr
    assignments = validate_assignment(lab_text, preference_text, output_path)
    assert_metrics_match_assignment(lab_text, preference_text, output_path, assignments)
    labs, capacities, student_ids, ranks = parse_instance(lab_text, preference_text)
    expected = exact_best_objective(capacities, ranks)
    actual = objective_from_solver_output(labs, capacities, student_ids, ranks, assignments)
    assert actual == expected
    assert read_metrics(output_path)["lab_counts"]["A"]["assigned"] == 2
    assert read_metrics(output_path)["lab_counts"]["B"]["assigned"] == 1
    assert read_metrics(output_path)["lab_counts"]["C"]["assigned"] == 1
