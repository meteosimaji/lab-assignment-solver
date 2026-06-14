# Laboratory Assignment Solver

Author: Meteo

This program assigns students to laboratories from two text input files.
The default mode writes only the simple student-to-laboratory assignment output.  Add
`--reports` when you also want metrics and a laboratory-wise checking report.

For the proof that the solver is exact for the selected objective and that the
speedups preserve the same answer, see
[`docs/ALGORITHM_PROOF.md`](docs/ALGORITHM_PROOF.md).  The longer TeX write-up
is in [`docs/algorithm_paper.tex`](docs/algorithm_paper.tex), with a generated
PDF at [`docs/algorithm_paper.pdf`](docs/algorithm_paper.pdf).

## Showcase

This is not a greedy first-choice filler or a random-search assignment script.
The solver computes exact optima for the implemented structured objectives,
writes inspectable report sidecars, and can compare multiple exact objective
candidates deterministically.

Representative capabilities:

| Feature | What it demonstrates |
| --- | --- |
| Exact objective solving | Uses min-cost flow, threshold search, and exact rational comparisons instead of heuristic acceptance. |
| Report sidecars | Writes metrics, per-lab fill rates, per-student ranks, outside-preference cases, and reason labels. |
| Portfolio mode | Runs several exact objective candidates on the same input and records the selected candidate. |
| Profile counters | Exposes solver calls, exact comparison counts, candidate counts, and phase timings. |
| Reproducible benchmarks | `make benchmark` regenerates representative benchmark data. |
| Safe packaging | `make source-archive` builds a clean public source archive from Git-tracked files only. |

Representative benchmark results from `docs/benchmark_results.tsv`:

| Case | Wall time | Avg rank | Rank stddev | Max rank | Avg fill | Min fill | Outside |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `uniform_256x1024_rubric` | 2.121s | 1.308 | 0.510 | 3 | 0.873 | 0.111 | 0 |
| `popular_256x1024_rubric` | 2.411s | 2.861 | 1.747 | 8 | 0.869 | 0.111 | 0 |
| `popular_256x1024_fair` | 0.810s | 2.939 | 1.644 | 6 | 0.867 | 0.111 | 0 |
| `popular_256x1024_weighted_evaluation` | 4.026s | 2.882 | 1.672 | 8 | 0.868 | 0.111 | 0 |
| `popular_128x512_portfolio_jobs4` | 0.553s | 2.803 | 1.599 | 6 | 0.871 | 0.143 | 0 |

For longer command transcripts, report snippets, and benchmark summaries, see
[`docs/SHOWCASE.md`](docs/SHOWCASE.md).  For planned exactness-preserving
performance work, see [`docs/OPTIMIZATION_NOTES.md`](docs/OPTIMIZATION_NOTES.md).

Tiny input/output example:

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

```sh
./assign_labs labs.txt prefs.txt out.txt --reports
```

`out.txt`

```text
4
00001 A
00002 A
00003 B
00004 C
```

Report sidecars then explain the result, for example:

```text
average_rank 1
rank_stddev 0
rank_max 1
average_fill_rate 1
minimum_fill_rate 1
outside_preference_count 0
reason_first_choice_selected 4
```

## Scope of Exactness and Complexity

For the fixed assignment model in this repository and for the objective selected
by `--objective`, the solver computes a global optimum exactly.  The
polynomial-time claim applies to the implemented structured objectives:
additive rank/outside/change costs, the documented lexicographic orders
involving `Q`, `Uavg`, and `Umin`, convex separable fill penalties, and
`weighted-exact` with nonnegative integer weights in the documented scalar
formula.  These are solved by integral min-cost flow plus a polynomial number
of exact threshold and rational comparisons; no floating-point score or
heuristic acceptance is used.

This is not a universal optimizer for every possible scoring rule.  If the
objective is an arbitrary black-box function of the complete assignment, exact
optimization has no flow structure to exploit and can require exponentially
many objective queries.  More concretely, if an optimizer fails to query some
feasible assignment, two black-box objectives can agree on every queried
assignment while making that unqueried assignment uniquely optimal in one case
and not optimal in the other.

If an unrestricted objective is given succinctly, the optimization problem
already contains NP-hard problems.  A SAT instance can be embedded even with
the repository's minimum-occupancy rule: for each Boolean variable `x_i`,
create two laboratories `T_i` and `F_i` of capacity 2, a variable student
`X_i`, and two dummy students `D_i_T` and `D_i_F`.  The objective scores 0
exactly when the dummies occupy `T_i` and `F_i`, `X_i` chooses one of them, and
the induced truth assignment satisfies the formula; otherwise it scores 1.
A polynomial-time exact optimizer for every such unrestricted succinct
objective would therefore decide SAT in polynomial time, implying P=NP.  The
repository avoids that claim by supporting only objective families with proved
flow, threshold, rational-comparison, or separable-convex structure.

## Build

```sh
make CFLAGS='-std=c11 -O2 -Wall -Wextra -Wshadow -Wconversion -pedantic'
```

For stricter local verification:

```sh
make clean
make CFLAGS='-std=c11 -O2 -Wall -Wextra -Wshadow -Wconversion -pedantic -Werror'
python3 -m pytest -q
```

To create a public source archive, use Git-tracked files only:

```sh
make source-archive
```

This uses `git archive` and does not include local build products, `.git/`,
working `tmp/` files, submission zips, or private course PDFs.

## Run

```sh
./assign_labs LAB_FILE PREFERENCE_FILE OUTPUT_FILE [options]
```

For command-line help:

```sh
./assign_labs --help
```

Example:

```sh
./assign_labs datasets/synthetic_sample_labs.txt datasets/synthetic_sample_prefs.txt out.txt
```

Recommended commands:

```sh
# Rubric-style metrics oriented
./assign_labs labs.txt prefs.txt out.txt --objective rubric --reports

# Student satisfaction oriented
./assign_labs labs.txt prefs.txt out.txt \
  --objective satisfaction \
  --rank-costs rank_costs/student_friendly.txt \
  --reports

# Worst-rank fairness oriented
./assign_labs labs.txt prefs.txt out.txt --objective fair --reports

# Require a contest-style quality line, then optimize the selected objective
./assign_labs labs.txt prefs.txt out.txt \
  --objective fair \
  --require-average-rank-at-most 2.0 \
  --require-minimum-fill-at-least 25% \
  --require-outside-at-most 0 \
  --reports

# Compare exact objective candidates
./assign_labs labs.txt prefs.txt out.txt \
  --portfolio \
  --portfolio-summary-only \
  --reports

# Deep comparison with heavier exact candidates, using parallel workers when available
./assign_labs labs.txt prefs.txt out.txt \
  --portfolio-deep \
  --portfolio-summary-only \
  --jobs 4 \
  --reports

# Manual adjustment while minimizing changes from a previous assignment
./assign_labs labs.txt prefs.txt adjusted.txt \
  --constraints constraints.txt \
  --base-assignment out.txt \
  --change-penalty 100 \
  --reports
```

Decision guide:

| Goal | Command pattern | Why |
| --- | --- | --- |
| Default evaluation-style result | `--objective rubric --reports` | Optimizes average rank, rank spread, worst rank, then fill rates in a deterministic exact order. |
| Student-facing satisfaction | `--objective satisfaction --rank-costs rank_costs/student_friendly.txt --reports` | Makes first-choice loss much more expensive without leaving min-cost flow. |
| Worst-rank fairness | `--objective fair --reports` | Protects the maximum assigned rank first. |
| Contest-style hard requirements | `--objective fair --require-average-rank-at-most 2.0 --require-minimum-fill-at-least 25% --require-outside-at-most 0 --reports` | Optimizes the selected exact objective only among assignments that satisfy every required target. |
| Compare exact candidates | `--portfolio --portfolio-summary-only --reports` | Produces several exact candidates and a TSV recommendation table. |
| Deep weighted study | `--objective weighted-exact --weights weights/evaluation_balance.txt --reports --profile` | Solves the full scalar weighted objective exactly and explains runtime through profile counters. |

Configuration inspection commands:

```sh
./assign_labs --print-objectives
./assign_labs --print-presets
./assign_labs --print-rank-costs
./assign_labs --explain-weights weights/evaluation_balance.txt
```

Default mode creates only:

- `out.txt`: student-wise assignment file for submission and external scoring.

Plain runs are quiet so that external tools can consume stdout freely.  When
`--reports`, `--profile`, `--portfolio`, or counterfactual explanation is used,
the program prints a short success summary such as `output=...`,
`reports=...`, and `portfolio=...`.  Add `--quiet` to suppress that summary.

For human review, run:

```sh
./assign_labs datasets/synthetic_sample_labs.txt datasets/synthetic_sample_prefs.txt out.txt --reports
```

This also creates:

- `out.txt.metrics.txt`: five evaluation metrics, rank sums, dissatisfaction
  summaries, reason counts, per-lab counts, and CPU-time measurements.  The
  `solver_cpu_seconds` / `selected_solver_cpu_seconds` values measure the main
  selected solve.  `counterfactual_cpu_seconds` is nonzero only when
  `--explain-student` or `--try-lock` performs an additional exact re-solve.
- `out.txt.by_lab.txt`: tab-separated laboratory-wise report with fill
  percentages and assigned student ids annotated by assigned rank.
- `out.txt.by_student.tsv`: tab-separated student-wise report with assigned
  laboratory, assigned rank, dissatisfaction cost, and whether the assignment is
  outside the submitted preference list.
- `out.txt.outside_preferences.tsv`: tab-separated list of only students who
  were assigned outside their submitted preference list.
- `out.txt.reasons.tsv`: compact per-student reason labels, including first
  choice demand, capacity, and assigned count.
- `out.txt.explain.tsv`: only when `--explain-student` or `--try-lock` is used;
  exact counterfactual re-solve result and metric deltas for one requested
  student/lab lock.
- `out.txt.target_status.tsv`: only when hard targets are used; one row per
  `--require-*` target with required value, actual value, pass/fail, and margin.
- `out.txt.adjustment_delta.tsv`: only when `--base-assignment` is used; compares
  the previous assignment with the adjusted assignment.
- `out.txt.profile.tsv`: only when `--profile` is used; solver call counters,
  exact path-cost comparison counts, BigUInt comparison counts, candidate
  counts, ordinary average-fill scalar fast-path counters, and phase CPU
  timings.

Report columns are intentionally machine-readable.  The most important fields
are:

- `metrics.txt`: `average_rank`, `rank_stddev`, `rank_max`,
  `average_fill_rate`, `minimum_fill_rate`, `outside_preference_count`,
  `solver_cpu_seconds`, `counterfactual_cpu_seconds`, and
  `total_cpu_seconds`.
- `by_student.tsv`: `student_id`, `assigned_lab`, `assigned_rank`,
  `dissatisfaction`, and `outside_preference`.
- `reasons.tsv`: `reason_label` plus evidence columns such as
  `first_choice_demand`, `first_choice_capacity`, and
  `first_choice_assigned`.
- `portfolio.tsv`: candidate metrics, local recommendation components,
  `strengths`, `weaknesses`, `solver_seconds`, `recommended`,
  `tie_break_order`, and `selection_reason`.
- `target_status.tsv`: `target`, `operator`, `required`, `actual`, `status`,
  and `margin`.
- `profile.tsv`: graph/flow call counters, threshold candidate counts,
  `exact_path_cost_comparisons`, `biguint_score_comparisons`,
  `ordinary_average_scalar_used`,
  branch-and-bound counters, and phase CPU timings.  The legacy
  `weighted_score_comparisons` key is still written as an alias for
  `exact_path_cost_comparisons`.

### Hard target constraints

Hard targets restrict the feasible region before the selected objective is
optimized.  They do not replace the objective.  For example, the command below
means: first require average rank at most 2.0, no laboratory below 25% fill, and
no outside-preference assignment; then find the best `fair` solution inside
that target-satisfying region.

```sh
./assign_labs labs.txt prefs.txt out.txt \
  --objective fair \
  --require-average-rank-at-most 2.0 \
  --require-minimum-fill-at-least 25% \
  --require-outside-at-most 0 \
  --reports
```

If no assignment satisfies all hard targets, the solver exits before writing the
assignment file and reports:

```text
No feasible solution
```

The same targets can be stored in a file:

```text
# targets/winning_line.txt
average_rank <= 2.0
minimum_fill_rate >= 25%
outside_preference_count <= 0
```

```sh
./assign_labs labs.txt prefs.txt out.txt \
  --objective fair \
  --targets targets/winning_line.txt \
  --reports
```

Example `out.txt.target_status.tsv`:

```tsv
target	operator	required	actual	status	margin
average_rank	<=	2	1.5	pass	0.5
minimum_fill_rate	>=	0.25	0.5	pass	0.25
outside_preference_count	<=	0	0	pass	0
```

Supported hard targets:

```text
--require-average-rank-at-most X
--require-rank-sum-at-most N
--require-max-rank-at-most K
--require-minimum-fill-at-least X
--require-no-outside
--require-outside-at-most 0
--targets FILE
```

Reserved hard targets that are parsed and rejected until an exact
resource-constrained engine is available:

```text
--require-average-fill-at-least X
--require-rank-square-at-most N
--require-outside-at-most N   # N > 0
```

`X` can be an integer, decimal, fraction such as `1/4`, or percentage such as
`25%`.  Structural targets such as maximum rank, minimum fill, and
outside-at-most-zero are enforced directly by the flow model.  Average-rank and
rank-sum hard targets are exact for `rubric`, `balanced`, `guarded`, and `fair`.
`minimum_fill_rate` is supported because it becomes per-laboratory lower
bounds.  `average_fill_rate` hard targets are intentionally not enabled for
arbitrary objectives yet because they are global rational side constraints.
Likewise, rank-square, outside-count above zero, and changed-student-count hard
targets are rejected until an exact resource-constrained engine is enabled for
them.  Average-rank and rank-sum hard targets are also rejected with a positive
`--change-penalty`, because the current rank-first cost component then becomes
`rank_sum + change_penalty * changed_students` rather than the pure rank sum.
Unsupported combinations are rejected instead of silently becoming heuristics.

The default objective is `rubric`, aligned with common evaluation metrics:

```text
R1 -> R2 -> Q -> Uavg -> Umin
```

Other objective modes are available:

```sh
./assign_labs labs.txt prefs.txt out.txt --objective satisfaction
./assign_labs labs.txt prefs.txt out.txt --objective fair
./assign_labs labs.txt prefs.txt out.txt --objective balanced
./assign_labs labs.txt prefs.txt out.txt --objective guarded --max-rank-slack 1
```

`satisfaction` uses a non-linear dissatisfaction cost `d(rank)` and optimizes
`D1 -> D2 -> Q -> Uavg -> Umin`, where `D1=sum d(rank)` and
`D2=sum d(rank)^2`.  This keeps the problem in min-cost flow while making a
drop from first choice to second choice much more expensive than a one-step
rank change under plain average-rank scoring.  By default:

```text
d(1)=0
d(r)=100 + 30*(r-2) + 5*(r-2)^2 for 2 <= r <= M
d(M+1)=10000
```

These values can be changed with formula options:

```sh
./assign_labs labs.txt prefs.txt out.txt \
  --objective satisfaction \
  --first-choice-gap 300 \
  --rank-tail-linear 30 \
  --rank-tail-quadratic 5 \
  --outside-cost 10000 \
  --reports
```

or by a table file:

```text
rank 1 0
rank 2 300
rank 3 360
outside 10000
```

```sh
./assign_labs labs.txt prefs.txt out.txt \
  --objective satisfaction \
  --rank-costs rank_costs/student_friendly.txt \
  --reports
```

`fair` preserves the earlier conservative order `Q -> R1 -> R2 -> Umin -> Uavg`.
`balanced` uses `R1 -> R2 -> Q -> Umin -> Uavg`.  `guarded` first computes the
minimum feasible worst rank, allows the configured slack, and then optimizes the
rubric order inside that guard.

A weighted exact mode is also available:

```sh
./assign_labs labs.txt prefs.txt out.txt \
  --objective weighted-exact \
  --weights weights/evaluation_balance.txt \
  --weight-rank-sum 50 \
  --weight-rank-square 20 \
  --weight-max-rank 15 \
  --weight-average-fill 5 \
  --weight-minimum-fill 5 \
  --weight-outside 30 \
  --weight-change 100 \
  --reports
```

Preset files are included under `rank_costs/` and `weights/`.  They are plain
text and can be edited without recompiling.  Configuration values are
nonnegative integers up to `1000000000`:

```text
rank_costs/student_friendly.txt
rank_costs/balanced_satisfaction.txt
rank_costs/strict_outside_avoidance.txt
weights/evaluation_balance.txt
weights/rank_only.txt
weights/fill_balance.txt
weights/minimum_change_adjustment.txt
```

To inspect the available objective modes, the default rank-cost sample, and
weighted-exact preset behavior:

```sh
./assign_labs --print-objectives
./assign_labs --print-presets
./assign_labs --print-rank-costs
./assign_labs --explain-weights weights/evaluation_balance.txt
```

`--explain-weights` is a top-level inspection command.  It does not take lab,
preference, or output files.

## Student Identifier Policies

The original assignment-style input uses numeric student identifiers and the
solver writes them as zero-padded five-digit IDs such as `00001`.  For public
and operational use, the solver can also treat the first column as larger
numeric IDs or opaque tokens such as `e23213`, email-style IDs, or
`学生_甲`.

By default, `--id-policy auto` is used.  If all input IDs are already
zero-padded assignment-style IDs, the run proceeds without interaction.  If the
solver detects IDs that need normalization, such as `1`, or non-assignment
tokens, it asks for y/N confirmation only when standard input is a terminal.
In scripts, CI, portfolio child processes, or other non-interactive runs, it
does not block; it exits with a hint asking you to choose an explicit policy or
pass `--assume-yes`.

```sh
# Original assignment compatibility. Numeric IDs in 1..99999 are output as 00001.
./assign_labs labs.txt prefs.txt out.txt --id-policy assignment5

# Larger numeric IDs. Leading zeros are normalized for duplicate detection.
./assign_labs labs.txt prefs.txt out.txt --id-policy numeric --student-id-width auto

# OSS/operations mode. The first column is an opaque whitespace-free token.
./assign_labs labs.txt prefs_token.txt out.txt --id-policy token

# Auto-detect in a batch job without a prompt.
./assign_labs labs.txt prefs.txt out.txt --id-policy auto --assume-yes
```

Numeric policies compare IDs after normalization, so `1`, `001`, and `00001`
refer to the same student.  Output formatting is separate from lookup:
`assignment5` always writes five digits, while `numeric` uses the widest input
or canonical numeric width unless `--student-id-width N` is supplied.  Token
mode preserves the input token exactly and does not lowercase or Unicode-
normalize it; if you need NFC normalization or CSV conversion, do that in an
input-preparation step.

`weighted-exact` minimizes the exact weighted score
`w1*R1 + w2*R2 + w3*Q - w4*Uavg - w5*Umin + w6*outside + w7*changed`.
Weights are nonnegative integers.  The mode uses `R2` rather than the standard
deviation itself because `R2` is additive and can be optimized exactly by
min-cost flow; the standard deviation is still reported.  When the exact
average-fill denominator fits safely in 64-bit signed costs, the solver folds
that reward into the ordinary min-cost-flow cost.  Otherwise it falls back to
the big-integer comparator, so the optimization remains exact.
For the global terms `Q` and `Umin`, the solver uses a two-dimensional exact
branch-and-bound over rank-threshold and minimum-fill-threshold boxes.  Each
box is pruned only when a mathematically valid lower bound proves that it
cannot improve the current best weighted score.  The search processes boxes in
lower-bound priority order and first seeds the incumbent with an exact
rank/outside/change-only solution evaluated under the full weighted score; this
only improves pruning and does not weaken the final proof.  Relaxed-corner
cache entries are allocated lazily, so large threshold grids do not allocate
full BigUInt score entries for corners that are never visited.

Manual constraints and adjustment runs are optional:

```text
lock 00001 LabA
forbid 00002 LabC
allow 00003 LabA LabD LabF
capacity LabB 4
```

```sh
./assign_labs labs.txt prefs.txt adjusted.txt \
  --constraints constraints.txt \
  --base-assignment out.txt \
  --change-penalty 100 \
  --reports
```

For an exact one-student counterfactual explanation, lock the requested student
to the requested laboratory and let the solver re-optimize the remaining
assignment under the selected objective:

```sh
./assign_labs labs.txt prefs.txt out.txt \
  --objective rubric \
  --explain-student 00001 \
  --try-lock 00001:LabA \
  --reports
```

This writes `out.txt.explain.tsv`.  The file reports whether the lock is
feasible, the selected student's current and trial ranks, metric deltas, and
how many students change from the original selected assignment.
The main output is written only after the requested student and laboratory
identifiers have been validated, so an invalid `--try-lock` cannot overwrite a
previous assignment.
Counterfactual explanation is intentionally available only with a single
`--objective` mode.  It is rejected with `--portfolio`, because portfolio mode
selects among several objectives and the explanation must re-optimize under
the same single decision rule as the selected assignment.

For comparison across objective modes:

```sh
./assign_labs labs.txt prefs.txt out.txt --portfolio --reports
```

This writes a light exact portfolio: `rubric`, `satisfaction`, `fair`,
`balanced`, and `guarded`.  It creates `out.txt.portfolio.tsv` and candidate
files such as `out.txt.rubric.txt` and `out.txt.satisfaction.txt`.  For heavier
exact fill-focused, fill-convex, and weighted-exact candidates as well, use:

```sh
./assign_labs labs.txt prefs.txt out.txt --portfolio-deep --reports
```

`out.txt.portfolio.tsv` includes normalized recommendation components:
average-rank component, rank-stddev component, max-rank component, average-fill
deficit, and minimum-fill deficit.  The recommendation score is a local proxy
for choosing among generated candidates, not an external relative-rank score,
because relative scoring depends on other submissions.
The same table also includes `strengths` and `weaknesses` columns, which mark
which generated-candidate metrics are relatively best or worst for that row.

To keep only the final assignment, summary portfolio table, and normal reports:

```sh
./assign_labs labs.txt prefs.txt out.txt \
  --portfolio \
  --portfolio-summary-only \
  --reports
```

Use `--keep-candidate-files` after `--portfolio-summary-only` to keep
per-candidate assignment files explicitly.

The C program can run portfolio candidates as parallel child processes:

```sh
./assign_labs labs.txt prefs.txt out.txt --portfolio --jobs 4 --reports
```

This keeps every candidate exact and deterministic; it only overlaps
independent objective solves.  The C implementation uses POSIX child processes
when available.  On Windows/MinGW it falls back to deterministic serial
portfolio solving; use the Python wrapper below for portable multi-process
execution when Python is available.  In parallel portfolio mode,
`solver_seconds` is the sum of candidate solver CPU seconds; the wall-clock
wait time can be much smaller.

For local operation on multi-core machines, the same exact candidate solves can
also be run as independent processes:

```sh
python3 scripts/run_portfolio_parallel.py labs.txt prefs.txt out.txt --jobs 4
```

Add `--deep` to include `fill_convex` and the rank-only weighted-exact
candidate in the parallel run.

The wrapper does not change the mathematical objective.  It runs exact
`assign_labs` invocations in parallel, copies the recommended candidate to
`out.txt`, and writes `out.txt.portfolio_parallel.tsv`.  It reports wall time
for each child process, but exact-score ties are resolved by the same
candidate-order rule as the C portfolio rather than by wall time.

`fill-convex` is also available as a direct objective:

```sh
./assign_labs labs.txt prefs.txt out.txt --objective fill-convex --reports
```

It keeps the assignment as an exact min-cost-flow problem by expanding each
lab-to-sink capacity into one-person marginal-cost edges.  The marginal costs
minimize a separable convex penalty for deviation from the capacity-proportional
ideal lab count, combined with the ordinary rank cost in one exact scalar
objective.  It is not a rank-first tie-breaker.

## Output Policy

The main output file is intentionally kept simple:

```text
student_count
student_id assigned_lab_name
student_id assigned_lab_name
...
```

The order of assignment lines is deterministic but not part of the
optimization objective.  The output contains exactly one assignment for every
input student after the selected identifier policy is applied.  With
`--id-policy assignment5`, IDs such as `1` are written as `00001`; with
`--id-policy numeric`, the configured numeric width is used; with
`--id-policy token`, the original token is preserved.  External evaluation does
not depend on whether the lines are sorted.

Numerical scores and readable summaries are written to sidecar files only when
`--reports` is specified.  This avoids breaking external tools that expect exactly
one plain assignment output file while still making the result easy to inspect
for local review.

Laboratory names are treated as whitespace-free identifiers, matching the
input format.  UTF-8 names are supported as byte strings, but spaces inside
one laboratory name are not supported.

The default benchmark size is 256 laboratories and 1024 students.
The program does not reject larger feasible inputs merely for exceeding those
two benchmark limits.  Practical scaling is limited by available memory and by
the effective student-laboratory edge count.

## Error Handling

For invalid input, the program writes an explanatory `error:` message to
standard error and terminates without producing a new assignment result.
Existing output files are not removed during input validation.

If an output file is being written and the write fails, only the temporary or
incomplete file created by the current run may be removed.

## Algorithm Summary

In default `rubric` mode, the solver first optimizes rank sum and rank-square
sum, then minimizes the worst assigned rank, then maximizes average and minimum
fill rates.  In `satisfaction` mode, the solver uses the same exact flow core
but replaces rank and rank-square edge costs with dissatisfaction cost and
dissatisfaction-square cost, then applies the same worst-rank and fill-rate
tie-breakers.  In `fair` mode, the solver first finds the smallest possible worst assigned rank with
lower-bound flow feasibility.  It then solves lexicographic min-cost flow to
optimize:

```text
worst rank -> rank sum -> rank squared sum -> minimum fill rate -> average fill rate
```

All five objectives are optimized exactly under this deterministic order.  For
the final average-fill tie-break, the program converts
`sum assigned_count / capacity` to an equivalent least-common-denominator
integer comparison with an internal unsigned big-integer type, so it does not
rely on floating point or scaled approximations.

When the least-common-denominator coefficients and the required big-M
lexicographic multiplier fit safely in signed integer costs, ordinary
rank-first objectives use a scalar fast path for that same average-fill
tie-break.  The solver minimizes `rank_square * M - U_scaled`, where
`U_scaled` is the exact LCD-scaled fill reward and `M` is larger than any
possible fill-reward difference.  This preserves the `(rank_square, -Uavg)`
order exactly.  The safety check only considers rank costs for edges that are
active under the current max-rank threshold, and it bounds the possible
`U_scaled` difference by sorting capacity-limited fill-reward slots.  If any
overflow or bound check fails, the solver falls back to the BigUInt
exact-average path.

In the fair-mode path, when all active students have the same rank table after
the worst-rank threshold is fixed, the solver switches to an exact count-based
shortcut.  In that case student identities are interchangeable: only the number
assigned to each laboratory can affect the fair-mode metrics.  The shortcut
therefore optimizes the laboratory counts directly, then expands the counts
back to student-wise output.  Other rank-first modes remain exact but use the
general grouped min-cost-flow path.

When only some students share identical active rank rows for the computed worst
rank bound, the general min-cost-flow path groups those students into one flow
node and expands the grouped result back to student-wise output.  Since every
student in one group has the same available laboratories and the same rank cost
on those active edges, grouping preserves all five objective values while
reducing graph size.  The feasibility search also groups students by the
allowed laboratory set for each tested rank threshold.

For the exact average-fill tie-breaker, laboratories with equal positive
capacity are grouped into capacity buckets before least-common-denominator integer
comparison.  This combines equal-denominator terms exactly and reduces the
coefficient dimension without introducing floating point or fixed-scale
approximation.

In `rubric` mode, after the best average fill-rate value is found, the solver
uses monotonicity of the minimum-fill lower-bound vectors to find the largest
minimum-fill threshold that preserves that same average fill-rate.  It checks a
short sequential prefix first, then switches to exponential and binary search
only when the optimum lies deeper in the threshold list.  This is an exact
search reduction, not a heuristic.

In `weighted-exact` mode, the solver treats the global max-rank and
minimum-fill components as two threshold axes.  It searches rank-threshold and
minimum-fill-threshold boxes, solves the relaxed corner of each box by exact
min-cost flow, and prunes a box only when the lower bound
`A(q_high,u_low) + wQ*q_low - wU*u_high` proves that no point in the box can
improve the incumbent.  This is still an exact weighted optimizer, not a
candidate-selection heuristic.  Equivalent minimum-fill thresholds are
compressed by their induced lower-bound vectors.  Max-rank thresholds are also
compressed to the rank values that actually change the student-lab edge set,
plus the forced-placement sentinel rank.

## Reproducible Verification

### Strict warning build

```sh
make strict
python3 -m pytest -q
```

### AddressSanitizer / UndefinedBehaviorSanitizer

On Linux or macOS with Clang:

```sh
make test-asan
```

If Clang is unavailable, GCC may also support:

```sh
make clean
make CC=gcc CFLAGS='-std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Wshadow -Wconversion -pedantic'
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 python3 -m pytest -q
```

ASan/UBSan may not be available in every MinGW environment.  The default
`gcc` build remains supported.

### Maximum-size stress input

```sh
mkdir -p tmp
python3 scripts/generate_stress.py \
  --labs 256 \
  --students 1024 \
  --preferences 256 \
  --name-bytes 250 \
  --capacity-mode varied \
  --seed 7 \
  --lab-file tmp/stress_labs.txt \
  --preference-file tmp/stress_prefs.txt

/usr/bin/time -p ./assign_labs tmp/stress_labs.txt tmp/stress_prefs.txt tmp/stress_out.txt
```

To regenerate the benchmark table shipped in `docs/benchmark_results.tsv`:

```sh
make benchmark
```

This runs `scripts/run_benchmarks.py`, generates uniform, popular-skewed,
dense, portfolio, weighted-exact, counterfactual, and long-name cases, and
records median wall time for the requested repeat count together with
`--profile` CPU-phase counters.

## Submission Packaging

If the directory is not managed by Git, create a clean source package with:

```sh
scripts/package_submission.sh
```

The package script excludes generated binaries, debug bundles, caches, `tmp/`,
sidecar reports, and OS metadata.
