# Laboratory Assignment Test Datasets

Each dataset is a pair of files:

- `*_labs.txt`: laboratory names and capacities.
- `*_prefs.txt`: student ids and ranked preferences.

Running the solver without extra options writes only the assignment file
requested on the command line.  Running it with `--reports` also writes TSV/text
sidecars beside it: `*.metrics.txt`, `*.by_lab.txt`, `*.by_student.tsv`,
`*.outside_preferences.tsv`, and `*.reasons.tsv`.  Optional modes add
`*.portfolio.tsv`, `*.profile.tsv`, `*.explain.tsv`, and
`*.adjustment_delta.tsv`.  The reports expose the five evaluation metrics,
rank/dissatisfaction summaries, fill rates, reason counts, and per-student or
per-lab views so users can inspect the result without running a separate
conversion program.

Laboratory names are whitespace-free identifiers.  UTF-8 names are accepted,
but spaces inside one laboratory name are not part of the input format.

The datasets cover:

- `synthetic_sample`: a small synthetic input/output-format dataset.
- `forced_outside`: all students prefer the same laboratory, so forced outside-preference placement is required.
- `zero_capacity`: verifies that zero-capacity laboratories are never assigned.
- `rank_variance`: small instance for exhaustive optimality checking of rank sum and rank squared sum.
- `fill_rate`: small instance for exhaustive optimality checking including minimum fill rate.

Small datasets are verified by brute force in `tests/test_assignment.py`; larger-valid datasets are verified by the exact flow optimality proof in `docs/algorithm_paper.tex`.
The test suite also includes an in-test huge-capacity instance to verify that
the average fill-rate tie-breaker is exact even when ordinary fixed scaling
would lose the ordering.
