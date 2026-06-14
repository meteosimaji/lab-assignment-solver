# Optimization Notes

The solver is already exact for the implemented structured objectives.  The
items below are performance improvements that preserve the same mathematical
objective.  Any fast path must fall back to the existing exact path when its
safety conditions are not met.

## Implemented Safe Fast Paths

- Minimum-fill vector deduplication uses a hash of the exact lower-bound vector
  instead of scanning every existing vector.
- Ratio ceiling uses an overflow-checked `__uint128_t` path when the compiler
  supports it, with the original safe loop retained as the fallback.
- Rank-threshold candidates are generated from the input preference length and
  the outside-preference sentinel instead of scanning the whole dense rank
  matrix.
- Rank-target threshold checks reuse a rank-indexed student-group cache instead
  of rebuilding the same active-rank grouping for every repeated check.
- Fair objective with an average-rank or rank-sum hard target uses binary search
  over rank-threshold candidates.  The predicate is monotone: if the target is
  achievable with `max_rank <= q`, it remains achievable for every larger
  threshold.
- Rank-first objectives reject impossible rank-sum hard targets immediately
  after the rank-optimal base solve, before running fill tie-break searches.
- Minimum-fill hard targets prune generated fill-ratio candidates below the
  required lower bound.  All lower ratios map to the same target-clamped
  minimum-count vector, so this preserves the feasible-region search exactly.
- Minimum-fill ratio generation also skips ratios above safe infeasibility
  bounds from total student count and per-laboratory graph capacity.  Such
  ratios cannot produce a feasible lower-bound vector.
- Structural hard targets are prechecked with the existing lower-bound
  feasibility flow before expensive objective-specific solves.
- That structural precheck is skipped when an average-rank or rank-sum hard
  target is present, because the later rank-sum min-cost-flow feasibility check
  is stronger and would otherwise duplicate the same structural proof.
- Average-rank and rank-sum hard targets are rejected with positive
  `--change-penalty`.  In that mode the rank-first cost component contains the
  change term, so comparing it with a pure rank-sum bound would be incorrect.
- Weighted-exact branch-and-bound splits boxes by weighted objective spread
  (`max_rank` spread versus `minimum_fill` spread) before falling back to index
  width.  This changes only search order, not the lower-bound proof.
- Weighted-exact can seed its incumbent with additional light exact objectives
  on larger threshold grids.  These seeds are upper bounds for pruning only;
  the final solution is still certified by branch-and-bound.

## Safe Fast Paths To Consider

- Use integer scalarization for exact average-fill tie-breaks when the LCM,
  path-comparison coefficient bounds, and lexicographic bounds fit in a checked
  scalar representation.
- Cache exact average scores for coefficient vectors to reduce repeated BigUInt
  accumulation.
- Add scalar shortest-path queues only after overflow-checked scalarization
  proves that lexicographic order is preserved.

## Scalar Shortest-Path Queue Roadmap

Dial heaps are useful only for small bounded nonnegative integer reduced costs.
They should not be connected directly to tuple costs or BigUInt exact average
comparison.

Planned safe order:

1. Preserve the existing exact comparator as the fallback.
2. Add overflow-checked scalarization only when it preserves lexicographic
   ordering.
3. Run the scalarized min-cost-flow path with the current heap and compare
   objective values against the existing path.
4. Use a radix heap for general monotone unsigned Dijkstra keys.
5. Use a Dial bucket queue only when the maximum reduced edge cost is small.

This is a performance optimization only.  It must not change the mathematical
objective or replace the BigUInt exact-average path when scalarization is
unsafe.

## Reuse And Sparse Structure

- Cache student groups across threshold and ratio checks when the grouping key
  includes all state that affects allowed edges and costs.
- Reuse immutable min-cost-flow graph templates for repeated solves with the
  same rank threshold and different minimum-count vectors.
- Represent student group signatures sparsely for short preference lists.
- Investigate outside-preference edge compression only under strict conditions
  that preserve listed-vs-unlisted laboratory identity.

## Search Strategy

- Split weighted-exact branch-and-bound boxes by estimated objective spread
  instead of raw index width.
- Add cheap incumbent seeds only when their computation cost is smaller than
  the expected pruning benefit.

## Profile Counters To Watch

- `exact_path_cost_comparisons`: exact average-fill comparator pressure.
- `biguint_score_comparisons`: weighted exact score-comparison pressure.
- `student_group_builds`: opportunity for group cache or sparse signatures.
- `mcf_edges_added`: opportunity for graph templates or sparse arcs.
- `min_cost_flow_calls`: opportunity for repeated-solve reduction.
- `dinic_calls`: opportunity for improved threshold feasibility checks.
- `weighted_bound_prunes`: quality of branch-and-bound lower bounds and seeds.
- `weighted_corner_cache_hits` and `weighted_corner_cache_misses`: repeated
  corner reuse in weighted-exact.
