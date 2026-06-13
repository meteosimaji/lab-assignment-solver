# Showcase

This document collects representative command transcripts and output snippets.
The goal is to show that the solver is exact, inspectable, reproducible, and
not merely a formatter for a small assignment example.

## 1. Minimal Assignment Output

Purpose:

- Shows the submission-compatible output format.
- Keeps the main output intentionally small.
- Leaves audit details in sidecar reports.

Input:

`labs.txt`

```text
3
A 2
B 1
C 1
```

`prefs.txt`

```text
4 2
00001 A B
00002 A C
00003 B A
00004 C A
```

Command:

```sh
./assign_labs labs.txt prefs.txt out.txt --reports
```

Main output:

```text
4
00001 A
00002 A
00003 B
00004 C
```

Metrics snippet:

```text
students 4
labs 3
average_rank 1
rank_stddev 0
rank_max 1
average_fill_rate 1
minimum_fill_rate 1
outside_preference_count 0
reason_first_choice_selected 4
```

Lab report snippet:

```text
lab_name	assigned	capacity	fill_rate	fill_percent	students
A	2	2	1	100.000000	00001(rank=1),00002(rank=1)
B	1	1	1	100.000000	00003(rank=1)
C	1	1	1	100.000000	00004(rank=1)
```

## 2. Forced Outside-Preference Case

Purpose:

- Shows that unavoidable outside-preference assignments are not hidden.
- Shows that the solver still reports explicit evidence.
- Keeps capacity and minimum-occupancy constraints visible.

Command:

```sh
./assign_labs datasets/forced_outside_labs.txt \
  datasets/forced_outside_prefs.txt \
  forced.txt \
  --reports
```

Outside-preference snippet:

```tsv
student_id	assigned_lab	assigned_rank	submitted_preferences
00001	C	4	A
00002	B	4	A
```

## 3. Portfolio Comparison

Purpose:

- Runs several exact objective candidates on the same input.
- Records which candidate is recommended by the local normalized proxy.
- Shows that the result is not manually cherry-picked.

Command:

```sh
./assign_labs datasets/synthetic_sample_labs.txt \
  datasets/synthetic_sample_prefs.txt \
  out.txt \
  --portfolio \
  --portfolio-summary-only \
  --reports
```

Portfolio snippet:

```tsv
candidate	avg_rank	rank_stddev	max_rank	avg_fill	min_fill	outside_count	recommended	selection_reason
rubric	1	0	1	0.50238095238095237	0.20000000000000001	0	yes	lowest_recommendation_score_then_candidate_order
satisfaction	1	0	1	0.50238095238095237	0.20000000000000001	0	no	not_selected
fair	1	0	1	0.50238095238095237	0.20000000000000001	0	no	not_selected
balanced	1	0	1	0.50238095238095237	0.20000000000000001	0	no	not_selected
guarded	1	0	1	0.50238095238095237	0.20000000000000001	0	no	not_selected
```

## 4. Hard Target Constraints

Purpose:

- Shows contest-style quality gates before objective optimization.
- Confirms that the solver reports target pass/fail margins.
- Keeps infeasible target sets explicit instead of writing a misleading result.

`targets/winning_line.txt`

```text
average_rank <= 2.0
minimum_fill_rate >= 25%
outside_preference_count <= 0
```

Command:

```sh
./assign_labs labs.txt prefs.txt out.txt \
  --objective fair \
  --targets targets/winning_line.txt \
  --reports
```

Target status snippet:

```tsv
target	operator	required	actual	status	margin
average_rank	<=	2	1.5	pass	0.5
minimum_fill_rate	>=	0.25	0.5	pass	0.25
outside_preference_count	<=	0	0	pass	0
```

If no assignment satisfies all required targets, the solver exits before
writing `out.txt` and includes `No feasible solution` in the error message.

## 5. Large Benchmark

Purpose:

- Shows large-input behavior.
- Shows runtime and exact-comparison counters.
- Keeps benchmark data reproducible.

Command:

```sh
make benchmark
```

The full table is stored in
[`docs/benchmark_results.tsv`](benchmark_results.tsv).  Important columns are:

- `wall`: end-to-end wall time for the candidate run.
- `avg_rank`, `stddev`, `max_rank`: student-side objective metrics.
- `avg_fill`, `min_fill`: laboratory fill metrics.
- `exact_path_cost_comparisons`: exact rational path-comparison work.
- `biguint_score_comparisons`: exact weighted score comparisons.

Representative rows from the current benchmark table:

| Case | Wall time | Avg rank | Rank stddev | Max rank | Avg fill | Min fill | Exact path comparisons |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `uniform_256x1024_rubric` | 1.975s | 1.308 | 0.510 | 3 | 0.873 | 0.111 | 32,577,654 |
| `popular_256x1024_rubric` | 2.373s | 2.861 | 1.747 | 8 | 0.869 | 0.111 | 40,797,870 |
| `popular_256x1024_fair` | 0.821s | 2.939 | 1.644 | 6 | 0.867 | 0.111 | 18,186,622 |
| `popular_256x1024_weighted_evaluation` | 4.039s | 2.882 | 1.672 | 8 | 0.868 | 0.111 | 0 |
| `popular_128x512_portfolio_jobs4` | 0.551s | 2.803 | 1.599 | 6 | 0.871 | 0.143 | 64,084,680 |

## 6. Source Archive Hygiene

Purpose:

- Shows that public packaging is independent from local working artifacts.

Command:

```sh
make source-archive
```

Expected property:

```text
The archive is created from Git-tracked files only.

It must not include:
- .git/
- tmp/
- submit.zip
- assignment.plist
- course PDFs
- local executables
- .DS_Store
```
