# Algorithmic Proof and Design Notes

This solver is not a random search or a greedy first-choice filler.  It reduces
the assignment problem to exact flow problems and then applies only
structure-preserving reductions.

## Problem Model

Each student must be assigned to exactly one laboratory.  Each laboratory has an
integer capacity.  A positive-capacity laboratory is required to receive at
least one student when the instance is feasible.  A laboratory with zero
capacity is never assigned.

For a student-laboratory assignment, `rank` is the submitted preference rank.
Assignments outside the submitted preference list use the sentinel rank
`lab_count + 1`.

The main metrics are:

- `R1 = sum(rank)`
- `R2 = sum(rank * rank)`
- `Q = max(rank)`
- `Uavg = average(assigned_count / capacity)` over positive-capacity labs
- `Umin = min(assigned_count / capacity)` over positive-capacity labs

The default `rubric` objective optimizes:

```text
R1 -> R2 -> Q -> Uavg -> Umin
```

Other modes change the objective, but still solve the selected objective
exactly.

The exactness claim is for the implemented structured objectives only:
lexicographic rank/fill objectives, additive rank/outside/change costs,
separable convex fill penalties, and the documented `weighted-exact` scalar
objective with nonnegative integer weights. It is not a claim for arbitrary
black-box assignment objectives.

## Why Flow Is the Right Core

The assignment can be represented as a bipartite flow network:

```text
source -> student units -> laboratories -> sink
```

Each student sends one unit of flow.  Laboratory-to-sink capacities enforce
capacity limits.  Minimum occupancy requirements are represented with required
slots or lower-bound-equivalent edges.  All capacities are integers, so the
integrality theorem for flows guarantees an integral optimum: no student is
split fractionally between laboratories.

Once an objective is additive over student-laboratory choices, min-cost flow
optimizes it globally.  This includes rank sum, rank-square sum,
dissatisfaction cost, outside-preference penalty, and changed-from-base penalty.
The solver therefore considers exchange paths and cycles that local swap
heuristics can miss.

## Exact Lexicographic Objectives

Lexicographic objectives are handled by solving one exact phase, preserving the
optimal value, and then solving the next phase inside that optimal face.

For example, `rubric` first finds the exact minimum `(R1, R2)`.  It then finds
the smallest feasible `Q` that preserves that rank target, then maximizes
`Uavg`, then maximizes `Umin` without weakening the earlier values.

The use of `R2` after fixed `R1` is exactly aligned with rank standard
deviation:

```text
variance = R2 / N - (R1 / N)^2
```

When `N` and `R1` are fixed, minimizing `R2` minimizes variance and standard
deviation.

## Exact Average Fill

Average fill is a rational quantity:

```text
sum(assigned_count_j / capacity_j)
```

The solver does not compare this with floating point.  It groups equal
capacities and compares the resulting rational values with a least common
denominator and a small unsigned big-integer helper when ordinary integer
scaling is unsafe.  Equal-capacity grouping is algebraically identical because:

```text
sum over labs with capacity d of n_j / d = (sum n_j) / d
```

## Exact Weighted Mode

`weighted-exact` minimizes:

```text
w1*R1 + w2*R2 + w3*Q - w4*Uavg - w5*Umin + w6*outside + w7*changed
```

The additive terms are pushed into min-cost flow.  The global terms `Q` and
`Umin` are not additive, so the solver treats them as threshold axes:

```text
Q <= q
Umin >= u
```

For a box of thresholds:

```text
q in [q_low, q_high]
u in [u_low, u_high]
```

the solver computes a lower bound from the relaxed corner:

```text
A(q_high, u_low) + wQ*q_low - wU*u_high
```

Here `A(q, u)` is the exact additive weighted min-cost-flow optimum under those
thresholds.  If the lower bound is already no better than the incumbent, the
whole box is pruned.  This is branch-and-bound with a mathematical lower bound,
not a heuristic.

The lower bound follows from monotonic relaxation. For any assignment `x` in
the box, `q_low <= Q(x) <= q_high` and
`u_low <= Umin(x) <= u_high`. The relaxed corner `(q_high, u_low)` has a
superset of the feasible assignments, so `A(q_high, u_low) <= A(x)`. Because
all weights are nonnegative, `wQ*q_low <= wQ*Q(x)` and
`-wU*u_high <= -wU*Umin(x)`. Adding these inequalities gives a value no larger
than the full weighted score of any assignment in the box, so pruning on this
bound cannot remove a better solution.

The initial wide-rank seed is skipped when the rank axis is large because the
first relaxed corner supplies a valid incumbent anyway.  This avoids a costly
pre-scan on full-preference inputs while preserving exactness.

## Exact-Preserving Speedups

The implementation uses several reductions.  Each one preserves the exact
feasible set or objective value.

- Rank-threshold compression: only thresholds that change the edge set are
  tested.
- Minimum-fill vector compression: thresholds inducing the same lower-bound
  vector are equivalent.
- Allowed-set grouping: feasibility checks merge students with the same allowed
  lab set.
- Active-rank-row grouping: optimization merges students whose active edges and
  costs are identical.
- Same-rank-table shortcut: in the fair-mode path, if all active students have
  the same rank table after the worst-rank threshold is fixed, only lab counts
  matter, so the count problem is solved directly and expanded. Other
  rank-first modes remain exact but use the general grouped min-cost-flow path.
- Capacity-bucket average-fill comparison: equal denominators are combined
  exactly.
- Relaxed-corner cache: repeated weighted-exact corners are solved once.
- Portfolio process parallelism: independent exact objective candidates run in
  parallel and are reduced deterministically.

## Why This Is Not a Black Box

Every nontrivial optimization has a local invariant:

- grouping requires identical reachable lab sets and identical costs;
- threshold compression requires unchanged edge sets or unchanged lower-bound
  vectors;
- average-fill scaling is exact rational comparison;
- branch-and-bound pruning requires a lower bound that cannot exceed the true
  best value in the box;
- parallel portfolio execution only overlaps independent exact solves.

No candidate assignment is accepted merely because it is fast.  Heuristic or
preview methods are not used as proof of optimality.

## Limits of the Claim

The solver is exact for the objective selected on the command line.  It does
not claim that one objective is universally best for every institution.  That
is why the repository exposes `rubric`, `satisfaction`, `fair`, `guarded`,
`fill-convex`, `weighted-exact`, and portfolio comparison modes.

The P-time statement is about this structured family, not about arbitrary
assignment objectives.  In a valid instance with `N` students and `M`
laboratories, each implemented exact mode reduces to one or more integral
min-cost-flow or lower-bound feasibility solves.  `Q` has at most `M + 1`
relevant thresholds, and `Umin` has at most
`sum_j min(capacity_j, N) + 1` fill-ratio candidates before equivalent vectors
are compressed. Under the public benchmark limits this quantity is at most
`M*N + 1 <= 256*1024 + 1`; for larger OSS inputs the same polynomial expression
scales with the supplied `M` and `N`. `weighted-exact` is therefore an exact
search over a polynomial threshold grid; branch-and-bound is a pruning strategy
over that grid, not the source of correctness.

An arbitrary objective `F(assignment)` is outside that theorem. With only
oracle access to `F`, exact optimization can require checking exponentially
many feasible assignments. With a succinct unrestricted formula for `F`, the
objective can encode SAT: let each variable correspond to a student choosing
one of two laboratories, and let the objective evaluate the Boolean formula
under the induced truth assignment, scoring 0 for satisfying assignments and 1
otherwise. A polynomial-time exact optimizer for all such succinct objectives
would decide whether the optimum is 0 and therefore solve SAT in polynomial
time, implying P=NP. New objective modes should therefore either reduce to
min-cost flow, polynomial threshold enumeration, or another proved exact
polynomial subroutine, or be documented as exponential/heuristic.

The algorithm is also not claiming to beat every possible specialized solver on
every synthetic input.  The claim is narrower and stronger: for the stated
assignment model and selected objective, it computes an exact optimum while
using the standard polynomial flow machinery and exact-preserving reductions
that remove duplicate work without changing the answer.
