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
- Ordinary rank-first average-fill tie-breaks use an integer scalar fast path
  when the capacity LCM, big-M multiplier, and per-edge cost bounds fit safely
  in signed `long long`.  It rewrites the final `(rank_square, -Uavg)` order as
  `rank_square * M - U_scaled`, then canonicalizes the returned solution cost
  back to the unscaled rank tuple.  If any safety check fails, the existing
  BigUInt exact-average path is used unchanged.  The safety bound only uses
  rank costs for ranks active under the current max-rank threshold, and its
  big-M reward range is computed from the largest and smallest feasible
  fill-reward slot sums after each laboratory is capped by both graph capacity
  and active incoming students, rather than from the coarser
  `student_count * (max_reward - min_reward)` bound.
- Weighted-exact branch-and-bound splits boxes by weighted objective spread
  (`max_rank` spread versus `minimum_fill` spread) before falling back to index
  width.  This changes only search order, not the lower-bound proof.
- Weighted-exact average-fill scalar safety checks use the active rank upper
  bound (`q_end`) when hard targets forbid larger ranks, avoiding overflow
  fallbacks caused by assignment edges that cannot appear in any searched box.
- Average-fill hard targets now use an exact bounded count-vector engine for
  supported single objectives.  The engine first handles passive cases where
  the target is implied by lower bounds or average fill is constant.  Otherwise
  it scales the rational average-fill expression to an integer resource,
  enumerates laboratory count vectors that satisfy the resource bound, and
  solves each count slice exactly with min-cost flow.  The count-vector limit
  is a safety guard: when it is exceeded, the solver rejects the run rather
  than silently switching to a heuristic.
- Initial min-cost-flow potentials use layered DAG relaxation on fresh
  assignment graphs and fall back to the previous queue method if the graph is
  not in layer order.
- Repeated ungrouped min-cost-flow solves reuse an active-arc template keyed by
  the problem pointer, constraint pointer, and rank threshold.  The template
  stores only active student-laboratory arcs and adjacency reservation counts;
  costs, residual capacities, reverse edges, and lower-bound sink edges are
  rebuilt for every solve.
- Ordinary min-cost-flow Dijkstra uses a radix heap when the current reduced
  residual graph can be encoded as a monotone unsigned scalar key.  The runtime
  check requires nonnegative reduced tuple components and overflow-checked
  scales that preserve lexicographic `(first, second, third)` order for every
  simple path.  Otherwise it falls back to the binary heap.
- Weighted-exact can seed its incumbent with additional light exact objectives
  on larger threshold grids.  These seeds are upper bounds for pruning only;
  the final solution is still certified by branch-and-bound.

## Safe Fast Paths To Consider

- Cache exact average scores for coefficient vectors to reduce repeated BigUInt
  accumulation.
- Add scalar shortest-path queues only after overflow-checked scalarization
  proves that lexicographic order is preserved.

## Scalar Shortest-Path Queue Roadmap

Radix heaps are now enabled for ordinary min-cost-flow Dijkstra only after a
per-augmentation reduced-cost scan proves that tuple costs can be mapped to
monotone unsigned keys.  Dial heaps are still useful only for small bounded
nonnegative integer reduced costs.  They should not be connected directly to
tuple costs or BigUInt exact average comparison.

Remaining safe order:

1. Preserve the existing exact comparator as the fallback.
2. Keep radix heap limited to checked scalarized ordinary min-cost-flow keys.
3. Extend scalar-key caching if the reduced-cost scan becomes a bottleneck.
4. Use a Dial bucket queue only when the maximum reduced edge cost is small.

This is a performance optimization only.  It must not change the mathematical
objective or replace the BigUInt exact-average path when scalarization is
unsafe.

## Reuse And Sparse Structure

- Cache student groups across threshold and ratio checks when the grouping key
  includes all state that affects allowed edges and costs.
- Extend active-arc template reuse to grouped solves only after the group-cache
  key also carries a stable generation identifier.  Pointer identity alone is
  not enough for stack-allocated `StudentGroups`.
- Reuse fuller immutable min-cost-flow graph templates only if residual
  capacities and reverse-edge ownership remain isolated for ASan-clean repeated
  solves.
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
- `active_arc_template_hits` and `active_arc_template_misses`: topology reuse
  for repeated ungrouped min-cost-flow solves.
- `radix_heap_attempts`, `radix_heap_used`, and `radix_heap_fallbacks`:
  checked scalar Dijkstra queue usage.
- `average_fill_resource_vectors_tested` and
  `average_fill_resource_vector_limit_hits`: bounded exact count-vector
  enumeration for nontrivial average-fill hard targets.
- `mcf_edges_added`: opportunity for fuller graph templates or sparse arcs.
- `min_cost_flow_calls`: opportunity for repeated-solve reduction.
- `dinic_calls`: opportunity for improved threshold feasibility checks.
- `weighted_bound_prunes`: quality of branch-and-bound lower bounds and seeds.
- `weighted_corner_cache_hits` and `weighted_corner_cache_misses`: repeated
  corner reuse in weighted-exact.
