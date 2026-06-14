#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#define MAX_NAME_BYTES 255
#define INF_COST 2305843009213693951LL
#define HEAP_ARITY 4
#define BIG_BASE_BITS 31
#define BIG_BASE_MASK 2147483647ULL
#define MAX_BIG_LIMBS 8192
#define FAST_INPUT_BUFFER_SIZE 1048576U
#define FNV_OFFSET_BASIS 1469598103934665603ULL
#define FNV_PRIME 1099511628211ULL
#define MAX_CONFIG_WEIGHT 1000000000LL
#define PRINT_RANK_COST_SAMPLE_LIMIT 8
#define AVERAGE_FILL_LINEAR_PROBE_LIMIT 8
#define AVERAGE_FILL_RESOURCE_VECTOR_LIMIT 20000
#define RADIX_HEAP_SCAN_EDGE_LIMIT 200000
#define WEIGHTED_SEED_MAX_Q_AXIS 32
#define WEIGHTED_SEED_MIN_GRID_SIZE 128LL
#define WEIGHTED_LIGHT_SEED_MIN_GRID_SIZE 32LL
#define LIGHT_PORTFOLIO_CANDIDATE_COUNT 5
#define MAX_PORTFOLIO_CANDIDATES 8
#define AUTO_STUDENT_ID_WIDTH -1

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define NORETURN _Noreturn
#elif defined(__GNUC__) || defined(__clang__)
#define NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define NORETURN __declspec(noreturn)
#else
#define NORETURN
#endif

typedef struct ConstraintSet ConstraintSet;
typedef struct RankCostModel RankCostModel;
typedef struct TargetConstraints TargetConstraints;

typedef enum {
    ID_POLICY_ASSIGNMENT5,
    ID_POLICY_NUMERIC,
    ID_POLICY_TOKEN,
    ID_POLICY_AUTO
} StudentIdPolicy;

typedef struct {
    char *name;
    long long capacity_value;
    int graph_capacity;
} LabInfo;

typedef struct {
    char *raw_id;
    char *canonical_id;
    char *student_id;
} StudentInfo;

typedef struct {
    int lab_count;
    int student_count;
    int max_preferences;
    LabInfo *labs;
    StudentInfo *students;
    int *rank_by_student_lab;
    int lab_hash_size;
    int *lab_hash_indices;
    int student_hash_size;
    int *student_hash_indices;
    StudentIdPolicy id_policy;
    int student_id_width;
    const ConstraintSet *constraints;
    const int *base_assignment;
    const RankCostModel *rank_cost_model;
    const TargetConstraints *targets;
    long long change_penalty;
} ProblemData;

typedef struct {
    char *cursor;
} Tokenizer;

typedef struct {
    FILE *file;
    const char *path;
    unsigned char *buffer;
    size_t buffer_size;
    size_t position;
    size_t length;
    int reached_eof;
} FastInput;

typedef struct {
    int hash_size;
    int *indices;
} StudentHashIndex;

typedef struct {
    int to_node;
    int reverse_index;
    int capacity;
} DinicEdge;

typedef struct {
    DinicEdge *items;
    int size;
    int capacity;
} DinicEdgeList;

typedef struct {
    DinicEdgeList *adjacency;
    int node_count;
    int *level;
    int *work_index;
} DinicGraph;

typedef struct {
    long long first;
    long long second;
    long long third;
} Cost;

typedef struct {
    int to_node;
    int reverse_index;
    int capacity;
    Cost cost;
    int fill_lab_index;
    int fill_delta;
} McfEdge;

typedef struct {
    McfEdge *items;
    int size;
    int capacity;
} McfEdgeList;

typedef struct {
    McfEdgeList *adjacency;
    int node_count;
} McfGraph;

typedef struct {
    int student_index;
    int lab_index;
    int edge_index;
} AssignmentArc;

typedef struct {
    AssignmentArc *items;
    int size;
    int capacity;
} AssignmentArcList;

typedef struct {
    int student_index;
    int lab_index;
} ActiveAssignmentArc;

typedef struct {
    int valid;
    const ProblemData *problem_data;
    const ConstraintSet *constraints;
    int best_max_rank;
    int student_count;
    int lab_count;
    ActiveAssignmentArc *arcs;
    int arc_count;
    int arc_capacity;
    int *allowed_counts_by_student;
    int *incoming_counts_by_lab;
} UngroupedActiveArcTemplate;

typedef struct {
    int total_flow;
    Cost total_cost;
} MinCostResult;

typedef struct {
    long long numerator;
    long long denominator;
} RatioValue;

typedef struct {
    unsigned long long high;
    unsigned long long low;
} WideProduct;

typedef struct {
    RatioValue *items;
    int size;
    int capacity;
} RatioList;

struct TargetConstraints {
    int has_average_rank_max;
    RatioValue average_rank_max;
    int has_average_fill_min;
    RatioValue average_fill_min;
    int has_rank_sum_max;
    long long rank_sum_max;
    int has_rank_square_max;
    long long rank_square_max;
    int has_max_rank_max;
    int max_rank_max;
    int has_minimum_fill_min;
    RatioValue minimum_fill_min;
    int has_outside_max;
    int outside_max;
};

typedef struct {
    int *items;
    int size;
    int capacity;
} IntList;

typedef struct {
    RatioValue ratio;
    int *minimum_counts;
    int minimum_count_sum;
} MinimumCountCandidate;

typedef struct {
    MinimumCountCandidate *items;
    int size;
    int capacity;
} MinimumCountCandidateList;

typedef struct {
    unsigned int limbs[MAX_BIG_LIMBS];
    int length;
} BigUInt;

typedef struct {
    BigUInt *fill_weights;
    long long *term_capacities;
    int *term_by_lab;
    int term_count;
    BigUInt common_denominator;
    int positive_lab_count;
    int use_weighted_scalar;
    long long average_fill_weight;
} ExactAverageContext;

typedef struct {
    Cost primary_cost;
} ExactPathCost;

typedef struct {
    int node;
} ExactHeapItem;

typedef struct {
    ExactHeapItem *items;
    int *positions;
    int size;
    int capacity;
    const ExactPathCost *distances;
    const int *coefficients;
    int coefficient_count;
    const ExactAverageContext *context;
} ExactMinHeap;

typedef struct {
    int representative_student;
    int first_member;
    int size;
} StudentGroup;

typedef struct {
    StudentGroup *items;
    int *next_member;
    int *student_to_group;
    int count;
} StudentGroups;

typedef enum {
    STUDENT_GROUP_ACTIVE_RANK,
    STUDENT_GROUP_ALLOWED_SET
} StudentGroupingMode;

typedef enum {
    OBJECTIVE_RUBRIC,
    OBJECTIVE_SATISFACTION,
    OBJECTIVE_FAIR,
    OBJECTIVE_BALANCED,
    OBJECTIVE_GUARDED,
    OBJECTIVE_FILL_CONVEX,
    OBJECTIVE_WEIGHTED_EXACT
} ObjectiveMode;

typedef enum {
    FILL_TIE_AVERAGE_THEN_MINIMUM,
    FILL_TIE_MINIMUM_THEN_AVERAGE
} FillTieOrder;

typedef struct {
    long long rank_sum;
    long long rank_square;
    long long max_rank;
    long long average_fill;
    long long minimum_fill;
    long long outside;
    long long change;
} WeightedObjective;

struct RankCostModel {
    int use_explicit_table;
    long long *rank_costs;
    long long first_choice_gap;
    long long tail_linear;
    long long tail_quadratic;
    long long outside_cost;
};

struct ConstraintSet {
    int *locked_lab_by_student;
    unsigned char *forbidden_matrix;
    unsigned char *allowed_matrix;
    unsigned char *has_allowed_set;
};

typedef struct {
    int available;
    WeightedObjective scaled_weights;
    long long *lab_average_rewards;
} WeightedAverageFastPath;

typedef struct {
    int available;
    long long third_multiplier;
    long long *lab_average_rewards;
} OrdinaryAverageFastPath;

typedef struct {
    int available;
    long long *resource_by_lab;
    long long target_resource;
    long long maximum_resource;
} AverageFillResourceContext;

typedef enum {
    AVERAGE_REWARD_NONE,
    AVERAGE_REWARD_SECOND,
    AVERAGE_REWARD_THIRD
} AverageRewardPlacement;

typedef struct {
    long long reward;
    int capacity;
} AverageRewardBucket;

typedef struct {
    long long total_graph_capacity;
    long long student_count;
    long long per_unit_limit;
} ConvexFillContext;

typedef struct {
    long long dinic_calls;
    long long min_cost_flow_calls;
    long long exact_min_cost_flow_calls;
    long long student_group_builds;
    long long rank_threshold_candidate_builds;
    long long rank_target_checks;
    long long average_target_checks;
    long long q_candidates_tested;
    long long minimum_candidates_tested;
    long long exact_path_cost_comparisons;
    long long biguint_score_comparisons;
    long long layered_initial_potentials_used;
    long long ordinary_average_scalar_attempts;
    long long ordinary_average_scalar_used;
    long long ordinary_average_scalar_fallback_lcm;
    long long ordinary_average_scalar_fallback_overflow;
    long long ordinary_average_scalar_fallback_not_applicable;
    long long active_arc_template_hits;
    long long active_arc_template_misses;
    long long radix_heap_attempts;
    long long radix_heap_used;
    long long radix_heap_fallbacks;
    long long average_fill_resource_vectors_tested;
    long long average_fill_resource_vector_limit_hits;
    long long weighted_bound_prunes;
    long long weighted_corner_cache_hits;
    long long weighted_corner_cache_misses;
    long long try_solve_infeasible;
    long long dinic_edges_added;
    long long mcf_edges_added;
    int max_dinic_nodes;
    int max_mcf_nodes;
    long double read_cpu_seconds;
    long double solver_cpu_seconds;
    long double counterfactual_cpu_seconds;
    long double report_cpu_seconds;
    long double total_cpu_seconds;
} SolverProfile;

typedef struct {
    int write_reports;
    int write_profile;
    int portfolio_mode; /* 0: off, 1: light, 2: deep */
    int keep_candidate_files;
    int quiet;
    StudentIdPolicy id_policy;
    int student_id_width;
    int interactive;
    int assume_yes;
    int jobs;
    ObjectiveMode objective_mode;
    int max_rank_slack;
    WeightedObjective weights;
    TargetConstraints targets;
    RankCostModel rank_cost_model;
    const char *rank_costs_path;
    const char *weights_path;
    const char *targets_path;
    const char *constraints_path;
    const char *base_assignment_path;
    const char *explain_student_id;
    const char *try_lock_text;
    const char *program_path;
    const char *lab_file_path;
    const char *preference_file_path;
    long long change_penalty;
} ProgramOptions;

typedef struct {
    BigUInt positive;
    BigUInt negative;
} SignedBigScore;

typedef struct {
    int q_low_index;
    int q_high_index;
    int u_low_index;
    int u_high_index;
    SignedBigScore lower_bound;
} WeightedSearchBox;

typedef struct {
    WeightedSearchBox *items;
    int size;
    int capacity;
} WeightedSearchBoxHeap;

typedef struct {
    int group_index;
    int lab_index;
    int edge_index;
} GroupAssignmentArc;

typedef struct {
    GroupAssignmentArc *items;
    int size;
    int capacity;
} GroupAssignmentArcList;

typedef struct {
    int *assignment;
    int *lab_counts;
    Cost total_cost;
} SolutionResult;

typedef struct {
    int feasible;
    SolutionResult solution;
} OptionalSolutionResult;

typedef struct {
    int computed;
    OptionalSolutionResult result;
    SignedBigScore additive_score;
} WeightedRelaxedCornerCacheEntry;

typedef struct {
    WeightedRelaxedCornerCacheEntry **items;
    int q_count;
    int u_count;
} WeightedRelaxedCornerCache;

typedef struct {
    StudentGroups *groups;
    unsigned char *computed;
    int count;
    int enabled;
} WeightedStudentGroupCache;

typedef struct {
    StudentGroups *groups;
    unsigned char *computed;
    int count;
    StudentGroupingMode grouping_mode;
} StudentGroupCache;

typedef struct {
    int lab_index;
    int rank_value;
} LabRankOrder;

typedef struct {
    int rank_value;
    int start_index;
    int end_index;
    int total_count;
} RankGroupPlan;

typedef struct {
    int lab_index;
    long long capacity_value;
} LabFillOrder;

typedef struct {
    long long rank_sum;
    long long rank_square_sum;
    long long dissatisfaction_sum;
    long long dissatisfaction_square_sum;
    long long max_dissatisfaction;
    int max_rank;
    long double average_rank;
    long double rank_stddev;
    long double average_dissatisfaction;
    long double dissatisfaction_stddev;
    long double average_fill_rate;
    long double minimum_fill_rate;
    long double solver_cpu_seconds;
    long double counterfactual_cpu_seconds;
    long double program_cpu_seconds_before_metrics;
    int *lab_counts;
} EvaluationMetrics;

typedef struct {
    char name[32];
    ObjectiveMode objective_mode;
    int max_rank_slack;
    WeightedObjective weights;
    int *assignment;
    EvaluationMetrics metrics;
    long double solver_cpu_seconds;
    long double recommendation_score;
    int recommended;
} PortfolioCandidate;

typedef struct {
    long double average_rank_component;
    long double stddev_component;
    long double max_rank_component;
    long double average_fill_deficit;
    long double minimum_fill_deficit;
    long double total;
} PortfolioScoreComponents;

typedef struct {
    Cost distance;
    int node;
} HeapItem;

typedef struct {
    HeapItem *items;
    int *positions;
    int size;
    int capacity;
} MinHeap;

typedef struct {
    unsigned long long key;
    int node;
} RadixHeapItem;

typedef struct {
    RadixHeapItem *items;
    int size;
    int capacity;
} RadixHeapBucket;

typedef struct {
    RadixHeapBucket buckets[65];
    unsigned long long last_key;
    int size;
} RadixHeap;

typedef struct {
    int available;
    unsigned long long first_scale;
    unsigned long long second_scale;
    unsigned long long third_scale;
} RadixCostContext;

static const char *cleanup_output_path = NULL;
static char *cleanup_metrics_output_path = NULL;
static char *cleanup_lab_report_output_path = NULL;
static char *cleanup_student_report_output_path = NULL;
static char *cleanup_outside_report_output_path = NULL;
static char *cleanup_reasons_report_output_path = NULL;
static char *cleanup_adjustment_report_output_path = NULL;
static char *cleanup_portfolio_report_output_path = NULL;
static char *cleanup_profile_output_path = NULL;
static char *cleanup_explanation_report_output_path = NULL;
static char *cleanup_target_status_output_path = NULL;
static unsigned int temporary_file_counter = 0U;
static SolverProfile *active_profile = NULL;

static void *checked_malloc(size_t byte_count);
static void *checked_calloc(size_t item_count, size_t item_size);
static void *checked_realloc(void *pointer, size_t byte_count);
static size_t checked_multiply_size(size_t left, size_t right, const char *context);
static NORETURN void fail_with_message(const char *message);
static NORETURN void fail_with_context(const char *context, const char *detail);
static StudentGroups build_student_groups(const ProblemData *problem_data,
                                          int max_rank,
                                          StudentGroupingMode grouping_mode);
static void free_student_groups(StudentGroups *groups);
static long long max_rank_for_solution(const ProblemData *problem_data,
                                       const int *assignment);
static int find_fair_max_rank_satisfying_rank_sum_targets(
    const ProblemData *problem_data);
static SolutionResult empty_solution_result(void);
static int first_choice_lab_for_student(const ProblemData *problem_data,
                                        int student_index);
static void rank_cost_model_init_default(RankCostModel *model);
static long long rank_cost_formula_value(const RankCostModel *model, int rank_value);
static const char *reason_for_assignment(const ProblemData *problem_data,
                                         const int *assignment,
                                         const EvaluationMetrics *metrics,
                                         int student_index);
static NORETURN void fail_with_context_format(const char *context,
                                              const char *format,
                                              ...);
static NORETURN void fail_with_context_format_hint(const char *context,
                                                   const char *hint,
                                                   const char *format,
                                                   ...);
static void solver_profile_init(SolverProfile *profile);
static void solver_profile_write(const char *profile_path,
                                 const SolverProfile *profile);
static int target_constraints_are_empty(const TargetConstraints *targets);
static int target_rank_upper_bound(const ProblemData *problem_data,
                                   int base_upper_bound);
static int target_minimum_count_for_lab(const ProblemData *problem_data,
                                        int lab_index);
static int *build_base_minimum_counts(const ProblemData *problem_data);
static int ratio_compare_value(RatioValue left, RatioValue right);
static int ceil_ratio_times_capacity(RatioValue ratio,
                                     long long capacity_value,
                                     int count_limit);

static inline Cost cost_make(long long first, long long second, long long third)
{
    Cost cost_value;
    cost_value.first = first;
    cost_value.second = second;
    cost_value.third = third;
    return cost_value;
}

static inline int cost_is_infinity_value(Cost value)
{
    return value.first == INF_COST &&
           value.second == INF_COST &&
           value.third == INF_COST;
}

static inline long long checked_cost_component_add(long long left,
                                                   long long right,
                                                   const char *context)
{
    if ((right > 0LL && left > LLONG_MAX - right) ||
        (right < 0LL && left < LLONG_MIN - right)) {
        fail_with_context(context, "cost overflow");
    }
    return left + right;
}

static inline long long checked_cost_component_subtract(long long left,
                                                        long long right,
                                                        const char *context)
{
    if ((right > 0LL && left < LLONG_MIN + right) ||
        (right < 0LL && left > LLONG_MAX + right)) {
        fail_with_context(context, "cost overflow");
    }
    return left - right;
}

static inline long long checked_cost_component_multiply(long long value,
                                                        long long multiplier,
                                                        const char *context)
{
    if (value == 0LL || multiplier == 0LL) {
        return 0LL;
    }
    if (value > 0LL) {
        if (multiplier > 0LL) {
            if (value > LLONG_MAX / multiplier) {
                fail_with_context(context, "cost overflow");
            }
        } else if (multiplier < LLONG_MIN / value) {
            fail_with_context(context, "cost overflow");
        }
    } else {
        if (multiplier > 0LL) {
            if (value < LLONG_MIN / multiplier) {
                fail_with_context(context, "cost overflow");
            }
        } else if (value != 0LL && multiplier < LLONG_MAX / value) {
            fail_with_context(context, "cost overflow");
        }
    }
    return value * multiplier;
}

static inline int cost_less(Cost left, Cost right)
{
    if (left.first != right.first) {
        return left.first < right.first;
    }
    if (left.second != right.second) {
        return left.second < right.second;
    }
    if (left.third != right.third) {
        return left.third < right.third;
    }
    return 0;
}

static inline int cost_equal(Cost left, Cost right)
{
    return left.first == right.first &&
           left.second == right.second &&
           left.third == right.third;
}

static inline Cost cost_add(Cost left, Cost right)
{
    if (cost_is_infinity_value(left) || cost_is_infinity_value(right)) {
        return cost_make(INF_COST, INF_COST, INF_COST);
    }
    return cost_make(checked_cost_component_add(left.first, right.first, "cost add"),
                     checked_cost_component_add(left.second, right.second, "cost add"),
                     checked_cost_component_add(left.third, right.third, "cost add"));
}

static inline Cost cost_subtract(Cost left, Cost right)
{
    if (cost_is_infinity_value(left)) {
        return cost_make(INF_COST, INF_COST, INF_COST);
    }
    if (cost_is_infinity_value(right)) {
        fail_with_context("cost subtract", "cost overflow");
    }
    return cost_make(checked_cost_component_subtract(left.first,
                                                     right.first,
                                                     "cost subtract"),
                     checked_cost_component_subtract(left.second,
                                                     right.second,
                                                     "cost subtract"),
                     checked_cost_component_subtract(left.third,
                                                     right.third,
                                                     "cost subtract"));
}

static inline Cost cost_negate(Cost value)
{
    if (cost_is_infinity_value(value)) {
        fail_with_context("cost negate", "cost overflow");
    }
    return cost_make(checked_cost_component_subtract(0LL,
                                                     value.first,
                                                     "cost negate"),
                     checked_cost_component_subtract(0LL,
                                                     value.second,
                                                     "cost negate"),
                     checked_cost_component_subtract(0LL,
                                                     value.third,
                                                     "cost negate"));
}

static inline Cost cost_multiply(Cost value, int multiplier)
{
    long long multiplier_value = (long long)multiplier;
    if (cost_is_infinity_value(value)) {
        return cost_make(INF_COST, INF_COST, INF_COST);
    }
    return cost_make(checked_cost_component_multiply(value.first,
                                                     multiplier_value,
                                                     "cost multiply"),
                     checked_cost_component_multiply(value.second,
                                                     multiplier_value,
                                                     "cost multiply"),
                     checked_cost_component_multiply(value.third,
                                                     multiplier_value,
                                                     "cost multiply"));
}

static inline Cost cost_infinity(void)
{
    return cost_make(INF_COST, INF_COST, INF_COST);
}

static inline Cost cost_zero(void)
{
    return cost_make(0LL, 0LL, 0LL);
}

static void big_uint_zero(BigUInt *value)
{
    value->length = 0;
}

static void big_uint_extend_to(BigUInt *value, int required_length)
{
    int limb_index;
    if (required_length > MAX_BIG_LIMBS) {
        fail_with_context("exact average fill", "big integer overflow");
    }
    for (limb_index = value->length; limb_index < required_length; limb_index++) {
        value->limbs[limb_index] = 0U;
    }
    value->length = required_length;
}

static void big_uint_set_u64(BigUInt *value, unsigned long long source)
{
    int limb_index = 0;
    big_uint_zero(value);
    while (source > 0ULL) {
        if (limb_index >= MAX_BIG_LIMBS) {
            fail_with_context("exact average fill", "big integer overflow");
        }
        value->limbs[limb_index] = (unsigned int)(source & BIG_BASE_MASK);
        source >>= BIG_BASE_BITS;
        limb_index++;
    }
    value->length = limb_index;
}

static int big_uint_to_u64_checked(const BigUInt *value, unsigned long long *result)
{
    unsigned long long result_value = 0ULL;
    int limb_index;
    for (limb_index = value->length - 1; limb_index >= 0; limb_index--) {
        if (result_value > (ULLONG_MAX >> BIG_BASE_BITS)) {
            return 0;
        }
        result_value <<= BIG_BASE_BITS;
        if (result_value > ULLONG_MAX - (unsigned long long)value->limbs[limb_index]) {
            return 0;
        }
        result_value += (unsigned long long)value->limbs[limb_index];
    }
    *result = result_value;
    return 1;
}

static void big_uint_normalize(BigUInt *value)
{
    while (value->length > 0 && value->limbs[value->length - 1] == 0U) {
        value->length--;
    }
}

static void big_uint_add_shifted_scaled(BigUInt *target,
                                        const BigUInt *source,
                                        unsigned int scale,
                                        int shift)
{
    unsigned long long carry = 0ULL;
    int source_index;
    if (scale == 0U || source->length == 0) {
        return;
    }
    if (target->length < source->length + shift) {
        big_uint_extend_to(target, source->length + shift);
    }
    for (source_index = 0; source_index < source->length; source_index++) {
        int target_index = source_index + shift;
        unsigned long long total =
            (unsigned long long)target->limbs[target_index] +
            (unsigned long long)source->limbs[source_index] *
                (unsigned long long)scale +
            carry;
        target->limbs[target_index] = (unsigned int)(total & BIG_BASE_MASK);
        carry = total >> BIG_BASE_BITS;
    }
    source_index = source->length + shift;
    while (carry > 0ULL) {
        unsigned long long total;
        if (source_index >= MAX_BIG_LIMBS) {
            fail_with_context("exact average fill", "big integer overflow");
        }
        if (source_index >= target->length) {
            big_uint_extend_to(target, source_index + 1);
        }
        total = (unsigned long long)target->limbs[source_index] + carry;
        target->limbs[source_index] = (unsigned int)(total & BIG_BASE_MASK);
        carry = total >> BIG_BASE_BITS;
        source_index++;
    }
    big_uint_normalize(target);
}

static void big_uint_add_scaled_u64(BigUInt *target,
                                    const BigUInt *source,
                                    unsigned long long scale)
{
    int shift = 0;
    while (scale > 0ULL) {
        unsigned int scale_part = (unsigned int)(scale & BIG_BASE_MASK);
        if (scale_part != 0U) {
            big_uint_add_shifted_scaled(target, source, scale_part, shift);
        }
        scale >>= BIG_BASE_BITS;
        shift++;
    }
}

static void big_uint_multiply_u64(BigUInt *value, unsigned long long multiplier)
{
    BigUInt original = *value;
    BigUInt result;
    int shift = 0;
    big_uint_zero(&result);
    if (original.length == 0 || multiplier == 0ULL) {
        big_uint_zero(value);
        return;
    }
    while (multiplier > 0ULL) {
        unsigned int multiplier_part = (unsigned int)(multiplier & BIG_BASE_MASK);
        if (multiplier_part != 0U) {
            big_uint_add_shifted_scaled(&result, &original, multiplier_part, shift);
        }
        multiplier >>= BIG_BASE_BITS;
        shift++;
    }
    *value = result;
}

static int big_uint_compare(const BigUInt *left, const BigUInt *right)
{
    int index_value;
    if (left->length < right->length) {
        return -1;
    }
    if (left->length > right->length) {
        return 1;
    }
    for (index_value = left->length - 1; index_value >= 0; index_value--) {
        if (left->limbs[index_value] < right->limbs[index_value]) {
            return -1;
        }
        if (left->limbs[index_value] > right->limbs[index_value]) {
            return 1;
        }
    }
    return 0;
}

static void big_uint_add_big(BigUInt *target, const BigUInt *source)
{
    big_uint_add_scaled_u64(target, source, 1ULL);
}

static unsigned long long checked_multiply_u64(unsigned long long left,
                                               unsigned long long right,
                                               const char *context)
{
    if (left != 0ULL && right > ULLONG_MAX / left) {
        fail_with_context(context, "integer scale overflow");
    }
    return left * right;
}

static unsigned long long absolute_long_long_as_u64(long long value,
                                                    const char *context)
{
    if (value == LLONG_MIN) {
        fail_with_context(context, "integer scale overflow");
    }
    return value < 0LL ? (unsigned long long)(-value) : (unsigned long long)value;
}

static int multiply_nonnegative_ll_fits(long long left_value,
                                        long long right_value,
                                        long long limit_value,
                                        long long *result)
{
    if (left_value < 0LL || right_value < 0LL || limit_value < 0LL) {
        fail_with_context("weighted exact objective", "negative value in overflow guard");
    }
    if (right_value != 0LL && left_value > limit_value / right_value) {
        return 0;
    }
    *result = left_value * right_value;
    return 1;
}

static int add_nonnegative_ll_fits(long long left_value,
                                   long long right_value,
                                   long long limit_value,
                                   long long *result)
{
    if (left_value < 0LL || right_value < 0LL || limit_value < 0LL) {
        fail_with_context("weighted exact objective", "negative value in overflow guard");
    }
    if (left_value > limit_value - right_value) {
        return 0;
    }
    *result = left_value + right_value;
    return 1;
}

static void signed_big_score_zero(SignedBigScore *score)
{
    big_uint_zero(&score->positive);
    big_uint_zero(&score->negative);
}

static void signed_big_score_add_scaled(SignedBigScore *score,
                                        const BigUInt *base,
                                        unsigned long long scale,
                                        int negative)
{
    if (scale == 0ULL || base->length == 0) {
        return;
    }
    if (negative) {
        big_uint_add_scaled_u64(&score->negative, base, scale);
    } else {
        big_uint_add_scaled_u64(&score->positive, base, scale);
    }
}

static int signed_big_score_compare(const SignedBigScore *left,
                                    const SignedBigScore *right)
{
    BigUInt left_total;
    BigUInt right_total;
    int comparison;
    if (active_profile != NULL) {
        active_profile->biguint_score_comparisons++;
    }
    left_total = left->positive;
    right_total = right->positive;
    big_uint_add_big(&left_total, &right->negative);
    big_uint_add_big(&right_total, &left->negative);
    comparison = big_uint_compare(&left_total, &right_total);
    if (comparison < 0) {
        return -1;
    }
    if (comparison > 0) {
        return 1;
    }
    return 0;
}

static unsigned long long gcd_u64(unsigned long long left, unsigned long long right)
{
    while (right != 0ULL) {
        unsigned long long remainder = left % right;
        left = right;
        right = remainder;
    }
    return left;
}

static unsigned long long big_uint_mod_u64(const BigUInt *value,
                                           unsigned long long divisor)
{
    unsigned long long remainder = 0ULL;
    int limb_index;
    if (divisor == 0ULL) {
        fail_with_context("exact average fill", "division by zero");
    }
    for (limb_index = value->length - 1; limb_index >= 0; limb_index--) {
        __uint128_t combined =
            ((__uint128_t)remainder << BIG_BASE_BITS) |
            (__uint128_t)value->limbs[limb_index];
        remainder = (unsigned long long)(combined % divisor);
    }
    return remainder;
}

static void big_uint_divide_u64_exact(BigUInt *value, unsigned long long divisor)
{
    unsigned long long remainder = 0ULL;
    int limb_index;
    if (divisor == 0ULL) {
        fail_with_context("exact average fill", "division by zero");
    }
    for (limb_index = value->length - 1; limb_index >= 0; limb_index--) {
        __uint128_t combined =
            ((__uint128_t)remainder << BIG_BASE_BITS) |
            (__uint128_t)value->limbs[limb_index];
        value->limbs[limb_index] = (unsigned int)(combined / divisor);
        remainder = (unsigned long long)(combined % divisor);
    }
    if (remainder != 0ULL) {
        fail_with_context("exact average fill", "non-exact integer division");
    }
    big_uint_normalize(value);
}

static ExactAverageContext exact_average_context_create(const ProblemData *problem_data)
{
    ExactAverageContext context;
    BigUInt common_denominator;
    int lab_index;
    int term_index;
    context.term_count = 0;
    context.term_capacities =
        checked_malloc((size_t)problem_data->lab_count * sizeof(long long));
    context.term_by_lab =
        checked_malloc((size_t)problem_data->lab_count * sizeof(int));
    context.fill_weights = NULL;
    big_uint_zero(&context.common_denominator);
    context.positive_lab_count = 0;
    context.use_weighted_scalar = 0;
    context.average_fill_weight = 0LL;

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        long long capacity_value = problem_data->labs[lab_index].capacity_value;

        context.term_by_lab[lab_index] = -1;
        if (capacity_value <= 0LL) {
            continue;
        }
        context.positive_lab_count++;

        for (term_index = 0; term_index < context.term_count; term_index++) {
            if (context.term_capacities[term_index] == capacity_value) {
                break;
            }
        }
        if (term_index == context.term_count) {
            context.term_capacities[context.term_count] = capacity_value;
            context.term_count++;
        }
        context.term_by_lab[lab_index] = term_index;
    }

    context.fill_weights =
        checked_calloc((size_t)context.term_count, sizeof(BigUInt));
    big_uint_set_u64(&common_denominator, 1ULL);
    for (term_index = 0; term_index < context.term_count; term_index++) {
        unsigned long long capacity_value =
            (unsigned long long)context.term_capacities[term_index];
        unsigned long long common_divisor =
            gcd_u64(big_uint_mod_u64(&common_denominator, capacity_value),
                    capacity_value);
        big_uint_divide_u64_exact(&common_denominator, common_divisor);
        big_uint_multiply_u64(&common_denominator, capacity_value);
    }
    for (term_index = 0; term_index < context.term_count; term_index++) {
        context.fill_weights[term_index] = common_denominator;
        big_uint_divide_u64_exact(
            &context.fill_weights[term_index],
            (unsigned long long)context.term_capacities[term_index]);
    }
    context.common_denominator = common_denominator;
    return context;
}

static void exact_average_context_free(ExactAverageContext *context)
{
    free(context->fill_weights);
    free(context->term_capacities);
    free(context->term_by_lab);
    context->fill_weights = NULL;
    context->term_capacities = NULL;
    context->term_by_lab = NULL;
    context->term_count = 0;
    big_uint_zero(&context->common_denominator);
    context->positive_lab_count = 0;
    context->use_weighted_scalar = 0;
    context->average_fill_weight = 0LL;
}

static int exact_average_compare_two_coefficients(const ExactAverageContext *context,
                                                  const int *left_coefficients,
                                                  const int *right_coefficients)
{
    BigUInt positive_sum;
    BigUInt negative_sum;
    int term_index;
    if (left_coefficients == right_coefficients ||
        memcmp(left_coefficients,
               right_coefficients,
               (size_t)context->term_count * sizeof(int)) == 0) {
        return 0;
    }
    big_uint_zero(&positive_sum);
    big_uint_zero(&negative_sum);
    for (term_index = 0; term_index < context->term_count; term_index++) {
        long long coefficient_difference =
            (long long)left_coefficients[term_index] -
            (long long)right_coefficients[term_index];
        if (coefficient_difference > 0LL) {
            big_uint_add_scaled_u64(&positive_sum,
                                    &context->fill_weights[term_index],
                                    (unsigned long long)coefficient_difference);
        } else if (coefficient_difference < 0LL) {
            big_uint_add_scaled_u64(&negative_sum,
                                    &context->fill_weights[term_index],
                                    (unsigned long long)(-coefficient_difference));
        }
    }
    return big_uint_compare(&positive_sum, &negative_sum);
}

static ExactPathCost exact_path_cost_make(Cost primary_cost)
{
    ExactPathCost cost_value;
    cost_value.primary_cost = primary_cost;
    return cost_value;
}

static ExactPathCost exact_path_cost_zero(void)
{
    return exact_path_cost_make(cost_zero());
}

static ExactPathCost exact_path_cost_infinity(void)
{
    return exact_path_cost_make(cost_infinity());
}

static ExactPathCost exact_path_cost_add(ExactPathCost left, ExactPathCost right)
{
    return exact_path_cost_make(cost_add(left.primary_cost, right.primary_cost));
}

static ExactPathCost exact_path_cost_subtract(ExactPathCost left, ExactPathCost right)
{
    return exact_path_cost_make(cost_subtract(left.primary_cost, right.primary_cost));
}

static ExactPathCost exact_path_cost_from_edge(const McfEdge *edge)
{
    Cost primary_cost = edge->cost;
    return exact_path_cost_make(primary_cost);
}

static int exact_path_cost_is_infinity(ExactPathCost cost_value)
{
    Cost infinite_cost = cost_infinity();
    return cost_equal(cost_value.primary_cost, infinite_cost);
}

static int exact_path_cost_less(const ExactAverageContext *context,
                                const ExactPathCost *left,
                                const int *left_coefficients,
                                const ExactPathCost *right,
                                const int *right_coefficients)
{
    if (active_profile != NULL) {
        active_profile->exact_path_cost_comparisons++;
    }
    if (left->primary_cost.first != right->primary_cost.first) {
        return left->primary_cost.first < right->primary_cost.first;
    }
    if (context->use_weighted_scalar) {
        SignedBigScore difference;
        long long additive_difference =
            left->primary_cost.second - right->primary_cost.second;
        int term_index;
        signed_big_score_zero(&difference);
        if (additive_difference != 0LL) {
            unsigned long long scale =
                absolute_long_long_as_u64(additive_difference,
                                          "weighted exact objective");
            scale = checked_multiply_u64(scale,
                                         (unsigned long long)context->positive_lab_count,
                                         "weighted exact objective");
            signed_big_score_add_scaled(&difference,
                                        &context->common_denominator,
                                        scale,
                                        additive_difference < 0LL);
        }
        for (term_index = 0; term_index < context->term_count; term_index++) {
            long long coefficient_difference =
                (long long)left_coefficients[term_index] -
                (long long)right_coefficients[term_index];
            long long weighted_difference;
            unsigned long long scale;
            if (coefficient_difference == 0LL ||
                context->average_fill_weight == 0LL) {
                continue;
            }
            if (coefficient_difference > 0LL &&
                context->average_fill_weight > LLONG_MAX / coefficient_difference) {
                fail_with_context("weighted exact objective", "integer scale overflow");
            }
            if (coefficient_difference < 0LL &&
                context->average_fill_weight > LLONG_MAX / -coefficient_difference) {
                fail_with_context("weighted exact objective", "integer scale overflow");
            }
            weighted_difference = coefficient_difference * context->average_fill_weight;
            scale = absolute_long_long_as_u64(weighted_difference,
                                             "weighted exact objective");
            signed_big_score_add_scaled(&difference,
                                        &context->fill_weights[term_index],
                                        scale,
                                        weighted_difference < 0LL);
        }
        {
            int comparison;
            SignedBigScore zero_score;
            signed_big_score_zero(&zero_score);
            comparison = signed_big_score_compare(&difference, &zero_score);
            if (comparison != 0) {
                return comparison < 0;
            }
        }
        if (left->primary_cost.second != right->primary_cost.second) {
            return left->primary_cost.second < right->primary_cost.second;
        }
        if (left->primary_cost.third != right->primary_cost.third) {
            return left->primary_cost.third < right->primary_cost.third;
        }
        return exact_average_compare_two_coefficients(context,
                                                      left_coefficients,
                                                      right_coefficients) < 0;
    }
    if (left->primary_cost.second != right->primary_cost.second) {
        return left->primary_cost.second < right->primary_cost.second;
    }
    if (left->primary_cost.third != right->primary_cost.third) {
        return left->primary_cost.third < right->primary_cost.third;
    }
    return exact_average_compare_two_coefficients(context,
                                                  left_coefficients,
                                                  right_coefficients) < 0;
}

static int *coefficient_row(int *coefficients, int coefficient_count, int node)
{
    return coefficients + (size_t)node * (size_t)coefficient_count;
}

static const int *const_coefficient_row(const int *coefficients,
                                        int coefficient_count,
                                        int node)
{
    return coefficients + (size_t)node * (size_t)coefficient_count;
}

static void copy_coefficients(int *target, const int *source, int coefficient_count)
{
    memcpy(target, source, (size_t)coefficient_count * sizeof(int));
}

static void add_coefficients(int *target, const int *source, int coefficient_count)
{
    int coefficient_index;
    for (coefficient_index = 0;
         coefficient_index < coefficient_count;
         coefficient_index++) {
        target[coefficient_index] += source[coefficient_index];
    }
}

static void build_edge_adjusted_coefficients(int *target,
                                             const int *base_coefficients,
                                             const McfEdge *edge,
                                             int coefficient_count)
{
    copy_coefficients(target, base_coefficients, coefficient_count);
    if (edge->fill_lab_index >= 0) {
        target[edge->fill_lab_index] += edge->fill_delta;
    }
}

static long double square_root_long_double(long double value)
{
    long double estimate;
    int iteration;
    if (value <= 0.0L) {
        return 0.0L;
    }
    estimate = value >= 1.0L ? value : 1.0L;
    for (iteration = 0; iteration < 80; iteration++) {
        estimate = (estimate + value / estimate) / 2.0L;
    }
    return estimate;
}

static long double elapsed_cpu_seconds(clock_t start_clock, clock_t end_clock)
{
    if (start_clock == (clock_t)-1 || end_clock == (clock_t)-1) {
        return -1.0L;
    }
    return (long double)(end_clock - start_clock) / (long double)CLOCKS_PER_SEC;
}

static void solver_profile_init(SolverProfile *profile)
{
    memset(profile, 0, sizeof(*profile));
}

static void solver_profile_note_dinic_graph(int node_count)
{
    if (active_profile != NULL && node_count > active_profile->max_dinic_nodes) {
        active_profile->max_dinic_nodes = node_count;
    }
}

static void solver_profile_note_mcf_graph(int node_count)
{
    if (active_profile != NULL && node_count > active_profile->max_mcf_nodes) {
        active_profile->max_mcf_nodes = node_count;
    }
}

static void add_capacity_saturating(long long *total_capacity,
                                    long long capacity_value,
                                    long long saturation_limit)
{
    if (*total_capacity >= saturation_limit) {
        return;
    }
    if (capacity_value > saturation_limit - *total_capacity) {
        *total_capacity = saturation_limit;
    } else {
        *total_capacity += capacity_value;
    }
}

static WideProduct multiply_small_nonnegative(long long small_value, long long large_value)
{
    const unsigned long long lower_mask = 0xffffffffULL;
    unsigned long long small_part = (unsigned long long)small_value;
    unsigned long long large_part = (unsigned long long)large_value;
    unsigned long long low_product = (large_part & lower_mask) * small_part;
    unsigned long long high_product = (large_part >> 32) * small_part;
    unsigned long long shifted_low = high_product << 32;
    WideProduct product;

    product.low = low_product + shifted_low;
    product.high = (high_product >> 32) + (product.low < low_product ? 1ULL : 0ULL);
    return product;
}

static int wide_product_compare(WideProduct left, WideProduct right)
{
    if (left.high < right.high) {
        return -1;
    }
    if (left.high > right.high) {
        return 1;
    }
    if (left.low < right.low) {
        return -1;
    }
    if (left.low > right.low) {
        return 1;
    }
    return 0;
}

static void *checked_malloc(size_t byte_count)
{
    if (byte_count == 0U) {
        byte_count = 1U;
    }
    void *pointer = malloc(byte_count);
    if (pointer == NULL) {
        fprintf(stderr, "error: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return pointer;
}

static void *checked_calloc(size_t item_count, size_t item_size)
{
    if (item_count == 0U) {
        item_count = 1U;
    }
    if (item_size == 0U) {
        item_size = 1U;
    }
    void *pointer = calloc(item_count, item_size);
    if (pointer == NULL) {
        fprintf(stderr, "error: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return pointer;
}

static void *checked_realloc(void *pointer, size_t byte_count)
{
    if (byte_count == 0U) {
        byte_count = 1U;
    }
    void *new_pointer = realloc(pointer, byte_count);
    if (new_pointer == NULL) {
        fprintf(stderr, "error: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return new_pointer;
}

static size_t checked_multiply_size(size_t left, size_t right, const char *context)
{
    if (left != 0U && right > (size_t)-1 / left) {
        fail_with_context(context, "size calculation overflow");
    }
    return left * right;
}

static char *duplicate_text(const char *text)
{
    size_t length = strlen(text);
    char *copy = checked_malloc(length + 1U);
    memcpy(copy, text, length + 1U);
    return copy;
}

static void remove_cleanup_paths(void)
{
    if (cleanup_output_path != NULL) {
        (void)remove(cleanup_output_path);
    }
    if (cleanup_metrics_output_path != NULL) {
        (void)remove(cleanup_metrics_output_path);
    }
    if (cleanup_lab_report_output_path != NULL) {
        (void)remove(cleanup_lab_report_output_path);
    }
    if (cleanup_student_report_output_path != NULL) {
        (void)remove(cleanup_student_report_output_path);
    }
    if (cleanup_outside_report_output_path != NULL) {
        (void)remove(cleanup_outside_report_output_path);
    }
    if (cleanup_reasons_report_output_path != NULL) {
        (void)remove(cleanup_reasons_report_output_path);
    }
    if (cleanup_adjustment_report_output_path != NULL) {
        (void)remove(cleanup_adjustment_report_output_path);
    }
    if (cleanup_portfolio_report_output_path != NULL) {
        (void)remove(cleanup_portfolio_report_output_path);
    }
    if (cleanup_profile_output_path != NULL) {
        (void)remove(cleanup_profile_output_path);
    }
    if (cleanup_explanation_report_output_path != NULL) {
        (void)remove(cleanup_explanation_report_output_path);
    }
    if (cleanup_target_status_output_path != NULL) {
        (void)remove(cleanup_target_status_output_path);
    }
}

static NORETURN void fail_with_message(const char *message)
{
    remove_cleanup_paths();
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static NORETURN void fail_with_context(const char *context, const char *detail)
{
    remove_cleanup_paths();
    fprintf(stderr, "error: %s: %s\n", context, detail);
    exit(EXIT_FAILURE);
}

static NORETURN void fail_with_context_format(const char *context, const char *format, ...)
{
    va_list arguments;

    remove_cleanup_paths();
    fprintf(stderr, "error: %s: ", context);
    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    va_end(arguments);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static NORETURN void fail_with_context_format_hint(const char *context,
                                                   const char *hint,
                                                   const char *format,
                                                   ...)
{
    va_list arguments;

    remove_cleanup_paths();
    fprintf(stderr, "error: %s: ", context);
    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    va_end(arguments);
    fprintf(stderr, "\n");
    if (hint != NULL) {
        fprintf(stderr, "hint: %s\n", hint);
    }
    exit(EXIT_FAILURE);
}

static void print_usage(const char *program_name)
{
    fprintf(stderr,
            "usage: %s LAB_FILE PREFERENCE_FILE OUTPUT_FILE [options]\n",
            program_name);
}

static void print_help(const char *program_name)
{
    printf("usage: %s LAB_FILE PREFERENCE_FILE OUTPUT_FILE [options]\n",
           program_name);
    printf("\n");
    printf("Arguments:\n");
    printf("  LAB_FILE          laboratory list file\n");
    printf("  PREFERENCE_FILE   student preference list file\n");
    printf("  OUTPUT_FILE       student-to-laboratory assignment output file\n");
    printf("\n");
    printf("Options:\n");
    printf("  --reports         also write metrics and human-readable reports\n");
    printf("  --profile         also write solver profile counters and phase timings\n");
    printf("  --quiet           suppress success summary on stdout\n");
    printf("  --portfolio       solve light exact objective portfolio\n");
    printf("  --portfolio-deep  include fill-focused, fill-convex, and weighted-exact candidates\n");
    printf("  --portfolio-summary-only  keep only final output and portfolio summary files\n");
    printf("  --keep-candidate-files    keep per-candidate portfolio assignment files\n");
    printf("  --jobs N          run portfolio candidates in parallel processes when available\n");
    printf("  --id-policy MODE  student id policy: auto, assignment5, numeric, or token\n");
    printf("  --student-id-width N|auto  zero-padding width for numeric ids, default auto\n");
    printf("  --interactive     allow guided y/N confirmation for --id-policy auto\n");
    printf("  --no-interactive  never prompt; require explicit id policy when ambiguous\n");
    printf("  --assume-yes      accept --id-policy auto detection without prompting\n");
    printf("  --objective MODE  objective mode: rubric, satisfaction, fair, balanced, guarded, fill-convex, or weighted-exact\n");
    printf("  --max-rank-slack N       guarded mode rank slack from minimum feasible max-rank\n");
    printf("  --rank-costs FILE        rank cost table for satisfaction mode\n");
    printf("  --weights FILE           weighted-exact weight preset file\n");
    printf("  --targets FILE           hard target constraint file\n");
    printf("  --require-average-rank-at-most X  require average assigned rank <= X\n");
    printf("  --require-rank-sum-at-most N      require rank_sum <= N\n");
    printf("  --require-rank-square-at-most N   reserved; parsed but not yet exact\n");
    printf("  --require-max-rank-at-most K      require max assigned rank <= K\n");
    printf("  --require-minimum-fill-at-least X require minimum fill rate >= X or percent\n");
    printf("  --require-average-fill-at-least X require exact average fill >= X when safely supported\n");
    printf("  --require-no-outside             require no outside-preference assignment\n");
    printf("  --require-outside-at-most N       require outside-preference count <= N (exactly supports 0)\n");
    printf("  --first-choice-gap N     satisfaction cost for rank 2, default 100\n");
    printf("  --rank-tail-linear N     linear tail cost after rank 2, default 30\n");
    printf("  --rank-tail-quadratic N  quadratic tail cost after rank 2, default 5\n");
    printf("  --outside-cost N         satisfaction cost for outside preference, default 10000\n");
    printf("  --constraints FILE       optional lock/forbid/allow/capacity constraint file\n");
    printf("  --base-assignment FILE   previous assignment output used for change penalties\n");
    printf("  --change-penalty N       additive penalty for moving a student from base assignment\n");
    printf("  --explain-student ID     exact counterfactual explanation for one student\n");
    printf("  --try-lock ID:LAB        exact counterfactual by locking one student to one lab\n");
    printf("  --weight-rank-sum N       weighted-exact rank-sum weight\n");
    printf("  --weight-rank-square N    weighted-exact rank-square weight\n");
    printf("  --weight-max-rank N       weighted-exact max-rank weight\n");
    printf("  --weight-average-fill N   weighted-exact average-fill reward weight\n");
    printf("  --weight-minimum-fill N   weighted-exact minimum-fill reward weight\n");
    printf("  --weight-outside N        weighted-exact outside-preference penalty\n");
    printf("  --weight-change N         weighted-exact moved-from-base penalty\n");
    printf("  --print-objectives        list objective modes and exit\n");
    printf("  --print-presets           list bundled rank-cost and weighted preset files\n");
    printf("  --print-rank-costs        print default rank-cost table sample and exit\n");
    printf("  --explain-weights [FILE]  top-level command: explain weighted-exact terms and optional preset file\n");
    printf("  --help            show this help message\n");
}

static void print_objectives(void)
{
    printf("rubric\n");
    printf("  R1 -> R2 -> Q -> Uavg -> Umin; evaluation-metric aligned default.\n");
    printf("satisfaction\n");
    printf("  D1 -> D2 -> Q -> Uavg -> Umin; uses configurable dissatisfaction cost d(rank).\n");
    printf("fair\n");
    printf("  Q -> R1 -> R2 -> Umin -> Uavg; protects worst assigned rank first.\n");
    printf("balanced\n");
    printf("  R1 -> R2 -> Q -> Umin -> Uavg; rank-first with minimum-fill tie-break.\n");
    printf("guarded\n");
    printf("  First constrains Q <= Qmin + slack, then uses rubric order.\n");
    printf("fill-convex\n");
    printf("  Exact min-cost-flow objective combining rank cost with separable convex lab-balance penalty.\n");
    printf("weighted-exact\n");
    printf("  Exact scalar score: w1*R1 + w2*R2 + w3*Q - w4*Uavg - w5*Umin + w6*outside + w7*change.\n");
}

static void print_presets(void)
{
    printf("rank-cost presets for --objective satisfaction:\n");
    printf("  rank_costs/student_friendly.txt\n");
    printf("    Strong first-choice preference, larger tail penalties, high outside-preference cost.\n");
    printf("  rank_costs/balanced_satisfaction.txt\n");
    printf("    Moderate first-choice preference and smoother rank tail.\n");
    printf("  rank_costs/strict_outside_avoidance.txt\n");
    printf("    Keeps ordinary rank costs moderate but makes outside-preference assignments very expensive.\n");
    printf("\n");
    printf("weighted-exact presets for --objective weighted-exact:\n");
    printf("  weights/evaluation_balance.txt\n");
    printf("    Balanced scalar weights for rank, worst rank, fill rates, and outside-preference count.\n");
    printf("  weights/rank_only.txt\n");
    printf("    Integer-only rank/outside weighted objective; fastest weighted-exact preset.\n");
    printf("  weights/fill_balance.txt\n");
    printf("    Fill-rate-heavy exact weighted objective; may use threshold search.\n");
    printf("  weights/minimum_change_adjustment.txt\n");
    printf("    Designed for --base-assignment runs where changed_students should be expensive.\n");
    printf("\n");
    printf("inspection commands:\n");
    printf("  ./assign_labs --print-objectives\n");
    printf("  ./assign_labs --print-presets\n");
    printf("  ./assign_labs --print-rank-costs\n");
    printf("  ./assign_labs --explain-weights weights/evaluation_balance.txt\n");
}

static void print_default_rank_costs(void)
{
    RankCostModel model;
    int rank_value;
    rank_cost_model_init_default(&model);
    printf("# Default satisfaction rank-cost sample\n");
    printf("# Formula: d(1)=0; d(r)=A+B(r-2)+C(r-2)^2 for r>=2; outside=O\n");
    printf("# A=%lld B=%lld C=%lld O=%lld\n",
           model.first_choice_gap,
           model.tail_linear,
           model.tail_quadratic,
           model.outside_cost);
    for (rank_value = 1;
         rank_value <= PRINT_RANK_COST_SAMPLE_LIMIT;
         rank_value++) {
        printf("rank %d %lld\n", rank_value, rank_cost_formula_value(&model, rank_value));
    }
    printf("outside %lld\n", model.outside_cost);
}

static void print_weighted_objective_explanation(const WeightedObjective *weights,
                                                 const char *weights_path)
{
    if (weights_path != NULL) {
        printf("weights file: %s\n", weights_path);
    } else {
        printf("weights file: (default built-in weights)\n");
    }
    printf("weighted-exact score:\n");
    printf("  +%lld * rank_sum\n", weights->rank_sum);
    printf("  +%lld * rank_square_sum\n", weights->rank_square);
    printf("  +%lld * max_rank\n", weights->max_rank);
    printf("  -%lld * average_fill_rate\n", weights->average_fill);
    printf("  -%lld * minimum_fill_rate\n", weights->minimum_fill);
    printf("  +%lld * outside_preference_count\n", weights->outside);
    printf("  +%lld * changed_students_from_base\n", weights->change);
    printf("\n");
    printf("Additive terms are rank_sum, rank_square_sum, outside_preference_count, and changed_students.\n");
    printf("average_fill_rate is additive after exact rational scaling when possible.\n");
    printf("Global terms max_rank and minimum_fill_rate require threshold search; nonzero weights can make weighted-exact slower.\n");
    printf("Configuration values are nonnegative integers up to %lld.\n",
           MAX_CONFIG_WEIGHT);
    printf("\n");
    printf("complexity notes:\n");
    if (weights->minimum_fill == 0LL && weights->average_fill == 0LL) {
        printf("  expected mode: integer-only weighted min-cost flow; relatively fast.\n");
    } else if (weights->max_rank == 0LL && weights->minimum_fill == 0LL) {
        printf("  expected mode: additive weighted min-cost flow with exact average-fill scaling; usually moderate.\n");
    } else {
        printf("  expected mode: heavy exact weighted search because max_rank and/or minimum_fill use threshold axes.\n");
    }
    if (weights->max_rank != 0LL) {
        printf("  max_rank requires rank-threshold search.\n");
    }
    if (weights->minimum_fill != 0LL) {
        printf("  minimum_fill_rate requires fill-threshold search.\n");
    }
    if (weights->average_fill != 0LL) {
        printf("  average_fill_rate uses exact rational scaling or BigUInt fallback.\n");
    }
}

static void fast_input_open(FastInput *input, const char *path)
{
    input->file = fopen(path, "rb");
    input->path = path;
    input->buffer = NULL;
    input->buffer_size = FAST_INPUT_BUFFER_SIZE;
    input->position = 0U;
    input->length = 0U;
    input->reached_eof = 0;
    if (input->file == NULL) {
        fail_with_context(path, "cannot open file");
    }
    input->buffer = checked_malloc(input->buffer_size);
}

static void fast_input_close(FastInput *input)
{
    const char *path = input->path;
    int close_status = fclose(input->file);
    free(input->buffer);
    input->file = NULL;
    input->path = NULL;
    input->buffer = NULL;
    input->buffer_size = 0U;
    input->position = 0U;
    input->length = 0U;
    input->reached_eof = 0;
    if (close_status != 0) {
        fail_with_context(path, "close failed");
    }
}

static int fast_input_getc(FastInput *input)
{
    if (input->position >= input->length) {
        if (input->reached_eof) {
            return EOF;
        }
        input->length = fread(input->buffer,
                              1U,
                              input->buffer_size,
                              input->file);
        input->position = 0U;
        if (input->length == 0U) {
            if (ferror(input->file)) {
                fail_with_context(input->path, "read failed");
            }
            input->reached_eof = 1;
            return EOF;
        }
    }
    return (int)input->buffer[input->position++];
}

static char *fast_read_line_dynamic(FastInput *input)
{
    size_t capacity = 256U;
    size_t length = 0U;
    char *buffer = checked_malloc(capacity);
    int character_value;

    while ((character_value = fast_input_getc(input)) != EOF) {
        if (length + 1U >= capacity) {
            if (capacity > (size_t)-1 / 2U) {
                free(buffer);
                fail_with_context("input line", "line is too long");
            }
            capacity *= 2U;
            buffer = checked_realloc(buffer, capacity);
        }
        buffer[length] = (char)character_value;
        length++;
        if (character_value == '\n') {
            break;
        }
    }

    if (length == 0U && character_value == EOF) {
        free(buffer);
        return NULL;
    }

    buffer[length] = '\0';
    return buffer;
}

static int line_is_blank(const char *line)
{
    const unsigned char *position = (const unsigned char *)line;
    while (*position != '\0') {
        if (!isspace(*position)) {
            return 0;
        }
        position++;
    }
    return 1;
}

static char *read_required_data_line(FastInput *input, const char *context)
{
    char *line = fast_read_line_dynamic(input);
    if (line == NULL) {
        fail_with_context(context, "missing line");
    }
    if (line_is_blank(line)) {
        free(line);
        fail_with_context(context, "blank lines are not valid data lines");
    }
    return line;
}

static void tokenizer_init(Tokenizer *tokenizer, char *line)
{
    tokenizer->cursor = line;
}

static char *next_token(Tokenizer *tokenizer)
{
    char *start;

    while (*(tokenizer->cursor) != '\0' &&
           isspace((unsigned char)*(tokenizer->cursor))) {
        tokenizer->cursor++;
    }

    if (*(tokenizer->cursor) == '\0') {
        return NULL;
    }

    start = tokenizer->cursor;
    while (*(tokenizer->cursor) != '\0' &&
           !isspace((unsigned char)*(tokenizer->cursor))) {
        tokenizer->cursor++;
    }

    if (*(tokenizer->cursor) != '\0') {
        *(tokenizer->cursor) = '\0';
        tokenizer->cursor++;
    }

    return start;
}

static const char *without_utf8_bom(const char *text)
{
    const unsigned char *bytes = (const unsigned char *)text;
    if (bytes[0] == 0xEFU && bytes[1] == 0xBBU && bytes[2] == 0xBFU) {
        return text + 3;
    }
    return text;
}

static long long parse_long_long_range(const char *text,
                                       long long minimum_value,
                                       long long maximum_value,
                                       const char *context)
{
    char *end_pointer;
    long long parsed_value;
    const char *number_text = without_utf8_bom(text);

    errno = 0;
    parsed_value = strtoll(number_text, &end_pointer, 10);
    if (errno != 0 || end_pointer == number_text || *end_pointer != '\0' ||
        parsed_value < minimum_value || parsed_value > maximum_value) {
        fail_with_context(context, "invalid integer");
    }
    return parsed_value;
}

static int parse_int_range(const char *text,
                           int minimum_value,
                           int maximum_value,
                           const char *context)
{
    long long parsed_value = parse_long_long_range(text,
                                                   (long long)minimum_value,
                                                   (long long)maximum_value,
                                                   context);
    return (int)parsed_value;
}

static unsigned long long gcd_nonzero_u64(unsigned long long left,
                                          unsigned long long right)
{
    while (right != 0ULL) {
        unsigned long long remainder = left % right;
        left = right;
        right = remainder;
    }
    return left == 0ULL ? 1ULL : left;
}

static RatioValue ratio_value_reduce(RatioValue ratio)
{
    unsigned long long divisor;
    if (ratio.denominator <= 0LL || ratio.numerator < 0LL) {
        fail_with_context("ratio", "invalid ratio value");
    }
    if (ratio.numerator == 0LL) {
        ratio.denominator = 1LL;
        return ratio;
    }
    divisor = gcd_nonzero_u64((unsigned long long)ratio.numerator,
                              (unsigned long long)ratio.denominator);
    ratio.numerator = (long long)((unsigned long long)ratio.numerator / divisor);
    ratio.denominator = (long long)((unsigned long long)ratio.denominator / divisor);
    return ratio;
}

static RatioValue parse_ratio_bound(const char *text, const char *context)
{
    const char *number_text = without_utf8_bom(text);
    size_t length = strlen(number_text);
    const char *slash_pointer;
    int has_percent = 0;
    int has_digit = 0;
    int seen_decimal = 0;
    long long numerator = 0LL;
    long long denominator = 1LL;
    size_t index_value;
    RatioValue ratio;

    if (length == 0U) {
        fail_with_context(context, "invalid numeric target");
    }
    slash_pointer = strchr(number_text, '/');
    if (slash_pointer != NULL) {
        char *end_pointer;
        long long numerator_value;
        long long denominator_value;
        if (strchr(slash_pointer + 1, '/') != NULL) {
            fail_with_context(context, "invalid ratio target");
        }
        errno = 0;
        numerator_value = strtoll(number_text, &end_pointer, 10);
        if (errno != 0 || end_pointer != slash_pointer || numerator_value < 0LL) {
            fail_with_context(context, "invalid ratio target numerator");
        }
        errno = 0;
        denominator_value = strtoll(slash_pointer + 1, &end_pointer, 10);
        if (errno != 0 || end_pointer == slash_pointer + 1 ||
            *end_pointer != '\0' || denominator_value <= 0LL) {
            fail_with_context(context, "invalid ratio target denominator");
        }
        ratio.numerator = numerator_value;
        ratio.denominator = denominator_value;
        return ratio_value_reduce(ratio);
    }
    if (number_text[length - 1U] == '%') {
        has_percent = 1;
        length--;
        if (length == 0U) {
            fail_with_context(context, "invalid percent target");
        }
    }

    for (index_value = 0U; index_value < length; index_value++) {
        unsigned char character = (unsigned char)number_text[index_value];
        if (character == '.') {
            if (seen_decimal) {
                fail_with_context(context, "invalid decimal target");
            }
            seen_decimal = 1;
            continue;
        }
        if (!isdigit(character)) {
            fail_with_context(context, "invalid numeric target");
        }
        has_digit = 1;
        if (numerator > (LLONG_MAX - (long long)(character - '0')) / 10LL) {
            fail_with_context(context, "numeric target is too large");
        }
        numerator = numerator * 10LL + (long long)(character - '0');
        if (seen_decimal) {
            if (denominator > LLONG_MAX / 10LL) {
                fail_with_context(context, "numeric target has too many decimal places");
            }
            denominator *= 10LL;
        }
    }
    if (!has_digit) {
        fail_with_context(context, "invalid numeric target");
    }
    if (has_percent) {
        if (denominator > LLONG_MAX / 100LL) {
            fail_with_context(context, "percent target is too precise");
        }
        denominator *= 100LL;
    }
    ratio.numerator = numerator;
    ratio.denominator = denominator;
    return ratio_value_reduce(ratio);
}

static void target_constraints_init(TargetConstraints *targets)
{
    memset(targets, 0, sizeof(*targets));
    targets->average_rank_max.denominator = 1LL;
    targets->average_fill_min.denominator = 1LL;
    targets->minimum_fill_min.denominator = 1LL;
}

static int target_constraints_are_empty(const TargetConstraints *targets)
{
    if (targets == NULL) {
        return 1;
    }
    return !targets->has_average_rank_max &&
           !targets->has_average_fill_min &&
           !targets->has_rank_sum_max &&
           !targets->has_rank_square_max &&
           !targets->has_max_rank_max &&
           !targets->has_minimum_fill_min &&
           !targets->has_outside_max;
}

static void target_constraints_set_average_rank_max(TargetConstraints *targets,
                                                    RatioValue bound)
{
    if (targets->has_average_rank_max &&
        ratio_compare_value(bound, targets->average_rank_max) > 0) {
        return;
    }
    targets->has_average_rank_max = 1;
    targets->average_rank_max = ratio_value_reduce(bound);
}

static void target_constraints_set_minimum_fill_min(TargetConstraints *targets,
                                                    RatioValue bound)
{
    if (targets->has_minimum_fill_min &&
        ratio_compare_value(bound, targets->minimum_fill_min) < 0) {
        return;
    }
    targets->has_minimum_fill_min = 1;
    targets->minimum_fill_min = ratio_value_reduce(bound);
}

static void target_constraints_set_average_fill_min(TargetConstraints *targets,
                                                    RatioValue bound)
{
    if (targets->has_average_fill_min &&
        ratio_compare_value(bound, targets->average_fill_min) < 0) {
        return;
    }
    targets->has_average_fill_min = 1;
    targets->average_fill_min = ratio_value_reduce(bound);
}

static void target_constraints_set_rank_sum_max(TargetConstraints *targets,
                                                long long bound)
{
    if (targets->has_rank_sum_max && bound > targets->rank_sum_max) {
        return;
    }
    targets->has_rank_sum_max = 1;
    targets->rank_sum_max = bound;
}

static void target_constraints_set_rank_square_max(TargetConstraints *targets,
                                                   long long bound)
{
    if (targets->has_rank_square_max && bound > targets->rank_square_max) {
        return;
    }
    targets->has_rank_square_max = 1;
    targets->rank_square_max = bound;
}

static void target_constraints_set_max_rank_max(TargetConstraints *targets,
                                                int bound)
{
    if (targets->has_max_rank_max && bound > targets->max_rank_max) {
        return;
    }
    targets->has_max_rank_max = 1;
    targets->max_rank_max = bound;
}

static void target_constraints_set_outside_max(TargetConstraints *targets,
                                               int bound)
{
    if (targets->has_outside_max && bound > targets->outside_max) {
        return;
    }
    targets->has_outside_max = 1;
    targets->outside_max = bound;
}

static int text_is_digits(const char *text)
{
    size_t position;
    if (text[0] == '\0') {
        return 0;
    }
    for (position = 0U; text[position] != '\0'; position++) {
        if (!isdigit((unsigned char)text[position])) {
            return 0;
        }
    }
    return 1;
}

static int student_id_token_is_assignment5(const char *student_id)
{
    size_t length = strlen(student_id);
    if (length == 0U || length > 5U) {
        return 0;
    }
    if (!text_is_digits(student_id)) {
        return 0;
    }
    errno = 0;
    {
        char *end_pointer = NULL;
        long long numeric_value = strtoll(student_id, &end_pointer, 10);
        return errno == 0 &&
               end_pointer != student_id &&
               *end_pointer == '\0' &&
               numeric_value >= 1LL &&
               numeric_value <= 99999LL;
    }
}

static int student_id_token_is_zero_padded_assignment5(const char *student_id)
{
    return strlen(student_id) == 5U &&
           student_id_token_is_assignment5(student_id);
}

static long long parse_student_numeric_id_value(const char *student_id,
                                                const char *context)
{
    if (!text_is_digits(student_id)) {
        fail_with_context(context, "student id must contain only digits");
    }
    return parse_long_long_range(student_id, 1LL, LLONG_MAX / 4LL, context);
}

static char *canonical_numeric_student_id(const char *student_id,
                                          const char *context)
{
    char buffer[64];
    long long numeric_value =
        parse_student_numeric_id_value(student_id, context);
    int written = snprintf(buffer, sizeof(buffer), "%lld", numeric_value);
    if (written < 0 || (size_t)written >= sizeof(buffer)) {
        fail_with_context(context, "student id is too large");
    }
    return duplicate_text(buffer);
}

static char *display_numeric_student_id(const char *canonical_id,
                                        int display_width,
                                        const char *context)
{
    size_t canonical_length = strlen(canonical_id);
    size_t width = display_width > 0 ? (size_t)display_width : canonical_length;
    char *display_id;
    if (width < canonical_length) {
        width = canonical_length;
    }
    display_id = checked_malloc(width + 1U);
    if (width > canonical_length) {
        memset(display_id, '0', width - canonical_length);
    }
    memcpy(display_id + (width - canonical_length),
           canonical_id,
           canonical_length + 1U);
    (void)context;
    return display_id;
}

static int stdin_is_terminal(void)
{
#ifdef _WIN32
    return _isatty(_fileno(stdin)) != 0;
#else
    return isatty(0) != 0;
#endif
}

static int confirm_auto_student_id_policy(const char *mode_text,
                                          char **examples,
                                          int example_count)
{
    char answer[32];
    int example_index;
    fprintf(stderr,
            "warning: student identifiers need normalization or a non-default policy.\n\n");
    fprintf(stderr, "Examples:\n");
    for (example_index = 0; example_index < example_count; example_index++) {
        fprintf(stderr, "  %s\n", examples[example_index]);
    }
    fprintf(stderr,
            "\nOriginal assignment format outputs numeric IDs as 00001..99999.\n");
    fprintf(stderr,
            "Treat the first column as %s student identifiers instead? [y/N]: ",
            mode_text);
    fflush(stderr);
    if (fgets(answer, sizeof(answer), stdin) == NULL) {
        return 0;
    }
    return answer[0] == 'y' || answer[0] == 'Y';
}

static const char *student_id_policy_name(StudentIdPolicy policy)
{
    switch (policy) {
    case ID_POLICY_ASSIGNMENT5:
        return "assignment5";
    case ID_POLICY_NUMERIC:
        return "numeric";
    case ID_POLICY_TOKEN:
        return "token";
    case ID_POLICY_AUTO:
        return "auto";
    }
    return "assignment5";
}

static StudentIdPolicy parse_student_id_policy(const char *text)
{
    if (strcmp(text, "assignment5") == 0) {
        return ID_POLICY_ASSIGNMENT5;
    }
    if (strcmp(text, "numeric") == 0) {
        return ID_POLICY_NUMERIC;
    }
    if (strcmp(text, "token") == 0) {
        return ID_POLICY_TOKEN;
    }
    if (strcmp(text, "auto") == 0) {
        return ID_POLICY_AUTO;
    }
    fail_with_context_format_hint(
        "--id-policy",
        "use assignment5, numeric, token, or auto",
        "unknown id policy '%s'",
        text);
    return ID_POLICY_ASSIGNMENT5;
}

static int parse_student_id_width(const char *text)
{
    if (strcmp(text, "auto") == 0) {
        return AUTO_STUDENT_ID_WIDTH;
    }
    return parse_int_range(text, 1, 64, "--student-id-width");
}

static unsigned long long hash_text_bytes(const char *text)
{
    const unsigned char *position = (const unsigned char *)text;
    unsigned long long hash_value = FNV_OFFSET_BASIS;
    while (*position != '\0') {
        hash_value ^= (unsigned long long)(*position);
        hash_value *= FNV_PRIME;
        position++;
    }
    return hash_value;
}

static int hash_table_size_for_count(int item_count, const char *context)
{
    int hash_size = 1;
    if (item_count < 1 || item_count > INT_MAX / 4) {
        fail_with_context(context, "hash table size overflow");
    }
    while (hash_size < item_count * 4) {
        if (hash_size > INT_MAX / 2) {
            fail_with_context(context, "hash table size overflow");
        }
        hash_size *= 2;
    }
    return hash_size;
}

static void build_lab_hash_index(ProblemData *problem_data)
{
    int hash_size = hash_table_size_for_count(problem_data->lab_count,
                                              "lab hash index");
    int lab_index;
    problem_data->lab_hash_size = hash_size;
    problem_data->lab_hash_indices =
        checked_malloc((size_t)hash_size * sizeof(int));
    for (lab_index = 0; lab_index < hash_size; lab_index++) {
        problem_data->lab_hash_indices[lab_index] = -1;
    }
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int slot_index =
            (int)(hash_text_bytes(problem_data->labs[lab_index].name) &
                  (unsigned long long)(hash_size - 1));
        while (problem_data->lab_hash_indices[slot_index] >= 0) {
            slot_index = (slot_index + 1) & (hash_size - 1);
        }
        problem_data->lab_hash_indices[slot_index] = lab_index;
    }
}

static int find_lab_index(const ProblemData *problem_data, const char *lab_name)
{
    int lab_index;
    if (problem_data->lab_hash_indices != NULL && problem_data->lab_hash_size > 0) {
        int slot_index =
            (int)(hash_text_bytes(lab_name) &
                  (unsigned long long)(problem_data->lab_hash_size - 1));
        for (;;) {
            lab_index = problem_data->lab_hash_indices[slot_index];
            if (lab_index < 0) {
                return -1;
            }
            if (strcmp(problem_data->labs[lab_index].name, lab_name) == 0) {
                return lab_index;
            }
            slot_index = (slot_index + 1) & (problem_data->lab_hash_size - 1);
        }
    }
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        if (strcmp(problem_data->labs[lab_index].name, lab_name) == 0) {
            return lab_index;
        }
    }
    return -1;
}

static char *canonical_student_id_for_policy(StudentIdPolicy policy,
                                             const char *student_id,
                                             const char *context)
{
    if (policy == ID_POLICY_TOKEN) {
        if (student_id[0] == '\0') {
            fail_with_context(context, "student id must not be empty");
        }
        return duplicate_text(student_id);
    }
    if (policy == ID_POLICY_ASSIGNMENT5 && !student_id_token_is_assignment5(student_id)) {
        fail_with_context(context, "student id must be a natural number in 1..99999");
    }
    return canonical_numeric_student_id(student_id, context);
}

static int find_student_index(const ProblemData *problem_data,
                              const char *student_id,
                              const char *context)
{
    char *canonical_id =
        canonical_student_id_for_policy(problem_data->id_policy, student_id, context);
    int student_index;
    if (problem_data->student_hash_indices != NULL &&
        problem_data->student_hash_size > 0) {
        int slot_index =
            (int)(hash_text_bytes(canonical_id) &
                  (unsigned long long)(problem_data->student_hash_size - 1));
        for (;;) {
            student_index = problem_data->student_hash_indices[slot_index];
            if (student_index < 0) {
                free(canonical_id);
                return -1;
            }
            if (strcmp(problem_data->students[student_index].canonical_id,
                       canonical_id) == 0) {
                free(canonical_id);
                return student_index;
            }
            slot_index = (slot_index + 1) &
                         (problem_data->student_hash_size - 1);
        }
    }
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        if (strcmp(problem_data->students[student_index].canonical_id, canonical_id) == 0) {
            free(canonical_id);
            return student_index;
        }
    }
    free(canonical_id);
    return -1;
}

static size_t student_lab_matrix_index(const ProblemData *problem_data,
                                       int student_index,
                                       int lab_index)
{
    return (size_t)student_index * (size_t)problem_data->lab_count + (size_t)lab_index;
}

static StudentHashIndex student_hash_index_create(int student_count)
{
    StudentHashIndex index;
    int slot_index;

    index.hash_size = hash_table_size_for_count(student_count,
                                                "student hash index");
    index.indices = checked_malloc((size_t)index.hash_size * sizeof(int));
    for (slot_index = 0; slot_index < index.hash_size; slot_index++) {
        index.indices[slot_index] = -1;
    }
    return index;
}

static int student_id_already_seen_or_insert(StudentHashIndex *index,
                                             const ProblemData *problem_data,
                                             int student_index)
{
    int slot_index =
        (int)(hash_text_bytes(problem_data->students[student_index].canonical_id) &
              (unsigned long long)(index->hash_size - 1));

    for (;;) {
        int existing_index = index->indices[slot_index];
        if (existing_index < 0) {
            index->indices[slot_index] = student_index;
            return 0;
        }
        if (strcmp(problem_data->students[existing_index].canonical_id,
                   problem_data->students[student_index].canonical_id) == 0) {
            return 1;
        }
        slot_index = (slot_index + 1) & (index->hash_size - 1);
    }
}

static void build_student_hash_index(ProblemData *problem_data)
{
    StudentHashIndex index = student_hash_index_create(problem_data->student_count);
    int student_index;
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        if (student_id_already_seen_or_insert(&index, problem_data, student_index)) {
            fail_with_context_format_hint("preference file",
                                          "student identifiers are compared after normalization",
                                          "duplicate student id '%s'",
                                          problem_data->students[student_index].raw_id);
        }
    }
    problem_data->student_hash_size = index.hash_size;
    problem_data->student_hash_indices = index.indices;
}

static void remember_student_id_example(char **examples,
                                        int *example_count,
                                        const char *student_id)
{
    int example_index;
    for (example_index = 0; example_index < *example_count; example_index++) {
        if (strcmp(examples[example_index], student_id) == 0) {
            return;
        }
    }
    if (*example_count < 3) {
        examples[*example_count] = (char *)student_id;
        (*example_count)++;
    }
}

static StudentIdPolicy effective_student_id_policy(
    const ProblemData *problem_data,
    StudentIdPolicy requested_policy,
    int interactive,
    int assume_yes)
{
    int student_index;
    int all_assignment5 = 1;
    int all_zero_padded_assignment5 = 1;
    int all_numeric = 1;
    char *examples[3] = {NULL, NULL, NULL};
    int example_count = 0;

    if (requested_policy != ID_POLICY_AUTO) {
        return requested_policy;
    }

    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        const char *raw_id = problem_data->students[student_index].raw_id;
        if (!student_id_token_is_assignment5(raw_id)) {
            all_assignment5 = 0;
            remember_student_id_example(examples, &example_count, raw_id);
        }
        if (!student_id_token_is_zero_padded_assignment5(raw_id)) {
            all_zero_padded_assignment5 = 0;
            remember_student_id_example(examples, &example_count, raw_id);
        }
        if (!text_is_digits(raw_id)) {
            all_numeric = 0;
        }
    }

    if (all_assignment5 && all_zero_padded_assignment5) {
        return ID_POLICY_ASSIGNMENT5;
    }

    if (assume_yes) {
        return all_assignment5 ? ID_POLICY_ASSIGNMENT5 :
               (all_numeric ? ID_POLICY_NUMERIC : ID_POLICY_TOKEN);
    }

    if (interactive) {
        const char *mode_text =
            all_assignment5 ? "zero-padded assignment-style" :
            (all_numeric ? "normalized numeric" : "opaque token");
        if (!stdin_is_terminal()) {
            fail_with_context_format_hint(
                "--id-policy auto",
                "rerun with --id-policy assignment5, --id-policy numeric, --id-policy token, or --id-policy auto --assume-yes",
                "cannot ask for confirmation because stdin is not a terminal");
        }
        if (confirm_auto_student_id_policy(mode_text, examples, example_count)) {
            return all_assignment5 ? ID_POLICY_ASSIGNMENT5 :
                   (all_numeric ? ID_POLICY_NUMERIC : ID_POLICY_TOKEN);
        }
        fail_with_context("--id-policy auto", "student identifier policy was not confirmed");
    }

    fail_with_context_format_hint(
        "--id-policy auto",
        "rerun with --id-policy assignment5, --id-policy numeric, --id-policy token, --id-policy auto --assume-yes, or --interactive",
        "student identifiers require confirmation or an explicit policy");
    return ID_POLICY_ASSIGNMENT5;
}

static int auto_numeric_student_id_width(const ProblemData *problem_data,
                                         int explicit_width)
{
    int width = explicit_width == AUTO_STUDENT_ID_WIDTH ? 5 : explicit_width;
    int student_index;
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        size_t raw_length = strlen(problem_data->students[student_index].raw_id);
        size_t canonical_length =
            strlen(problem_data->students[student_index].canonical_id);
        if (raw_length > (size_t)width) {
            width = (int)raw_length;
        }
        if (canonical_length > (size_t)width) {
            width = (int)canonical_length;
        }
    }
    return width;
}

static void finalize_student_ids(ProblemData *problem_data,
                                 StudentIdPolicy requested_policy,
                                 int requested_width,
                                 int interactive,
                                 int assume_yes)
{
    int student_index;
    int display_width;
    StudentIdPolicy policy = effective_student_id_policy(problem_data,
                                                         requested_policy,
                                                         interactive,
                                                         assume_yes);

    problem_data->id_policy = policy;
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        problem_data->students[student_index].canonical_id =
            canonical_student_id_for_policy(policy,
                                            problem_data->students[student_index].raw_id,
                                            "student id");
    }

    if (policy == ID_POLICY_TOKEN) {
        for (student_index = 0;
             student_index < problem_data->student_count;
             student_index++) {
            problem_data->students[student_index].student_id =
                duplicate_text(problem_data->students[student_index].raw_id);
        }
    } else {
        display_width =
            policy == ID_POLICY_ASSIGNMENT5 ?
            5 :
            auto_numeric_student_id_width(problem_data, requested_width);
        problem_data->student_id_width = display_width;
        for (student_index = 0;
             student_index < problem_data->student_count;
             student_index++) {
            problem_data->students[student_index].student_id =
                display_numeric_student_id(
                    problem_data->students[student_index].canonical_id,
                    display_width,
                    "student id");
        }
    }
    build_student_hash_index(problem_data);
}

static void check_no_extra_token(Tokenizer *tokenizer, const char *context)
{
    if (next_token(tokenizer) != NULL) {
        fail_with_context(context, "too many fields");
    }
}

static void free_problem_data(ProblemData *problem_data)
{
    int index_value;
    if (problem_data->labs != NULL) {
        for (index_value = 0; index_value < problem_data->lab_count; index_value++) {
            free(problem_data->labs[index_value].name);
        }
    }
    if (problem_data->students != NULL) {
        for (index_value = 0; index_value < problem_data->student_count; index_value++) {
            free(problem_data->students[index_value].raw_id);
            free(problem_data->students[index_value].canonical_id);
            free(problem_data->students[index_value].student_id);
        }
    }
    free(problem_data->labs);
    free(problem_data->students);
    free(problem_data->rank_by_student_lab);
    free(problem_data->lab_hash_indices);
    free(problem_data->student_hash_indices);
}

static void read_lab_file(const char *lab_file_path, ProblemData *problem_data)
{
    FastInput lab_input;
    char *line;
    Tokenizer tokenizer;
    char *token;
    int lab_index;
    long long total_capacity = 0LL;

    fast_input_open(&lab_input, lab_file_path);

    line = read_required_data_line(&lab_input, "lab file header");
    tokenizer_init(&tokenizer, line);
    token = next_token(&tokenizer);
    if (token == NULL) {
        fail_with_context("lab file header", "missing lab count");
    }
    problem_data->lab_count = parse_int_range(token, 1, INT_MAX - 2, "lab count");
    check_no_extra_token(&tokenizer, "lab file header");
    free(line);

    problem_data->labs = checked_calloc((size_t)problem_data->lab_count, sizeof(LabInfo));

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        char context[64];
        char *name_token;
        char *capacity_token;
        int previous_index;
        long long capacity_value;

        snprintf(context, sizeof(context), "lab line %d", lab_index + 2);
        line = read_required_data_line(&lab_input, context);
        tokenizer_init(&tokenizer, line);
        name_token = next_token(&tokenizer);
        capacity_token = next_token(&tokenizer);
        if (name_token == NULL || capacity_token == NULL) {
            fail_with_context(context, "expected lab name and capacity");
        }
        if (strlen(name_token) > MAX_NAME_BYTES) {
            fail_with_context_format_hint(context,
                                          "shorten the laboratory name to 255 bytes or fewer",
                                          "lab name '%s' exceeds 255 bytes",
                                          name_token);
        }
        for (previous_index = 0; previous_index < lab_index; previous_index++) {
            if (strcmp(problem_data->labs[previous_index].name, name_token) == 0) {
                fail_with_context_format_hint(context,
                                              "each laboratory name may appear only once",
                                              "duplicate lab name '%s'",
                                              name_token);
            }
        }
        capacity_value = parse_long_long_range(capacity_token, 0LL, LLONG_MAX / 4LL, context);
        check_no_extra_token(&tokenizer, context);

        problem_data->labs[lab_index].name = duplicate_text(name_token);
        problem_data->labs[lab_index].capacity_value = capacity_value;
        problem_data->labs[lab_index].graph_capacity = 0;
        add_capacity_saturating(&total_capacity, capacity_value, LLONG_MAX / 4LL);
        free(line);
    }

    while ((line = fast_read_line_dynamic(&lab_input)) != NULL) {
        if (!line_is_blank(line)) {
            free(line);
            fail_with_context("lab file", "unexpected extra data after lab definitions");
        }
        free(line);
    }

    fast_input_close(&lab_input);
    if (total_capacity <= 0LL) {
        fail_with_context("lab file", "total capacity must be positive");
    }
}

static void read_preference_file(const char *preference_file_path,
                                 ProblemData *problem_data,
                                 const ProgramOptions *options)
{
    FastInput preference_input;
    char *line;
    Tokenizer tokenizer;
    char *token;
    int student_index;
    int default_rank;
    int *seen_stamp;

    fast_input_open(&preference_input, preference_file_path);

    line = read_required_data_line(&preference_input, "preference file header");
    tokenizer_init(&tokenizer, line);
    token = next_token(&tokenizer);
    if (token == NULL) {
        fail_with_context("preference file header", "missing student count");
    }
    problem_data->student_count = parse_int_range(token,
                                                  1,
                                                  INT_MAX - 2,
                                                  "student count");
    token = next_token(&tokenizer);
    if (token == NULL) {
        fail_with_context("preference file header", "missing maximum preference count");
    }
    problem_data->max_preferences = parse_int_range(token,
                                                    1,
                                                    problem_data->lab_count,
                                                    "maximum preference count");
    check_no_extra_token(&tokenizer, "preference file header");
    free(line);

    problem_data->students = checked_calloc((size_t)problem_data->student_count,
                                            sizeof(StudentInfo));
    seen_stamp = checked_calloc((size_t)problem_data->lab_count, sizeof(int));
    default_rank = problem_data->lab_count + 1;
    {
        size_t rank_cell_count =
            checked_multiply_size((size_t)problem_data->student_count,
                                  (size_t)problem_data->lab_count,
                                  "rank table");
        size_t rank_byte_count =
            checked_multiply_size(rank_cell_count, sizeof(int), "rank table");
        size_t rank_index;
        problem_data->rank_by_student_lab = checked_malloc(rank_byte_count);
        for (rank_index = 0U; rank_index < rank_cell_count; rank_index++) {
            problem_data->rank_by_student_lab[rank_index] = default_rank;
        }
    }

    for (student_index = 0; student_index < problem_data->student_count; student_index++) {
        char context[64];
        char *student_id_token;
        int preference_index;
        int stamp = student_index + 1;

        snprintf(context, sizeof(context), "preference line %d", student_index + 2);
        line = read_required_data_line(&preference_input, context);
        tokenizer_init(&tokenizer, line);
        student_id_token = next_token(&tokenizer);
        if (student_id_token == NULL) {
            fail_with_context(context, "missing student id");
        }
        problem_data->students[student_index].raw_id =
            duplicate_text(student_id_token);

        for (preference_index = 0;
             preference_index < problem_data->max_preferences;
             preference_index++) {
            char *lab_name_token = next_token(&tokenizer);
            int lab_index;
            if (lab_name_token == NULL) {
                fail_with_context(context, "not enough preference fields");
            }
            lab_index = find_lab_index(problem_data, lab_name_token);
            if (lab_index < 0) {
                fail_with_context_format_hint(context,
                                              "check that the laboratory name appears in the lab list file",
                                              "unknown lab name '%s'",
                                              lab_name_token);
            }
            if (seen_stamp[lab_index] == stamp) {
                fail_with_context_format_hint(context,
                                              "each student may list each laboratory at most once",
                                              "duplicate lab name '%s' in one student's preferences",
                                              lab_name_token);
            }
            seen_stamp[lab_index] = stamp;
            problem_data->rank_by_student_lab[(size_t)student_index *
                                                  (size_t)problem_data->lab_count +
                                              (size_t)lab_index] =
                preference_index + 1;
        }

        check_no_extra_token(&tokenizer, context);
        free(line);
    }

    while ((line = fast_read_line_dynamic(&preference_input)) != NULL) {
        if (!line_is_blank(line)) {
            free(line);
            fail_with_context("preference file",
                              "unexpected extra data after student preferences");
        }
        free(line);
    }

    free(seen_stamp);
    fast_input_close(&preference_input);
    finalize_student_ids(problem_data,
                         options->id_policy,
                         options->student_id_width,
                         options->interactive,
                         options->assume_yes);
}

static ProblemData read_problem_data(const char *lab_file_path,
                                     const char *preference_file_path,
                                     const ProgramOptions *options)
{
    ProblemData problem_data;

    memset(&problem_data, 0, sizeof(problem_data));
    problem_data.id_policy = options->id_policy;
    problem_data.student_id_width = options->student_id_width;
    read_lab_file(lab_file_path, &problem_data);
    build_lab_hash_index(&problem_data);
    read_preference_file(preference_file_path, &problem_data, options);
    return problem_data;
}

static void finalize_problem_capacities(ProblemData *problem_data)
{
    int lab_index;
    long long total_capacity = 0LL;
    int positive_capacity_labs = 0;

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        add_capacity_saturating(&total_capacity,
                                problem_data->labs[lab_index].capacity_value,
                                (long long)problem_data->student_count);
        if (problem_data->labs[lab_index].capacity_value >
            (long long)problem_data->student_count) {
            problem_data->labs[lab_index].graph_capacity = problem_data->student_count;
        } else {
            problem_data->labs[lab_index].graph_capacity =
                (int)problem_data->labs[lab_index].capacity_value;
        }
        if (problem_data->labs[lab_index].capacity_value > 0LL) {
            positive_capacity_labs++;
        }
    }
    if (total_capacity < (long long)problem_data->student_count) {
        fail_with_context_format_hint("input",
                                      "increase lab capacities or reduce the number of students",
                                      "total lab capacity %lld is smaller than student count %d",
                                      total_capacity,
                                      problem_data->student_count);
    }
    if (positive_capacity_labs > problem_data->student_count) {
        fail_with_context_format_hint("input",
                                      "every positive-capacity lab must receive at least one student",
                                      "%d positive-capacity labs but only %d students; minimum occupancy is impossible",
                                      positive_capacity_labs,
                                      problem_data->student_count);
    }
}

static void constraint_set_init(ConstraintSet *constraints,
                                const ProblemData *problem_data)
{
    size_t matrix_size =
        checked_multiply_size((size_t)problem_data->student_count,
                              (size_t)problem_data->lab_count,
                              "constraints");
    int student_index;
    constraints->locked_lab_by_student =
        checked_malloc((size_t)problem_data->student_count * sizeof(int));
    constraints->forbidden_matrix = checked_calloc(matrix_size, sizeof(unsigned char));
    constraints->allowed_matrix = checked_calloc(matrix_size, sizeof(unsigned char));
    constraints->has_allowed_set =
        checked_calloc((size_t)problem_data->student_count, sizeof(unsigned char));
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        constraints->locked_lab_by_student[student_index] = -1;
    }
}

static void constraint_set_free(ConstraintSet *constraints)
{
    free(constraints->locked_lab_by_student);
    free(constraints->forbidden_matrix);
    free(constraints->allowed_matrix);
    free(constraints->has_allowed_set);
    constraints->locked_lab_by_student = NULL;
    constraints->forbidden_matrix = NULL;
    constraints->allowed_matrix = NULL;
    constraints->has_allowed_set = NULL;
}

static void constraint_set_copy(ConstraintSet *destination,
                                const ProblemData *problem_data,
                                const ConstraintSet *source)
{
    size_t matrix_size =
        checked_multiply_size((size_t)problem_data->student_count,
                              (size_t)problem_data->lab_count,
                              "constraint matrix");
    size_t student_size = (size_t)problem_data->student_count;
    constraint_set_init(destination, problem_data);
    if (source == NULL) {
        return;
    }
    memcpy(destination->locked_lab_by_student,
           source->locked_lab_by_student,
           student_size * sizeof(int));
    memcpy(destination->forbidden_matrix,
           source->forbidden_matrix,
           matrix_size * sizeof(unsigned char));
    memcpy(destination->allowed_matrix,
           source->allowed_matrix,
           matrix_size * sizeof(unsigned char));
    memcpy(destination->has_allowed_set,
           source->has_allowed_set,
           student_size * sizeof(unsigned char));
}

static int parse_constraint_student(const ProblemData *problem_data,
                                    const char *student_id,
                                    const char *context)
{
    int student_index;
    student_index = find_student_index(problem_data, student_id, context);
    if (student_index < 0) {
        fail_with_context_format(context, "unknown student id '%s'", student_id);
    }
    return student_index;
}

static int parse_constraint_lab(const ProblemData *problem_data,
                                const char *lab_name,
                                const char *context)
{
    int lab_index = find_lab_index(problem_data, lab_name);
    if (lab_index < 0) {
        fail_with_context_format(context, "unknown lab name '%s'", lab_name);
    }
    return lab_index;
}

static void validate_constraint_consistency(const ProblemData *problem_data,
                                            const ConstraintSet *constraints)
{
    int student_index;
    int lab_index;
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        int locked_lab = constraints->locked_lab_by_student[student_index];
        if (locked_lab >= 0) {
            size_t matrix_index =
                student_lab_matrix_index(problem_data, student_index, locked_lab);
            if (constraints->forbidden_matrix[matrix_index]) {
                fail_with_context("constraints", "locked assignment is also forbidden");
            }
            if (constraints->has_allowed_set[student_index] &&
                !constraints->allowed_matrix[matrix_index]) {
                fail_with_context("constraints", "locked assignment is outside allowed set");
            }
        }
        for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
            size_t matrix_index =
                student_lab_matrix_index(problem_data, student_index, lab_index);
            if (constraints->has_allowed_set[student_index] &&
                constraints->allowed_matrix[matrix_index] &&
                constraints->forbidden_matrix[matrix_index]) {
                fail_with_context("constraints", "allowed assignment is also forbidden");
            }
        }
    }
}

static void read_constraints_file(const char *constraints_path,
                                  ProblemData *problem_data,
                                  ConstraintSet *constraints)
{
    FastInput input;
    char *line;
    int line_number = 0;

    constraint_set_init(constraints, problem_data);
    fast_input_open(&input, constraints_path);
    while ((line = fast_read_line_dynamic(&input)) != NULL) {
        Tokenizer tokenizer;
        char *command_token;
        char context[96];
        line_number++;
        snprintf(context, sizeof(context), "constraints line %d", line_number);
        if (line_is_blank(line)) {
            free(line);
            continue;
        }
        tokenizer_init(&tokenizer, line);
        command_token = next_token(&tokenizer);
        if (command_token == NULL || command_token[0] == '#') {
            free(line);
            continue;
        }
        if (strcmp(command_token, "lock") == 0) {
            char *student_token = next_token(&tokenizer);
            char *lab_token = next_token(&tokenizer);
            int student_index;
            int lab_index;
            if (student_token == NULL || lab_token == NULL) {
                fail_with_context(context, "lock requires student id and lab name");
            }
            student_index = parse_constraint_student(problem_data, student_token, context);
            lab_index = parse_constraint_lab(problem_data, lab_token, context);
            if (constraints->locked_lab_by_student[student_index] >= 0 &&
                constraints->locked_lab_by_student[student_index] != lab_index) {
                fail_with_context(context, "student is locked to multiple labs");
            }
            constraints->locked_lab_by_student[student_index] = lab_index;
            check_no_extra_token(&tokenizer, context);
        } else if (strcmp(command_token, "forbid") == 0) {
            char *student_token = next_token(&tokenizer);
            char *lab_token = next_token(&tokenizer);
            int student_index;
            int lab_index;
            if (student_token == NULL || lab_token == NULL) {
                fail_with_context(context, "forbid requires student id and lab name");
            }
            student_index = parse_constraint_student(problem_data, student_token, context);
            lab_index = parse_constraint_lab(problem_data, lab_token, context);
            constraints->forbidden_matrix[student_lab_matrix_index(problem_data,
                                                                   student_index,
                                                                   lab_index)] = 1U;
            check_no_extra_token(&tokenizer, context);
        } else if (strcmp(command_token, "allow") == 0) {
            char *student_token = next_token(&tokenizer);
            char *lab_token;
            int student_index;
            int allowed_count = 0;
            if (student_token == NULL) {
                fail_with_context(context, "allow requires student id and at least one lab name");
            }
            student_index = parse_constraint_student(problem_data, student_token, context);
            while ((lab_token = next_token(&tokenizer)) != NULL) {
                int lab_index = parse_constraint_lab(problem_data, lab_token, context);
                constraints->allowed_matrix[student_lab_matrix_index(problem_data,
                                                                     student_index,
                                                                     lab_index)] = 1U;
                constraints->has_allowed_set[student_index] = 1U;
                allowed_count++;
            }
            if (allowed_count == 0) {
                fail_with_context(context, "allow requires at least one lab name");
            }
        } else if (strcmp(command_token, "capacity") == 0) {
            char *lab_token = next_token(&tokenizer);
            char *capacity_token = next_token(&tokenizer);
            int lab_index;
            if (lab_token == NULL || capacity_token == NULL) {
                fail_with_context(context, "capacity requires lab name and nonnegative capacity");
            }
            lab_index = parse_constraint_lab(problem_data, lab_token, context);
            problem_data->labs[lab_index].capacity_value =
                parse_long_long_range(capacity_token, 0LL, LLONG_MAX / 4LL, context);
            check_no_extra_token(&tokenizer, context);
        } else {
            fail_with_context(context, "unknown constraint command");
        }
        free(line);
    }
    fast_input_close(&input);
    validate_constraint_consistency(problem_data, constraints);
}

static void validate_complete_assignment(const ProblemData *problem_data,
                                         const int *assignment,
                                         const char *context)
{
    int *lab_counts =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));
    int student_index;
    int lab_index;
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        lab_index = assignment[student_index];
        if (lab_index < 0 || lab_index >= problem_data->lab_count) {
            free(lab_counts);
            fail_with_context(context, "assignment is missing a student");
        }
        lab_counts[lab_index]++;
    }
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        if ((long long)lab_counts[lab_index] >
            problem_data->labs[lab_index].capacity_value) {
            free(lab_counts);
            fail_with_context(context, "assignment exceeds lab capacity");
        }
        if (problem_data->labs[lab_index].capacity_value > 0LL &&
            lab_counts[lab_index] < 1) {
            free(lab_counts);
            fail_with_context(context, "assignment violates minimum occupancy");
        }
    }
    free(lab_counts);
}

static int *read_assignment_file_as_base(const char *assignment_path,
                                         const ProblemData *problem_data)
{
    FastInput input;
    char *line;
    Tokenizer tokenizer;
    char *token;
    int expected_count;
    int line_number;
    int *assignment =
        checked_malloc((size_t)problem_data->student_count * sizeof(int));
    unsigned char *seen_students =
        checked_calloc((size_t)problem_data->student_count, sizeof(unsigned char));
    int student_index;

    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        assignment[student_index] = -1;
    }

    fast_input_open(&input, assignment_path);
    line = read_required_data_line(&input, "base assignment header");
    tokenizer_init(&tokenizer, line);
    token = next_token(&tokenizer);
    if (token == NULL) {
        fail_with_context("base assignment header", "missing student count");
    }
    expected_count = parse_int_range(token,
                                     problem_data->student_count,
                                     problem_data->student_count,
                                     "base assignment student count");
    (void)expected_count;
    check_no_extra_token(&tokenizer, "base assignment header");
    free(line);

    for (line_number = 0;
         line_number < problem_data->student_count;
         line_number++) {
        char context[96];
        char *student_token;
        char *lab_token;
        int parsed_student_index;
        int lab_index;
        snprintf(context, sizeof(context), "base assignment line %d", line_number + 2);
        line = read_required_data_line(&input, context);
        tokenizer_init(&tokenizer, line);
        student_token = next_token(&tokenizer);
        lab_token = next_token(&tokenizer);
        if (student_token == NULL || lab_token == NULL) {
            fail_with_context(context, "expected student id and lab name");
        }
        parsed_student_index =
            parse_constraint_student(problem_data, student_token, context);
        lab_index = parse_constraint_lab(problem_data, lab_token, context);
        if (seen_students[parsed_student_index]) {
            fail_with_context(context, "duplicate student id in base assignment");
        }
        seen_students[parsed_student_index] = 1U;
        assignment[parsed_student_index] = lab_index;
        check_no_extra_token(&tokenizer, context);
        free(line);
    }

    while ((line = fast_read_line_dynamic(&input)) != NULL) {
        if (!line_is_blank(line)) {
            free(line);
            fail_with_context("base assignment", "unexpected extra data");
        }
        free(line);
    }

    fast_input_close(&input);
    validate_complete_assignment(problem_data, assignment, "base assignment");
    free(seen_students);
    return assignment;
}

static void dinic_list_push(DinicEdgeList *list, DinicEdge edge)
{
    if (list->size == list->capacity) {
        int new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        list->items = checked_realloc(list->items, (size_t)new_capacity * sizeof(DinicEdge));
        list->capacity = new_capacity;
    }
    list->items[list->size] = edge;
    list->size++;
}

static DinicGraph dinic_graph_create(int node_count)
{
    DinicGraph graph;
    graph.node_count = node_count;
    solver_profile_note_dinic_graph(node_count);
    graph.adjacency = checked_calloc((size_t)node_count, sizeof(DinicEdgeList));
    graph.level = checked_malloc((size_t)node_count * sizeof(int));
    graph.work_index = checked_malloc((size_t)node_count * sizeof(int));
    return graph;
}

static void dinic_graph_free(DinicGraph *graph)
{
    int node_index;
    for (node_index = 0; node_index < graph->node_count; node_index++) {
        free(graph->adjacency[node_index].items);
    }
    free(graph->adjacency);
    free(graph->level);
    free(graph->work_index);
}

static void dinic_add_edge(DinicGraph *graph, int from_node, int to_node, int capacity)
{
    DinicEdge forward_edge;
    DinicEdge reverse_edge;
    int forward_index = graph->adjacency[from_node].size;
    int reverse_index = graph->adjacency[to_node].size;

    forward_edge.to_node = to_node;
    forward_edge.reverse_index = reverse_index;
    forward_edge.capacity = capacity;
    reverse_edge.to_node = from_node;
    reverse_edge.reverse_index = forward_index;
    reverse_edge.capacity = 0;
    dinic_list_push(&graph->adjacency[from_node], forward_edge);
    dinic_list_push(&graph->adjacency[to_node], reverse_edge);
    if (active_profile != NULL) {
        active_profile->dinic_edges_added++;
    }
}

static void add_lower_bound_edge(DinicGraph *graph,
                                 long long *balance,
                                 int from_node,
                                 int to_node,
                                 int lower_bound,
                                 int upper_bound)
{
    if (lower_bound > upper_bound) {
        fail_with_message("internal lower-bound capacity error");
    }
    if (upper_bound > lower_bound) {
        dinic_add_edge(graph, from_node, to_node, upper_bound - lower_bound);
    }
    balance[from_node] -= (long long)lower_bound;
    balance[to_node] += (long long)lower_bound;
}

static int dinic_build_levels(DinicGraph *graph, int source_node, int sink_node)
{
    int *queue = checked_malloc((size_t)graph->node_count * sizeof(int));
    int head = 0;
    int tail = 0;
    int node_index;

    for (node_index = 0; node_index < graph->node_count; node_index++) {
        graph->level[node_index] = -1;
    }
    graph->level[source_node] = 0;
    queue[tail] = source_node;
    tail++;

    while (head < tail) {
        int current_node = queue[head];
        int edge_index;
        head++;
        for (edge_index = 0; edge_index < graph->adjacency[current_node].size; edge_index++) {
            DinicEdge *edge = &graph->adjacency[current_node].items[edge_index];
            if (edge->capacity > 0 && graph->level[edge->to_node] < 0) {
                graph->level[edge->to_node] = graph->level[current_node] + 1;
                queue[tail] = edge->to_node;
                tail++;
            }
        }
    }

    free(queue);
    return graph->level[sink_node] >= 0;
}

static int dinic_send_flow(DinicGraph *graph,
                           int current_node,
                           int sink_node,
                           int flow_limit)
{
    int edge_index;
    if (current_node == sink_node) {
        return flow_limit;
    }

    for (edge_index = graph->work_index[current_node];
         edge_index < graph->adjacency[current_node].size;
         edge_index++) {
        DinicEdge *edge = &graph->adjacency[current_node].items[edge_index];
        graph->work_index[current_node] = edge_index;
        if (edge->capacity > 0 &&
            graph->level[current_node] + 1 == graph->level[edge->to_node]) {
            int next_limit = flow_limit < edge->capacity ? flow_limit : edge->capacity;
            int pushed_flow = dinic_send_flow(graph, edge->to_node, sink_node, next_limit);
            if (pushed_flow > 0) {
                DinicEdge *reverse_edge =
                    &graph->adjacency[edge->to_node].items[edge->reverse_index];
                edge->capacity -= pushed_flow;
                reverse_edge->capacity += pushed_flow;
                return pushed_flow;
            }
        }
        graph->work_index[current_node] = edge_index + 1;
    }

    return 0;
}

static int dinic_max_flow(DinicGraph *graph, int source_node, int sink_node)
{
    int total_flow = 0;
    if (active_profile != NULL) {
        active_profile->dinic_calls++;
    }

    while (dinic_build_levels(graph, source_node, sink_node)) {
        int node_index;
        for (node_index = 0; node_index < graph->node_count; node_index++) {
            graph->work_index[node_index] = 0;
        }
        for (;;) {
            int pushed_flow = dinic_send_flow(graph, source_node, sink_node, INT_MAX);
            if (pushed_flow == 0) {
                break;
            }
            total_flow += pushed_flow;
        }
    }

    return total_flow;
}

static int rank_for_assignment(const ProblemData *problem_data,
                               int student_index,
                               int lab_index)
{
    return problem_data->rank_by_student_lab[(size_t)student_index *
                                                 (size_t)problem_data->lab_count +
                                             (size_t)lab_index];
}

static void int_list_init(IntList *list)
{
    list->items = NULL;
    list->size = 0;
    list->capacity = 0;
}

static void int_list_push(IntList *list, int value)
{
    if (list->size == list->capacity) {
        int new_capacity;
        if (list->capacity > INT_MAX / 2) {
            fail_with_context("rank threshold candidates", "candidate list overflow");
        }
        new_capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        list->items = checked_realloc(list->items,
                                      (size_t)new_capacity * sizeof(int));
        list->capacity = new_capacity;
    }
    list->items[list->size] = value;
    list->size++;
}

static void int_list_free(IntList *list)
{
    free(list->items);
    list->items = NULL;
    list->size = 0;
    list->capacity = 0;
}

static IntList build_rank_threshold_candidates(const ProblemData *problem_data)
{
    IntList candidates;
    int outside_rank = problem_data->lab_count + 1;
    int candidate_upper_bound =
        target_rank_upper_bound(problem_data, outside_rank);
    int rank_value;

    if (active_profile != NULL) {
        active_profile->rank_threshold_candidate_builds++;
    }
    int_list_init(&candidates);

    for (rank_value = 1;
         rank_value <= problem_data->max_preferences &&
         rank_value <= candidate_upper_bound &&
         rank_value < outside_rank;
         rank_value++) {
        int_list_push(&candidates, rank_value);
    }
    if (outside_rank <= candidate_upper_bound) {
        int_list_push(&candidates, outside_rank);
    }

    return candidates;
}

static int int_list_first_index_at_least(const IntList *list, int value)
{
    int low_index = 0;
    int high_index = list->size;
    while (low_index < high_index) {
        int middle_index = low_index + (high_index - low_index) / 2;
        if (list->items[middle_index] < value) {
            low_index = middle_index + 1;
        } else {
            high_index = middle_index;
        }
    }
    return low_index;
}

static int int_list_last_index_at_most(const IntList *list, int value)
{
    int low_index = 0;
    int high_index = list->size;
    while (low_index < high_index) {
        int middle_index = low_index + (high_index - low_index) / 2;
        if (list->items[middle_index] <= value) {
            low_index = middle_index + 1;
        } else {
            high_index = middle_index;
        }
    }
    return low_index - 1;
}

static int target_rank_upper_bound(const ProblemData *problem_data,
                                   int base_upper_bound)
{
    const TargetConstraints *targets = problem_data->targets;
    int upper_bound = base_upper_bound;
    if (targets == NULL) {
        return upper_bound;
    }
    if (targets->has_max_rank_max && targets->max_rank_max < upper_bound) {
        upper_bound = targets->max_rank_max;
    }
    if (targets->has_outside_max && targets->outside_max == 0 &&
        problem_data->max_preferences < upper_bound) {
        upper_bound = problem_data->max_preferences;
    }
    return upper_bound;
}

static int assignment_edge_allowed(const ProblemData *problem_data,
                                   int student_index,
                                   int lab_index,
                                   int max_rank)
{
    int rank_value;
    const ConstraintSet *constraints = problem_data->constraints;
    if (problem_data->labs[lab_index].graph_capacity <= 0) {
        return 0;
    }
    rank_value = rank_for_assignment(problem_data, student_index, lab_index);
    if (constraints != NULL) {
        int locked_lab = constraints->locked_lab_by_student[student_index];
        size_t matrix_index =
            student_lab_matrix_index(problem_data, student_index, lab_index);
        if (locked_lab >= 0 && lab_index != locked_lab) {
            return 0;
        }
        if (constraints->forbidden_matrix[matrix_index]) {
            return 0;
        }
        if (constraints->has_allowed_set[student_index] &&
            !constraints->allowed_matrix[matrix_index]) {
            return 0;
        }
    }
    return rank_value <= max_rank;
}

static UngroupedActiveArcTemplate ungrouped_active_arc_template_cache;

static void ungrouped_active_arc_template_clear(
    UngroupedActiveArcTemplate *template_value)
{
    free(template_value->arcs);
    free(template_value->allowed_counts_by_student);
    free(template_value->incoming_counts_by_lab);
    memset(template_value, 0, sizeof(*template_value));
}

static void active_assignment_arc_template_push(
    UngroupedActiveArcTemplate *template_value,
    int student_index,
    int lab_index)
{
    ActiveAssignmentArc arc;
    if (template_value->arc_count == template_value->arc_capacity) {
        int new_capacity =
            template_value->arc_capacity == 0 ?
            256 :
            template_value->arc_capacity * 2;
        if (new_capacity < template_value->arc_capacity) {
            fail_with_context("active arc template", "capacity overflow");
        }
        template_value->arcs =
            checked_realloc(template_value->arcs,
                            (size_t)new_capacity *
                                sizeof(ActiveAssignmentArc));
        template_value->arc_capacity = new_capacity;
    }
    arc.student_index = student_index;
    arc.lab_index = lab_index;
    template_value->arcs[template_value->arc_count] = arc;
    template_value->arc_count++;
}

static int ungrouped_active_arc_template_matches(
    const UngroupedActiveArcTemplate *template_value,
    const ProblemData *problem_data,
    int best_max_rank)
{
    return template_value->valid &&
           template_value->problem_data == problem_data &&
           template_value->constraints == problem_data->constraints &&
           template_value->best_max_rank == best_max_rank &&
           template_value->student_count == problem_data->student_count &&
           template_value->lab_count == problem_data->lab_count;
}

static const UngroupedActiveArcTemplate *ungrouped_active_arc_template_get(
    const ProblemData *problem_data,
    int best_max_rank)
{
    int student_index;
    int lab_index;

    if (ungrouped_active_arc_template_matches(
            &ungrouped_active_arc_template_cache,
            problem_data,
            best_max_rank)) {
        if (active_profile != NULL) {
            active_profile->active_arc_template_hits++;
        }
        return &ungrouped_active_arc_template_cache;
    }

    ungrouped_active_arc_template_clear(&ungrouped_active_arc_template_cache);
    ungrouped_active_arc_template_cache.problem_data = problem_data;
    ungrouped_active_arc_template_cache.constraints = problem_data->constraints;
    ungrouped_active_arc_template_cache.best_max_rank = best_max_rank;
    ungrouped_active_arc_template_cache.student_count =
        problem_data->student_count;
    ungrouped_active_arc_template_cache.lab_count = problem_data->lab_count;
    ungrouped_active_arc_template_cache.allowed_counts_by_student =
        checked_calloc((size_t)problem_data->student_count, sizeof(int));
    ungrouped_active_arc_template_cache.incoming_counts_by_lab =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));

    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        for (lab_index = 0;
             lab_index < problem_data->lab_count;
             lab_index++) {
            if (assignment_edge_allowed(problem_data,
                                        student_index,
                                        lab_index,
                                        best_max_rank)) {
                ungrouped_active_arc_template_cache
                    .allowed_counts_by_student[student_index]++;
                ungrouped_active_arc_template_cache
                    .incoming_counts_by_lab[lab_index]++;
                active_assignment_arc_template_push(
                    &ungrouped_active_arc_template_cache,
                    student_index,
                    lab_index);
            }
        }
    }

    ungrouped_active_arc_template_cache.valid = 1;
    if (active_profile != NULL) {
        active_profile->active_arc_template_misses++;
    }
    return &ungrouped_active_arc_template_cache;
}

static int all_students_have_identical_rank_rows(const ProblemData *problem_data)
{
    int student_index;
    int lab_index;
    if (problem_data->constraints != NULL ||
        (problem_data->base_assignment != NULL &&
         problem_data->change_penalty > 0LL)) {
        return 0;
    }
    if (problem_data->targets != NULL &&
        problem_data->targets->has_minimum_fill_min) {
        return 0;
    }
    for (student_index = 1;
         student_index < problem_data->student_count;
         student_index++) {
        for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
            if (rank_for_assignment(problem_data, student_index, lab_index) !=
                rank_for_assignment(problem_data, 0, lab_index)) {
                return 0;
            }
        }
    }
    return 1;
}

static int minimum_feasible_max_rank_for_identical_rows(const ProblemData *problem_data)
{
    int lab_index;
    int max_rank = 1;
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        if (problem_data->labs[lab_index].capacity_value > 0LL) {
            int rank_value = rank_for_assignment(problem_data, 0, lab_index);
            if (rank_value > max_rank) {
                max_rank = rank_value;
            }
        }
    }
    return max_rank;
}

static int has_feasible_assignment_with_minimum_counts(const ProblemData *problem_data,
                                                       int max_rank,
                                                       const int *minimum_lab_counts)
{
    int source_node = 0;
    StudentGroups groups =
        build_student_groups(problem_data, max_rank, STUDENT_GROUP_ALLOWED_SET);
    int first_group_node = 1;
    int first_lab_node = first_group_node + groups.count;
    int sink_node = first_lab_node + problem_data->lab_count;
    int base_node_count = sink_node + 1;
    int super_source = base_node_count;
    int super_sink = base_node_count + 1;
    DinicGraph graph = dinic_graph_create(base_node_count + 2);
    long long *balance = checked_calloc((size_t)base_node_count, sizeof(long long));
    long long required_flow = 0LL;
    int group_index;
    int lab_index;
    int node_index;
    int max_flow;
    int feasible;

    add_lower_bound_edge(&graph,
                         balance,
                         sink_node,
                         source_node,
                         problem_data->student_count,
                         problem_data->student_count);

    for (group_index = 0; group_index < groups.count; group_index++) {
        int group_node = first_group_node + group_index;
        int representative_student = groups.items[group_index].representative_student;
        add_lower_bound_edge(&graph,
                             balance,
                             source_node,
                             group_node,
                             0,
                             groups.items[group_index].size);
        for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
            if (assignment_edge_allowed(problem_data,
                                        representative_student,
                                        lab_index,
                                        max_rank)) {
                int lab_node = first_lab_node + lab_index;
                add_lower_bound_edge(&graph,
                                     balance,
                                     group_node,
                                     lab_node,
                                     0,
                                     groups.items[group_index].size);
            }
        }
    }

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int lab_node = first_lab_node + lab_index;
        int required_count = minimum_lab_counts == NULL ?
                             (problem_data->labs[lab_index].capacity_value > 0LL ? 1 : 0) :
                             minimum_lab_counts[lab_index];
        if (minimum_lab_counts == NULL &&
            problem_data->labs[lab_index].capacity_value > 0LL) {
            int target_count = target_minimum_count_for_lab(problem_data, lab_index);
            if (target_count > required_count) {
                required_count = target_count;
            }
        }
        int upper_count = problem_data->labs[lab_index].graph_capacity;
        if (required_count < 0 || required_count > upper_count) {
            free(balance);
            dinic_graph_free(&graph);
            free_student_groups(&groups);
            return 0;
        }
        add_lower_bound_edge(&graph, balance, lab_node, sink_node, required_count, upper_count);
    }

    for (node_index = 0; node_index < base_node_count; node_index++) {
        if (balance[node_index] > 0LL) {
            if (balance[node_index] > (long long)INT_MAX) {
                fail_with_message("internal flow demand is too large");
            }
            dinic_add_edge(&graph, super_source, node_index, (int)balance[node_index]);
            required_flow += balance[node_index];
        } else if (balance[node_index] < 0LL) {
            if (-balance[node_index] > (long long)INT_MAX) {
                fail_with_message("internal flow demand is too large");
            }
            dinic_add_edge(&graph, node_index, super_sink, (int)(-balance[node_index]));
        }
    }

    max_flow = dinic_max_flow(&graph, super_source, super_sink);
    feasible = (long long)max_flow == required_flow;

    free(balance);
    dinic_graph_free(&graph);
    free_student_groups(&groups);
    return feasible;
}

static int has_feasible_assignment(const ProblemData *problem_data, int max_rank)
{
    return has_feasible_assignment_with_minimum_counts(problem_data, max_rank, NULL);
}

static int target_constraints_have_structural_bound(
    const TargetConstraints *targets)
{
    if (targets == NULL) {
        return 0;
    }
    /*
      Rank-sum-like targets are checked by a stronger min-cost-flow solve later.
      That solve already includes structural max-rank, minimum-fill, and
      no-outside constraints, so a separate Dinic precheck would duplicate work.
    */
    if (targets->has_average_rank_max || targets->has_rank_sum_max) {
        return 0;
    }
    return targets->has_max_rank_max ||
           targets->has_minimum_fill_min ||
           targets->has_outside_max;
}

static void precheck_structural_target_constraints(
    const ProblemData *problem_data)
{
    int q_upper_bound;
    int *minimum_counts;
    int feasible;
    if (!target_constraints_have_structural_bound(problem_data->targets)) {
        return;
    }

    q_upper_bound = target_rank_upper_bound(problem_data,
                                           problem_data->lab_count + 1);
    if (q_upper_bound < 1) {
        fail_with_context("target constraints",
                          "No feasible solution: structural hard targets leave no allowable rank");
    }

    minimum_counts = build_base_minimum_counts(problem_data);
    feasible = has_feasible_assignment_with_minimum_counts(problem_data,
                                                           q_upper_bound,
                                                           minimum_counts);
    free(minimum_counts);
    if (!feasible) {
        fail_with_context("target constraints",
                          "No feasible solution: no assignment satisfies the structural hard targets");
    }
}

static int find_minimum_feasible_max_rank_at_most(const ProblemData *problem_data,
                                                  int q_upper_bound)
{
    IntList candidates;
    int low_index;
    int high_index;
    int answer_rank;

    q_upper_bound = target_rank_upper_bound(problem_data, q_upper_bound);
    if (q_upper_bound < 1) {
        fail_with_context("target constraints",
                          "No feasible solution: max-rank target leaves no allowable rank");
    }

    if (all_students_have_identical_rank_rows(problem_data)) {
        answer_rank = minimum_feasible_max_rank_for_identical_rows(problem_data);
        if (answer_rank > q_upper_bound ||
            !has_feasible_assignment(problem_data, q_upper_bound)) {
            fail_with_context("target constraints",
                              "No feasible solution: no feasible assignment satisfies the rank target");
        }
        return answer_rank;
    }

    candidates = build_rank_threshold_candidates(problem_data);
    if (candidates.size == 0) {
        fail_with_context("rank threshold candidates", "no rank thresholds found");
    }

    high_index = int_list_last_index_at_most(&candidates, q_upper_bound);
    if (high_index < 0) {
        int_list_free(&candidates);
        fail_with_context("target constraints",
                          "No feasible solution: no rank threshold candidate satisfies the target");
    }
    answer_rank = candidates.items[high_index];

    if (!has_feasible_assignment(problem_data, answer_rank)) {
        int_list_free(&candidates);
        fail_with_context("target constraints",
                          "No feasible solution: no feasible assignment satisfies the rank/fill targets");
    }

    low_index = 0;
    while (low_index <= high_index) {
        int middle_index = low_index + (high_index - low_index) / 2;
        int candidate_rank = candidates.items[middle_index];
        if (has_feasible_assignment(problem_data, candidate_rank)) {
            answer_rank = candidate_rank;
            high_index = middle_index - 1;
        } else {
            low_index = middle_index + 1;
        }
    }

    int_list_free(&candidates);
    return answer_rank;
}

static int find_minimum_feasible_max_rank(const ProblemData *problem_data)
{
    return find_minimum_feasible_max_rank_at_most(problem_data,
                                                 problem_data->lab_count + 1);
}

static void mcf_list_push(McfEdgeList *list, McfEdge edge)
{
    if (list->size == list->capacity) {
        int new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        list->items = checked_realloc(list->items, (size_t)new_capacity * sizeof(McfEdge));
        list->capacity = new_capacity;
    }
    list->items[list->size] = edge;
    list->size++;
}

static void mcf_list_reserve(McfEdgeList *list, int minimum_capacity)
{
    if (minimum_capacity > list->capacity) {
        list->items = checked_realloc(list->items,
                                      (size_t)minimum_capacity * sizeof(McfEdge));
        list->capacity = minimum_capacity;
    }
}

static McfGraph mcf_graph_create(int node_count)
{
    McfGraph graph;
    graph.node_count = node_count;
    solver_profile_note_mcf_graph(node_count);
    graph.adjacency = checked_calloc((size_t)node_count, sizeof(McfEdgeList));
    return graph;
}

static void mcf_graph_free(McfGraph *graph)
{
    int node_index;
    for (node_index = 0; node_index < graph->node_count; node_index++) {
        free(graph->adjacency[node_index].items);
    }
    free(graph->adjacency);
}

static int mcf_add_edge_with_fill(McfGraph *graph,
                                  int from_node,
                                  int to_node,
                                  int capacity,
                                  Cost cost,
                                  int fill_lab_index,
                                  int fill_delta)
{
    McfEdge forward_edge;
    McfEdge reverse_edge;
    int forward_index = graph->adjacency[from_node].size;
    int reverse_index = graph->adjacency[to_node].size;

    forward_edge.to_node = to_node;
    forward_edge.reverse_index = reverse_index;
    forward_edge.capacity = capacity;
    forward_edge.cost = cost;
    forward_edge.fill_lab_index = fill_lab_index;
    forward_edge.fill_delta = fill_delta;
    reverse_edge.to_node = from_node;
    reverse_edge.reverse_index = forward_index;
    reverse_edge.capacity = 0;
    reverse_edge.cost = cost_negate(cost);
    reverse_edge.fill_lab_index = fill_lab_index;
    reverse_edge.fill_delta = -fill_delta;
    mcf_list_push(&graph->adjacency[from_node], forward_edge);
    mcf_list_push(&graph->adjacency[to_node], reverse_edge);
    if (active_profile != NULL) {
        active_profile->mcf_edges_added++;
    }
    return forward_index;
}

static int mcf_add_edge(McfGraph *graph,
                        int from_node,
                        int to_node,
                        int capacity,
                        Cost cost)
{
    return mcf_add_edge_with_fill(graph,
                                  from_node,
                                  to_node,
                                  capacity,
                                  cost,
                                  -1,
                                  0);
}

static void assignment_arc_push(AssignmentArcList *list, AssignmentArc arc)
{
    if (list->size == list->capacity) {
        int new_capacity = list->capacity == 0 ? 256 : list->capacity * 2;
        list->items = checked_realloc(list->items,
                                      (size_t)new_capacity * sizeof(AssignmentArc));
        list->capacity = new_capacity;
    }
    list->items[list->size] = arc;
    list->size++;
}

static void assignment_arc_reserve(AssignmentArcList *list, int minimum_capacity)
{
    if (minimum_capacity > list->capacity) {
        list->items = checked_realloc(list->items,
                                      (size_t)minimum_capacity * sizeof(AssignmentArc));
        list->capacity = minimum_capacity;
    }
}

static void group_assignment_arc_push(GroupAssignmentArcList *list,
                                      GroupAssignmentArc arc)
{
    if (list->size == list->capacity) {
        int new_capacity = list->capacity == 0 ? 256 : list->capacity * 2;
        list->items = checked_realloc(list->items,
                                      (size_t)new_capacity *
                                          sizeof(GroupAssignmentArc));
        list->capacity = new_capacity;
    }
    list->items[list->size] = arc;
    list->size++;
}

static void group_assignment_arc_reserve(GroupAssignmentArcList *list,
                                         int minimum_capacity)
{
    if (minimum_capacity > list->capacity) {
        list->items = checked_realloc(list->items,
                                      (size_t)minimum_capacity *
                                          sizeof(GroupAssignmentArc));
        list->capacity = minimum_capacity;
    }
}

static unsigned int student_group_signature_value(const ProblemData *problem_data,
                                                  int student_index,
                                                  int lab_index,
                                                  int max_rank,
                                                  StudentGroupingMode grouping_mode)
{
    int rank_value;
    unsigned int signature_value;
    if (!assignment_edge_allowed(problem_data, student_index, lab_index, max_rank)) {
        return 0U;
    }
    rank_value = rank_for_assignment(problem_data, student_index, lab_index);
    if (grouping_mode == STUDENT_GROUP_ALLOWED_SET) {
        return 1U;
    }
    signature_value = (unsigned int)rank_value;
    if (problem_data->base_assignment != NULL &&
        problem_data->change_penalty > 0LL &&
        lab_index != problem_data->base_assignment[student_index]) {
        signature_value += (unsigned int)(problem_data->lab_count + 2);
    }
    return signature_value;
}

static unsigned long long hash_student_group_signature(
    const ProblemData *problem_data,
    int student_index,
    int max_rank,
    StudentGroupingMode grouping_mode)
{
    unsigned long long hash_value = FNV_OFFSET_BASIS;
    int lab_index;

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        unsigned int value = student_group_signature_value(problem_data,
                                                           student_index,
                                                           lab_index,
                                                           max_rank,
                                                           grouping_mode);
        int byte_index;
        for (byte_index = 0; byte_index < 4; byte_index++) {
            hash_value ^= (unsigned long long)(value & 0xffU);
            hash_value *= FNV_PRIME;
            value >>= 8;
        }
    }
    return hash_value;
}

static int same_student_group_signature(const ProblemData *problem_data,
                                        int left_student,
                                        int right_student,
                                        int max_rank,
                                        StudentGroupingMode grouping_mode)
{
    int lab_index;
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        unsigned int left_value =
            student_group_signature_value(problem_data,
                                          left_student,
                                          lab_index,
                                          max_rank,
                                          grouping_mode);
        unsigned int right_value =
            student_group_signature_value(problem_data,
                                          right_student,
                                          lab_index,
                                          max_rank,
                                          grouping_mode);
        if (left_value != right_value) {
            return 0;
        }
    }
    return 1;
}

static StudentGroups build_student_groups(const ProblemData *problem_data,
                                          int max_rank,
                                          StudentGroupingMode grouping_mode)
{
    StudentGroups groups;
    int hash_size = hash_table_size_for_count(problem_data->student_count,
                                              "student grouping");
    int *group_hash_indices;
    int student_index;
    int hash_index;

    if (active_profile != NULL) {
        active_profile->student_group_builds++;
    }
    groups.items =
        checked_calloc((size_t)problem_data->student_count, sizeof(StudentGroup));
    groups.next_member =
        checked_malloc((size_t)problem_data->student_count * sizeof(int));
    groups.student_to_group =
        checked_malloc((size_t)problem_data->student_count * sizeof(int));
    groups.count = 0;

    group_hash_indices = checked_malloc((size_t)hash_size * sizeof(int));
    for (hash_index = 0; hash_index < hash_size; hash_index++) {
        group_hash_indices[hash_index] = -1;
    }

    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        int slot_index =
            (int)(hash_student_group_signature(problem_data,
                                               student_index,
                                               max_rank,
                                               grouping_mode) &
                  (unsigned long long)(hash_size - 1));
        int group_index;

        for (;;) {
            group_index = group_hash_indices[slot_index];
            if (group_index < 0) {
                group_index = groups.count;
                groups.items[group_index].representative_student = student_index;
                groups.items[group_index].first_member = -1;
                groups.items[group_index].size = 0;
                groups.count++;
                group_hash_indices[slot_index] = group_index;
                break;
            }
            if (same_student_group_signature(
                    problem_data,
                    groups.items[group_index].representative_student,
                    student_index,
                    max_rank,
                    grouping_mode)) {
                break;
            }
            slot_index = (slot_index + 1) & (hash_size - 1);
        }

        groups.next_member[student_index] = groups.items[group_index].first_member;
        groups.items[group_index].first_member = student_index;
        groups.items[group_index].size++;
        groups.student_to_group[student_index] = group_index;
    }

    free(group_hash_indices);
    return groups;
}

static void free_student_groups(StudentGroups *groups)
{
    free(groups->items);
    free(groups->next_member);
    free(groups->student_to_group);
    groups->items = NULL;
    groups->next_member = NULL;
    groups->student_to_group = NULL;
    groups->count = 0;
}

static StudentGroupCache student_group_cache_create(int count,
                                                    StudentGroupingMode grouping_mode)
{
    StudentGroupCache cache;
    cache.count = count;
    cache.grouping_mode = grouping_mode;
    cache.groups = checked_calloc((size_t)count, sizeof(StudentGroups));
    cache.computed = checked_calloc((size_t)count, sizeof(unsigned char));
    return cache;
}

static const StudentGroups *student_group_cache_get(
    StudentGroupCache *cache,
    const ProblemData *problem_data,
    const IntList *rank_candidates,
    int q_index)
{
    if (q_index < 0 || q_index >= cache->count) {
        fail_with_context("student group cache", "cache index out of range");
    }
    if (!cache->computed[q_index]) {
        cache->groups[q_index] =
            build_student_groups(problem_data,
                                 rank_candidates->items[q_index],
                                 cache->grouping_mode);
        cache->computed[q_index] = 1U;
    }
    if (cache->groups[q_index].count >= problem_data->student_count) {
        return NULL;
    }
    return &cache->groups[q_index];
}

static void student_group_cache_free(StudentGroupCache *cache)
{
    int q_index;
    if (cache->groups != NULL && cache->computed != NULL) {
        for (q_index = 0; q_index < cache->count; q_index++) {
            if (cache->computed[q_index]) {
                free_student_groups(&cache->groups[q_index]);
            }
        }
    }
    free(cache->groups);
    free(cache->computed);
    cache->groups = NULL;
    cache->computed = NULL;
    cache->count = 0;
}

static void heap_init(MinHeap *heap, int node_count)
{
    int node_index;
    heap->items = checked_malloc((size_t)node_count * sizeof(HeapItem));
    heap->positions = checked_malloc((size_t)node_count * sizeof(int));
    heap->size = 0;
    heap->capacity = node_count;
    for (node_index = 0; node_index < node_count; node_index++) {
        heap->positions[node_index] = -1;
    }
}

static void heap_clear(MinHeap *heap)
{
    int item_index;
    for (item_index = 0; item_index < heap->size; item_index++) {
        heap->positions[heap->items[item_index].node] = -1;
    }
    heap->size = 0;
}

static void heap_place_item(MinHeap *heap, int item_index, HeapItem item)
{
    heap->items[item_index] = item;
    heap->positions[item.node] = item_index;
}

static void heap_sift_up(MinHeap *heap, int current_index)
{
    HeapItem item = heap->items[current_index];
    while (current_index > 0) {
        int parent_index = (current_index - 1) / HEAP_ARITY;
        if (!cost_less(item.distance, heap->items[parent_index].distance)) {
            break;
        }
        heap_place_item(heap, current_index, heap->items[parent_index]);
        current_index = parent_index;
    }
    heap_place_item(heap, current_index, item);
}

static void heap_push_or_decrease(MinHeap *heap, Cost distance, int node)
{
    int current_index = heap->positions[node];
    if (current_index >= 0) {
        if (!cost_less(distance, heap->items[current_index].distance)) {
            return;
        }
        heap->items[current_index].distance = distance;
        heap_sift_up(heap, current_index);
        return;
    }
    if (heap->size == heap->capacity) {
        fail_with_message("internal heap capacity error");
    }
    current_index = heap->size;
    heap->size++;
    heap->items[current_index].distance = distance;
    heap->items[current_index].node = node;
    heap->positions[node] = current_index;
    heap_sift_up(heap, current_index);
}

static int heap_pop(MinHeap *heap, HeapItem *result)
{
    HeapItem last_item;
    int current_index = 0;

    if (heap->size == 0) {
        return 0;
    }

    *result = heap->items[0];
    heap->positions[result->node] = -1;
    heap->size--;
    if (heap->size == 0) {
        return 1;
    }

    last_item = heap->items[heap->size];
    while (1) {
        int first_child = current_index * HEAP_ARITY + 1;
        int smaller_child;
        int child_offset;

        if (first_child >= heap->size) {
            break;
        }
        smaller_child = first_child;
        for (child_offset = 1; child_offset < HEAP_ARITY; child_offset++) {
            int child_index = first_child + child_offset;
            if (child_index < heap->size &&
                cost_less(heap->items[child_index].distance,
                          heap->items[smaller_child].distance)) {
                smaller_child = child_index;
            }
        }
        if (!cost_less(heap->items[smaller_child].distance, last_item.distance)) {
            break;
        }
        heap_place_item(heap, current_index, heap->items[smaller_child]);
        current_index = smaller_child;
    }
    heap_place_item(heap, current_index, last_item);
    return 1;
}

static void heap_free(MinHeap *heap)
{
    free(heap->items);
    free(heap->positions);
}

static int u64_add_fits(unsigned long long left,
                        unsigned long long right,
                        unsigned long long limit,
                        unsigned long long *out_value)
{
    if (left > limit || right > limit - left) {
        return 0;
    }
    *out_value = left + right;
    return 1;
}

static int u64_multiply_fits(unsigned long long left,
                             unsigned long long right,
                             unsigned long long limit,
                             unsigned long long *out_value)
{
    if (left != 0ULL && right > limit / left) {
        return 0;
    }
    *out_value = left * right;
    return 1;
}

static int radix_bucket_index(unsigned long long key,
                              unsigned long long last_key)
{
    unsigned long long difference = key ^ last_key;
    int index = 0;
    while (difference != 0ULL) {
        index++;
        difference >>= 1U;
    }
    return index;
}

static void radix_heap_bucket_push(RadixHeapBucket *bucket,
                                   RadixHeapItem item)
{
    if (bucket->size == bucket->capacity) {
        int new_capacity = bucket->capacity == 0 ? 8 : bucket->capacity * 2;
        if (new_capacity < bucket->capacity) {
            fail_with_context("radix heap", "capacity overflow");
        }
        bucket->items =
            checked_realloc(bucket->items,
                            (size_t)new_capacity * sizeof(RadixHeapItem));
        bucket->capacity = new_capacity;
    }
    bucket->items[bucket->size] = item;
    bucket->size++;
}

static void radix_heap_init(RadixHeap *heap)
{
    int bucket_index_value;
    heap->last_key = 0ULL;
    heap->size = 0;
    for (bucket_index_value = 0;
         bucket_index_value < 65;
         bucket_index_value++) {
        heap->buckets[bucket_index_value].items = NULL;
        heap->buckets[bucket_index_value].size = 0;
        heap->buckets[bucket_index_value].capacity = 0;
    }
}

static void radix_heap_push(RadixHeap *heap,
                            unsigned long long key,
                            int node)
{
    RadixHeapItem item;
    int bucket_index_value;
    if (key < heap->last_key) {
        fail_with_context("radix heap", "key order error");
    }
    item.key = key;
    item.node = node;
    bucket_index_value = radix_bucket_index(key, heap->last_key);
    radix_heap_bucket_push(&heap->buckets[bucket_index_value], item);
    heap->size++;
}

static int radix_heap_pop(RadixHeap *heap, RadixHeapItem *out_item)
{
    int bucket_index_value;
    if (heap->size == 0) {
        return 0;
    }
    if (heap->buckets[0].size == 0) {
        int source_bucket = -1;
        unsigned long long new_last_key = ULLONG_MAX;
        for (bucket_index_value = 1;
             bucket_index_value < 65;
             bucket_index_value++) {
            int item_index;
            if (heap->buckets[bucket_index_value].size == 0) {
                continue;
            }
            source_bucket = bucket_index_value;
            for (item_index = 0;
                 item_index < heap->buckets[bucket_index_value].size;
                 item_index++) {
                if (heap->buckets[bucket_index_value].items[item_index].key <
                    new_last_key) {
                    new_last_key =
                        heap->buckets[bucket_index_value].items[item_index].key;
                }
            }
            break;
        }
        if (source_bucket < 0) {
            fail_with_context("radix heap", "empty heap state");
        }
        heap->last_key = new_last_key;
        {
            RadixHeapBucket old_bucket = heap->buckets[source_bucket];
            int item_index;
            heap->buckets[source_bucket].items = NULL;
            heap->buckets[source_bucket].size = 0;
            heap->buckets[source_bucket].capacity = 0;
            for (item_index = 0; item_index < old_bucket.size; item_index++) {
                RadixHeapItem item = old_bucket.items[item_index];
                int new_bucket_index =
                    radix_bucket_index(item.key, heap->last_key);
                radix_heap_bucket_push(&heap->buckets[new_bucket_index], item);
            }
            free(old_bucket.items);
        }
    }

    if (heap->buckets[0].size == 0) {
        fail_with_context("radix heap", "missing minimum bucket");
    }
    heap->buckets[0].size--;
    *out_item = heap->buckets[0].items[heap->buckets[0].size];
    heap->size--;
    return 1;
}

static void radix_heap_free(RadixHeap *heap)
{
    int bucket_index_value;
    for (bucket_index_value = 0;
         bucket_index_value < 65;
         bucket_index_value++) {
        free(heap->buckets[bucket_index_value].items);
        heap->buckets[bucket_index_value].items = NULL;
        heap->buckets[bucket_index_value].size = 0;
        heap->buckets[bucket_index_value].capacity = 0;
    }
    heap->size = 0;
}

static int cost_to_radix_key(Cost value,
                             const RadixCostContext *context,
                             unsigned long long *out_key)
{
    unsigned long long first_part;
    unsigned long long second_part;
    unsigned long long lower_part;
    if (!context->available ||
        value.first < 0LL ||
        value.second < 0LL ||
        value.third < 0LL) {
        return 0;
    }
    if (!u64_multiply_fits((unsigned long long)value.first,
                           context->first_scale,
                           ULLONG_MAX,
                           &first_part)) {
        return 0;
    }
    if (!u64_multiply_fits((unsigned long long)value.second,
                           context->second_scale,
                           ULLONG_MAX,
                           &second_part)) {
        return 0;
    }
    if (!u64_add_fits(second_part,
                      (unsigned long long)value.third,
                      ULLONG_MAX,
                      &lower_part)) {
        return 0;
    }
    return u64_add_fits(first_part, lower_part, ULLONG_MAX, out_key);
}

static int try_build_radix_cost_context(const McfGraph *graph,
                                        const Cost *potential,
                                        RadixCostContext *context)
{
    int from_node;
    int residual_edge_slots = 0;
    unsigned long long max_first = 0ULL;
    unsigned long long max_second = 0ULL;
    unsigned long long max_third = 0ULL;
    unsigned long long max_path_first;
    unsigned long long max_first_key;
    unsigned long long max_path_third;
    unsigned long long second_scale;
    unsigned long long max_path_second;
    unsigned long long max_second_key;
    unsigned long long lower_component_range;
    unsigned long long lower_range_plus_one;

    context->available = 0;
    context->first_scale = 1ULL;
    context->second_scale = 1ULL;
    context->third_scale = 1ULL;
    if (active_profile != NULL) {
        active_profile->radix_heap_attempts++;
    }

    for (from_node = 0; from_node < graph->node_count; from_node++) {
        if (graph->adjacency[from_node].size >
            RADIX_HEAP_SCAN_EDGE_LIMIT - residual_edge_slots) {
            if (active_profile != NULL) {
                active_profile->radix_heap_fallbacks++;
            }
            return 0;
        }
        residual_edge_slots += graph->adjacency[from_node].size;
    }
    if (residual_edge_slots > RADIX_HEAP_SCAN_EDGE_LIMIT) {
        if (active_profile != NULL) {
            active_profile->radix_heap_fallbacks++;
        }
        return 0;
    }

    for (from_node = 0; from_node < graph->node_count; from_node++) {
        int edge_index;
        for (edge_index = 0;
             edge_index < graph->adjacency[from_node].size;
             edge_index++) {
            const McfEdge *edge =
                &graph->adjacency[from_node].items[edge_index];
            Cost reduced_cost;
            if (edge->capacity <= 0) {
                continue;
            }
            reduced_cost =
                cost_subtract(cost_add(edge->cost, potential[from_node]),
                              potential[edge->to_node]);
            if (cost_less(reduced_cost, cost_zero()) ||
                reduced_cost.first < 0LL ||
                reduced_cost.second < 0LL ||
                reduced_cost.third < 0LL) {
                if (active_profile != NULL) {
                    active_profile->radix_heap_fallbacks++;
                }
                return 0;
            }
            if ((unsigned long long)reduced_cost.first > max_first) {
                max_first = (unsigned long long)reduced_cost.first;
            }
            if ((unsigned long long)reduced_cost.second > max_second) {
                max_second = (unsigned long long)reduced_cost.second;
            }
            if ((unsigned long long)reduced_cost.third > max_third) {
                max_third = (unsigned long long)reduced_cost.third;
            }
        }
    }

    if (!u64_multiply_fits(max_third,
                           (unsigned long long)graph->node_count,
                           ULLONG_MAX - 1ULL,
                           &max_path_third) ||
        !u64_add_fits(max_path_third,
                      1ULL,
                      ULLONG_MAX,
                      &second_scale) ||
        !u64_multiply_fits(max_second,
                           (unsigned long long)graph->node_count,
                           ULLONG_MAX,
                           &max_path_second) ||
        !u64_multiply_fits(max_path_second,
                           second_scale,
                           ULLONG_MAX,
                           &max_second_key) ||
        !u64_add_fits(max_second_key,
                      max_path_third,
                      ULLONG_MAX - 1ULL,
                      &lower_component_range) ||
        !u64_add_fits(lower_component_range,
                      1ULL,
                      ULLONG_MAX,
                      &lower_range_plus_one) ||
        !u64_multiply_fits(max_first,
                           (unsigned long long)graph->node_count,
                           ULLONG_MAX,
                           &max_path_first) ||
        !u64_multiply_fits(max_path_first,
                           lower_range_plus_one,
                           ULLONG_MAX,
                           &max_first_key) ||
        !u64_add_fits(max_first_key,
                      lower_component_range,
                      ULLONG_MAX,
                      &max_first_key)) {
        if (active_profile != NULL) {
            active_profile->radix_heap_fallbacks++;
        }
        return 0;
    }
    (void)max_second_key;
    (void)max_first_key;
    context->available = 1;
    context->first_scale = lower_range_plus_one;
    context->second_scale = second_scale;
    context->third_scale = 1ULL;
    if (active_profile != NULL) {
        active_profile->radix_heap_used++;
    }
    return 1;
}

static int compute_initial_potentials_layered(const McfGraph *graph,
                                              int source_node,
                                              Cost *potential)
{
    int node_index;
    Cost infinite_cost = cost_infinity();

    for (node_index = 0; node_index < graph->node_count; node_index++) {
        potential[node_index] = infinite_cost;
    }
    potential[source_node] = cost_zero();

    for (node_index = 0; node_index < graph->node_count; node_index++) {
        int edge_index;
        if (cost_equal(potential[node_index], infinite_cost)) {
            continue;
        }
        for (edge_index = 0;
             edge_index < graph->adjacency[node_index].size;
             edge_index++) {
            const McfEdge *edge = &graph->adjacency[node_index].items[edge_index];
            Cost next_distance;
            if (edge->capacity <= 0) {
                continue;
            }
            if (edge->to_node <= node_index) {
                return 0;
            }
            next_distance = cost_add(potential[node_index], edge->cost);
            if (cost_less(next_distance, potential[edge->to_node])) {
                potential[edge->to_node] = next_distance;
            }
        }
    }

    for (node_index = 0; node_index < graph->node_count; node_index++) {
        if (cost_equal(potential[node_index], infinite_cost)) {
            potential[node_index] = cost_zero();
        }
    }
    if (active_profile != NULL) {
        active_profile->layered_initial_potentials_used++;
    }
    return 1;
}

static void exact_heap_init(ExactMinHeap *heap,
                            int node_count,
                            const ExactPathCost *distances,
                            const int *coefficients,
                            int coefficient_count,
                            const ExactAverageContext *context)
{
    int node_index;
    heap->items = checked_malloc((size_t)node_count * sizeof(ExactHeapItem));
    heap->positions = checked_malloc((size_t)node_count * sizeof(int));
    heap->size = 0;
    heap->capacity = node_count;
    heap->distances = distances;
    heap->coefficients = coefficients;
    heap->coefficient_count = coefficient_count;
    heap->context = context;
    for (node_index = 0; node_index < node_count; node_index++) {
        heap->positions[node_index] = -1;
    }
}

static void exact_heap_clear(ExactMinHeap *heap)
{
    int item_index;
    for (item_index = 0; item_index < heap->size; item_index++) {
        heap->positions[heap->items[item_index].node] = -1;
    }
    heap->size = 0;
}

static int exact_heap_node_less(const ExactMinHeap *heap, int left_node, int right_node)
{
    return exact_path_cost_less(heap->context,
                                &heap->distances[left_node],
                                const_coefficient_row(heap->coefficients,
                                                      heap->coefficient_count,
                                                      left_node),
                                &heap->distances[right_node],
                                const_coefficient_row(heap->coefficients,
                                                      heap->coefficient_count,
                                                      right_node));
}

static void exact_heap_place_item(ExactMinHeap *heap,
                                  int item_index,
                                  ExactHeapItem item)
{
    heap->items[item_index] = item;
    heap->positions[item.node] = item_index;
}

static void exact_heap_sift_up(ExactMinHeap *heap, int current_index)
{
    ExactHeapItem item = heap->items[current_index];
    while (current_index > 0) {
        int parent_index = (current_index - 1) / HEAP_ARITY;
        if (!exact_heap_node_less(heap, item.node, heap->items[parent_index].node)) {
            break;
        }
        exact_heap_place_item(heap, current_index, heap->items[parent_index]);
        current_index = parent_index;
    }
    exact_heap_place_item(heap, current_index, item);
}

static void exact_heap_push_or_decrease(ExactMinHeap *heap, int node)
{
    int current_index = heap->positions[node];
    if (current_index >= 0) {
        exact_heap_sift_up(heap, current_index);
        return;
    }
    if (heap->size == heap->capacity) {
        fail_with_message("internal exact heap capacity error");
    }
    current_index = heap->size;
    heap->size++;
    heap->items[current_index].node = node;
    heap->positions[node] = current_index;
    exact_heap_sift_up(heap, current_index);
}

static int exact_heap_pop(ExactMinHeap *heap, ExactHeapItem *result)
{
    ExactHeapItem last_item;
    int current_index = 0;

    if (heap->size == 0) {
        return 0;
    }

    *result = heap->items[0];
    heap->positions[result->node] = -1;
    heap->size--;
    if (heap->size == 0) {
        return 1;
    }

    last_item = heap->items[heap->size];
    while (1) {
        int first_child = current_index * HEAP_ARITY + 1;
        int smaller_child;
        int child_offset;

        if (first_child >= heap->size) {
            break;
        }
        smaller_child = first_child;
        for (child_offset = 1; child_offset < HEAP_ARITY; child_offset++) {
            int child_index = first_child + child_offset;
            if (child_index < heap->size &&
                exact_heap_node_less(heap,
                                     heap->items[child_index].node,
                                     heap->items[smaller_child].node)) {
                smaller_child = child_index;
            }
        }
        if (!exact_heap_node_less(heap, heap->items[smaller_child].node, last_item.node)) {
            break;
        }
        exact_heap_place_item(heap, current_index, heap->items[smaller_child]);
        current_index = smaller_child;
    }
    exact_heap_place_item(heap, current_index, last_item);
    return 1;
}

static void exact_heap_free(ExactMinHeap *heap)
{
    free(heap->items);
    free(heap->positions);
}

static void compute_initial_potentials(const McfGraph *graph,
                                       int source_node,
                                       Cost *potential)
{
    if (compute_initial_potentials_layered(graph, source_node, potential)) {
        return;
    }

    int queue_capacity = graph->node_count + 1;
    int *queue = checked_malloc((size_t)queue_capacity * sizeof(int));
    int *in_queue = checked_calloc((size_t)graph->node_count, sizeof(int));
    int head = 0;
    int tail = 0;
    int node_index;
    Cost infinite_cost = cost_infinity();

    for (node_index = 0; node_index < graph->node_count; node_index++) {
        potential[node_index] = infinite_cost;
    }
    potential[source_node] = cost_zero();
    queue[tail] = source_node;
    tail = (tail + 1) % queue_capacity;
    in_queue[source_node] = 1;

    while (head != tail) {
        int current_node = queue[head];
        int edge_index;
        head = (head + 1) % queue_capacity;
        in_queue[current_node] = 0;

        for (edge_index = 0; edge_index < graph->adjacency[current_node].size; edge_index++) {
            const McfEdge *edge = &graph->adjacency[current_node].items[edge_index];
            if (edge->capacity > 0 && !cost_equal(potential[current_node], infinite_cost)) {
                Cost next_distance = cost_add(potential[current_node], edge->cost);
                if (cost_less(next_distance, potential[edge->to_node])) {
                    potential[edge->to_node] = next_distance;
                    if (!in_queue[edge->to_node]) {
                        queue[tail] = edge->to_node;
                        tail = (tail + 1) % queue_capacity;
                        in_queue[edge->to_node] = 1;
                    }
                }
            }
        }
    }

    for (node_index = 0; node_index < graph->node_count; node_index++) {
        if (cost_equal(potential[node_index], infinite_cost)) {
            potential[node_index] = cost_zero();
        }
    }

    free(queue);
    free(in_queue);
}

static MinCostResult min_cost_flow(McfGraph *graph,
                                   int source_node,
                                   int sink_node,
                                   int requested_flow)
{
    Cost *potential = checked_malloc((size_t)graph->node_count * sizeof(Cost));
    Cost *distance = checked_malloc((size_t)graph->node_count * sizeof(Cost));
    int *previous_node = checked_malloc((size_t)graph->node_count * sizeof(int));
    int *previous_edge = checked_malloc((size_t)graph->node_count * sizeof(int));
    int *settled = checked_malloc((size_t)graph->node_count * sizeof(int));
    MinCostResult result;
    MinHeap heap;
    int radix_heap_disabled = 0;
    if (active_profile != NULL) {
        active_profile->min_cost_flow_calls++;
    }

    result.total_flow = 0;
    result.total_cost = cost_zero();
    heap_init(&heap, graph->node_count);
    compute_initial_potentials(graph, source_node, potential);

    while (result.total_flow < requested_flow) {
        HeapItem heap_item;
        int node_index;
        Cost infinite_cost = cost_infinity();

        heap_clear(&heap);

        for (node_index = 0; node_index < graph->node_count; node_index++) {
            distance[node_index] = infinite_cost;
            previous_node[node_index] = -1;
            previous_edge[node_index] = -1;
            settled[node_index] = 0;
        }

        distance[source_node] = cost_zero();
        {
            RadixCostContext radix_context;
            if (!radix_heap_disabled &&
                try_build_radix_cost_context(graph,
                                             potential,
                                             &radix_context)) {
                RadixHeap radix_heap;
                unsigned long long source_key;
                if (!cost_to_radix_key(cost_zero(),
                                       &radix_context,
                                       &source_key)) {
                    fail_with_message("internal radix heap source key error");
                }
                radix_heap_init(&radix_heap);
                radix_heap_push(&radix_heap, source_key, source_node);
                {
                    RadixHeapItem radix_item;
                    while (radix_heap_pop(&radix_heap, &radix_item)) {
                        int current_node = radix_item.node;
                        int edge_index;
                        unsigned long long current_key;
                        if (settled[current_node] ||
                            !cost_to_radix_key(distance[current_node],
                                               &radix_context,
                                               &current_key) ||
                            radix_item.key != current_key) {
                            continue;
                        }
                        settled[current_node] = 1;
                        for (edge_index = 0;
                             edge_index <
                                 graph->adjacency[current_node].size;
                             edge_index++) {
                            McfEdge *edge =
                                &graph->adjacency[current_node].items[edge_index];
                            if (edge->capacity > 0 &&
                                !settled[edge->to_node]) {
                                Cost reduced_cost =
                                    cost_subtract(
                                        cost_add(edge->cost,
                                                 potential[current_node]),
                                        potential[edge->to_node]);
                                Cost next_distance;
                                unsigned long long next_key;
                                if (cost_less(reduced_cost, cost_zero())) {
                                    fail_with_message(
                                        "internal min-cost-flow potential error");
                                }
                                next_distance =
                                    cost_add(distance[current_node],
                                             reduced_cost);
                                if (cost_less(next_distance,
                                              distance[edge->to_node])) {
                                    if (!cost_to_radix_key(next_distance,
                                                           &radix_context,
                                                           &next_key)) {
                                        fail_with_message(
                                            "internal radix heap key error");
                                    }
                                    distance[edge->to_node] = next_distance;
                                    previous_node[edge->to_node] =
                                        current_node;
                                    previous_edge[edge->to_node] = edge_index;
                                    radix_heap_push(&radix_heap,
                                                    next_key,
                                                    edge->to_node);
                                }
                            }
                        }
                    }
                }
                radix_heap_free(&radix_heap);
            } else {
                radix_heap_disabled = 1;
                heap_push_or_decrease(&heap, cost_zero(), source_node);

                while (heap_pop(&heap, &heap_item)) {
                    int current_node = heap_item.node;
                    int edge_index;
                    if (settled[current_node] ||
                        !cost_equal(heap_item.distance,
                                    distance[current_node])) {
                        continue;
                    }
                    settled[current_node] = 1;
                    for (edge_index = 0;
                         edge_index < graph->adjacency[current_node].size;
                         edge_index++) {
                        McfEdge *edge =
                            &graph->adjacency[current_node].items[edge_index];
                        if (edge->capacity > 0 && !settled[edge->to_node]) {
                            Cost reduced_cost =
                                cost_subtract(
                                    cost_add(edge->cost,
                                             potential[current_node]),
                                    potential[edge->to_node]);
                            Cost next_distance;
                            if (cost_less(reduced_cost, cost_zero())) {
                                fail_with_message(
                                    "internal min-cost-flow potential error");
                            }
                            next_distance =
                                cost_add(distance[current_node], reduced_cost);
                            if (cost_less(next_distance,
                                          distance[edge->to_node])) {
                                distance[edge->to_node] = next_distance;
                                previous_node[edge->to_node] = current_node;
                                previous_edge[edge->to_node] = edge_index;
                                heap_push_or_decrease(&heap,
                                                      next_distance,
                                                      edge->to_node);
                            }
                        }
                    }
                }
            }
        }

        if (previous_node[sink_node] < 0) {
            break;
        }

        for (node_index = 0; node_index < graph->node_count; node_index++) {
            if (!cost_equal(distance[node_index], infinite_cost)) {
                potential[node_index] = cost_add(potential[node_index], distance[node_index]);
            }
        }

        {
            int augment_flow = requested_flow - result.total_flow;
            int current_node = sink_node;
            while (current_node != source_node) {
                int parent_node = previous_node[current_node];
                int edge_index = previous_edge[current_node];
                McfEdge *edge = &graph->adjacency[parent_node].items[edge_index];
                if (edge->capacity < augment_flow) {
                    augment_flow = edge->capacity;
                }
                current_node = parent_node;
            }

            current_node = sink_node;
            while (current_node != source_node) {
                int parent_node = previous_node[current_node];
                int edge_index = previous_edge[current_node];
                McfEdge *edge = &graph->adjacency[parent_node].items[edge_index];
                McfEdge *reverse_edge = &graph->adjacency[edge->to_node].items[edge->reverse_index];
                edge->capacity -= augment_flow;
                reverse_edge->capacity += augment_flow;
                result.total_cost =
                    cost_add(result.total_cost, cost_multiply(edge->cost, augment_flow));
                current_node = parent_node;
            }
            result.total_flow += augment_flow;
        }
    }

    heap_free(&heap);
    free(potential);
    free(distance);
    free(previous_node);
    free(previous_edge);
    free(settled);
    return result;
}

static void compute_initial_exact_potentials(const McfGraph *graph,
                                             int source_node,
                                             ExactPathCost *potential,
                                             int *potential_coefficients,
                                             const ExactAverageContext *context)
{
    int layered_node_index;
    ExactPathCost layered_infinite_cost = exact_path_cost_infinity();
    int *layered_candidate_coefficients =
        checked_calloc((size_t)context->term_count, sizeof(int));
    int layered_ok = 1;

    for (layered_node_index = 0;
         layered_node_index < graph->node_count;
         layered_node_index++) {
        potential[layered_node_index] = layered_infinite_cost;
        memset(coefficient_row(potential_coefficients,
                               context->term_count,
                               layered_node_index),
               0,
               (size_t)context->term_count * sizeof(int));
    }
    potential[source_node] = exact_path_cost_zero();

    for (layered_node_index = 0;
         layered_ok && layered_node_index < graph->node_count;
         layered_node_index++) {
        int edge_index;
        if (exact_path_cost_is_infinity(potential[layered_node_index])) {
            continue;
        }
        for (edge_index = 0;
             edge_index < graph->adjacency[layered_node_index].size;
             edge_index++) {
            const McfEdge *edge =
                &graph->adjacency[layered_node_index].items[edge_index];
            ExactPathCost next_distance;
            if (edge->capacity <= 0) {
                continue;
            }
            if (edge->to_node <= layered_node_index) {
                layered_ok = 0;
                break;
            }
            next_distance =
                exact_path_cost_add(potential[layered_node_index],
                                    exact_path_cost_from_edge(edge));
            build_edge_adjusted_coefficients(
                layered_candidate_coefficients,
                const_coefficient_row(potential_coefficients,
                                      context->term_count,
                                      layered_node_index),
                edge,
                context->term_count);
            if (exact_path_cost_less(
                    context,
                    &next_distance,
                    layered_candidate_coefficients,
                    &potential[edge->to_node],
                    const_coefficient_row(potential_coefficients,
                                          context->term_count,
                                          edge->to_node))) {
                potential[edge->to_node] = next_distance;
                copy_coefficients(coefficient_row(potential_coefficients,
                                                  context->term_count,
                                                  edge->to_node),
                                  layered_candidate_coefficients,
                                  context->term_count);
            }
        }
    }
    if (layered_ok) {
        for (layered_node_index = 0;
             layered_node_index < graph->node_count;
             layered_node_index++) {
            if (exact_path_cost_is_infinity(potential[layered_node_index])) {
                potential[layered_node_index] = exact_path_cost_zero();
                memset(coefficient_row(potential_coefficients,
                                       context->term_count,
                                       layered_node_index),
                       0,
                       (size_t)context->term_count * sizeof(int));
            }
        }
        free(layered_candidate_coefficients);
        if (active_profile != NULL) {
            active_profile->layered_initial_potentials_used++;
        }
        return;
    }
    free(layered_candidate_coefficients);

    int queue_capacity = graph->node_count + 1;
    int *queue = checked_malloc((size_t)queue_capacity * sizeof(int));
    int *in_queue = checked_calloc((size_t)graph->node_count, sizeof(int));
    int *candidate_coefficients =
        checked_calloc((size_t)context->term_count, sizeof(int));
    int head = 0;
    int tail = 0;
    int node_index;
    ExactPathCost infinite_cost = exact_path_cost_infinity();

    for (node_index = 0; node_index < graph->node_count; node_index++) {
        potential[node_index] = infinite_cost;
        memset(coefficient_row(potential_coefficients, context->term_count, node_index),
               0,
               (size_t)context->term_count * sizeof(int));
    }
    potential[source_node] = exact_path_cost_zero();
    queue[tail] = source_node;
    tail = (tail + 1) % queue_capacity;
    in_queue[source_node] = 1;

    while (head != tail) {
        int current_node = queue[head];
        int edge_index;
        head = (head + 1) % queue_capacity;
        in_queue[current_node] = 0;

        for (edge_index = 0; edge_index < graph->adjacency[current_node].size; edge_index++) {
            const McfEdge *edge = &graph->adjacency[current_node].items[edge_index];
            if (edge->capacity > 0 && !exact_path_cost_is_infinity(potential[current_node])) {
                ExactPathCost next_distance =
                    exact_path_cost_add(potential[current_node],
                                        exact_path_cost_from_edge(edge));
                build_edge_adjusted_coefficients(
                    candidate_coefficients,
                    const_coefficient_row(potential_coefficients,
                                          context->term_count,
                                          current_node),
                    edge,
                    context->term_count);
                if (exact_path_cost_less(context,
                                         &next_distance,
                                         candidate_coefficients,
                                         &potential[edge->to_node],
                                         const_coefficient_row(potential_coefficients,
                                                               context->term_count,
                                                               edge->to_node))) {
                    potential[edge->to_node] = next_distance;
                    copy_coefficients(coefficient_row(potential_coefficients,
                                                      context->term_count,
                                                      edge->to_node),
                                      candidate_coefficients,
                                      context->term_count);
                    if (!in_queue[edge->to_node]) {
                        queue[tail] = edge->to_node;
                        tail = (tail + 1) % queue_capacity;
                        in_queue[edge->to_node] = 1;
                    }
                }
            }
        }
    }

    for (node_index = 0; node_index < graph->node_count; node_index++) {
        if (exact_path_cost_is_infinity(potential[node_index])) {
            potential[node_index] = exact_path_cost_zero();
            memset(coefficient_row(potential_coefficients, context->term_count, node_index),
                   0,
                   (size_t)context->term_count * sizeof(int));
        }
    }

    free(candidate_coefficients);
    free(queue);
    free(in_queue);
}

static MinCostResult min_cost_flow_exact_average(McfGraph *graph,
                                                 int source_node,
                                                 int sink_node,
                                                 int requested_flow,
                                                 const ExactAverageContext *context)
{
    ExactPathCost *potential =
        checked_malloc((size_t)graph->node_count * sizeof(ExactPathCost));
    ExactPathCost *distance =
        checked_malloc((size_t)graph->node_count * sizeof(ExactPathCost));
    int *potential_coefficients =
        checked_calloc((size_t)graph->node_count *
                       (size_t)context->term_count,
                       sizeof(int));
    int *distance_coefficients =
        checked_calloc((size_t)graph->node_count *
                       (size_t)context->term_count,
                       sizeof(int));
    int *reduced_coefficients =
        checked_calloc((size_t)context->term_count, sizeof(int));
    int *next_coefficients =
        checked_calloc((size_t)context->term_count, sizeof(int));
    int *zero_coefficients =
        checked_calloc((size_t)context->term_count, sizeof(int));
    int *previous_node = checked_malloc((size_t)graph->node_count * sizeof(int));
    int *previous_edge = checked_malloc((size_t)graph->node_count * sizeof(int));
    int *settled = checked_malloc((size_t)graph->node_count * sizeof(int));
    MinCostResult result;
    ExactMinHeap heap;
    if (active_profile != NULL) {
        active_profile->exact_min_cost_flow_calls++;
    }

    result.total_flow = 0;
    result.total_cost = cost_zero();
    compute_initial_exact_potentials(graph,
                                     source_node,
                                     potential,
                                     potential_coefficients,
                                     context);
    exact_heap_init(&heap,
                    graph->node_count,
                    distance,
                    distance_coefficients,
                    context->term_count,
                    context);

    while (result.total_flow < requested_flow) {
        ExactHeapItem heap_item;
        int node_index;
        ExactPathCost infinite_cost = exact_path_cost_infinity();

        exact_heap_clear(&heap);

        for (node_index = 0; node_index < graph->node_count; node_index++) {
            distance[node_index] = infinite_cost;
            previous_node[node_index] = -1;
            previous_edge[node_index] = -1;
            settled[node_index] = 0;
        }
        memset(distance_coefficients,
               0,
               (size_t)graph->node_count *
                   (size_t)context->term_count *
                   sizeof(int));

        distance[source_node] = exact_path_cost_zero();
        exact_heap_push_or_decrease(&heap, source_node);

        while (exact_heap_pop(&heap, &heap_item)) {
            int current_node = heap_item.node;
            int edge_index;
            if (settled[current_node]) {
                continue;
            }
            settled[current_node] = 1;
            for (edge_index = 0; edge_index < graph->adjacency[current_node].size; edge_index++) {
                McfEdge *edge = &graph->adjacency[current_node].items[edge_index];
                if (edge->capacity > 0 && !settled[edge->to_node]) {
                    ExactPathCost edge_cost = exact_path_cost_from_edge(edge);
                    ExactPathCost reduced_cost =
                        exact_path_cost_subtract(
                            exact_path_cost_add(edge_cost, potential[current_node]),
                            potential[edge->to_node]);
                    ExactPathCost next_distance;
                    int coefficient_index;
                    for (coefficient_index = 0;
                         coefficient_index < context->term_count;
                         coefficient_index++) {
                        reduced_coefficients[coefficient_index] =
                            potential_coefficients[(size_t)current_node *
                                                       (size_t)context->term_count +
                                                   (size_t)coefficient_index] -
                            potential_coefficients[(size_t)edge->to_node *
                                                       (size_t)context->term_count +
                                                   (size_t)coefficient_index];
                    }
                    if (edge->fill_lab_index >= 0) {
                        reduced_coefficients[edge->fill_lab_index] += edge->fill_delta;
                    }
                    if (exact_path_cost_less(context,
                                             &reduced_cost,
                                             reduced_coefficients,
                                             &(ExactPathCost){cost_zero()},
                                             zero_coefficients)) {
                        fail_with_message("internal exact min-cost-flow potential error");
                    }
                    next_distance = exact_path_cost_add(distance[current_node], reduced_cost);
                    for (coefficient_index = 0;
                         coefficient_index < context->term_count;
                         coefficient_index++) {
                        next_coefficients[coefficient_index] =
                            distance_coefficients[(size_t)current_node *
                                                      (size_t)context->term_count +
                                                  (size_t)coefficient_index] +
                            reduced_coefficients[coefficient_index];
                    }
                    if (exact_path_cost_less(context,
                                             &next_distance,
                                             next_coefficients,
                                             &distance[edge->to_node],
                                             const_coefficient_row(distance_coefficients,
                                                                   context->term_count,
                                                                   edge->to_node))) {
                        distance[edge->to_node] = next_distance;
                        copy_coefficients(coefficient_row(distance_coefficients,
                                                          context->term_count,
                                                          edge->to_node),
                                          next_coefficients,
                                          context->term_count);
                        previous_node[edge->to_node] = current_node;
                        previous_edge[edge->to_node] = edge_index;
                        exact_heap_push_or_decrease(&heap, edge->to_node);
                    }
                }
            }
        }

        if (previous_node[sink_node] < 0) {
            break;
        }

        for (node_index = 0; node_index < graph->node_count; node_index++) {
            if (!exact_path_cost_is_infinity(distance[node_index])) {
                potential[node_index] =
                    exact_path_cost_add(potential[node_index], distance[node_index]);
                add_coefficients(coefficient_row(potential_coefficients,
                                                 context->term_count,
                                                 node_index),
                                 const_coefficient_row(distance_coefficients,
                                                       context->term_count,
                                                       node_index),
                                 context->term_count);
            }
        }

        {
            int augment_flow = requested_flow - result.total_flow;
            int current_node = sink_node;
            while (current_node != source_node) {
                int parent_node = previous_node[current_node];
                int edge_index = previous_edge[current_node];
                McfEdge *edge = &graph->adjacency[parent_node].items[edge_index];
                if (edge->capacity < augment_flow) {
                    augment_flow = edge->capacity;
                }
                current_node = parent_node;
            }

            current_node = sink_node;
            while (current_node != source_node) {
                int parent_node = previous_node[current_node];
                int edge_index = previous_edge[current_node];
                McfEdge *edge = &graph->adjacency[parent_node].items[edge_index];
                McfEdge *reverse_edge =
                    &graph->adjacency[edge->to_node].items[edge->reverse_index];
                edge->capacity -= augment_flow;
                reverse_edge->capacity += augment_flow;
                result.total_cost =
                    cost_add(result.total_cost, cost_multiply(edge->cost, augment_flow));
                current_node = parent_node;
            }
            result.total_flow += augment_flow;
        }
    }

    exact_heap_free(&heap);
    free(potential);
    free(distance);
    free(potential_coefficients);
    free(distance_coefficients);
    free(reduced_coefficients);
    free(next_coefficients);
    free(zero_coefficients);
    free(previous_node);
    free(previous_edge);
    free(settled);
    return result;
}

static long long checked_add_weighted_term(long long total_value,
                                           long long weight,
                                           long long value,
                                           const char *context)
{
    long long term_value;
    if (weight < 0LL || value < 0LL) {
        fail_with_context(context, "negative weight is not supported");
    }
    if (value != 0LL && weight > INF_COST / value) {
        fail_with_context(context, "weighted objective cost overflow");
    }
    term_value = weight * value;
    if (total_value > INF_COST - term_value) {
        fail_with_context(context, "weighted objective cost overflow");
    }
    return total_value + term_value;
}

static long long checked_add_rank_cost(long long left_value,
                                       long long right_value,
                                       const char *context)
{
    if (left_value < 0LL || right_value < 0LL) {
        fail_with_context(context, "negative rank cost is not supported");
    }
    if (left_value > INF_COST - right_value) {
        fail_with_context(context, "rank cost overflow");
    }
    return left_value + right_value;
}

static long long checked_multiply_rank_cost(long long left_value,
                                            long long right_value,
                                            const char *context)
{
    if (left_value < 0LL || right_value < 0LL) {
        fail_with_context(context, "negative rank cost is not supported");
    }
    if (right_value != 0LL && left_value > INF_COST / right_value) {
        fail_with_context(context, "rank cost overflow");
    }
    return left_value * right_value;
}

static void rank_cost_model_init_default(RankCostModel *model)
{
    model->use_explicit_table = 0;
    model->rank_costs = NULL;
    model->first_choice_gap = 100LL;
    model->tail_linear = 30LL;
    model->tail_quadratic = 5LL;
    model->outside_cost = 10000LL;
}

static void rank_cost_model_free(RankCostModel *model)
{
    free(model->rank_costs);
    model->rank_costs = NULL;
    model->use_explicit_table = 0;
}

static long long rank_cost_formula_value(const RankCostModel *model, int rank_value)
{
    long long offset;
    long long offset_square;
    long long linear_part;
    long long quadratic_part;
    long long tail_part;

    if (rank_value <= 1) {
        return 0LL;
    }

    offset = (long long)rank_value - 2LL;
    offset_square =
        checked_multiply_rank_cost(offset, offset, "rank satisfaction cost");
    linear_part =
        checked_multiply_rank_cost(model->tail_linear,
                                   offset,
                                   "rank satisfaction cost");
    quadratic_part =
        checked_multiply_rank_cost(model->tail_quadratic,
                                   offset_square,
                                   "rank satisfaction cost");
    tail_part =
        checked_add_rank_cost(linear_part,
                              quadratic_part,
                              "rank satisfaction cost");
    return checked_add_rank_cost(model->first_choice_gap,
                                 tail_part,
                                 "rank satisfaction cost");
}

static long long dissatisfaction_cost_for_rank(const ProblemData *problem_data,
                                               const RankCostModel *model,
                                               int rank_value)
{
    int outside_rank = problem_data->lab_count + 1;

    if (model == NULL) {
        return (long long)rank_value;
    }
    if (rank_value <= 1) {
        return 0LL;
    }
    if (model->use_explicit_table) {
        return model->rank_costs[rank_value];
    }
    if (rank_value == outside_rank) {
        return model->outside_cost;
    }
    return rank_cost_formula_value(model, rank_value);
}

static void validate_rank_cost_model(const ProblemData *problem_data,
                                     const RankCostModel *model)
{
    int max_rank = problem_data->lab_count + 1;
    int rank_value;
    long long per_edge_limit;

    if (model == NULL) {
        return;
    }
    if (problem_data->student_count <= 0) {
        fail_with_context("rank cost", "student count must be positive");
    }
    per_edge_limit = INF_COST / (long long)problem_data->student_count / 4LL;
    if (per_edge_limit <= 0LL) {
        fail_with_context("rank cost", "rank cost limit overflow");
    }
    for (rank_value = 1; rank_value <= max_rank; rank_value++) {
        long long dissatisfaction =
            dissatisfaction_cost_for_rank(problem_data, model, rank_value);
        long long dissatisfaction_square;
        if (dissatisfaction < 0LL || dissatisfaction > per_edge_limit) {
            fail_with_context("rank cost", "rank cost is too large");
        }
        dissatisfaction_square =
            checked_multiply_rank_cost(dissatisfaction,
                                       dissatisfaction,
                                       "rank satisfaction cost");
        if (dissatisfaction_square > per_edge_limit) {
            fail_with_context("rank cost", "squared rank cost is too large");
        }
    }
}

static Cost assignment_rank_cost_scaled(const ProblemData *problem_data,
                                        int student_index,
                                        int lab_index,
                                        const RankCostModel *rank_cost_model,
                                        const WeightedObjective *weighted_objective,
                                        long long ordinary_third_multiplier)
{
    int rank_value = rank_for_assignment(problem_data, student_index, lab_index);
    int changed_from_base =
        problem_data->base_assignment != NULL &&
        lab_index != problem_data->base_assignment[student_index];
    if (weighted_objective == NULL) {
        long long dissatisfaction =
            rank_cost_model == NULL ?
            (long long)rank_value :
            dissatisfaction_cost_for_rank(problem_data, rank_cost_model, rank_value);
        long long primary_cost = dissatisfaction;
        long long square_cost =
            rank_cost_model == NULL ?
            (long long)rank_value * (long long)rank_value :
            checked_multiply_rank_cost(dissatisfaction,
                                       dissatisfaction,
                                       "rank satisfaction cost");
        if (ordinary_third_multiplier != 1LL) {
            square_cost =
                checked_cost_component_multiply(square_cost,
                                                ordinary_third_multiplier,
                                                "ordinary average-fill scalar fast path");
        }
        if (changed_from_base && problem_data->change_penalty > 0LL) {
            primary_cost =
                checked_add_weighted_term(primary_cost,
                                          problem_data->change_penalty,
                                          1LL,
                                          "change penalty");
        }
        return cost_make(0LL, primary_cost, square_cost);
    }
    {
        long long weighted_cost = 0LL;
        long long rank_square = (long long)rank_value * (long long)rank_value;
        weighted_cost = checked_add_weighted_term(weighted_cost,
                                                  weighted_objective->rank_sum,
                                                  (long long)rank_value,
                                                  "weighted exact objective");
        weighted_cost = checked_add_weighted_term(weighted_cost,
                                                  weighted_objective->rank_square,
                                                  rank_square,
                                                  "weighted exact objective");
        if (rank_value > problem_data->max_preferences) {
            weighted_cost = checked_add_weighted_term(weighted_cost,
                                                      weighted_objective->outside,
                                                      1LL,
                                                      "weighted exact objective");
        }
        if (changed_from_base) {
            weighted_cost = checked_add_weighted_term(weighted_cost,
                                                      weighted_objective->change,
                                                      1LL,
                                                      "weighted exact objective");
        }
        return cost_make(0LL, weighted_cost, (long long)rank_value);
    }
}

static Cost assignment_rank_cost(const ProblemData *problem_data,
                                 int student_index,
                                 int lab_index,
                                 const RankCostModel *rank_cost_model,
                                 const WeightedObjective *weighted_objective)
{
    return assignment_rank_cost_scaled(problem_data,
                                       student_index,
                                       lab_index,
                                       rank_cost_model,
                                       weighted_objective,
                                       1LL);
}

static Cost lab_sink_cost_with_reward(long long minimum_fill_component,
                                      int lab_index,
                                      const long long *lab_average_rewards,
                                      AverageRewardPlacement placement)
{
    long long average_reward = lab_average_rewards == NULL ?
                               0LL :
                               lab_average_rewards[lab_index];
    if (placement == AVERAGE_REWARD_SECOND) {
        return cost_make(minimum_fill_component, -average_reward, 0LL);
    }
    if (placement == AVERAGE_REWARD_THIRD) {
        return cost_make(minimum_fill_component, 0LL, -average_reward);
    }
    return cost_make(minimum_fill_component, 0LL, 0LL);
}

static void ordinary_average_fast_path_free(OrdinaryAverageFastPath *fast_path)
{
    free(fast_path->lab_average_rewards);
    fast_path->lab_average_rewards = NULL;
    fast_path->available = 0;
    fast_path->third_multiplier = 1LL;
}

static int average_reward_bucket_compare_ascending(const void *left,
                                                   const void *right)
{
    const AverageRewardBucket *left_bucket =
        (const AverageRewardBucket *)left;
    const AverageRewardBucket *right_bucket =
        (const AverageRewardBucket *)right;
    if (left_bucket->reward < right_bucket->reward) {
        return -1;
    }
    if (left_bucket->reward > right_bucket->reward) {
        return 1;
    }
    return 0;
}

static int average_reward_bucket_compare_descending(const void *left,
                                                    const void *right)
{
    return -average_reward_bucket_compare_ascending(left, right);
}

static int sum_extreme_average_rewards(const AverageRewardBucket *buckets,
                                       int bucket_count,
                                       int student_count,
                                       int descending,
                                       long long *out_sum)
{
    AverageRewardBucket *ordered_buckets;
    long long total_sum = 0LL;
    int remaining_students = student_count;
    int bucket_index;

    if (bucket_count <= 0) {
        return student_count == 0;
    }

    ordered_buckets =
        checked_malloc((size_t)bucket_count * sizeof(AverageRewardBucket));
    memcpy(ordered_buckets,
           buckets,
           (size_t)bucket_count * sizeof(AverageRewardBucket));
    qsort(ordered_buckets,
          (size_t)bucket_count,
          sizeof(AverageRewardBucket),
          descending ?
          average_reward_bucket_compare_descending :
          average_reward_bucket_compare_ascending);

    for (bucket_index = 0;
         bucket_index < bucket_count && remaining_students > 0;
         bucket_index++) {
        int take_count = ordered_buckets[bucket_index].capacity <
                         remaining_students ?
                         ordered_buckets[bucket_index].capacity :
                         remaining_students;
        long long reward_sum;
        long long new_total_sum;

        if (take_count <= 0) {
            continue;
        }
        if (!multiply_nonnegative_ll_fits(ordered_buckets[bucket_index].reward,
                                          (long long)take_count,
                                          LLONG_MAX,
                                          &reward_sum) ||
            !add_nonnegative_ll_fits(total_sum,
                                     reward_sum,
                                     LLONG_MAX,
                                     &new_total_sum)) {
            free(ordered_buckets);
            return 0;
        }
        total_sum = new_total_sum;
        remaining_students -= take_count;
    }

    free(ordered_buckets);
    if (remaining_students > 0) {
        return 0;
    }
    *out_sum = total_sum;
    return 1;
}

static int ordinary_average_reward_difference_bound(
    const ProblemData *problem_data,
    int best_max_rank,
    const long long *lab_average_rewards,
    long long *out_difference_bound)
{
    AverageRewardBucket *buckets =
        checked_malloc((size_t)problem_data->lab_count *
                       sizeof(AverageRewardBucket));
    int *active_incoming_counts =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));
    int bucket_count = 0;
    int lab_index;
    int student_index;
    long long maximum_reward_sum;
    long long minimum_reward_sum;

    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
            if (assignment_edge_allowed(problem_data,
                                        student_index,
                                        lab_index,
                                        best_max_rank)) {
                active_incoming_counts[lab_index]++;
            }
        }
    }

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int effective_capacity;

        if (problem_data->labs[lab_index].capacity_value <= 0LL ||
            problem_data->labs[lab_index].graph_capacity <= 0 ||
            active_incoming_counts[lab_index] <= 0) {
            continue;
        }
        effective_capacity = problem_data->labs[lab_index].graph_capacity;
        if (effective_capacity > active_incoming_counts[lab_index]) {
            effective_capacity = active_incoming_counts[lab_index];
        }
        buckets[bucket_count].reward = lab_average_rewards[lab_index];
        buckets[bucket_count].capacity = effective_capacity;
        bucket_count++;
    }

    if (!sum_extreme_average_rewards(buckets,
                                     bucket_count,
                                     problem_data->student_count,
                                     1,
                                     &maximum_reward_sum) ||
        !sum_extreme_average_rewards(buckets,
                                     bucket_count,
                                     problem_data->student_count,
                                     0,
                                     &minimum_reward_sum) ||
        maximum_reward_sum < minimum_reward_sum) {
        free(active_incoming_counts);
        free(buckets);
        return 0;
    }

    *out_difference_bound = maximum_reward_sum - minimum_reward_sum;
    free(active_incoming_counts);
    free(buckets);
    return 1;
}

static OrdinaryAverageFastPath ordinary_average_fast_path_create(
    const ProblemData *problem_data,
    const ExactAverageContext *context,
    const RankCostModel *rank_cost_model,
    int best_max_rank)
{
    OrdinaryAverageFastPath fast_path;
    unsigned long long lcm_u64;
    long long min_reward = LLONG_MAX;
    long long max_reward = 0LL;
    long long max_reward_difference;
    long long third_multiplier;
    long long max_third_unit = 0LL;
    long long max_scaled_third_unit;
    long long max_edge_third_abs;
    long long per_unit_limit;
    int lab_index;
    int rank_value;
    int active_rank_max = best_max_rank;
    int maximum_rank = problem_data->lab_count + 1;

    fast_path.available = 0;
    fast_path.third_multiplier = 1LL;
    fast_path.lab_average_rewards = NULL;

    if (active_profile != NULL) {
        active_profile->ordinary_average_scalar_attempts++;
    }
    if (context == NULL || context->positive_lab_count <= 0 ||
        problem_data->student_count <= 0) {
        if (active_profile != NULL) {
            active_profile->ordinary_average_scalar_fallback_not_applicable++;
        }
        return fast_path;
    }
    if (!big_uint_to_u64_checked(&context->common_denominator, &lcm_u64) ||
        lcm_u64 > (unsigned long long)LLONG_MAX) {
        if (active_profile != NULL) {
            active_profile->ordinary_average_scalar_fallback_lcm++;
        }
        return fast_path;
    }

    fast_path.lab_average_rewards =
        checked_calloc((size_t)problem_data->lab_count, sizeof(long long));
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        long long capacity_value = problem_data->labs[lab_index].capacity_value;
        long long reward_value;
        if (capacity_value <= 0LL) {
            continue;
        }
        if (lcm_u64 % (unsigned long long)capacity_value != 0ULL ||
            lcm_u64 / (unsigned long long)capacity_value >
                (unsigned long long)LLONG_MAX) {
            ordinary_average_fast_path_free(&fast_path);
            if (active_profile != NULL) {
                active_profile->ordinary_average_scalar_fallback_lcm++;
            }
            return fast_path;
        }
        reward_value = (long long)(lcm_u64 / (unsigned long long)capacity_value);
        fast_path.lab_average_rewards[lab_index] = reward_value;
        if (reward_value < min_reward) {
            min_reward = reward_value;
        }
        if (reward_value > max_reward) {
            max_reward = reward_value;
        }
    }
    if (min_reward == LLONG_MAX) {
        ordinary_average_fast_path_free(&fast_path);
        if (active_profile != NULL) {
            active_profile->ordinary_average_scalar_fallback_not_applicable++;
        }
        return fast_path;
    }

    if (!ordinary_average_reward_difference_bound(problem_data,
                                                  best_max_rank,
                                                  fast_path.lab_average_rewards,
                                                  &max_reward_difference)) {
        ordinary_average_fast_path_free(&fast_path);
        if (active_profile != NULL) {
            active_profile->ordinary_average_scalar_fallback_overflow++;
        }
        return fast_path;
    }
    if (!add_nonnegative_ll_fits(max_reward_difference,
                                 1LL,
                                 LLONG_MAX,
                                 &third_multiplier)) {
        ordinary_average_fast_path_free(&fast_path);
        if (active_profile != NULL) {
            active_profile->ordinary_average_scalar_fallback_overflow++;
        }
        return fast_path;
    }
    per_unit_limit = INF_COST / (long long)problem_data->student_count / 8LL;
    if (third_multiplier <= 0LL || per_unit_limit <= 0LL) {
        ordinary_average_fast_path_free(&fast_path);
        if (active_profile != NULL) {
            active_profile->ordinary_average_scalar_fallback_overflow++;
        }
        return fast_path;
    }

    if (active_rank_max > maximum_rank) {
        active_rank_max = maximum_rank;
    }
    if (active_rank_max < 1) {
        active_rank_max = 1;
    }

    for (rank_value = 1; rank_value <= active_rank_max; rank_value++) {
        long long unit_value;
        if (rank_cost_model == NULL) {
            unit_value = checked_multiply_rank_cost((long long)rank_value,
                                                    (long long)rank_value,
                                                    "ordinary average-fill scalar fast path");
        } else {
            long long dissatisfaction =
                dissatisfaction_cost_for_rank(problem_data, rank_cost_model, rank_value);
            unit_value = checked_multiply_rank_cost(
                dissatisfaction,
                dissatisfaction,
                "ordinary average-fill scalar fast path");
        }
        if (unit_value > max_third_unit) {
            max_third_unit = unit_value;
        }
    }

    if (!multiply_nonnegative_ll_fits(max_third_unit,
                                      third_multiplier,
                                      per_unit_limit,
                                      &max_scaled_third_unit) ||
        !add_nonnegative_ll_fits(max_scaled_third_unit,
                                 max_reward,
                                 per_unit_limit,
                                 &max_edge_third_abs)) {
        ordinary_average_fast_path_free(&fast_path);
        if (active_profile != NULL) {
            active_profile->ordinary_average_scalar_fallback_overflow++;
        }
        return fast_path;
    }

    fast_path.available = 1;
    fast_path.third_multiplier = third_multiplier;
    if (active_profile != NULL) {
        active_profile->ordinary_average_scalar_used++;
    }
    return fast_path;
}

static Cost canonical_rank_first_cost_for_solution(
    const ProblemData *problem_data,
    const int *assignment,
    const int *lab_counts,
    const int *minimum_lab_counts,
    const RankCostModel *rank_cost_model)
{
    Cost total = cost_zero();
    int lab_index;
    int student_index;
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int filled_required_slots = lab_counts[lab_index];
        if (filled_required_slots > minimum_lab_counts[lab_index]) {
            filled_required_slots = minimum_lab_counts[lab_index];
        }
        if (filled_required_slots > 0) {
            total.first =
                checked_cost_component_subtract(total.first,
                                                (long long)filled_required_slots,
                                                "canonical solution cost");
        }
    }
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        Cost student_cost =
            assignment_rank_cost(problem_data,
                                 student_index,
                                 assignment[student_index],
                                 rank_cost_model,
                                 NULL);
        total.second =
            checked_cost_component_add(total.second,
                                       student_cost.second,
                                       "canonical solution cost");
        total.third =
            checked_cost_component_add(total.third,
                                      student_cost.third,
                                      "canonical solution cost");
    }
    return total;
}

static ConvexFillContext convex_fill_context_create(const ProblemData *problem_data)
{
    ConvexFillContext context;
    int lab_index;
    long long total_graph_capacity = 0LL;
    if (problem_data->student_count <= 0) {
        fail_with_context("fill-convex objective", "student count must be positive");
    }
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        total_graph_capacity =
            checked_add_rank_cost(total_graph_capacity,
                                  (long long)problem_data->labs[lab_index].graph_capacity,
                                  "fill-convex objective");
    }
    if (total_graph_capacity <= 0LL) {
        fail_with_context("fill-convex objective", "positive graph capacity is required");
    }
    context.total_graph_capacity = total_graph_capacity;
    context.student_count = (long long)problem_data->student_count;
    context.per_unit_limit = INF_COST / context.student_count / 8LL;
    if (context.per_unit_limit <= 0LL) {
        fail_with_context("fill-convex objective", "cost limit overflow");
    }
    return context;
}

static long long convex_fill_marginal_cost(const ProblemData *problem_data,
                                           const ConvexFillContext *context,
                                           int lab_index,
                                           int one_based_slot)
{
    long long graph_capacity =
        (long long)problem_data->labs[lab_index].graph_capacity;
    long long total_capacity = context->total_graph_capacity;
    long long student_count = context->student_count;
    long long slot_factor = 2LL * (long long)one_based_slot - 1LL;
    long long total_capacity_square =
        checked_multiply_rank_cost(total_capacity,
                                   total_capacity,
                                   "fill-convex objective");
    long long first_term =
        checked_multiply_rank_cost(total_capacity_square,
                                   slot_factor,
                                   "fill-convex objective");
    long long second_term =
        checked_multiply_rank_cost(2LL,
                                   total_capacity,
                                   "fill-convex objective");
    second_term =
        checked_multiply_rank_cost(second_term,
                                   student_count,
                                   "fill-convex objective");
    second_term =
        checked_multiply_rank_cost(second_term,
                                   graph_capacity,
                                   "fill-convex objective");
    if (first_term >= second_term) {
        long long result = first_term - second_term;
        if (result > context->per_unit_limit) {
            fail_with_context("fill-convex objective", "cost overflow");
        }
        return result;
    }
    {
        long long result = second_term - first_term;
        if (result > context->per_unit_limit) {
            fail_with_context("fill-convex objective", "cost overflow");
        }
        return -result;
    }
}

static SolutionResult solve_with_minimum_counts_ungrouped(
    const ProblemData *problem_data,
    int best_max_rank,
    const int *minimum_lab_counts,
    int optimize_fill_average,
    int enforce_minimum_counts,
    const ExactAverageContext *exact_average_context,
    const RankCostModel *rank_cost_model,
    const WeightedObjective *weighted_objective,
    const long long *lab_average_rewards,
    const ConvexFillContext *convex_fill_context,
    const OrdinaryAverageFastPath *ordinary_average_fast_path,
    int fail_on_infeasible)
{
    int source_node = 0;
    int first_student_node = 1;
    int first_lab_node = first_student_node + problem_data->student_count;
    int sink_node = first_lab_node + problem_data->lab_count;
    int node_count = sink_node + 1;
    McfGraph graph = mcf_graph_create(node_count);
    AssignmentArcList arcs;
    MinCostResult result;
    SolutionResult solution;
    const UngroupedActiveArcTemplate *active_arc_template =
        ungrouped_active_arc_template_get(problem_data, best_max_rank);
    int student_index;
    int lab_index;
    int arc_index;
    int sink_reverse_edges = 0;
    int use_ordinary_average_scalar =
        optimize_fill_average &&
        ordinary_average_fast_path != NULL &&
        ordinary_average_fast_path->available;
    int add_exact_average_coefficients =
        exact_average_context != NULL &&
        optimize_fill_average &&
        !use_ordinary_average_scalar;
    long long ordinary_third_multiplier =
        use_ordinary_average_scalar ?
        ordinary_average_fast_path->third_multiplier :
        1LL;
    const long long *active_lab_average_rewards = lab_average_rewards;
    AverageRewardPlacement reward_placement =
        lab_average_rewards == NULL ?
        AVERAGE_REWARD_NONE :
        AVERAGE_REWARD_SECOND;

    if (use_ordinary_average_scalar) {
        active_lab_average_rewards =
            ordinary_average_fast_path->lab_average_rewards;
        reward_placement = AVERAGE_REWARD_THIRD;
    }

    solution.assignment = checked_malloc((size_t)problem_data->student_count * sizeof(int));
    solution.lab_counts = checked_calloc((size_t)problem_data->lab_count, sizeof(int));
    solution.total_cost = cost_zero();

    arcs.items = NULL;
    arcs.size = 0;
    arcs.capacity = 0;
    if (optimize_fill_average && exact_average_context == NULL) {
        fail_with_context("solver", "exact average-fill context is missing");
    }

    mcf_list_reserve(&graph.adjacency[source_node], problem_data->student_count);
    assignment_arc_reserve(&arcs, active_arc_template->arc_count);
    for (student_index = 0; student_index < problem_data->student_count; student_index++) {
        int student_node = first_student_node + student_index;
        mcf_list_reserve(&graph.adjacency[student_node],
                         active_arc_template
                             ->allowed_counts_by_student[student_index] + 1);
    }
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int lab_node = first_lab_node + lab_index;
        int graph_capacity = problem_data->labs[lab_index].graph_capacity;
        int required_slots = minimum_lab_counts[lab_index];
        int slot_edge_count = 0;
        if (graph_capacity > 0) {
            if (required_slots < 0 || required_slots > graph_capacity) {
                free(solution.assignment);
                free(solution.lab_counts);
                free(arcs.items);
                mcf_graph_free(&graph);
                if (fail_on_infeasible) {
                    fail_with_context("solver", "invalid lab minimum count");
                }
                return empty_solution_result();
            }
            if (convex_fill_context != NULL) {
                slot_edge_count = graph_capacity;
            } else {
                if (required_slots > 0) {
                    slot_edge_count++;
                }
                if (graph_capacity - required_slots > 0) {
                    slot_edge_count++;
                }
            }
        }
        sink_reverse_edges += slot_edge_count;
        mcf_list_reserve(&graph.adjacency[lab_node],
                         active_arc_template
                             ->incoming_counts_by_lab[lab_index] +
                             slot_edge_count);
    }
    mcf_list_reserve(&graph.adjacency[sink_node], sink_reverse_edges);

    for (student_index = 0; student_index < problem_data->student_count; student_index++) {
        int student_node = first_student_node + student_index;
        solution.assignment[student_index] = -1;
        mcf_add_edge(&graph, source_node, student_node, 1, cost_zero());
    }

    for (arc_index = 0;
         arc_index < active_arc_template->arc_count;
         arc_index++) {
        ActiveAssignmentArc active_arc =
            active_arc_template->arcs[arc_index];
        int student_node = first_student_node + active_arc.student_index;
        int lab_node = first_lab_node + active_arc.lab_index;
        int edge_index = mcf_add_edge(&graph,
                                      student_node,
                                      lab_node,
                                      1,
                                      assignment_rank_cost_scaled(
                                          problem_data,
                                          active_arc.student_index,
                                          active_arc.lab_index,
                                          rank_cost_model,
                                          weighted_objective,
                                          ordinary_third_multiplier));
        AssignmentArc arc;
        arc.student_index = active_arc.student_index;
        arc.lab_index = active_arc.lab_index;
        arc.edge_index = edge_index;
        assignment_arc_push(&arcs, arc);
    }

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int lab_node = first_lab_node + lab_index;
        int graph_capacity = problem_data->labs[lab_index].graph_capacity;
        int required_slots = minimum_lab_counts[lab_index];
        int optional_slots;
        if (graph_capacity <= 0) {
            continue;
        }
        if (required_slots < 0 || required_slots > graph_capacity) {
            free(solution.assignment);
            free(solution.lab_counts);
            free(arcs.items);
            mcf_graph_free(&graph);
            if (fail_on_infeasible) {
                fail_with_context("solver", "invalid lab minimum count");
            }
            return empty_solution_result();
        }
        if (convex_fill_context != NULL) {
            int slot_index;
            for (slot_index = 1; slot_index <= graph_capacity; slot_index++) {
                long long minimum_component =
                    slot_index <= required_slots ? -1LL : 0LL;
                long long convex_cost =
                    convex_fill_marginal_cost(problem_data,
                                              convex_fill_context,
                                              lab_index,
                                              slot_index);
                mcf_add_edge(&graph,
                             lab_node,
                             sink_node,
                             1,
                             cost_make(minimum_component, convex_cost, 0LL));
            }
            continue;
        }
        if (required_slots > 0) {
            if (add_exact_average_coefficients) {
                mcf_add_edge_with_fill(&graph,
                                       lab_node,
                                       sink_node,
                                       required_slots,
                                       lab_sink_cost_with_reward(
                                           -1LL,
                                           lab_index,
                                           active_lab_average_rewards,
                                           reward_placement),
                                       exact_average_context->term_by_lab[lab_index],
                                       -1);
            } else {
                mcf_add_edge(&graph,
                             lab_node,
                             sink_node,
                             required_slots,
                             lab_sink_cost_with_reward(-1LL,
                                                       lab_index,
                                                       active_lab_average_rewards,
                                                       reward_placement));
            }
        }
        optional_slots = graph_capacity - required_slots;
        if (optional_slots > 0) {
            if (add_exact_average_coefficients) {
                mcf_add_edge_with_fill(&graph,
                                       lab_node,
                                       sink_node,
                                       optional_slots,
                                       lab_sink_cost_with_reward(
                                           0LL,
                                           lab_index,
                                           active_lab_average_rewards,
                                           reward_placement),
                                       exact_average_context->term_by_lab[lab_index],
                                       -1);
            } else {
                mcf_add_edge(&graph,
                             lab_node,
                             sink_node,
                             optional_slots,
                             lab_sink_cost_with_reward(0LL,
                                                       lab_index,
                                                       active_lab_average_rewards,
                                                       reward_placement));
            }
        }
    }

    if (add_exact_average_coefficients) {
        result = min_cost_flow_exact_average(&graph,
                                             source_node,
                                             sink_node,
                                             problem_data->student_count,
                                             exact_average_context);
    } else {
        result = min_cost_flow(&graph, source_node, sink_node, problem_data->student_count);
    }
    solution.total_cost = result.total_cost;
    if (result.total_flow != problem_data->student_count) {
        free(solution.assignment);
        free(solution.lab_counts);
        free(arcs.items);
        mcf_graph_free(&graph);
        if (fail_on_infeasible) {
            fail_with_context("solver", "failed to send flow for every student");
        }
        if (active_profile != NULL) {
            active_profile->try_solve_infeasible++;
        }
        return empty_solution_result();
    }

    for (arc_index = 0; arc_index < arcs.size; arc_index++) {
        AssignmentArc arc = arcs.items[arc_index];
        int student_node = first_student_node + arc.student_index;
        const McfEdge *edge = &graph.adjacency[student_node].items[arc.edge_index];
        if (edge->capacity == 0) {
            if (solution.assignment[arc.student_index] >= 0) {
                free(solution.assignment);
                free(solution.lab_counts);
                free(arcs.items);
                mcf_graph_free(&graph);
                fail_with_context("solver", "student has multiple assignments");
            }
            solution.assignment[arc.student_index] = arc.lab_index;
            solution.lab_counts[arc.lab_index]++;
        }
    }

    for (student_index = 0; student_index < problem_data->student_count; student_index++) {
        if (solution.assignment[student_index] < 0) {
            free(solution.assignment);
            free(solution.lab_counts);
            free(arcs.items);
            mcf_graph_free(&graph);
            fail_with_context("solver", "student was not assigned");
        }
    }

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        if ((long long)solution.lab_counts[lab_index] >
            problem_data->labs[lab_index].capacity_value) {
            free(solution.assignment);
            free(solution.lab_counts);
            free(arcs.items);
            mcf_graph_free(&graph);
            fail_with_context("solver", "lab capacity was exceeded");
        }
        if (enforce_minimum_counts &&
            solution.lab_counts[lab_index] < minimum_lab_counts[lab_index]) {
            free(solution.assignment);
            free(solution.lab_counts);
            free(arcs.items);
            mcf_graph_free(&graph);
            fail_with_context("solver", "lab minimum count was not met");
        }
    }

    if (use_ordinary_average_scalar) {
        solution.total_cost =
            canonical_rank_first_cost_for_solution(problem_data,
                                                   solution.assignment,
                                                   solution.lab_counts,
                                                   minimum_lab_counts,
                                                   rank_cost_model);
    }

    free(arcs.items);
    mcf_graph_free(&graph);
    return solution;
}

static SolutionResult solve_with_minimum_counts_grouped(
    const ProblemData *problem_data,
    int best_max_rank,
    const int *minimum_lab_counts,
    int optimize_fill_average,
    int enforce_minimum_counts,
    const ExactAverageContext *exact_average_context,
    const RankCostModel *rank_cost_model,
    const WeightedObjective *weighted_objective,
    const long long *lab_average_rewards,
    const ConvexFillContext *convex_fill_context,
    const OrdinaryAverageFastPath *ordinary_average_fast_path,
    const StudentGroups *groups,
    int fail_on_infeasible)
{
    int source_node = 0;
    int first_group_node = 1;
    int first_lab_node = first_group_node + groups->count;
    int sink_node = first_lab_node + problem_data->lab_count;
    int node_count = sink_node + 1;
    McfGraph graph = mcf_graph_create(node_count);
    GroupAssignmentArcList arcs;
    MinCostResult result;
    SolutionResult solution;
    int group_index;
    int lab_index;
    int arc_index;
    int *allowed_counts_by_group =
        checked_calloc((size_t)groups->count, sizeof(int));
    int *incoming_counts_by_lab =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));
    int *member_cursor = checked_malloc((size_t)groups->count * sizeof(int));
    int total_assignment_arcs = 0;
    int sink_reverse_edges = 0;
    int use_ordinary_average_scalar =
        optimize_fill_average &&
        ordinary_average_fast_path != NULL &&
        ordinary_average_fast_path->available;
    int add_exact_average_coefficients =
        exact_average_context != NULL &&
        optimize_fill_average &&
        !use_ordinary_average_scalar;
    long long ordinary_third_multiplier =
        use_ordinary_average_scalar ?
        ordinary_average_fast_path->third_multiplier :
        1LL;
    const long long *active_lab_average_rewards = lab_average_rewards;
    AverageRewardPlacement reward_placement =
        lab_average_rewards == NULL ?
        AVERAGE_REWARD_NONE :
        AVERAGE_REWARD_SECOND;

    if (use_ordinary_average_scalar) {
        active_lab_average_rewards =
            ordinary_average_fast_path->lab_average_rewards;
        reward_placement = AVERAGE_REWARD_THIRD;
    }

    solution.assignment =
        checked_malloc((size_t)problem_data->student_count * sizeof(int));
    solution.lab_counts =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));
    solution.total_cost = cost_zero();

    arcs.items = NULL;
    arcs.size = 0;
    arcs.capacity = 0;

    if (optimize_fill_average && exact_average_context == NULL) {
        fail_with_context("solver", "exact average-fill context is missing");
    }

    for (group_index = 0; group_index < groups->count; group_index++) {
        int representative_student = groups->items[group_index].representative_student;
        for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
            if (assignment_edge_allowed(problem_data,
                                        representative_student,
                                        lab_index,
                                        best_max_rank)) {
                allowed_counts_by_group[group_index]++;
                incoming_counts_by_lab[lab_index]++;
                total_assignment_arcs++;
            }
        }
    }

    mcf_list_reserve(&graph.adjacency[source_node], groups->count);
    group_assignment_arc_reserve(&arcs, total_assignment_arcs);
    for (group_index = 0; group_index < groups->count; group_index++) {
        int group_node = first_group_node + group_index;
        mcf_list_reserve(&graph.adjacency[group_node],
                         allowed_counts_by_group[group_index] + 1);
        member_cursor[group_index] = groups->items[group_index].first_member;
    }
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int lab_node = first_lab_node + lab_index;
        int graph_capacity = problem_data->labs[lab_index].graph_capacity;
        int required_slots = minimum_lab_counts[lab_index];
        int slot_edge_count = 0;
        if (graph_capacity > 0) {
            if (required_slots < 0 || required_slots > graph_capacity) {
                free(solution.assignment);
                free(solution.lab_counts);
                free(arcs.items);
                free(allowed_counts_by_group);
                free(incoming_counts_by_lab);
                free(member_cursor);
                mcf_graph_free(&graph);
                if (fail_on_infeasible) {
                    fail_with_context("solver", "invalid lab minimum count");
                }
                return empty_solution_result();
            }
            if (convex_fill_context != NULL) {
                slot_edge_count = graph_capacity;
            } else {
                if (required_slots > 0) {
                    slot_edge_count++;
                }
                if (graph_capacity - required_slots > 0) {
                    slot_edge_count++;
                }
            }
        }
        sink_reverse_edges += slot_edge_count;
        mcf_list_reserve(&graph.adjacency[lab_node],
                         incoming_counts_by_lab[lab_index] + slot_edge_count);
    }
    mcf_list_reserve(&graph.adjacency[sink_node], sink_reverse_edges);
    free(allowed_counts_by_group);
    free(incoming_counts_by_lab);

    for (group_index = 0; group_index < groups->count; group_index++) {
        int group_node = first_group_node + group_index;
        int representative_student = groups->items[group_index].representative_student;
        mcf_add_edge(&graph,
                     source_node,
                     group_node,
                     groups->items[group_index].size,
                     cost_zero());
        for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
            if (assignment_edge_allowed(problem_data,
                                        representative_student,
                                        lab_index,
                                        best_max_rank)) {
                int lab_node = first_lab_node + lab_index;
                int edge_index = mcf_add_edge(&graph,
                                              group_node,
                                              lab_node,
                                              groups->items[group_index].size,
                                              assignment_rank_cost_scaled(
                                                  problem_data,
                                                  representative_student,
                                                  lab_index,
                                                  rank_cost_model,
                                                  weighted_objective,
                                                  ordinary_third_multiplier));
                GroupAssignmentArc arc;
                arc.group_index = group_index;
                arc.lab_index = lab_index;
                arc.edge_index = edge_index;
                group_assignment_arc_push(&arcs, arc);
            }
        }
    }

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int lab_node = first_lab_node + lab_index;
        int graph_capacity = problem_data->labs[lab_index].graph_capacity;
        int required_slots = minimum_lab_counts[lab_index];
        int optional_slots;
        if (graph_capacity <= 0) {
            continue;
        }
        if (required_slots < 0 || required_slots > graph_capacity) {
            free(solution.assignment);
            free(solution.lab_counts);
            free(arcs.items);
            free(member_cursor);
            mcf_graph_free(&graph);
            if (fail_on_infeasible) {
                fail_with_context("solver", "invalid lab minimum count");
            }
            return empty_solution_result();
        }
        if (convex_fill_context != NULL) {
            int slot_index;
            for (slot_index = 1; slot_index <= graph_capacity; slot_index++) {
                long long minimum_component =
                    slot_index <= required_slots ? -1LL : 0LL;
                long long convex_cost =
                    convex_fill_marginal_cost(problem_data,
                                              convex_fill_context,
                                              lab_index,
                                              slot_index);
                mcf_add_edge(&graph,
                             lab_node,
                             sink_node,
                             1,
                             cost_make(minimum_component, convex_cost, 0LL));
            }
            continue;
        }
        if (required_slots > 0) {
            if (add_exact_average_coefficients) {
                mcf_add_edge_with_fill(&graph,
                                       lab_node,
                                       sink_node,
                                       required_slots,
                                       lab_sink_cost_with_reward(
                                           -1LL,
                                           lab_index,
                                           active_lab_average_rewards,
                                           reward_placement),
                                       exact_average_context->term_by_lab[lab_index],
                                       -1);
            } else {
                mcf_add_edge(&graph,
                             lab_node,
                             sink_node,
                             required_slots,
                             lab_sink_cost_with_reward(-1LL,
                                                       lab_index,
                                                       active_lab_average_rewards,
                                                       reward_placement));
            }
        }
        optional_slots = graph_capacity - required_slots;
        if (optional_slots > 0) {
            if (add_exact_average_coefficients) {
                mcf_add_edge_with_fill(&graph,
                                       lab_node,
                                       sink_node,
                                       optional_slots,
                                       lab_sink_cost_with_reward(
                                           0LL,
                                           lab_index,
                                           active_lab_average_rewards,
                                           reward_placement),
                                       exact_average_context->term_by_lab[lab_index],
                                       -1);
            } else {
                mcf_add_edge(&graph,
                             lab_node,
                             sink_node,
                             optional_slots,
                             lab_sink_cost_with_reward(0LL,
                                                       lab_index,
                                                       active_lab_average_rewards,
                                                       reward_placement));
            }
        }
    }

    if (add_exact_average_coefficients) {
        result = min_cost_flow_exact_average(&graph,
                                             source_node,
                                             sink_node,
                                             problem_data->student_count,
                                             exact_average_context);
    } else {
        result = min_cost_flow(&graph, source_node, sink_node, problem_data->student_count);
    }
    solution.total_cost = result.total_cost;
    if (result.total_flow != problem_data->student_count) {
        free(solution.assignment);
        free(solution.lab_counts);
        free(arcs.items);
        free(member_cursor);
        mcf_graph_free(&graph);
        if (fail_on_infeasible) {
            fail_with_context("solver", "failed to send flow for every student");
        }
        if (active_profile != NULL) {
            active_profile->try_solve_infeasible++;
        }
        return empty_solution_result();
    }

    for (group_index = 0; group_index < groups->count; group_index++) {
        int member_index = groups->items[group_index].first_member;
        while (member_index >= 0) {
            solution.assignment[member_index] = -1;
            member_index = groups->next_member[member_index];
        }
    }

    for (arc_index = 0; arc_index < arcs.size; arc_index++) {
        GroupAssignmentArc arc = arcs.items[arc_index];
        int group_node = first_group_node + arc.group_index;
        const McfEdge *edge = &graph.adjacency[group_node].items[arc.edge_index];
        int flow_sent = groups->items[arc.group_index].size - edge->capacity;
        while (flow_sent > 0) {
            int student_index = member_cursor[arc.group_index];
            if (student_index < 0) {
                free(solution.assignment);
                free(solution.lab_counts);
                free(arcs.items);
                free(member_cursor);
                mcf_graph_free(&graph);
                fail_with_context("solver", "group assignment count is inconsistent");
            }
            member_cursor[arc.group_index] = groups->next_member[student_index];
            if (solution.assignment[student_index] >= 0) {
                free(solution.assignment);
                free(solution.lab_counts);
                free(arcs.items);
                free(member_cursor);
                mcf_graph_free(&graph);
                fail_with_context("solver", "student has multiple assignments");
            }
            solution.assignment[student_index] = arc.lab_index;
            solution.lab_counts[arc.lab_index]++;
            flow_sent--;
        }
    }

    for (group_index = 0; group_index < groups->count; group_index++) {
        if (member_cursor[group_index] >= 0) {
            free(solution.assignment);
            free(solution.lab_counts);
            free(arcs.items);
            free(member_cursor);
            mcf_graph_free(&graph);
            fail_with_context("solver", "student was not assigned");
        }
    }

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        if ((long long)solution.lab_counts[lab_index] >
            problem_data->labs[lab_index].capacity_value) {
            free(solution.assignment);
            free(solution.lab_counts);
            free(arcs.items);
            free(member_cursor);
            mcf_graph_free(&graph);
            fail_with_context("solver", "lab capacity was exceeded");
        }
        if (enforce_minimum_counts &&
            solution.lab_counts[lab_index] < minimum_lab_counts[lab_index]) {
            free(solution.assignment);
            free(solution.lab_counts);
            free(arcs.items);
            free(member_cursor);
            mcf_graph_free(&graph);
            fail_with_context("solver", "lab minimum count was not met");
        }
    }

    if (use_ordinary_average_scalar) {
        solution.total_cost =
            canonical_rank_first_cost_for_solution(problem_data,
                                                   solution.assignment,
                                                   solution.lab_counts,
                                                   minimum_lab_counts,
                                                   rank_cost_model);
    }

    free(arcs.items);
    free(member_cursor);
    mcf_graph_free(&graph);
    return solution;
}

static SolutionResult solve_with_minimum_counts(
    const ProblemData *problem_data,
    int best_max_rank,
    const int *minimum_lab_counts,
    int optimize_fill_average,
    int enforce_minimum_counts,
    const ExactAverageContext *exact_average_context,
    const RankCostModel *rank_cost_model,
    const WeightedObjective *weighted_objective,
    const long long *lab_average_rewards,
    const ConvexFillContext *convex_fill_context,
    const StudentGroups *student_groups)
{
    OrdinaryAverageFastPath ordinary_average_fast_path;
    const OrdinaryAverageFastPath *ordinary_average_fast_path_pointer = NULL;
    SolutionResult result;

    ordinary_average_fast_path.available = 0;
    ordinary_average_fast_path.third_multiplier = 1LL;
    ordinary_average_fast_path.lab_average_rewards = NULL;

    if (optimize_fill_average &&
        exact_average_context != NULL &&
        weighted_objective == NULL &&
        lab_average_rewards == NULL &&
        convex_fill_context == NULL) {
        ordinary_average_fast_path =
            ordinary_average_fast_path_create(problem_data,
                                              exact_average_context,
                                              rank_cost_model,
                                              best_max_rank);
        if (ordinary_average_fast_path.available) {
            ordinary_average_fast_path_pointer = &ordinary_average_fast_path;
        }
    }

    if (student_groups == NULL ||
        student_groups->count == problem_data->student_count) {
        result = solve_with_minimum_counts_ungrouped(problem_data,
                                                     best_max_rank,
                                                     minimum_lab_counts,
                                                     optimize_fill_average,
                                                     enforce_minimum_counts,
                                                     exact_average_context,
                                                     rank_cost_model,
                                                     weighted_objective,
                                                     lab_average_rewards,
                                                     convex_fill_context,
                                                     ordinary_average_fast_path_pointer,
                                                     1);
    } else {
        result = solve_with_minimum_counts_grouped(problem_data,
                                                   best_max_rank,
                                                   minimum_lab_counts,
                                                   optimize_fill_average,
                                                   enforce_minimum_counts,
                                                   exact_average_context,
                                                   rank_cost_model,
                                                   weighted_objective,
                                                   lab_average_rewards,
                                                   convex_fill_context,
                                                   ordinary_average_fast_path_pointer,
                                                   student_groups,
                                                   1);
    }

    ordinary_average_fast_path_free(&ordinary_average_fast_path);
    return result;
}

static OptionalSolutionResult try_solve_with_minimum_counts(
    const ProblemData *problem_data,
    int best_max_rank,
    const int *minimum_lab_counts,
    int optimize_fill_average,
    int enforce_minimum_counts,
    const ExactAverageContext *exact_average_context,
    const RankCostModel *rank_cost_model,
    const WeightedObjective *weighted_objective,
    const long long *lab_average_rewards,
    const ConvexFillContext *convex_fill_context,
    const StudentGroups *student_groups)
{
    OptionalSolutionResult result;
    OrdinaryAverageFastPath ordinary_average_fast_path;
    const OrdinaryAverageFastPath *ordinary_average_fast_path_pointer = NULL;

    ordinary_average_fast_path.available = 0;
    ordinary_average_fast_path.third_multiplier = 1LL;
    ordinary_average_fast_path.lab_average_rewards = NULL;

    if (optimize_fill_average &&
        exact_average_context != NULL &&
        weighted_objective == NULL &&
        lab_average_rewards == NULL &&
        convex_fill_context == NULL) {
        ordinary_average_fast_path =
            ordinary_average_fast_path_create(problem_data,
                                              exact_average_context,
                                              rank_cost_model,
                                              best_max_rank);
        if (ordinary_average_fast_path.available) {
            ordinary_average_fast_path_pointer = &ordinary_average_fast_path;
        }
    }

    result.feasible = 0;
    if (student_groups == NULL ||
        student_groups->count == problem_data->student_count) {
        result.solution =
            solve_with_minimum_counts_ungrouped(problem_data,
                                                best_max_rank,
                                                minimum_lab_counts,
                                                optimize_fill_average,
                                                enforce_minimum_counts,
                                                exact_average_context,
                                                rank_cost_model,
                                                weighted_objective,
                                                lab_average_rewards,
                                                convex_fill_context,
                                                ordinary_average_fast_path_pointer,
                                                0);
    } else {
        result.solution =
            solve_with_minimum_counts_grouped(problem_data,
                                             best_max_rank,
                                             minimum_lab_counts,
                                             optimize_fill_average,
                                             enforce_minimum_counts,
                                             exact_average_context,
                                             rank_cost_model,
                                             weighted_objective,
                                             lab_average_rewards,
                                             convex_fill_context,
                                             ordinary_average_fast_path_pointer,
                                             student_groups,
                                             0);
    }
    result.feasible = result.solution.assignment != NULL;
    ordinary_average_fast_path_free(&ordinary_average_fast_path);
    return result;
}

static void free_solution_result(SolutionResult *solution)
{
    free(solution->assignment);
    free(solution->lab_counts);
    solution->assignment = NULL;
    solution->lab_counts = NULL;
}

static SolutionResult empty_solution_result(void)
{
    SolutionResult solution;
    solution.assignment = NULL;
    solution.lab_counts = NULL;
    solution.total_cost = cost_zero();
    return solution;
}

static SolutionResult copy_solution_result(const ProblemData *problem_data,
                                           const SolutionResult *source)
{
    SolutionResult copy = empty_solution_result();
    copy.total_cost = source->total_cost;
    if (source->assignment != NULL) {
        copy.assignment =
            checked_malloc((size_t)problem_data->student_count * sizeof(int));
        memcpy(copy.assignment,
               source->assignment,
               (size_t)problem_data->student_count * sizeof(int));
    }
    if (source->lab_counts != NULL) {
        copy.lab_counts =
            checked_malloc((size_t)problem_data->lab_count * sizeof(int));
        memcpy(copy.lab_counts,
               source->lab_counts,
               (size_t)problem_data->lab_count * sizeof(int));
    }
    return copy;
}

static OptionalSolutionResult copy_optional_solution_result(
    const ProblemData *problem_data,
    const OptionalSolutionResult *source)
{
    OptionalSolutionResult copy;
    copy.feasible = source->feasible;
    copy.solution = copy_solution_result(problem_data, &source->solution);
    return copy;
}

static int *build_base_minimum_counts(const ProblemData *problem_data)
{
    int lab_index;
    int *minimum_lab_counts =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        if (problem_data->labs[lab_index].capacity_value > 0LL) {
            minimum_lab_counts[lab_index] = 1;
            {
                int target_count =
                    target_minimum_count_for_lab(problem_data, lab_index);
                if (target_count > minimum_lab_counts[lab_index]) {
                    minimum_lab_counts[lab_index] = target_count;
                }
            }
        }
    }
    return minimum_lab_counts;
}

static int sum_minimum_counts(const ProblemData *problem_data, const int *minimum_lab_counts)
{
    int lab_index;
    int total_count = 0;
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        total_count += minimum_lab_counts[lab_index];
    }
    return total_count;
}

static int graph_capacity_sum_equals_student_count(const ProblemData *problem_data)
{
    int lab_index;
    int total_capacity = 0;
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        if (problem_data->labs[lab_index].graph_capacity >
            problem_data->student_count - total_capacity) {
            return 0;
        }
        total_capacity += problem_data->labs[lab_index].graph_capacity;
    }
    return total_capacity == problem_data->student_count;
}

static int positive_capacities_are_uniform(const ProblemData *problem_data)
{
    int lab_index;
    long long first_positive_capacity = -1LL;
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        long long capacity_value = problem_data->labs[lab_index].capacity_value;
        if (capacity_value <= 0LL) {
            continue;
        }
        if (first_positive_capacity < 0LL) {
            first_positive_capacity = capacity_value;
        } else if (capacity_value != first_positive_capacity) {
            return 0;
        }
    }
    return 1;
}

static void ratio_list_push(RatioList *list, RatioValue ratio)
{
    if (list->size == list->capacity) {
        int new_capacity = list->capacity == 0 ? 256 : list->capacity * 2;
        list->items = checked_realloc(list->items,
                                      (size_t)new_capacity * sizeof(RatioValue));
        list->capacity = new_capacity;
    }
    list->items[list->size] = ratio;
    list->size++;
}

static int ratio_compare_value(RatioValue left, RatioValue right)
{
    WideProduct left_scaled =
        multiply_small_nonnegative(left.numerator, right.denominator);
    WideProduct right_scaled =
        multiply_small_nonnegative(right.numerator, left.denominator);
    return wide_product_compare(left_scaled, right_scaled);
}

static int ratio_qsort_compare(const void *left_pointer, const void *right_pointer)
{
    const RatioValue *left = (const RatioValue *)left_pointer;
    const RatioValue *right = (const RatioValue *)right_pointer;
    return ratio_compare_value(*left, *right);
}

static RatioList build_ratio_candidates(const ProblemData *problem_data)
{
    const TargetConstraints *targets = problem_data->targets;
    RatioValue lower_bound;
    RatioValue upper_bound;
    int has_lower_bound = 0;
    int total_capacity_bound_is_available = 1;
    long long total_positive_capacity = 0LL;
    int lab_index;
    RatioList list;
    list.items = NULL;
    list.size = 0;
    list.capacity = 0;

    lower_bound.numerator = 0LL;
    lower_bound.denominator = 1LL;
    upper_bound.numerator = 1LL;
    upper_bound.denominator = 1LL;
    if (targets != NULL && targets->has_minimum_fill_min) {
        lower_bound = targets->minimum_fill_min;
        has_lower_bound = 1;
        ratio_list_push(&list, lower_bound);
    }

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        long long capacity_value = problem_data->labs[lab_index].capacity_value;
        RatioValue lab_upper_bound;
        if (capacity_value <= 0LL) {
            continue;
        }
        if (total_capacity_bound_is_available) {
            if (capacity_value > LLONG_MAX - total_positive_capacity) {
                total_capacity_bound_is_available = 0;
            } else {
                total_positive_capacity += capacity_value;
            }
        }
        lab_upper_bound.numerator =
            (long long)problem_data->labs[lab_index].graph_capacity;
        lab_upper_bound.denominator = capacity_value;
        lab_upper_bound = ratio_value_reduce(lab_upper_bound);
        if (ratio_compare_value(lab_upper_bound, upper_bound) < 0) {
            upper_bound = lab_upper_bound;
        }
    }
    if (total_capacity_bound_is_available && total_positive_capacity > 0LL) {
        RatioValue total_upper_bound;
        total_upper_bound.numerator = (long long)problem_data->student_count;
        total_upper_bound.denominator = total_positive_capacity;
        total_upper_bound = ratio_value_reduce(total_upper_bound);
        if (ratio_compare_value(total_upper_bound, upper_bound) < 0) {
            upper_bound = total_upper_bound;
        }
    }

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int count_value;
        int max_count = problem_data->labs[lab_index].graph_capacity;
        long long capacity_value = problem_data->labs[lab_index].capacity_value;
        if (capacity_value <= 0LL) {
            continue;
        }
        for (count_value = 1; count_value <= max_count; count_value++) {
            RatioValue ratio;
            ratio.numerator = (long long)count_value;
            ratio.denominator = capacity_value;
            if (has_lower_bound &&
                ratio_compare_value(ratio, lower_bound) < 0) {
                continue;
            }
            if (ratio_compare_value(ratio, upper_bound) > 0) {
                continue;
            }
            ratio_list_push(&list, ratio);
        }
    }

    if (list.size > 1) {
        int read_index;
        int write_index = 1;
        qsort(list.items, (size_t)list.size, sizeof(RatioValue), ratio_qsort_compare);
        for (read_index = 1; read_index < list.size; read_index++) {
            if (ratio_compare_value(list.items[read_index], list.items[write_index - 1]) != 0) {
                list.items[write_index] = list.items[read_index];
                write_index++;
            }
        }
        list.size = write_index;
    }

    return list;
}

static int ceil_ratio_times_capacity(RatioValue ratio,
                                     long long capacity_value,
                                     int count_limit)
{
#if defined(__SIZEOF_INT128__)
    __uint128_t numerator;
    __uint128_t denominator;
    __uint128_t capacity;
    __uint128_t limit;
    __uint128_t product;
    __uint128_t result;
#else
    long long quotient = 0LL;
    long long remainder = 0LL;
    long long whole_part;
    long long partial_remainder;
    long long carry;
    long long iteration;
#endif
    if (capacity_value <= 0LL || ratio.numerator <= 0LL) {
        return 0;
    }

#if defined(__SIZEOF_INT128__)
    numerator = (__uint128_t)ratio.numerator;
    denominator = (__uint128_t)ratio.denominator;
    capacity = (__uint128_t)capacity_value;
    limit = (__uint128_t)count_limit;
    product = numerator * capacity;

    if (product > limit * denominator) {
        return count_limit + 1;
    }
    result = (product + denominator - 1U) / denominator;
    if (result > limit) {
        return count_limit + 1;
    }
    return (int)result;
#else
    whole_part = capacity_value / ratio.denominator;
    partial_remainder = capacity_value % ratio.denominator;

    for (iteration = 0LL; iteration < ratio.numerator; iteration++) {
        if (whole_part > (long long)count_limit ||
            quotient > (long long)count_limit - whole_part) {
            return count_limit + 1;
        }
        quotient += whole_part;
        remainder += partial_remainder;
        if (remainder >= ratio.denominator) {
            carry = remainder / ratio.denominator;
            remainder %= ratio.denominator;
            if (carry > (long long)count_limit ||
                quotient > (long long)count_limit - carry) {
                return count_limit + 1;
            }
            quotient += carry;
        }
    }

    if (remainder > 0LL) {
        if (quotient >= (long long)count_limit) {
            return count_limit + 1;
        }
        quotient++;
    }
    return (int)quotient;
#endif
}

static int target_minimum_count_for_lab(const ProblemData *problem_data,
                                        int lab_index)
{
    const TargetConstraints *targets = problem_data->targets;
    long long capacity_value = problem_data->labs[lab_index].capacity_value;
    if (targets == NULL || !targets->has_minimum_fill_min ||
        capacity_value <= 0LL) {
        return 0;
    }
    return ceil_ratio_times_capacity(targets->minimum_fill_min,
                                     capacity_value,
                                     problem_data->student_count);
}

static int floor_ratio_times_student_count(RatioValue ratio,
                                           int student_count,
                                           long long *result)
{
#if defined(__SIZEOF_INT128__)
    __uint128_t product =
        (__uint128_t)ratio.numerator * (__uint128_t)student_count;
    __uint128_t quotient = product / (__uint128_t)ratio.denominator;
    if (quotient > (__uint128_t)LLONG_MAX) {
        return 0;
    }
    *result = (long long)quotient;
    return 1;
#else
    long long product;
    if (!multiply_nonnegative_ll_fits(ratio.numerator,
                                      (long long)student_count,
                                      LLONG_MAX,
                                      &product)) {
        return 0;
    }
    *result = product / ratio.denominator;
    return 1;
#endif
}

static int target_rank_sum_limit(const ProblemData *problem_data,
                                 long long *limit_out)
{
    const TargetConstraints *targets = problem_data->targets;
    int has_limit = 0;
    long long limit_value = LLONG_MAX / 4LL;
    if (targets == NULL) {
        return 0;
    }
    if (targets->has_rank_sum_max) {
        limit_value = targets->rank_sum_max;
        has_limit = 1;
    }
    if (targets->has_average_rank_max) {
        long long average_limit;
        if (!floor_ratio_times_student_count(targets->average_rank_max,
                                             problem_data->student_count,
                                             &average_limit)) {
            fail_with_context("target constraints", "average-rank target is too large");
        }
        if (!has_limit || average_limit < limit_value) {
            limit_value = average_limit;
            has_limit = 1;
        }
    }
    if (has_limit) {
        *limit_out = limit_value;
    }
    return has_limit;
}

static int target_average_rank_limit(const ProblemData *problem_data,
                                     long long *limit_out)
{
    const TargetConstraints *targets = problem_data->targets;
    if (targets == NULL || !targets->has_average_rank_max) {
        return 0;
    }
    if (!floor_ratio_times_student_count(targets->average_rank_max,
                                         problem_data->student_count,
                                         limit_out)) {
        fail_with_context("target constraints", "average-rank target is too large");
    }
    return 1;
}

static int rank_sum_satisfies_targets(const ProblemData *problem_data,
                                      long long rank_sum)
{
    long long rank_sum_limit;
    if (!target_rank_sum_limit(problem_data, &rank_sum_limit)) {
        return 1;
    }
    return rank_sum <= rank_sum_limit;
}

static unsigned long long nonnegative_ll_to_u64(long long value,
                                                const char *context)
{
    if (value < 0LL) {
        fail_with_context(context, "negative integer in unsigned conversion");
    }
    return (unsigned long long)value;
}

static int average_fill_counts_satisfy_bound(
    const ProblemData *problem_data,
    const int *lab_counts,
    RatioValue bound)
{
    ExactAverageContext context = exact_average_context_create(problem_data);
    BigUInt left_scaled_sum;
    BigUInt right_scaled_bound;
    int term_index;
    int lab_index;
    int *coefficients =
        checked_calloc((size_t)context.term_count, sizeof(int));
    unsigned long long target_scale;
    int satisfied;

    if (context.positive_lab_count <= 0) {
        exact_average_context_free(&context);
        free(coefficients);
        return 1;
    }

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int term = context.term_by_lab[lab_index];
        if (term >= 0) {
            coefficients[term] += lab_counts[lab_index];
        }
    }

    big_uint_zero(&left_scaled_sum);
    for (term_index = 0; term_index < context.term_count; term_index++) {
        unsigned long long coefficient =
            (unsigned long long)coefficients[term_index];
        unsigned long long denominator =
            nonnegative_ll_to_u64(bound.denominator,
                                  "average-fill target");
        unsigned long long scale =
            checked_multiply_u64(coefficient,
                                 denominator,
                                 "average-fill target");
        big_uint_add_scaled_u64(&left_scaled_sum,
                                &context.fill_weights[term_index],
                                scale);
    }

    right_scaled_bound = context.common_denominator;
    target_scale =
        checked_multiply_u64(nonnegative_ll_to_u64(bound.numerator,
                                                   "average-fill target"),
                             (unsigned long long)context.positive_lab_count,
                             "average-fill target");
    big_uint_multiply_u64(&right_scaled_bound, target_scale);

    satisfied = big_uint_compare(&left_scaled_sum, &right_scaled_bound) >= 0;
    exact_average_context_free(&context);
    free(coefficients);
    return satisfied;
}

static int build_any_complete_lab_counts(const ProblemData *problem_data,
                                         int *lab_counts)
{
    int lab_index;
    int remaining_students = problem_data->student_count;

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        lab_counts[lab_index] = 0;
    }
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        if (problem_data->labs[lab_index].capacity_value > 0LL) {
            if (problem_data->labs[lab_index].graph_capacity <= 0) {
                return 0;
            }
            lab_counts[lab_index] = 1;
            remaining_students--;
            if (remaining_students < 0) {
                return 0;
            }
        }
    }
    for (lab_index = 0;
         lab_index < problem_data->lab_count && remaining_students > 0;
         lab_index++) {
        int spare_capacity =
            problem_data->labs[lab_index].graph_capacity - lab_counts[lab_index];
        int add_count;
        if (spare_capacity <= 0) {
            continue;
        }
        add_count = spare_capacity < remaining_students ?
                    spare_capacity :
                    remaining_students;
        lab_counts[lab_index] += add_count;
        remaining_students -= add_count;
    }
    return remaining_students == 0;
}

static int average_fill_is_assignment_constant(const ProblemData *problem_data)
{
    return graph_capacity_sum_equals_student_count(problem_data) ||
           positive_capacities_are_uniform(problem_data);
}

static int average_fill_target_implied_by_minimum_counts(
    const ProblemData *problem_data,
    RatioValue average_fill_min)
{
    int *minimum_counts = build_base_minimum_counts(problem_data);
    int implied =
        average_fill_counts_satisfy_bound(problem_data,
                                          minimum_counts,
                                          average_fill_min);
    free(minimum_counts);
    return implied;
}

static void average_fill_resource_context_free(
    AverageFillResourceContext *resource_context)
{
    free(resource_context->resource_by_lab);
    resource_context->resource_by_lab = NULL;
    resource_context->available = 0;
    resource_context->target_resource = 0LL;
    resource_context->maximum_resource = 0LL;
}

static AverageFillResourceContext average_fill_resource_context_create(
    const ProblemData *problem_data,
    RatioValue average_fill_min)
{
    AverageFillResourceContext resource_context;
    ExactAverageContext exact_average_context =
        exact_average_context_create(problem_data);
    unsigned long long lcm_u64;
    long long lcm_value;
    long long numerator_times_lab_count;
    long long scaled_target_numerator;
    long long target_with_rounding;
    int lab_index;

    resource_context.available = 0;
    resource_context.resource_by_lab = NULL;
    resource_context.target_resource = 0LL;
    resource_context.maximum_resource = 0LL;

    if (exact_average_context.positive_lab_count <= 0 ||
        !big_uint_to_u64_checked(&exact_average_context.common_denominator,
                                 &lcm_u64) ||
        lcm_u64 > (unsigned long long)LLONG_MAX) {
        exact_average_context_free(&exact_average_context);
        return resource_context;
    }
    lcm_value = (long long)lcm_u64;
    if (!multiply_nonnegative_ll_fits(
            average_fill_min.numerator,
            (long long)exact_average_context.positive_lab_count,
            LLONG_MAX,
            &numerator_times_lab_count) ||
        !multiply_nonnegative_ll_fits(numerator_times_lab_count,
                                      lcm_value,
                                      LLONG_MAX,
                                      &scaled_target_numerator) ||
        !add_nonnegative_ll_fits(scaled_target_numerator,
                                 average_fill_min.denominator - 1LL,
                                 LLONG_MAX,
                                 &target_with_rounding)) {
        exact_average_context_free(&exact_average_context);
        return resource_context;
    }

    resource_context.resource_by_lab =
        checked_calloc((size_t)problem_data->lab_count, sizeof(long long));
    resource_context.target_resource =
        target_with_rounding / average_fill_min.denominator;

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        long long capacity_value = problem_data->labs[lab_index].capacity_value;
        long long lab_maximum_resource;
        if (capacity_value <= 0LL) {
            continue;
        }
        if (lcm_u64 % (unsigned long long)capacity_value != 0ULL ||
            lcm_u64 / (unsigned long long)capacity_value >
                (unsigned long long)LLONG_MAX) {
            average_fill_resource_context_free(&resource_context);
            exact_average_context_free(&exact_average_context);
            return resource_context;
        }
        resource_context.resource_by_lab[lab_index] =
            (long long)(lcm_u64 / (unsigned long long)capacity_value);
        if (!multiply_nonnegative_ll_fits(
                resource_context.resource_by_lab[lab_index],
                (long long)problem_data->labs[lab_index].graph_capacity,
                LLONG_MAX,
                &lab_maximum_resource) ||
            !add_nonnegative_ll_fits(resource_context.maximum_resource,
                                     lab_maximum_resource,
                                     LLONG_MAX,
                                     &resource_context.maximum_resource)) {
            average_fill_resource_context_free(&resource_context);
            exact_average_context_free(&exact_average_context);
            return resource_context;
        }
    }

    resource_context.available = 1;
    exact_average_context_free(&exact_average_context);
    return resource_context;
}

static int average_fill_target_has_passive_support(
    const ProblemData *problem_data,
    RatioValue average_fill_min)
{
    if (average_fill_target_implied_by_minimum_counts(problem_data,
                                                      average_fill_min)) {
        return 1;
    }
    if (average_fill_is_assignment_constant(problem_data)) {
        int *complete_counts =
            checked_calloc((size_t)problem_data->lab_count, sizeof(int));
        int target_satisfied;
        if (!build_any_complete_lab_counts(problem_data, complete_counts)) {
            free(complete_counts);
            fail_with_context(
                "target constraints",
                "No feasible solution: no complete assignment can satisfy capacity bounds");
        }
        target_satisfied =
            average_fill_counts_satisfy_bound(problem_data,
                                              complete_counts,
                                              average_fill_min);
        free(complete_counts);
        if (!target_satisfied) {
            fail_with_context(
                "target constraints",
                "No feasible solution: average-fill target exceeds the constant fill rate");
        }
        return 1;
    }
    return 0;
}

static void validate_average_fill_target_support(
    const ProblemData *problem_data,
    const ProgramOptions *options)
{
    const TargetConstraints *targets = problem_data->targets;
    if (targets == NULL || !targets->has_average_fill_min) {
        return;
    }
    if (average_fill_target_has_passive_support(problem_data,
                                                targets->average_fill_min)) {
        return;
    }
    if (options->portfolio_mode ||
        options->change_penalty > 0LL ||
        (options->objective_mode != OBJECTIVE_RUBRIC &&
         options->objective_mode != OBJECTIVE_SATISFACTION &&
         options->objective_mode != OBJECTIVE_BALANCED &&
         options->objective_mode != OBJECTIVE_FAIR &&
         options->objective_mode != OBJECTIVE_GUARDED)) {
        fail_with_context(
            "target constraints",
            "average_fill_rate hard targets need a supported bounded-resource objective");
    }
    {
        AverageFillResourceContext resource_context =
            average_fill_resource_context_create(problem_data,
                                                 targets->average_fill_min);
        if (!resource_context.available) {
            fail_with_context(
                "target constraints",
                "average_fill_rate hard target resource scaling is too large for the bounded exact engine");
        }
        if (resource_context.target_resource >
            resource_context.maximum_resource) {
            average_fill_resource_context_free(&resource_context);
            fail_with_context(
                "target constraints",
                "No feasible solution: average-fill target exceeds total capacity");
        }
        average_fill_resource_context_free(&resource_context);
    }
}

static void validate_target_constraints_against_problem(
    const ProblemData *problem_data,
    const ProgramOptions *options)
{
    const TargetConstraints *targets = problem_data->targets;
    long long rank_sum_limit;
    int lab_index;
    int minimum_sum = 0;
    if (targets == NULL || target_constraints_are_empty(targets)) {
        return;
    }
    if (target_rank_sum_limit(problem_data, &rank_sum_limit) &&
        rank_sum_limit < (long long)problem_data->student_count) {
        fail_with_context("target constraints",
                          "No feasible solution: average-rank/rank-sum target is below the theoretical minimum");
    }
    validate_average_fill_target_support(problem_data, options);
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int required_count = target_minimum_count_for_lab(problem_data, lab_index);
        if (required_count > problem_data->labs[lab_index].graph_capacity) {
            fail_with_context("target constraints",
                              "No feasible solution: minimum-fill target exceeds a lab capacity");
        }
        minimum_sum += required_count;
        if (minimum_sum > problem_data->student_count) {
            fail_with_context("target constraints",
                              "No feasible solution: minimum-fill targets require more students than available");
        }
    }
}

static int build_minimum_counts_for_ratio(const ProblemData *problem_data,
                                          RatioValue ratio,
                                          int *minimum_lab_counts)
{
    int lab_index;
    int total_count = 0;
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int required_count = 0;
        long long capacity_value = problem_data->labs[lab_index].capacity_value;
        if (capacity_value > 0LL) {
            required_count =
                ceil_ratio_times_capacity(ratio,
                                          capacity_value,
                                          problem_data->student_count);
            if (required_count < 1) {
                required_count = 1;
            }
            {
                int target_count =
                    target_minimum_count_for_lab(problem_data, lab_index);
                if (target_count > required_count) {
                    required_count = target_count;
                }
            }
            if (required_count > problem_data->labs[lab_index].graph_capacity) {
                return 0;
            }
        }
        minimum_lab_counts[lab_index] = required_count;
        total_count += required_count;
        if (total_count > problem_data->student_count) {
            return 0;
        }
    }
    return 1;
}

static void minimum_count_candidate_list_push(MinimumCountCandidateList *list,
                                              MinimumCountCandidate candidate)
{
    if (list->size == list->capacity) {
        int new_capacity = list->capacity == 0 ? 64 : list->capacity * 2;
        list->items =
            checked_realloc(list->items,
                            (size_t)new_capacity * sizeof(MinimumCountCandidate));
        list->capacity = new_capacity;
    }
    list->items[list->size] = candidate;
    list->size++;
}

static int minimum_count_vectors_equal(const int *left_counts,
                                       const int *right_counts,
                                       int lab_count)
{
    return memcmp(left_counts,
                  right_counts,
                  (size_t)lab_count * sizeof(int)) == 0;
}

static unsigned long long minimum_count_vector_hash(const int *minimum_counts,
                                                    int lab_count)
{
    int lab_index;
    unsigned long long hash_value = FNV_OFFSET_BASIS;
    for (lab_index = 0; lab_index < lab_count; lab_index++) {
        unsigned int count_value = (unsigned int)minimum_counts[lab_index];
        int byte_index;
        for (byte_index = 0; byte_index < 4; byte_index++) {
            hash_value ^= (unsigned long long)(count_value & 255U);
            hash_value *= FNV_PRIME;
            count_value >>= 8;
        }
    }
    return hash_value;
}

static void minimum_count_candidate_list_free(MinimumCountCandidateList *list)
{
    int candidate_index;
    for (candidate_index = 0; candidate_index < list->size; candidate_index++) {
        free(list->items[candidate_index].minimum_counts);
    }
    free(list->items);
    list->items = NULL;
    list->size = 0;
    list->capacity = 0;
}

static int minimum_count_candidate_qsort_compare(const void *left_pointer,
                                                 const void *right_pointer)
{
    const MinimumCountCandidate *left =
        (const MinimumCountCandidate *)left_pointer;
    const MinimumCountCandidate *right =
        (const MinimumCountCandidate *)right_pointer;
    return ratio_compare_value(left->ratio, right->ratio);
}

static MinimumCountCandidateList build_minimum_count_candidates(
    const ProblemData *problem_data,
    int include_all_ratios)
{
    MinimumCountCandidateList candidates;
    RatioList ratios;
    int hash_size;
    int *candidate_hash_indices;
    int ratio_index;

    candidates.items = NULL;
    candidates.size = 0;
    candidates.capacity = 0;
    ratios.items = NULL;
    ratios.size = 0;
    ratios.capacity = 0;

    if (include_all_ratios) {
        ratios = build_ratio_candidates(problem_data);
    } else {
        RatioValue base_ratio;
        base_ratio.numerator = 0LL;
        base_ratio.denominator = 1LL;
        ratio_list_push(&ratios, base_ratio);
    }

    hash_size = hash_table_size_for_count(ratios.size,
                                          "minimum-count candidate hash index");
    candidate_hash_indices = checked_malloc((size_t)hash_size * sizeof(int));
    for (ratio_index = 0; ratio_index < hash_size; ratio_index++) {
        candidate_hash_indices[ratio_index] = -1;
    }

    for (ratio_index = 0; ratio_index < ratios.size; ratio_index++) {
        int duplicate_index = -1;
        unsigned long long vector_hash;
        int slot_index;
        int *minimum_counts =
            checked_calloc((size_t)problem_data->lab_count, sizeof(int));
        MinimumCountCandidate candidate;

        if (!build_minimum_counts_for_ratio(problem_data,
                                            ratios.items[ratio_index],
                                            minimum_counts)) {
            free(minimum_counts);
            continue;
        }

        vector_hash = minimum_count_vector_hash(minimum_counts,
                                                problem_data->lab_count);
        slot_index = (int)(vector_hash & (unsigned long long)(hash_size - 1));
        for (;;) {
            int existing_index = candidate_hash_indices[slot_index];
            if (existing_index < 0) {
                break;
            }
            if (minimum_count_vectors_equal(candidates.items[existing_index].minimum_counts,
                                            minimum_counts,
                                            problem_data->lab_count)) {
                duplicate_index = existing_index;
                break;
            }
            slot_index = (slot_index + 1) & (hash_size - 1);
        }

        if (duplicate_index >= 0) {
            if (ratio_compare_value(ratios.items[ratio_index],
                                    candidates.items[duplicate_index].ratio) > 0) {
                candidates.items[duplicate_index].ratio = ratios.items[ratio_index];
            }
            free(minimum_counts);
            continue;
        }

        candidate.ratio = ratios.items[ratio_index];
        candidate.minimum_counts = minimum_counts;
        candidate.minimum_count_sum = sum_minimum_counts(problem_data, minimum_counts);
        minimum_count_candidate_list_push(&candidates, candidate);
        candidate_hash_indices[slot_index] = candidates.size - 1;
    }

    free(candidate_hash_indices);
    free(ratios.items);
    if (candidates.size == 0) {
        fail_with_context("weighted exact objective", "no feasible fill threshold exists");
    }
    if (candidates.size > 1) {
        qsort(candidates.items,
              (size_t)candidates.size,
              sizeof(MinimumCountCandidate),
              minimum_count_candidate_qsort_compare);
    }
    return candidates;
}

static int ratio_candidate_is_feasible(const ProblemData *problem_data,
                                       int best_max_rank,
                                       RatioValue ratio,
                                       Cost rank_target,
                                       const RankCostModel *rank_cost_model,
                                       const StudentGroups *student_groups,
                                       SolutionResult *retained_solution)
{
    int *minimum_lab_counts =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));
    int required_count;
    int feasible;
    OptionalSolutionResult candidate_solution;

    if (!build_minimum_counts_for_ratio(problem_data, ratio, minimum_lab_counts)) {
        free(minimum_lab_counts);
        return 0;
    }

    required_count = sum_minimum_counts(problem_data, minimum_lab_counts);
    candidate_solution = try_solve_with_minimum_counts(problem_data,
                                                       best_max_rank,
                                                       minimum_lab_counts,
                                                       0,
                                                       0,
                                                       NULL,
                                                       rank_cost_model,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       student_groups);
    feasible = candidate_solution.feasible &&
               candidate_solution.solution.total_cost.first == -(long long)required_count &&
               candidate_solution.solution.total_cost.second == rank_target.second &&
               candidate_solution.solution.total_cost.third == rank_target.third;

    if (feasible && retained_solution != NULL) {
        free_solution_result(retained_solution);
        *retained_solution = candidate_solution.solution;
        candidate_solution.solution = empty_solution_result();
    } else {
        free_solution_result(&candidate_solution.solution);
    }
    free(minimum_lab_counts);
    return feasible;
}

static RatioValue find_best_minimum_fill_ratio(const ProblemData *problem_data,
                                               int best_max_rank,
                                               Cost rank_target,
                                               const RankCostModel *rank_cost_model,
                                               const StudentGroups *student_groups,
                                               SolutionResult *retained_solution)
{
    RatioList candidates = build_ratio_candidates(problem_data);
    int low_index = 0;
    int high_index = candidates.size - 1;
    int answer_index = 0;
    RatioValue answer;

    if (candidates.size == 0) {
        fail_with_context("solver", "no positive-capacity lab exists");
    }

    while (low_index <= high_index) {
        int middle_index = low_index + (high_index - low_index) / 2;
        if (ratio_candidate_is_feasible(problem_data,
                                        best_max_rank,
                                        candidates.items[middle_index],
                                        rank_target,
                                        rank_cost_model,
                                        student_groups,
                                        retained_solution)) {
            answer_index = middle_index;
            low_index = middle_index + 1;
        } else {
            high_index = middle_index - 1;
        }
    }

    answer = candidates.items[answer_index];
    free(candidates.items);
    return answer;
}

static int lab_rank_order_compare(const void *left_pointer, const void *right_pointer)
{
    const LabRankOrder *left = (const LabRankOrder *)left_pointer;
    const LabRankOrder *right = (const LabRankOrder *)right_pointer;
    if (left->rank_value != right->rank_value) {
        return left->rank_value < right->rank_value ? -1 : 1;
    }
    if (left->lab_index != right->lab_index) {
        return left->lab_index < right->lab_index ? -1 : 1;
    }
    return 0;
}

static int lab_fill_order_compare(const void *left_pointer, const void *right_pointer)
{
    const LabFillOrder *left = (const LabFillOrder *)left_pointer;
    const LabFillOrder *right = (const LabFillOrder *)right_pointer;
    if (left->capacity_value != right->capacity_value) {
        return left->capacity_value < right->capacity_value ? -1 : 1;
    }
    if (left->lab_index != right->lab_index) {
        return left->lab_index < right->lab_index ? -1 : 1;
    }
    return 0;
}

static int identical_ratio_is_feasible(const ProblemData *problem_data,
                                       RatioValue ratio,
                                       const int *group_by_lab,
                                       const RankGroupPlan *groups,
                                       int group_count,
                                       int *minimum_lab_counts)
{
    int lab_index;
    int group_index;
    int feasible = 1;
    int *required_by_group = checked_calloc((size_t)group_count, sizeof(int));

    if (minimum_lab_counts != NULL) {
        for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
            minimum_lab_counts[lab_index] = 0;
        }
    }

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int required_count = 0;
        long long capacity_value = problem_data->labs[lab_index].capacity_value;
        if (capacity_value > 0LL) {
            required_count =
                ceil_ratio_times_capacity(ratio,
                                          capacity_value,
                                          problem_data->student_count);
            if (required_count < 1) {
                required_count = 1;
            }
            {
                int target_count =
                    target_minimum_count_for_lab(problem_data, lab_index);
                if (target_count > required_count) {
                    required_count = target_count;
                }
            }
            if (required_count > problem_data->labs[lab_index].graph_capacity) {
                feasible = 0;
                break;
            }
            group_index = group_by_lab[lab_index];
            if (group_index < 0 || group_index >= group_count) {
                free(required_by_group);
                fail_with_context("solver", "invalid identical-rank group");
            }
            required_by_group[group_index] += required_count;
            if (required_by_group[group_index] > groups[group_index].total_count) {
                feasible = 0;
                break;
            }
        }
        if (minimum_lab_counts != NULL) {
            minimum_lab_counts[lab_index] = required_count;
        }
    }

    if (feasible) {
        for (group_index = 0; group_index < group_count; group_index++) {
            if (required_by_group[group_index] > groups[group_index].total_count) {
                feasible = 0;
                break;
            }
        }
    }

    free(required_by_group);
    return feasible;
}

static RatioValue find_best_identical_minimum_fill_ratio(
    const ProblemData *problem_data,
    const int *group_by_lab,
    const RankGroupPlan *groups,
    int group_count)
{
    RatioList candidates = build_ratio_candidates(problem_data);
    int low_index = 0;
    int high_index = candidates.size - 1;
    int answer_index = 0;
    RatioValue answer;

    if (candidates.size == 0) {
        fail_with_context("solver", "no positive-capacity lab exists");
    }

    while (low_index <= high_index) {
        int middle_index = low_index + (high_index - low_index) / 2;
        if (identical_ratio_is_feasible(problem_data,
                                        candidates.items[middle_index],
                                        group_by_lab,
                                        groups,
                                        group_count,
                                        NULL)) {
            answer_index = middle_index;
            low_index = middle_index + 1;
        } else {
            high_index = middle_index - 1;
        }
    }

    answer = candidates.items[answer_index];
    free(candidates.items);
    return answer;
}

static void distribute_identical_counts_for_exact_average(
    const ProblemData *problem_data,
    const LabRankOrder *ordered_labs,
    const RankGroupPlan *groups,
    int group_count,
    const int *minimum_lab_counts,
    int *lab_counts)
{
    int lab_index;
    int group_index;

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        lab_counts[lab_index] = minimum_lab_counts[lab_index];
    }

    for (group_index = 0; group_index < group_count; group_index++) {
        int order_index;
        int group_size = groups[group_index].end_index - groups[group_index].start_index;
        int assigned_in_group = 0;
        int surplus_count;
        LabFillOrder *fill_order =
            checked_malloc((size_t)group_size * sizeof(LabFillOrder));

        for (order_index = groups[group_index].start_index;
             order_index < groups[group_index].end_index;
             order_index++) {
            int local_index = order_index - groups[group_index].start_index;
            int current_lab = ordered_labs[order_index].lab_index;
            assigned_in_group += lab_counts[current_lab];
            fill_order[local_index].lab_index = current_lab;
            fill_order[local_index].capacity_value =
                problem_data->labs[current_lab].capacity_value;
        }

        surplus_count = groups[group_index].total_count - assigned_in_group;
        if (surplus_count < 0) {
            free(fill_order);
            fail_with_context("solver", "identical-rank lower bounds exceed group total");
        }

        qsort(fill_order,
              (size_t)group_size,
              sizeof(LabFillOrder),
              lab_fill_order_compare);

        for (order_index = 0; order_index < group_size && surplus_count > 0; order_index++) {
            int current_lab = fill_order[order_index].lab_index;
            int remaining_capacity =
                problem_data->labs[current_lab].graph_capacity - lab_counts[current_lab];
            int added_count = remaining_capacity < surplus_count ?
                              remaining_capacity :
                              surplus_count;
            if (added_count > 0) {
                lab_counts[current_lab] += added_count;
                surplus_count -= added_count;
            }
        }

        free(fill_order);
        if (surplus_count != 0) {
            fail_with_context("solver", "identical-rank group capacity is inconsistent");
        }
    }
}

static int *build_assignment_from_lab_counts(const ProblemData *problem_data,
                                             const int *lab_counts)
{
    int student_index = 0;
    int lab_index;
    int *assignment =
        checked_malloc((size_t)problem_data->student_count * sizeof(int));

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int count_index;
        for (count_index = 0; count_index < lab_counts[lab_index]; count_index++) {
            if (student_index >= problem_data->student_count) {
                free(assignment);
                fail_with_context("solver", "too many assignments were generated");
            }
            assignment[student_index] = lab_index;
            student_index++;
        }
    }

    if (student_index != problem_data->student_count) {
        free(assignment);
        fail_with_context("solver", "too few assignments were generated");
    }
    return assignment;
}

static int *solve_identical_rank_problem(const ProblemData *problem_data,
                                         int best_max_rank)
{
    int lab_index;
    int group_count = 0;
    int remaining_students;
    int positive_lab_count = 0;
    LabRankOrder *ordered_labs =
        checked_malloc((size_t)problem_data->lab_count * sizeof(LabRankOrder));
    RankGroupPlan *groups =
        checked_malloc((size_t)problem_data->lab_count * sizeof(RankGroupPlan));
    int *group_by_lab =
        checked_malloc((size_t)problem_data->lab_count * sizeof(int));
    int *minimum_lab_counts =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));
    int *lab_counts =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));
    int *assignment;
    RatioValue best_minimum_fill_ratio;

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        group_by_lab[lab_index] = -1;
        if (problem_data->labs[lab_index].capacity_value > 0LL) {
            int rank_value = rank_for_assignment(problem_data, 0, lab_index);
            if (rank_value > best_max_rank) {
                free(ordered_labs);
                free(groups);
                free(group_by_lab);
                free(minimum_lab_counts);
                free(lab_counts);
                fail_with_context("solver", "identical-rank max rank is inconsistent");
            }
            ordered_labs[positive_lab_count].lab_index = lab_index;
            ordered_labs[positive_lab_count].rank_value = rank_value;
            positive_lab_count++;
        }
    }

    if (positive_lab_count == 0) {
        free(ordered_labs);
        free(groups);
        free(group_by_lab);
        free(minimum_lab_counts);
        free(lab_counts);
        fail_with_context("solver", "no positive-capacity lab exists");
    }
    remaining_students = problem_data->student_count - positive_lab_count;
    if (remaining_students < 0) {
        free(ordered_labs);
        free(groups);
        free(group_by_lab);
        free(minimum_lab_counts);
        free(lab_counts);
        fail_with_context("solver", "minimum occupancy is impossible");
    }

    qsort(ordered_labs,
          (size_t)positive_lab_count,
          sizeof(LabRankOrder),
          lab_rank_order_compare);

    for (lab_index = 0; lab_index < positive_lab_count;) {
        int start_index = lab_index;
        int end_index;
        int extra_capacity = 0;
        int extra_taken;
        int order_index;
        while (lab_index < positive_lab_count &&
               ordered_labs[lab_index].rank_value == ordered_labs[start_index].rank_value) {
            lab_index++;
        }
        end_index = lab_index;
        for (order_index = start_index; order_index < end_index; order_index++) {
            int current_lab = ordered_labs[order_index].lab_index;
            group_by_lab[current_lab] = group_count;
            extra_capacity += problem_data->labs[current_lab].graph_capacity - 1;
        }
        extra_taken = extra_capacity < remaining_students ?
                      extra_capacity :
                      remaining_students;
        groups[group_count].rank_value = ordered_labs[start_index].rank_value;
        groups[group_count].start_index = start_index;
        groups[group_count].end_index = end_index;
        groups[group_count].total_count = (end_index - start_index) + extra_taken;
        group_count++;
        remaining_students -= extra_taken;
    }

    if (remaining_students != 0) {
        free(ordered_labs);
        free(groups);
        free(group_by_lab);
        free(minimum_lab_counts);
        free(lab_counts);
        fail_with_context("solver", "total capacity is insufficient");
    }

    best_minimum_fill_ratio =
        find_best_identical_minimum_fill_ratio(problem_data,
                                               group_by_lab,
                                               groups,
                                               group_count);
    if (!identical_ratio_is_feasible(problem_data,
                                     best_minimum_fill_ratio,
                                     group_by_lab,
                                     groups,
                                     group_count,
                                     minimum_lab_counts)) {
        free(ordered_labs);
        free(groups);
        free(group_by_lab);
        free(minimum_lab_counts);
        free(lab_counts);
        fail_with_context("solver", "failed to rebuild identical fill ratio");
    }

    distribute_identical_counts_for_exact_average(problem_data,
                                                  ordered_labs,
                                                  groups,
                                                  group_count,
                                                  minimum_lab_counts,
                                                  lab_counts);
    assignment = build_assignment_from_lab_counts(problem_data, lab_counts);

    free(ordered_labs);
    free(groups);
    free(group_by_lab);
    free(minimum_lab_counts);
    free(lab_counts);
    return assignment;
}

static int *solve_problem(const ProblemData *problem_data, int best_max_rank)
{
    int *base_minimum_counts;
    int *best_minimum_counts;
    int required_count;
    RatioValue best_minimum_fill_ratio;
    SolutionResult base_solution;
    SolutionResult best_ratio_solution = empty_solution_result();
    SolutionResult final_solution;
    StudentGroups active_groups;
    const StudentGroups *active_group_pointer;
    int *assignment;
    int average_fill_is_fixed;

    if (all_students_have_identical_rank_rows(problem_data)) {
        return solve_identical_rank_problem(problem_data, best_max_rank);
    }

    active_groups =
        build_student_groups(problem_data, best_max_rank, STUDENT_GROUP_ACTIVE_RANK);
    active_group_pointer =
        active_groups.count < problem_data->student_count ? &active_groups : NULL;

    base_minimum_counts = build_base_minimum_counts(problem_data);
    best_minimum_counts =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));

    base_solution = solve_with_minimum_counts(problem_data,
                                              best_max_rank,
                                              base_minimum_counts,
                                              0,
                                              1,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              active_group_pointer);

    if (graph_capacity_sum_equals_student_count(problem_data)) {
        assignment = base_solution.assignment;
        base_solution.assignment = NULL;
        free_solution_result(&base_solution);
        free_student_groups(&active_groups);
        free(base_minimum_counts);
        free(best_minimum_counts);
        return assignment;
    }

    best_minimum_fill_ratio = find_best_minimum_fill_ratio(problem_data,
                                                           best_max_rank,
                                                           base_solution.total_cost,
                                                           NULL,
                                                           active_group_pointer,
                                                           &best_ratio_solution);
    if (!build_minimum_counts_for_ratio(problem_data,
                                        best_minimum_fill_ratio,
                                        best_minimum_counts)) {
        fail_with_context("solver", "failed to rebuild best minimum fill ratio");
    }
    required_count = sum_minimum_counts(problem_data, best_minimum_counts);
    average_fill_is_fixed = required_count == problem_data->student_count ||
                            positive_capacities_are_uniform(problem_data);

    if (average_fill_is_fixed) {
        if (best_ratio_solution.assignment == NULL ||
            best_ratio_solution.total_cost.first != -(long long)required_count ||
            best_ratio_solution.total_cost.second != base_solution.total_cost.second ||
            best_ratio_solution.total_cost.third != base_solution.total_cost.third) {
            fail_with_context("solver", "retained fill-ratio solution is invalid");
        }
        assignment = best_ratio_solution.assignment;
        best_ratio_solution.assignment = NULL;
        free_solution_result(&base_solution);
        free_solution_result(&best_ratio_solution);
        free_student_groups(&active_groups);
        free(base_minimum_counts);
        free(best_minimum_counts);
        return assignment;
    }

    {
        ExactAverageContext exact_average_context =
            exact_average_context_create(problem_data);
        final_solution = solve_with_minimum_counts(problem_data,
                                                   best_max_rank,
                                                   best_minimum_counts,
                                                   1,
                                                   1,
                                                   &exact_average_context,
                                                   NULL,
                                                   NULL,
                                                   NULL,
                                                   NULL,
                                                   active_group_pointer);
        exact_average_context_free(&exact_average_context);
    }
    if (final_solution.total_cost.first != -(long long)required_count ||
        final_solution.total_cost.second != base_solution.total_cost.second ||
        final_solution.total_cost.third != base_solution.total_cost.third) {
        fail_with_context("solver", "final solution worsened rank-optimal objective");
    }

    assignment = final_solution.assignment;
    final_solution.assignment = NULL;

    free_solution_result(&base_solution);
    free_solution_result(&best_ratio_solution);
    free_solution_result(&final_solution);
    free_student_groups(&active_groups);
    free(base_minimum_counts);
    free(best_minimum_counts);
    return assignment;
}

static int rank_cost_matches_target(Cost candidate_cost, Cost target_cost)
{
    return candidate_cost.second == target_cost.second &&
           candidate_cost.third == target_cost.third;
}

static int rank_target_is_achievable(const ProblemData *problem_data,
                                     int max_rank,
                                     const int *minimum_lab_counts,
                                     Cost rank_target,
                                     const RankCostModel *rank_cost_model,
                                     const StudentGroups *active_group_pointer)
{
    OptionalSolutionResult candidate_solution;
    int required_count = sum_minimum_counts(problem_data, minimum_lab_counts);
    int achievable;
    if (active_profile != NULL) {
        active_profile->rank_target_checks++;
    }
    candidate_solution = try_solve_with_minimum_counts(problem_data,
                                                       max_rank,
                                                       minimum_lab_counts,
                                                       0,
                                                       0,
                                                       NULL,
                                                       rank_cost_model,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       active_group_pointer);
    achievable = candidate_solution.feasible &&
                 candidate_solution.solution.total_cost.first == -(long long)required_count &&
                 rank_cost_matches_target(candidate_solution.solution.total_cost,
                                          rank_target);
    free_solution_result(&candidate_solution.solution);
    return achievable;
}

static int find_minimum_q_preserving_rank_target(const ProblemData *problem_data,
                                                 int q_upper_bound,
                                                 const int *minimum_lab_counts,
                                                 Cost rank_target,
                                                 const RankCostModel *rank_cost_model)
{
    IntList candidates = build_rank_threshold_candidates(problem_data);
    StudentGroupCache group_cache =
        student_group_cache_create(candidates.size, STUDENT_GROUP_ACTIVE_RANK);
    int low_index = int_list_first_index_at_least(&candidates, 1);
    int high_index = int_list_last_index_at_most(&candidates, q_upper_bound);
    int answer_rank;

    if (low_index > high_index) {
        student_group_cache_free(&group_cache);
        int_list_free(&candidates);
        fail_with_context("solver", "no rank threshold candidate is available under guard");
    }

    answer_rank = candidates.items[high_index];
    while (low_index <= high_index) {
        int middle_index = low_index + (high_index - low_index) / 2;
        int middle_rank = candidates.items[middle_index];
        const StudentGroups *active_group_pointer =
            student_group_cache_get(&group_cache,
                                    problem_data,
                                    &candidates,
                                    middle_index);
        if (rank_target_is_achievable(problem_data,
                                      middle_rank,
                                      minimum_lab_counts,
                                      rank_target,
                                      rank_cost_model,
                                      active_group_pointer)) {
            answer_rank = middle_rank;
            high_index = middle_index - 1;
        } else {
            low_index = middle_index + 1;
        }
    }
    student_group_cache_free(&group_cache);
    int_list_free(&candidates);
    return answer_rank;
}

static void build_average_fill_coefficients(const ProblemData *problem_data,
                                            const ExactAverageContext *context,
                                            const int *lab_counts,
                                            int *coefficients)
{
    int term_index;
    int lab_index;
    for (term_index = 0; term_index < context->term_count; term_index++) {
        coefficients[term_index] = 0;
    }
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        term_index = context->term_by_lab[lab_index];
        if (term_index >= 0) {
            coefficients[term_index] += lab_counts[lab_index];
        }
    }
}

static int average_target_is_achievable(
    const ProblemData *problem_data,
    int best_max_rank,
    Cost rank_target,
    const StudentGroups *active_group_pointer,
    const ExactAverageContext *exact_average_context,
    const int *target_average_coefficients,
    const int *minimum_lab_counts,
    const RankCostModel *rank_cost_model,
    SolutionResult *retained_solution)
{
    int required_count = sum_minimum_counts(problem_data, minimum_lab_counts);
    int *candidate_coefficients =
        checked_calloc((size_t)exact_average_context->term_count, sizeof(int));
    OptionalSolutionResult candidate_solution;
    int comparison;
    int achievable = 0;

    if (active_profile != NULL) {
        active_profile->average_target_checks++;
    }

    candidate_solution = try_solve_with_minimum_counts(problem_data,
                                                       best_max_rank,
                                                       minimum_lab_counts,
                                                       1,
                                                       0,
                                                       exact_average_context,
                                                       rank_cost_model,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       active_group_pointer);
    if (!candidate_solution.feasible ||
        candidate_solution.solution.total_cost.first != -(long long)required_count ||
        !rank_cost_matches_target(candidate_solution.solution.total_cost, rank_target)) {
        free(candidate_coefficients);
        free_solution_result(&candidate_solution.solution);
        return 0;
    }

    build_average_fill_coefficients(problem_data,
                                    exact_average_context,
                                    candidate_solution.solution.lab_counts,
                                    candidate_coefficients);
    comparison =
        exact_average_compare_two_coefficients(exact_average_context,
                                               candidate_coefficients,
                                               target_average_coefficients);
    if (comparison > 0) {
        free(candidate_coefficients);
        free_solution_result(&candidate_solution.solution);
        fail_with_context("solver", "average-fill target was not maximal");
    }
    achievable = comparison == 0;
    free(candidate_coefficients);

    if (achievable && retained_solution != NULL) {
        free_solution_result(retained_solution);
        *retained_solution = candidate_solution.solution;
        candidate_solution.solution = empty_solution_result();
    } else {
        free_solution_result(&candidate_solution.solution);
    }
    return achievable;
}

static SolutionResult solve_average_then_minimum_fill(
    const ProblemData *problem_data,
    int best_max_rank,
    Cost rank_target,
    const RankCostModel *rank_cost_model,
    const StudentGroups *active_group_pointer)
{
    MinimumCountCandidateList minimum_candidates =
        build_minimum_count_candidates(problem_data, 1);
    ExactAverageContext exact_average_context =
        exact_average_context_create(problem_data);
    SolutionResult best_solution;
    const int *base_minimum_counts = minimum_candidates.items[0].minimum_counts;
    int base_required_count = minimum_candidates.items[0].minimum_count_sum;
    int *target_coefficients =
        checked_calloc((size_t)exact_average_context.term_count, sizeof(int));
    int last_true_index = 0;
    int first_false_index = minimum_candidates.size;
    int sequential_limit =
        minimum_candidates.size < AVERAGE_FILL_LINEAR_PROBE_LIMIT ?
        minimum_candidates.size :
        AVERAGE_FILL_LINEAR_PROBE_LIMIT;
    int probe_index = 1;

    best_solution = solve_with_minimum_counts(problem_data,
                                              best_max_rank,
                                              base_minimum_counts,
                                              1,
                                              0,
                                              &exact_average_context,
                                              rank_cost_model,
                                              NULL,
                                              NULL,
                                              NULL,
                                              active_group_pointer);
    if (best_solution.total_cost.first != -(long long)base_required_count ||
        !rank_cost_matches_target(best_solution.total_cost, rank_target)) {
        free(target_coefficients);
        free_solution_result(&best_solution);
        exact_average_context_free(&exact_average_context);
        minimum_count_candidate_list_free(&minimum_candidates);
        fail_with_context("solver", "failed to optimize average fill rate");
    }
    build_average_fill_coefficients(problem_data,
                                    &exact_average_context,
                                    best_solution.lab_counts,
                                    target_coefficients);

    while (probe_index < sequential_limit) {
        const int *minimum_lab_counts =
            minimum_candidates.items[probe_index].minimum_counts;
        if (average_target_is_achievable(problem_data,
                                         best_max_rank,
                                         rank_target,
                                         active_group_pointer,
                                         &exact_average_context,
                                         target_coefficients,
                                         minimum_lab_counts,
                                         rank_cost_model,
                                         &best_solution)) {
            last_true_index = probe_index;
            probe_index++;
        } else {
            first_false_index = probe_index;
            break;
        }
    }

    while (first_false_index == minimum_candidates.size &&
           probe_index < minimum_candidates.size) {
        const int *minimum_lab_counts =
            minimum_candidates.items[probe_index].minimum_counts;
        if (average_target_is_achievable(problem_data,
                                         best_max_rank,
                                         rank_target,
                                         active_group_pointer,
                                         &exact_average_context,
                                         target_coefficients,
                                         minimum_lab_counts,
                                         rank_cost_model,
                                         &best_solution)) {
            last_true_index = probe_index;
            if (probe_index > (minimum_candidates.size - 1) / 2) {
                probe_index = minimum_candidates.size;
            } else {
                probe_index *= 2;
            }
        } else {
            first_false_index = probe_index;
            break;
        }
    }

    {
        int low_index = last_true_index + 1;
        int high_index = first_false_index < minimum_candidates.size ?
                         first_false_index - 1 :
                         minimum_candidates.size - 1;
        while (low_index <= high_index) {
            int middle_index = low_index + (high_index - low_index) / 2;
            const int *minimum_lab_counts =
                minimum_candidates.items[middle_index].minimum_counts;
            if (average_target_is_achievable(problem_data,
                                             best_max_rank,
                                             rank_target,
                                             active_group_pointer,
                                             &exact_average_context,
                                             target_coefficients,
                                             minimum_lab_counts,
                                             rank_cost_model,
                                             &best_solution)) {
                low_index = middle_index + 1;
            } else {
                high_index = middle_index - 1;
            }
        }
    }

    free(target_coefficients);
    exact_average_context_free(&exact_average_context);
    minimum_count_candidate_list_free(&minimum_candidates);
    return best_solution;
}

static SolutionResult solve_minimum_then_average_fill(
    const ProblemData *problem_data,
    int best_max_rank,
    Cost rank_target,
    const RankCostModel *rank_cost_model,
    const StudentGroups *active_group_pointer)
{
    int *best_minimum_counts =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));
    RatioValue best_minimum_fill_ratio;
    SolutionResult retained_solution = empty_solution_result();
    SolutionResult final_solution;
    int required_count;

    best_minimum_fill_ratio = find_best_minimum_fill_ratio(problem_data,
                                                           best_max_rank,
                                                           rank_target,
                                                           rank_cost_model,
                                                           active_group_pointer,
                                                           &retained_solution);
    if (!build_minimum_counts_for_ratio(problem_data,
                                        best_minimum_fill_ratio,
                                        best_minimum_counts)) {
        free(best_minimum_counts);
        fail_with_context("solver", "failed to rebuild best minimum fill ratio");
    }
    required_count = sum_minimum_counts(problem_data, best_minimum_counts);
    if (required_count == problem_data->student_count ||
        positive_capacities_are_uniform(problem_data)) {
        if (retained_solution.assignment == NULL ||
            retained_solution.total_cost.first != -(long long)required_count ||
            !rank_cost_matches_target(retained_solution.total_cost, rank_target)) {
            free(best_minimum_counts);
            free_solution_result(&retained_solution);
            fail_with_context("solver", "retained fill-ratio solution is invalid");
        }
        free(best_minimum_counts);
        return retained_solution;
    }
    {
        ExactAverageContext exact_average_context =
            exact_average_context_create(problem_data);
        final_solution = solve_with_minimum_counts(problem_data,
                                                   best_max_rank,
                                                   best_minimum_counts,
                                                   1,
                                                   1,
                                                   &exact_average_context,
                                                   rank_cost_model,
                                                   NULL,
                                                   NULL,
                                                   NULL,
                                                   active_group_pointer);
        exact_average_context_free(&exact_average_context);
    }
    if (final_solution.total_cost.first != -(long long)required_count ||
        !rank_cost_matches_target(final_solution.total_cost, rank_target)) {
        free(best_minimum_counts);
        free_solution_result(&retained_solution);
        free_solution_result(&final_solution);
        fail_with_context("solver", "final solution worsened rank-optimal objective");
    }
    free(best_minimum_counts);
    free_solution_result(&retained_solution);
    return final_solution;
}

static int *solve_rank_first_problem(const ProblemData *problem_data,
                                     int q_upper_bound,
                                     FillTieOrder fill_tie_order,
                                     const RankCostModel *rank_cost_model)
{
    int *base_minimum_counts = build_base_minimum_counts(problem_data);
    StudentGroups active_groups;
    const StudentGroups *active_group_pointer;
    SolutionResult base_solution;
    SolutionResult final_solution;
    OptionalSolutionResult optional_base_solution;
    long long base_solution_max_rank;
    int best_max_rank;
    int *assignment;

    q_upper_bound = target_rank_upper_bound(problem_data, q_upper_bound);
    if (q_upper_bound < 1) {
        fail_with_context("target constraints",
                          "No feasible solution: max-rank target leaves no allowable rank");
    }
    active_groups =
        build_student_groups(problem_data, q_upper_bound, STUDENT_GROUP_ACTIVE_RANK);
    active_group_pointer =
        active_groups.count < problem_data->student_count ? &active_groups : NULL;
    optional_base_solution =
        try_solve_with_minimum_counts(problem_data,
                                      q_upper_bound,
                                      base_minimum_counts,
                                      0,
                                      1,
                                      NULL,
                                      rank_cost_model,
                                      NULL,
                                      NULL,
                                      NULL,
                                      active_group_pointer);
    if (!optional_base_solution.feasible) {
        free_student_groups(&active_groups);
        free(base_minimum_counts);
        fail_with_context("target constraints",
                          "No feasible solution: no feasible assignment satisfies the rank/fill targets");
    }
    base_solution = optional_base_solution.solution;
    if (rank_cost_model == NULL) {
        long long rank_sum_limit;
        if (target_rank_sum_limit(problem_data, &rank_sum_limit) &&
            base_solution.total_cost.second > rank_sum_limit) {
            free_solution_result(&base_solution);
            free_student_groups(&active_groups);
            free(base_minimum_counts);
            fail_with_context(
                "target constraints",
                "No feasible solution: no feasible assignment satisfies the average-rank/rank-sum target");
        }
    }
    base_solution_max_rank = max_rank_for_solution(problem_data, base_solution.assignment);
    if (base_solution_max_rank < (long long)q_upper_bound) {
        q_upper_bound = (int)base_solution_max_rank;
    }
    best_max_rank = find_minimum_q_preserving_rank_target(problem_data,
                                                          q_upper_bound,
                                                          base_minimum_counts,
                                                          base_solution.total_cost,
                                                          rank_cost_model);
    free_student_groups(&active_groups);
    active_groups =
        build_student_groups(problem_data, best_max_rank, STUDENT_GROUP_ACTIVE_RANK);
    active_group_pointer =
        active_groups.count < problem_data->student_count ? &active_groups : NULL;

    if (graph_capacity_sum_equals_student_count(problem_data)) {
        final_solution = solve_with_minimum_counts(problem_data,
                                                   best_max_rank,
                                                   base_minimum_counts,
                                                   0,
                                                   1,
                                                   NULL,
                                                   rank_cost_model,
                                                   NULL,
                                                   NULL,
                                                   NULL,
                                                   active_group_pointer);
        if (!rank_cost_matches_target(final_solution.total_cost,
                                      base_solution.total_cost)) {
            free_solution_result(&base_solution);
            free_solution_result(&final_solution);
            free_student_groups(&active_groups);
            free(base_minimum_counts);
            fail_with_context("solver", "fixed-fill solution worsened rank objective");
        }
        assignment = final_solution.assignment;
        final_solution.assignment = NULL;
        free_solution_result(&base_solution);
        free_solution_result(&final_solution);
        free_student_groups(&active_groups);
        free(base_minimum_counts);
        return assignment;
    }

    if (fill_tie_order == FILL_TIE_AVERAGE_THEN_MINIMUM) {
        final_solution = solve_average_then_minimum_fill(problem_data,
                                                         best_max_rank,
                                                         base_solution.total_cost,
                                                         rank_cost_model,
                                                         active_group_pointer);
    } else {
        final_solution = solve_minimum_then_average_fill(problem_data,
                                                         best_max_rank,
                                                         base_solution.total_cost,
                                                         rank_cost_model,
                                                         active_group_pointer);
    }

    assignment = final_solution.assignment;
    final_solution.assignment = NULL;
    free_solution_result(&base_solution);
    free_solution_result(&final_solution);
    free_student_groups(&active_groups);
    free(base_minimum_counts);
    return assignment;
}

typedef struct {
    const ProblemData *problem_data;
    int q_upper_bound;
    FillTieOrder fill_tie_order;
    int max_rank_first;
    const RankCostModel *rank_cost_model;
    const AverageFillResourceContext *resource_context;
    int *lower_counts;
    int *suffix_min_counts;
    int *suffix_max_counts;
    int *current_counts;
    int vectors_tested;
    int limit_hit;
    int best_is_set;
    SolutionResult best_solution;
    long long best_max_rank;
    long long best_average_resource;
    RatioValue best_minimum_fill;
} AverageFillResourceSearch;

static long long average_fill_resource_for_counts(
    const ProblemData *problem_data,
    const AverageFillResourceContext *resource_context,
    const int *lab_counts)
{
    int lab_index;
    long long total_resource = 0LL;
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        long long lab_resource;
        if (resource_context->resource_by_lab[lab_index] == 0LL ||
            lab_counts[lab_index] == 0) {
            continue;
        }
        if (!multiply_nonnegative_ll_fits(
                resource_context->resource_by_lab[lab_index],
                (long long)lab_counts[lab_index],
                LLONG_MAX,
                &lab_resource) ||
            !add_nonnegative_ll_fits(total_resource,
                                     lab_resource,
                                     LLONG_MAX,
                                     &total_resource)) {
            fail_with_context("average-fill resource target",
                              "resource score overflow");
        }
    }
    return total_resource;
}

static RatioValue minimum_fill_ratio_for_counts(
    const ProblemData *problem_data,
    const int *lab_counts)
{
    RatioValue best_ratio;
    int lab_index;
    int has_ratio = 0;
    best_ratio.numerator = 0LL;
    best_ratio.denominator = 1LL;
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        long long capacity_value = problem_data->labs[lab_index].capacity_value;
        RatioValue candidate;
        if (capacity_value <= 0LL) {
            continue;
        }
        candidate.numerator = (long long)lab_counts[lab_index];
        candidate.denominator = capacity_value;
        if (!has_ratio || ratio_compare_value(candidate, best_ratio) < 0) {
            best_ratio = candidate;
            has_ratio = 1;
        }
    }
    return ratio_value_reduce(best_ratio);
}

static int bounded_resource_candidate_better(
    int max_rank_first,
    FillTieOrder fill_tie_order,
    const SolutionResult *candidate_solution,
    long long candidate_max_rank,
    long long candidate_average_resource,
    RatioValue candidate_minimum_fill,
    const SolutionResult *best_solution,
    long long best_max_rank,
    long long best_average_resource,
    RatioValue best_minimum_fill)
{
    if (max_rank_first) {
        int fill_comparison;
        if (candidate_max_rank != best_max_rank) {
            return candidate_max_rank < best_max_rank;
        }
        if (candidate_solution->total_cost.second !=
            best_solution->total_cost.second) {
            return candidate_solution->total_cost.second <
                   best_solution->total_cost.second;
        }
        if (candidate_solution->total_cost.third !=
            best_solution->total_cost.third) {
            return candidate_solution->total_cost.third <
                   best_solution->total_cost.third;
        }
        fill_comparison =
            ratio_compare_value(candidate_minimum_fill, best_minimum_fill);
        if (fill_comparison != 0) {
            return fill_comparison > 0;
        }
        return candidate_average_resource > best_average_resource;
    }
    if (candidate_solution->total_cost.second !=
        best_solution->total_cost.second) {
        return candidate_solution->total_cost.second <
               best_solution->total_cost.second;
    }
    if (candidate_solution->total_cost.third !=
        best_solution->total_cost.third) {
        return candidate_solution->total_cost.third <
               best_solution->total_cost.third;
    }
    if (candidate_max_rank != best_max_rank) {
        return candidate_max_rank < best_max_rank;
    }
    if (fill_tie_order == FILL_TIE_AVERAGE_THEN_MINIMUM) {
        if (candidate_average_resource != best_average_resource) {
            return candidate_average_resource > best_average_resource;
        }
        return ratio_compare_value(candidate_minimum_fill,
                                   best_minimum_fill) > 0;
    }
    if (ratio_compare_value(candidate_minimum_fill, best_minimum_fill) != 0) {
        return ratio_compare_value(candidate_minimum_fill,
                                   best_minimum_fill) > 0;
    }
    return candidate_average_resource > best_average_resource;
}

static OptionalSolutionResult solve_rank_first_for_exact_counts(
    const ProblemData *problem_data,
    int q_upper_bound,
    const int *exact_lab_counts,
    const RankCostModel *rank_cost_model,
    long long *out_max_rank)
{
    OptionalSolutionResult result;
    OptionalSolutionResult base_solution;
    StudentGroups active_groups =
        build_student_groups(problem_data,
                             q_upper_bound,
                             STUDENT_GROUP_ACTIVE_RANK);
    const StudentGroups *active_group_pointer =
        active_groups.count < problem_data->student_count ? &active_groups : NULL;
    int best_max_rank;

    result.feasible = 0;
    result.solution = empty_solution_result();

    base_solution =
        try_solve_with_minimum_counts(problem_data,
                                      q_upper_bound,
                                      exact_lab_counts,
                                      0,
                                      0,
                                      NULL,
                                      rank_cost_model,
                                      NULL,
                                      NULL,
                                      NULL,
                                      active_group_pointer);
    free_student_groups(&active_groups);
    if (!base_solution.feasible ||
        base_solution.solution.total_cost.first !=
            -(long long)problem_data->student_count) {
        free_solution_result(&base_solution.solution);
        return result;
    }

    best_max_rank =
        find_minimum_q_preserving_rank_target(problem_data,
                                              q_upper_bound,
                                              exact_lab_counts,
                                              base_solution.solution.total_cost,
                                              rank_cost_model);
    active_groups =
        build_student_groups(problem_data,
                             best_max_rank,
                             STUDENT_GROUP_ACTIVE_RANK);
    active_group_pointer =
        active_groups.count < problem_data->student_count ? &active_groups : NULL;
    result =
        try_solve_with_minimum_counts(problem_data,
                                      best_max_rank,
                                      exact_lab_counts,
                                      0,
                                      0,
                                      NULL,
                                      rank_cost_model,
                                      NULL,
                                      NULL,
                                      NULL,
                                      active_group_pointer);
    free_student_groups(&active_groups);
    if (result.feasible &&
        result.solution.total_cost.first !=
            -(long long)problem_data->student_count) {
        free_solution_result(&result.solution);
        result.feasible = 0;
        result.solution = empty_solution_result();
    }
    if (result.feasible &&
        !rank_cost_matches_target(result.solution.total_cost,
                                  base_solution.solution.total_cost)) {
        free_solution_result(&base_solution.solution);
        free_solution_result(&result.solution);
        result.feasible = 0;
        result.solution = empty_solution_result();
        fail_with_context("average-fill resource target",
                          "exact-count solve worsened rank target");
    }
    free_solution_result(&base_solution.solution);
    if (result.feasible) {
        *out_max_rank = (long long)best_max_rank;
    }
    return result;
}

static int fair_exact_counts_feasible_at_q(
    const ProblemData *problem_data,
    const IntList *rank_candidates,
    int q_index,
    const int *exact_lab_counts,
    int has_rank_sum_limit,
    long long rank_sum_limit,
    StudentGroupCache *group_cache)
{
    OptionalSolutionResult candidate_solution;
    const StudentGroups *active_group_pointer =
        student_group_cache_get(group_cache,
                                problem_data,
                                rank_candidates,
                                q_index);
    int feasible;

    candidate_solution =
        try_solve_with_minimum_counts(problem_data,
                                      rank_candidates->items[q_index],
                                      exact_lab_counts,
                                      0,
                                      0,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      active_group_pointer);
    feasible =
        candidate_solution.feasible &&
        candidate_solution.solution.total_cost.first ==
            -(long long)problem_data->student_count &&
        (!has_rank_sum_limit ||
         candidate_solution.solution.total_cost.second <= rank_sum_limit);
    free_solution_result(&candidate_solution.solution);
    return feasible;
}

static OptionalSolutionResult solve_fair_for_exact_counts(
    const ProblemData *problem_data,
    int q_upper_bound,
    const int *exact_lab_counts,
    long long *out_max_rank)
{
    OptionalSolutionResult result;
    IntList candidates = build_rank_threshold_candidates(problem_data);
    StudentGroupCache group_cache =
        student_group_cache_create(candidates.size, STUDENT_GROUP_ACTIVE_RANK);
    int low_index = int_list_first_index_at_least(&candidates, 1);
    int high_index = int_list_last_index_at_most(&candidates, q_upper_bound);
    int answer_index;
    int best_max_rank;
    long long rank_sum_limit = 0LL;
    int has_rank_sum_limit =
        target_rank_sum_limit(problem_data, &rank_sum_limit);
    StudentGroups active_groups;
    const StudentGroups *active_group_pointer;

    result.feasible = 0;
    result.solution = empty_solution_result();

    if (low_index > high_index ||
        !fair_exact_counts_feasible_at_q(problem_data,
                                         &candidates,
                                         high_index,
                                         exact_lab_counts,
                                         has_rank_sum_limit,
                                         rank_sum_limit,
                                         &group_cache)) {
        student_group_cache_free(&group_cache);
        int_list_free(&candidates);
        return result;
    }

    answer_index = high_index;
    while (low_index <= high_index) {
        int middle_index = low_index + (high_index - low_index) / 2;
        if (fair_exact_counts_feasible_at_q(problem_data,
                                            &candidates,
                                            middle_index,
                                            exact_lab_counts,
                                            has_rank_sum_limit,
                                            rank_sum_limit,
                                            &group_cache)) {
            answer_index = middle_index;
            high_index = middle_index - 1;
        } else {
            low_index = middle_index + 1;
        }
    }

    best_max_rank = candidates.items[answer_index];
    student_group_cache_free(&group_cache);
    int_list_free(&candidates);

    active_groups =
        build_student_groups(problem_data,
                             best_max_rank,
                             STUDENT_GROUP_ACTIVE_RANK);
    active_group_pointer =
        active_groups.count < problem_data->student_count ? &active_groups : NULL;
    result =
        try_solve_with_minimum_counts(problem_data,
                                      best_max_rank,
                                      exact_lab_counts,
                                      0,
                                      0,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      active_group_pointer);
    free_student_groups(&active_groups);
    if (result.feasible &&
        (result.solution.total_cost.first !=
             -(long long)problem_data->student_count ||
         (has_rank_sum_limit &&
          result.solution.total_cost.second > rank_sum_limit))) {
        free_solution_result(&result.solution);
        result.feasible = 0;
        result.solution = empty_solution_result();
    }
    if (result.feasible) {
        *out_max_rank = (long long)best_max_rank;
    }
    return result;
}

static void average_fill_resource_search_consider_current(
    AverageFillResourceSearch *search)
{
    long long average_resource =
        average_fill_resource_for_counts(search->problem_data,
                                         search->resource_context,
                                         search->current_counts);
    long long candidate_max_rank = 0LL;
    long long rank_sum_limit;
    RatioValue minimum_fill;
    OptionalSolutionResult candidate_solution;

    search->vectors_tested++;
    if (active_profile != NULL) {
        active_profile->average_fill_resource_vectors_tested++;
    }
    if (search->vectors_tested > AVERAGE_FILL_RESOURCE_VECTOR_LIMIT) {
        search->limit_hit = 1;
        if (active_profile != NULL) {
            active_profile->average_fill_resource_vector_limit_hits++;
        }
        return;
    }
    if (average_resource < search->resource_context->target_resource) {
        return;
    }

    if (search->max_rank_first) {
        candidate_solution =
            solve_fair_for_exact_counts(search->problem_data,
                                        search->q_upper_bound,
                                        search->current_counts,
                                        &candidate_max_rank);
    } else {
        candidate_solution =
            solve_rank_first_for_exact_counts(search->problem_data,
                                              search->q_upper_bound,
                                              search->current_counts,
                                              search->rank_cost_model,
                                              &candidate_max_rank);
    }
    if (!candidate_solution.feasible) {
        return;
    }
    if (target_rank_sum_limit(search->problem_data, &rank_sum_limit) &&
        candidate_solution.solution.total_cost.second > rank_sum_limit) {
        free_solution_result(&candidate_solution.solution);
        return;
    }

    minimum_fill =
        minimum_fill_ratio_for_counts(search->problem_data,
                                      search->current_counts);
    if (!search->best_is_set ||
        bounded_resource_candidate_better(
            search->max_rank_first,
            search->fill_tie_order,
            &candidate_solution.solution,
            candidate_max_rank,
            average_resource,
            minimum_fill,
            &search->best_solution,
            search->best_max_rank,
            search->best_average_resource,
            search->best_minimum_fill)) {
        free_solution_result(&search->best_solution);
        search->best_solution = candidate_solution.solution;
        candidate_solution.solution = empty_solution_result();
        search->best_max_rank = candidate_max_rank;
        search->best_average_resource = average_resource;
        search->best_minimum_fill = minimum_fill;
        search->best_is_set = 1;
    }
    free_solution_result(&candidate_solution.solution);
}

static void average_fill_resource_search_dfs(AverageFillResourceSearch *search,
                                             int lab_index,
                                             int remaining_students)
{
    int minimum_take;
    int maximum_take;
    int take_count;
    if (search->limit_hit) {
        return;
    }
    if (lab_index == search->problem_data->lab_count) {
        if (remaining_students == 0) {
            average_fill_resource_search_consider_current(search);
        }
        return;
    }
    minimum_take = search->lower_counts[lab_index];
    if (minimum_take <
        remaining_students - search->suffix_max_counts[lab_index + 1]) {
        minimum_take =
            remaining_students - search->suffix_max_counts[lab_index + 1];
    }
    maximum_take = search->problem_data->labs[lab_index].graph_capacity;
    if (maximum_take >
        remaining_students - search->suffix_min_counts[lab_index + 1]) {
        maximum_take =
            remaining_students - search->suffix_min_counts[lab_index + 1];
    }
    if (minimum_take > maximum_take) {
        return;
    }
    for (take_count = minimum_take;
         take_count <= maximum_take;
         take_count++) {
        search->current_counts[lab_index] = take_count;
        average_fill_resource_search_dfs(search,
                                         lab_index + 1,
                                         remaining_students - take_count);
        if (search->limit_hit) {
            return;
        }
    }
    search->current_counts[lab_index] = 0;
}

static int *solve_average_resource_bounded_problem(
    const ProblemData *problem_data,
    int q_upper_bound,
    FillTieOrder fill_tie_order,
    const RankCostModel *rank_cost_model,
    int max_rank_first)
{
    const TargetConstraints *targets = problem_data->targets;
    AverageFillResourceContext resource_context;
    AverageFillResourceSearch search;
    int lab_index;
    int *assignment;

    if (targets == NULL || !targets->has_average_fill_min) {
        return solve_rank_first_problem(problem_data,
                                        q_upper_bound,
                                        fill_tie_order,
                                        rank_cost_model);
    }
    if (average_fill_target_has_passive_support(problem_data,
                                                targets->average_fill_min)) {
        return solve_rank_first_problem(problem_data,
                                        q_upper_bound,
                                        fill_tie_order,
                                        rank_cost_model);
    }

    resource_context =
        average_fill_resource_context_create(problem_data,
                                             targets->average_fill_min);
    if (!resource_context.available) {
        fail_with_context("target constraints",
                          "average_fill_rate hard target resource scaling is too large for the bounded exact engine");
    }

    memset(&search, 0, sizeof(search));
    search.problem_data = problem_data;
    search.q_upper_bound = target_rank_upper_bound(problem_data, q_upper_bound);
    search.fill_tie_order = fill_tie_order;
    search.max_rank_first = max_rank_first;
    search.rank_cost_model = rank_cost_model;
    search.resource_context = &resource_context;
    search.lower_counts = build_base_minimum_counts(problem_data);
    search.suffix_min_counts =
        checked_calloc((size_t)problem_data->lab_count + 1U, sizeof(int));
    search.suffix_max_counts =
        checked_calloc((size_t)problem_data->lab_count + 1U, sizeof(int));
    search.current_counts =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));
    search.best_solution = empty_solution_result();
    search.best_minimum_fill.numerator = 0LL;
    search.best_minimum_fill.denominator = 1LL;

    if (search.q_upper_bound < 1) {
        fail_with_context("target constraints",
                          "No feasible solution: max-rank target leaves no allowable rank");
    }

    for (lab_index = problem_data->lab_count - 1;
         lab_index >= 0;
         lab_index--) {
        search.suffix_min_counts[lab_index] =
            search.suffix_min_counts[lab_index + 1] +
            search.lower_counts[lab_index];
        search.suffix_max_counts[lab_index] =
            search.suffix_max_counts[lab_index + 1] +
            problem_data->labs[lab_index].graph_capacity;
    }
    if (search.suffix_min_counts[0] > problem_data->student_count ||
        search.suffix_max_counts[0] < problem_data->student_count) {
        free(search.lower_counts);
        free(search.suffix_min_counts);
        free(search.suffix_max_counts);
        free(search.current_counts);
        average_fill_resource_context_free(&resource_context);
        fail_with_context("target constraints",
                          "No feasible solution: capacity bounds cannot place every student");
    }

    average_fill_resource_search_dfs(&search,
                                     0,
                                     problem_data->student_count);
    if (search.limit_hit) {
        free_solution_result(&search.best_solution);
        free(search.lower_counts);
        free(search.suffix_min_counts);
        free(search.suffix_max_counts);
        free(search.current_counts);
        average_fill_resource_context_free(&resource_context);
        fail_with_context("target constraints",
                          "average_fill_rate bounded-resource engine exceeded its exact count-vector limit");
    }
    if (!search.best_is_set || search.best_solution.assignment == NULL) {
        free_solution_result(&search.best_solution);
        free(search.lower_counts);
        free(search.suffix_min_counts);
        free(search.suffix_max_counts);
        free(search.current_counts);
        average_fill_resource_context_free(&resource_context);
        fail_with_context("target constraints",
                          "No feasible solution: no assignment satisfies the average-fill hard target");
    }

    assignment = search.best_solution.assignment;
    search.best_solution.assignment = NULL;
    free_solution_result(&search.best_solution);
    free(search.lower_counts);
    free(search.suffix_min_counts);
    free(search.suffix_max_counts);
    free(search.current_counts);
    average_fill_resource_context_free(&resource_context);
    return assignment;
}

static int *solve_rank_first_average_resource_bounded_problem(
    const ProblemData *problem_data,
    int q_upper_bound,
    FillTieOrder fill_tie_order,
    const RankCostModel *rank_cost_model)
{
    return solve_average_resource_bounded_problem(problem_data,
                                                  q_upper_bound,
                                                  fill_tie_order,
                                                  rank_cost_model,
                                                  0);
}

static int *solve_fair_average_resource_bounded_problem(
    const ProblemData *problem_data,
    int q_upper_bound)
{
    const TargetConstraints *targets = problem_data->targets;
    if (targets == NULL || !targets->has_average_fill_min ||
        average_fill_target_has_passive_support(problem_data,
                                                targets->average_fill_min)) {
        int best_max_rank = find_fair_max_rank_satisfying_rank_sum_targets(
            problem_data);
        if (best_max_rank > q_upper_bound) {
            fail_with_context("target constraints",
                              "No feasible solution: no fair solution satisfies the guarded max-rank bound");
        }
        return solve_problem(problem_data, best_max_rank);
    }
    return solve_average_resource_bounded_problem(problem_data,
                                                  q_upper_bound,
                                                  FILL_TIE_MINIMUM_THEN_AVERAGE,
                                                  NULL,
                                                  1);
}

static int average_fill_target_needs_bounded_resource(
    const ProblemData *problem_data)
{
    const TargetConstraints *targets = problem_data->targets;
    return targets != NULL &&
           targets->has_average_fill_min &&
           !average_fill_target_has_passive_support(problem_data,
                                                    targets->average_fill_min);
}

static int rank_sum_target_is_feasible_at_q(const ProblemData *problem_data,
                                            const IntList *candidates,
                                            int q_index,
                                            const int *base_minimum_counts,
                                            int required_count,
                                            long long rank_sum_limit,
                                            StudentGroupCache *group_cache)
{
    int q_limit = candidates->items[q_index];
    const StudentGroups *active_group_pointer =
        student_group_cache_get(group_cache,
                                problem_data,
                                candidates,
                                q_index);
    OptionalSolutionResult candidate_solution =
        try_solve_with_minimum_counts(problem_data,
                                      q_limit,
                                      base_minimum_counts,
                                      0,
                                      0,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      active_group_pointer);
    int feasible =
        candidate_solution.feasible &&
        candidate_solution.solution.total_cost.first == -(long long)required_count &&
        candidate_solution.solution.total_cost.second <= rank_sum_limit;
    free_solution_result(&candidate_solution.solution);
    return feasible;
}

static int find_fair_max_rank_satisfying_rank_sum_targets(
    const ProblemData *problem_data)
{
    long long rank_sum_limit;
    IntList candidates;
    StudentGroupCache group_cache;
    int q_upper_bound;
    int first_q_index;
    int last_q_index;
    int low_index;
    int high_index;
    int answer_index;
    int *base_minimum_counts;
    int required_count;

    if (!target_rank_sum_limit(problem_data, &rank_sum_limit)) {
        return find_minimum_feasible_max_rank(problem_data);
    }

    q_upper_bound = target_rank_upper_bound(problem_data,
                                           problem_data->lab_count + 1);
    if (q_upper_bound < 1) {
        fail_with_context("target constraints",
                          "No feasible solution: max-rank target leaves no allowable rank");
    }
    candidates = build_rank_threshold_candidates(problem_data);
    first_q_index = int_list_first_index_at_least(&candidates, 1);
    last_q_index = int_list_last_index_at_most(&candidates, q_upper_bound);
    if (first_q_index > last_q_index) {
        int_list_free(&candidates);
        fail_with_context("target constraints",
                          "No feasible solution: no rank threshold candidate satisfies the target");
    }

    base_minimum_counts = build_base_minimum_counts(problem_data);
    required_count = sum_minimum_counts(problem_data, base_minimum_counts);
    group_cache =
        student_group_cache_create(candidates.size, STUDENT_GROUP_ACTIVE_RANK);

    if (!rank_sum_target_is_feasible_at_q(problem_data,
                                          &candidates,
                                          last_q_index,
                                          base_minimum_counts,
                                          required_count,
                                          rank_sum_limit,
                                          &group_cache)) {
        student_group_cache_free(&group_cache);
        free(base_minimum_counts);
        int_list_free(&candidates);
        fail_with_context(
            "target constraints",
            "No feasible solution: no feasible assignment satisfies the average-rank/rank-sum target");
    }

    low_index = first_q_index;
    high_index = last_q_index;
    answer_index = last_q_index;
    while (low_index <= high_index) {
        int middle_index = low_index + (high_index - low_index) / 2;
        if (rank_sum_target_is_feasible_at_q(problem_data,
                                             &candidates,
                                             middle_index,
                                             base_minimum_counts,
                                             required_count,
                                             rank_sum_limit,
                                             &group_cache)) {
            answer_index = middle_index;
            high_index = middle_index - 1;
        } else {
            low_index = middle_index + 1;
        }
    }

    student_group_cache_free(&group_cache);
    free(base_minimum_counts);
    {
        int answer_rank = candidates.items[answer_index];
        int_list_free(&candidates);
        return answer_rank;
    }
}

static long long max_rank_for_solution(const ProblemData *problem_data,
                                       const int *assignment)
{
    int student_index;
    long long max_rank = 0LL;
    for (student_index = 0; student_index < problem_data->student_count; student_index++) {
        int rank_value =
            rank_for_assignment(problem_data, student_index, assignment[student_index]);
        if ((long long)rank_value > max_rank) {
            max_rank = (long long)rank_value;
        }
    }
    return max_rank;
}

static void weighted_score_add_integer(SignedBigScore *score,
                                       const ExactAverageContext *context,
                                       long long integer_value)
{
    unsigned long long scale;
    if (integer_value == 0LL) {
        return;
    }
    scale = absolute_long_long_as_u64(integer_value, "weighted exact score");
    scale = checked_multiply_u64(scale,
                                 (unsigned long long)context->positive_lab_count,
                                 "weighted exact score");
    signed_big_score_add_scaled(score,
                                &context->common_denominator,
                                scale,
                                integer_value < 0LL);
}

static void weighted_score_add_fill_term(SignedBigScore *score,
                                         const BigUInt *fill_weight,
                                         long long signed_weight,
                                         int count_value,
                                         const char *context_text)
{
    unsigned long long scale;
    if (signed_weight == 0LL || count_value == 0) {
        return;
    }
    scale = absolute_long_long_as_u64(signed_weight, context_text);
    scale = checked_multiply_u64(scale,
                                 (unsigned long long)count_value,
                                 context_text);
    signed_big_score_add_scaled(score, fill_weight, scale, signed_weight < 0LL);
}

static SignedBigScore weighted_score_for_solution(
    const ProblemData *problem_data,
    const SolutionResult *solution,
    const WeightedObjective *weights,
    const ExactAverageContext *context)
{
    SignedBigScore score;
    long long integer_part = solution->total_cost.second;
    int lab_index;
    int min_fill_lab = -1;
    signed_big_score_zero(&score);

    integer_part =
        checked_add_weighted_term(integer_part,
                                  weights->max_rank,
                                  max_rank_for_solution(problem_data, solution->assignment),
                                  "weighted exact score");
    weighted_score_add_integer(&score, context, integer_part);

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int term_index = context->term_by_lab[lab_index];
        if (term_index >= 0) {
            weighted_score_add_fill_term(&score,
                                         &context->fill_weights[term_index],
                                         -weights->average_fill,
                                         solution->lab_counts[lab_index],
                                         "weighted exact score");
            if (min_fill_lab < 0 ||
                ratio_compare_value(
                    (RatioValue){(long long)solution->lab_counts[lab_index],
                                 problem_data->labs[lab_index].capacity_value},
                    (RatioValue){(long long)solution->lab_counts[min_fill_lab],
                                 problem_data->labs[min_fill_lab].capacity_value}) < 0) {
                min_fill_lab = lab_index;
            }
        }
    }

    if (min_fill_lab >= 0 && weights->minimum_fill != 0LL) {
        int term_index = context->term_by_lab[min_fill_lab];
        unsigned long long scale =
            absolute_long_long_as_u64(weights->minimum_fill,
                                      "weighted exact score");
        scale = checked_multiply_u64(scale,
                                     (unsigned long long)context->positive_lab_count,
                                     "weighted exact score");
        scale = checked_multiply_u64(
            scale,
            (unsigned long long)solution->lab_counts[min_fill_lab],
            "weighted exact score");
        signed_big_score_add_scaled(&score,
                                    &context->fill_weights[term_index],
                                    scale,
                                    1);
    }
    return score;
}

static int weighted_score_less(const SignedBigScore *left,
                               const SignedBigScore *right)
{
    return signed_big_score_compare(left, right) < 0;
}

static long long weighted_integer_score_for_solution(
    const ProblemData *problem_data,
    const SolutionResult *solution,
    const WeightedObjective *weights)
{
    return checked_add_weighted_term(solution->total_cost.second,
                                     weights->max_rank,
                                     max_rank_for_solution(problem_data,
                                                           solution->assignment),
                                     "weighted exact score");
}

static int *solve_weighted_exact_integer_only_problem(
    const ProblemData *problem_data,
    const WeightedObjective *weights)
{
    int *base_minimum_counts = build_base_minimum_counts(problem_data);
    IntList rank_candidates = build_rank_threshold_candidates(problem_data);
    SolutionResult best_solution = empty_solution_result();
    long long best_score = 0LL;
    int best_score_is_set = 0;
    int q_start = weights->max_rank == 0LL ?
                  target_rank_upper_bound(problem_data, problem_data->lab_count + 1) :
                  find_minimum_feasible_max_rank(problem_data);
    int first_q_index = int_list_first_index_at_least(&rank_candidates, q_start);
    int last_q_index =
        int_list_last_index_at_most(
            &rank_candidates,
            target_rank_upper_bound(problem_data, problem_data->lab_count + 1));
    int q_index;

    if (first_q_index > last_q_index) {
        free(base_minimum_counts);
        int_list_free(&rank_candidates);
        fail_with_context("weighted exact objective", "no rank threshold candidate found");
    }

    for (q_index = first_q_index; q_index <= last_q_index; q_index++) {
        int q_limit = rank_candidates.items[q_index];
        StudentGroups active_groups = {NULL, NULL, NULL, 0};
        const StudentGroups *active_group_pointer = NULL;
        OptionalSolutionResult candidate_solution;
        long long candidate_score;

        if (!(weights->change > 0LL && problem_data->base_assignment != NULL)) {
            active_groups =
                build_student_groups(problem_data, q_limit, STUDENT_GROUP_ACTIVE_RANK);
            active_group_pointer =
                active_groups.count < problem_data->student_count ? &active_groups : NULL;
        }
        if (active_profile != NULL) {
            active_profile->q_candidates_tested++;
            active_profile->minimum_candidates_tested++;
        }
        candidate_solution = try_solve_with_minimum_counts(problem_data,
                                                           q_limit,
                                                           base_minimum_counts,
                                                           0,
                                                           0,
                                                           NULL,
                                                           NULL,
                                                           weights,
                                                           NULL,
                                                           NULL,
                                                           active_group_pointer);
        free_student_groups(&active_groups);
        if (!candidate_solution.feasible ||
            candidate_solution.solution.total_cost.first !=
                -(long long)sum_minimum_counts(problem_data, base_minimum_counts)) {
            free_solution_result(&candidate_solution.solution);
            continue;
        }
        candidate_score =
            weighted_integer_score_for_solution(problem_data,
                                                &candidate_solution.solution,
                                                weights);
        if (!best_score_is_set || candidate_score < best_score) {
            free_solution_result(&best_solution);
            best_solution = candidate_solution.solution;
            candidate_solution.solution = empty_solution_result();
            best_score = candidate_score;
            best_score_is_set = 1;
        }
        free_solution_result(&candidate_solution.solution);
        if (weights->max_rank == 0LL) {
            break;
        }
    }

    free(base_minimum_counts);
    int_list_free(&rank_candidates);
    if (!best_score_is_set || best_solution.assignment == NULL) {
        fail_with_context("weighted exact objective", "no feasible assignment found");
    }
    {
        int *assignment = best_solution.assignment;
        best_solution.assignment = NULL;
        free_solution_result(&best_solution);
        return assignment;
    }
}

static WeightedObjective default_weighted_objective(void)
{
    WeightedObjective weights;
    weights.rank_sum = 50LL;
    weights.rank_square = 20LL;
    weights.max_rank = 15LL;
    weights.average_fill = 5LL;
    weights.minimum_fill = 5LL;
    weights.outside = 30LL;
    weights.change = 0LL;
    return weights;
}

static WeightedObjective fill_focused_weighted_objective(void)
{
    WeightedObjective weights;
    weights.rank_sum = 1LL;
    weights.rank_square = 0LL;
    weights.max_rank = 0LL;
    weights.average_fill = 1000LL;
    weights.minimum_fill = 1000LL;
    weights.outside = 1000LL;
    weights.change = 0LL;
    return weights;
}

static int *solve_convex_fill_problem(const ProblemData *problem_data)
{
    int *base_minimum_counts = build_base_minimum_counts(problem_data);
    int max_rank = target_rank_upper_bound(problem_data, problem_data->lab_count + 1);
    ConvexFillContext convex_fill_context =
        convex_fill_context_create(problem_data);
    if (max_rank < 1) {
        fail_with_context("target constraints",
                          "No feasible solution: max-rank target leaves no allowable rank");
    }
    StudentGroups active_groups =
        build_student_groups(problem_data, max_rank, STUDENT_GROUP_ACTIVE_RANK);
    const StudentGroups *active_group_pointer =
        active_groups.count < problem_data->student_count ? &active_groups : NULL;
    SolutionResult solution =
        solve_with_minimum_counts(problem_data,
                                  max_rank,
                                  base_minimum_counts,
                                  0,
                                  1,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &convex_fill_context,
                                  active_group_pointer);
    int *assignment = solution.assignment;
    solution.assignment = NULL;
    free_solution_result(&solution);
    free_student_groups(&active_groups);
    free(base_minimum_counts);
    return assignment;
}

static long long weighted_additive_cost_for_assignment(const ProblemData *problem_data,
                                                       const int *assignment,
                                                       const WeightedObjective *weights)
{
    long long total_cost = 0LL;
    int student_index;
    for (student_index = 0; student_index < problem_data->student_count; student_index++) {
        int rank_value =
            rank_for_assignment(problem_data, student_index, assignment[student_index]);
        long long rank_square = (long long)rank_value * (long long)rank_value;
        long long student_cost = 0LL;
        student_cost = checked_add_weighted_term(student_cost,
                                                 weights->rank_sum,
                                                 (long long)rank_value,
                                                 "weighted exact score");
        student_cost = checked_add_weighted_term(student_cost,
                                                 weights->rank_square,
                                                 rank_square,
                                                 "weighted exact score");
        if (rank_value > problem_data->max_preferences) {
            student_cost = checked_add_weighted_term(student_cost,
                                                     weights->outside,
                                                     1LL,
                                                     "weighted exact score");
        }
        if (problem_data->base_assignment != NULL &&
            assignment[student_index] != problem_data->base_assignment[student_index]) {
            student_cost = checked_add_weighted_term(student_cost,
                                                     weights->change,
                                                     1LL,
                                                     "weighted exact score");
        }
        if (total_cost > INF_COST - student_cost) {
            fail_with_context("weighted exact score", "weighted objective cost overflow");
        }
        total_cost += student_cost;
    }
    return total_cost;
}

static void weighted_average_fast_path_free(WeightedAverageFastPath *fast_path)
{
    free(fast_path->lab_average_rewards);
    fast_path->lab_average_rewards = NULL;
    fast_path->available = 0;
}

static WeightedAverageFastPath weighted_average_fast_path_create(
    const ProblemData *problem_data,
    const ExactAverageContext *context,
    const WeightedObjective *weights,
    int best_max_rank)
{
    WeightedAverageFastPath fast_path;
    unsigned long long lcm_u64 = 0ULL;
    long long lcm_value;
    long long scalar_multiplier;
    long long rank_limit;
    long long rank_square_limit;
    long long per_edge_limit;
    long long term_value;
    long long edge_cost_bound = 0LL;
    int lab_index;

    fast_path.available = 0;
    fast_path.scaled_weights = *weights;
    fast_path.lab_average_rewards = NULL;

    if (weights->average_fill == 0LL || context->positive_lab_count <= 0) {
        return fast_path;
    }
    if (!big_uint_to_u64_checked(&context->common_denominator, &lcm_u64) ||
        lcm_u64 > (unsigned long long)LLONG_MAX) {
        return fast_path;
    }
    lcm_value = (long long)lcm_u64;
    if (!multiply_nonnegative_ll_fits((long long)context->positive_lab_count,
                                      lcm_value,
                                      LLONG_MAX,
                                      &scalar_multiplier)) {
        return fast_path;
    }
    if (problem_data->student_count <= 0) {
        return fast_path;
    }
    per_edge_limit = INF_COST / (long long)problem_data->student_count / 4LL;
    if (per_edge_limit <= 0LL) {
        return fast_path;
    }

    rank_limit = (long long)best_max_rank;
    if (rank_limit > (long long)problem_data->lab_count + 1LL) {
        rank_limit = (long long)problem_data->lab_count + 1LL;
    }
    if (rank_limit < 1LL) {
        return fast_path;
    }
    if (!multiply_nonnegative_ll_fits(rank_limit,
                                      rank_limit,
                                      per_edge_limit,
                                      &rank_square_limit)) {
        return fast_path;
    }

    if (!multiply_nonnegative_ll_fits(weights->rank_sum,
                                      scalar_multiplier,
                                      per_edge_limit,
                                      &fast_path.scaled_weights.rank_sum) ||
        !multiply_nonnegative_ll_fits(weights->rank_square,
                                      scalar_multiplier,
                                      per_edge_limit,
                                      &fast_path.scaled_weights.rank_square) ||
        !multiply_nonnegative_ll_fits(weights->outside,
                                      scalar_multiplier,
                                      per_edge_limit,
                                      &fast_path.scaled_weights.outside) ||
        !multiply_nonnegative_ll_fits(weights->change,
                                      scalar_multiplier,
                                      per_edge_limit,
                                      &fast_path.scaled_weights.change)) {
        return fast_path;
    }
    fast_path.scaled_weights.max_rank = 0LL;
    fast_path.scaled_weights.average_fill = 0LL;
    fast_path.scaled_weights.minimum_fill = 0LL;

    if (!multiply_nonnegative_ll_fits(fast_path.scaled_weights.rank_sum,
                                      rank_limit,
                                      per_edge_limit,
                                      &term_value) ||
        !add_nonnegative_ll_fits(edge_cost_bound,
                                 term_value,
                                 per_edge_limit,
                                 &edge_cost_bound) ||
        !multiply_nonnegative_ll_fits(fast_path.scaled_weights.rank_square,
                                      rank_square_limit,
                                      per_edge_limit,
                                      &term_value) ||
        !add_nonnegative_ll_fits(edge_cost_bound,
                                 term_value,
                                 per_edge_limit,
                                 &edge_cost_bound) ||
        !add_nonnegative_ll_fits(edge_cost_bound,
                                 fast_path.scaled_weights.outside,
                                 per_edge_limit,
                                 &edge_cost_bound) ||
        !add_nonnegative_ll_fits(edge_cost_bound,
                                 fast_path.scaled_weights.change,
                                 per_edge_limit,
                                 &edge_cost_bound)) {
        return fast_path;
    }

    fast_path.lab_average_rewards =
        checked_calloc((size_t)problem_data->lab_count, sizeof(long long));
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        long long capacity_value = problem_data->labs[lab_index].capacity_value;
        long long denominator_share;
        long long reward_value;
        if (capacity_value <= 0LL) {
            continue;
        }
        if (lcm_u64 % (unsigned long long)capacity_value != 0ULL) {
            weighted_average_fast_path_free(&fast_path);
            return fast_path;
        }
        if (lcm_u64 / (unsigned long long)capacity_value >
            (unsigned long long)LLONG_MAX) {
            weighted_average_fast_path_free(&fast_path);
            return fast_path;
        }
        denominator_share = (long long)(lcm_u64 / (unsigned long long)capacity_value);
        if (!multiply_nonnegative_ll_fits(weights->average_fill,
                                          denominator_share,
                                          per_edge_limit,
                                          &reward_value)) {
            weighted_average_fast_path_free(&fast_path);
            return fast_path;
        }
        if (!add_nonnegative_ll_fits(edge_cost_bound,
                                     reward_value,
                                     per_edge_limit,
                                     &term_value)) {
            weighted_average_fast_path_free(&fast_path);
            return fast_path;
        }
        fast_path.lab_average_rewards[lab_index] = reward_value;
    }

    fast_path.available = 1;
    return fast_path;
}

static SignedBigScore weighted_additive_score_for_solution(
    const ProblemData *problem_data,
    const SolutionResult *solution,
    const WeightedObjective *weights,
    const ExactAverageContext *context)
{
    WeightedObjective additive_weights = *weights;
    additive_weights.max_rank = 0LL;
    additive_weights.minimum_fill = 0LL;
    return weighted_score_for_solution(problem_data,
                                       solution,
                                       &additive_weights,
                                       context);
}

static int weighted_search_box_less(const WeightedSearchBox *left,
                                    const WeightedSearchBox *right)
{
    int comparison =
        signed_big_score_compare(&left->lower_bound, &right->lower_bound);
    if (comparison != 0) {
        return comparison < 0;
    }
    if (left->q_low_index != right->q_low_index) {
        return left->q_low_index < right->q_low_index;
    }
    if (left->u_low_index != right->u_low_index) {
        return left->u_low_index < right->u_low_index;
    }
    if (left->q_high_index != right->q_high_index) {
        return left->q_high_index < right->q_high_index;
    }
    return left->u_high_index < right->u_high_index;
}

static void weighted_search_box_heap_swap(WeightedSearchBox *left,
                                          WeightedSearchBox *right)
{
    WeightedSearchBox temporary = *left;
    *left = *right;
    *right = temporary;
}

static void weighted_search_box_heap_push(WeightedSearchBoxHeap *heap,
                                          WeightedSearchBox box)
{
    int index_value;
    if (heap->size == heap->capacity) {
        int new_capacity = heap->capacity == 0 ? 64 : heap->capacity * 2;
        heap->items =
            checked_realloc(heap->items,
                            (size_t)new_capacity * sizeof(WeightedSearchBox));
        heap->capacity = new_capacity;
    }
    index_value = heap->size;
    heap->items[heap->size++] = box;
    while (index_value > 0) {
        int parent_index = (index_value - 1) / 2;
        if (!weighted_search_box_less(&heap->items[index_value],
                                      &heap->items[parent_index])) {
            break;
        }
        weighted_search_box_heap_swap(&heap->items[index_value],
                                      &heap->items[parent_index]);
        index_value = parent_index;
    }
}

static WeightedSearchBox weighted_search_box_heap_pop(WeightedSearchBoxHeap *heap)
{
    WeightedSearchBox result;
    int index_value = 0;
    if (heap->size <= 0) {
        fail_with_context("weighted exact objective", "empty search box heap");
    }
    result = heap->items[0];
    heap->size--;
    if (heap->size > 0) {
        heap->items[0] = heap->items[heap->size];
        while (1) {
            int left_child = index_value * 2 + 1;
            int right_child = left_child + 1;
            int smallest = index_value;
            if (left_child < heap->size &&
                weighted_search_box_less(&heap->items[left_child],
                                         &heap->items[smallest])) {
                smallest = left_child;
            }
            if (right_child < heap->size &&
                weighted_search_box_less(&heap->items[right_child],
                                         &heap->items[smallest])) {
                smallest = right_child;
            }
            if (smallest == index_value) {
                break;
            }
            weighted_search_box_heap_swap(&heap->items[index_value],
                                          &heap->items[smallest]);
            index_value = smallest;
        }
    }
    return result;
}

static void weighted_search_box_heap_free(WeightedSearchBoxHeap *heap)
{
    free(heap->items);
    heap->items = NULL;
    heap->size = 0;
    heap->capacity = 0;
}

static WeightedRelaxedCornerCache weighted_relaxed_corner_cache_create(
    int q_count,
    int u_count)
{
    WeightedRelaxedCornerCache cache;
    size_t item_count =
        checked_multiply_size((size_t)q_count,
                              (size_t)u_count,
                              "weighted exact relaxed-corner cache");
    cache.items =
        checked_calloc(item_count, sizeof(WeightedRelaxedCornerCacheEntry *));
    cache.q_count = q_count;
    cache.u_count = u_count;
    return cache;
}

static WeightedRelaxedCornerCacheEntry *weighted_relaxed_corner_cache_entry(
    WeightedRelaxedCornerCache *cache,
    int q_index,
    int u_index)
{
    size_t offset;
    if (q_index < 0 || q_index >= cache->q_count ||
        u_index < 0 || u_index >= cache->u_count) {
        fail_with_context("weighted exact relaxed-corner cache",
                          "cache index out of range");
    }
    offset = (size_t)q_index * (size_t)cache->u_count + (size_t)u_index;
    if (cache->items[offset] == NULL) {
        cache->items[offset] =
            checked_calloc(1U, sizeof(WeightedRelaxedCornerCacheEntry));
    }
    return cache->items[offset];
}

static void weighted_relaxed_corner_cache_free(
    WeightedRelaxedCornerCache *cache)
{
    int q_index;
    int u_index;
    if (cache->items != NULL) {
        for (q_index = 0; q_index < cache->q_count; q_index++) {
            for (u_index = 0; u_index < cache->u_count; u_index++) {
                size_t offset =
                    (size_t)q_index * (size_t)cache->u_count + (size_t)u_index;
                WeightedRelaxedCornerCacheEntry *entry = cache->items[offset];
                if (entry != NULL) {
                    if (entry->computed) {
                        free_solution_result(&entry->result.solution);
                    }
                    free(entry);
                }
            }
        }
    }
    free(cache->items);
    cache->items = NULL;
    cache->q_count = 0;
    cache->u_count = 0;
}

static WeightedStudentGroupCache weighted_student_group_cache_create(
    int q_count,
    int enabled)
{
    WeightedStudentGroupCache cache;
    cache.count = q_count;
    cache.enabled = enabled;
    cache.groups = NULL;
    cache.computed = NULL;
    if (enabled) {
        cache.groups =
            checked_calloc((size_t)q_count, sizeof(StudentGroups));
        cache.computed =
            checked_calloc((size_t)q_count, sizeof(unsigned char));
    }
    return cache;
}

static const StudentGroups *weighted_student_group_cache_get(
    WeightedStudentGroupCache *cache,
    const ProblemData *problem_data,
    const IntList *rank_candidates,
    int q_index)
{
    if (!cache->enabled) {
        return NULL;
    }
    if (q_index < 0 || q_index >= cache->count) {
        fail_with_context("weighted exact group cache",
                          "cache index out of range");
    }
    if (!cache->computed[q_index]) {
        cache->groups[q_index] =
            build_student_groups(problem_data,
                                 rank_candidates->items[q_index],
                                 STUDENT_GROUP_ACTIVE_RANK);
        cache->computed[q_index] = 1U;
    }
    if (cache->groups[q_index].count >= problem_data->student_count) {
        return NULL;
    }
    return &cache->groups[q_index];
}

static void weighted_student_group_cache_free(WeightedStudentGroupCache *cache)
{
    int q_index;
    if (cache->groups != NULL && cache->computed != NULL) {
        for (q_index = 0; q_index < cache->count; q_index++) {
            if (cache->computed[q_index]) {
                free_student_groups(&cache->groups[q_index]);
            }
        }
    }
    free(cache->groups);
    free(cache->computed);
    cache->groups = NULL;
    cache->computed = NULL;
    cache->count = 0;
    cache->enabled = 0;
}

static void weighted_score_add_ratio_term(SignedBigScore *score,
                                          const ExactAverageContext *context,
                                          long long signed_weight,
                                          RatioValue ratio,
                                          const char *context_text)
{
    BigUInt denominator_weight = context->common_denominator;
    unsigned long long denominator =
        (unsigned long long)ratio.denominator;
    unsigned long long scale;
    if (signed_weight == 0LL || ratio.numerator == 0LL) {
        return;
    }
    if (ratio.denominator <= 0LL || ratio.numerator < 0LL) {
        fail_with_context(context_text, "invalid ratio");
    }
    big_uint_divide_u64_exact(&denominator_weight, denominator);
    scale = absolute_long_long_as_u64(signed_weight, context_text);
    scale = checked_multiply_u64(scale,
                                 (unsigned long long)context->positive_lab_count,
                                 context_text);
    scale = checked_multiply_u64(scale,
                                 (unsigned long long)ratio.numerator,
                                 context_text);
    signed_big_score_add_scaled(score,
                                &denominator_weight,
                                scale,
                                signed_weight < 0LL);
}

static SignedBigScore weighted_lower_bound_for_box(
    const SignedBigScore *relaxed_additive_score,
    const ExactAverageContext *context,
    const WeightedObjective *weights,
    int minimum_q,
    RatioValue maximum_minimum_fill)
{
    SignedBigScore lower_bound = *relaxed_additive_score;
    long long max_rank_part =
        checked_add_weighted_term(0LL,
                                  weights->max_rank,
                                  (long long)minimum_q,
                                  "weighted exact score");
    weighted_score_add_integer(&lower_bound, context, max_rank_part);
    weighted_score_add_ratio_term(&lower_bound,
                                  context,
                                  -weights->minimum_fill,
                                  maximum_minimum_fill,
                                  "weighted exact score");
    return lower_bound;
}

static RatioValue maximum_ratio_in_minimum_candidate_range(
    const MinimumCountCandidateList *minimum_candidates,
    int low_index,
    int high_index)
{
    /*
       build_minimum_count_candidates() keeps only the maximum ratio for each
       minimum-count vector and sorts candidates by ratio ascending, so the
       largest ratio in any contiguous search range is at high_index.
    */
    (void)low_index;
    return minimum_candidates->items[high_index].ratio;
}

static long double ratio_value_to_long_double(RatioValue ratio)
{
    if (ratio.denominator <= 0LL) {
        return 0.0L;
    }
    return (long double)ratio.numerator / (long double)ratio.denominator;
}

static int weighted_box_should_split_q(
    const WeightedSearchBox *box,
    const WeightedObjective *weights,
    const IntList *rank_candidates,
    const MinimumCountCandidateList *minimum_candidates)
{
    int q_width = box->q_high_index - box->q_low_index;
    int u_width = box->u_high_index - box->u_low_index;
    long double q_spread;
    long double u_spread;
    long double u_low;
    long double u_high;

    if (q_width <= 0) {
        return 0;
    }
    if (u_width <= 0) {
        return 1;
    }
    if (weights->max_rank == 0LL && weights->minimum_fill != 0LL) {
        return 0;
    }
    if (weights->minimum_fill == 0LL && weights->max_rank != 0LL) {
        return 1;
    }

    q_spread =
        (long double)weights->max_rank *
        (long double)(rank_candidates->items[box->q_high_index] -
                      rank_candidates->items[box->q_low_index]);
    u_low = ratio_value_to_long_double(
        minimum_candidates->items[box->u_low_index].ratio);
    u_high = ratio_value_to_long_double(
        minimum_candidates->items[box->u_high_index].ratio);
    u_spread = (long double)weights->minimum_fill * (u_high - u_low);

    if (q_spread > u_spread) {
        return 1;
    }
    if (u_spread > q_spread) {
        return 0;
    }
    return q_width >= u_width;
}

static SolutionResult weighted_solution_result_from_assignment(
    const ProblemData *problem_data,
    const int *assignment,
    const WeightedObjective *weights)
{
    SolutionResult solution = empty_solution_result();
    int student_index;
    solution.assignment =
        checked_malloc((size_t)problem_data->student_count * sizeof(int));
    solution.lab_counts =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));
    memcpy(solution.assignment,
           assignment,
           (size_t)problem_data->student_count * sizeof(int));
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        int lab_index = assignment[student_index];
        if (lab_index < 0 || lab_index >= problem_data->lab_count) {
            free_solution_result(&solution);
            fail_with_context("weighted exact incumbent",
                              "assignment contains invalid lab index");
        }
        solution.lab_counts[lab_index]++;
    }
    solution.total_cost.second =
        weighted_additive_cost_for_assignment(problem_data, assignment, weights);
    return solution;
}

static void weighted_consider_solution(
    const ProblemData *problem_data,
    SolutionResult *candidate_solution,
    const WeightedObjective *weights,
    const ExactAverageContext *score_context,
    SolutionResult *best_solution,
    SignedBigScore *best_score,
    int *best_score_is_set)
{
    SignedBigScore candidate_score =
        weighted_score_for_solution(problem_data,
                                    candidate_solution,
                                    weights,
                                    score_context);
    if (!*best_score_is_set ||
        weighted_score_less(&candidate_score, best_score)) {
        free_solution_result(best_solution);
        *best_solution = *candidate_solution;
        *candidate_solution = empty_solution_result();
        *best_score = candidate_score;
        *best_score_is_set = 1;
    }
}

static void weighted_consider_assignment_seed(
    const ProblemData *problem_data,
    int *seed_assignment,
    const WeightedObjective *weights,
    const ExactAverageContext *score_context,
    SolutionResult *best_solution,
    SignedBigScore *best_score,
    int *best_score_is_set)
{
    SolutionResult seed_solution;
    if (seed_assignment == NULL) {
        return;
    }
    seed_solution =
        weighted_solution_result_from_assignment(problem_data,
                                                 seed_assignment,
                                                 weights);
    free(seed_assignment);
    weighted_consider_solution(problem_data,
                               &seed_solution,
                               weights,
                               score_context,
                               best_solution,
                               best_score,
                               best_score_is_set);
    free_solution_result(&seed_solution);
}

static void weighted_seed_initial_incumbent(
    const ProblemData *problem_data,
    const WeightedObjective *weights,
    const ExactAverageContext *score_context,
    SolutionResult *best_solution,
    SignedBigScore *best_score,
    int *best_score_is_set,
    int include_light_objective_seeds)
{
    WeightedObjective seed_weights = *weights;
    int *seed_assignment;
    seed_weights.average_fill = 0LL;
    seed_weights.minimum_fill = 0LL;
    seed_assignment =
        solve_weighted_exact_integer_only_problem(problem_data, &seed_weights);
    weighted_consider_assignment_seed(problem_data,
                                      seed_assignment,
                                      weights,
                                      score_context,
                                      best_solution,
                                      best_score,
                                      best_score_is_set);

    if (include_light_objective_seeds) {
        int minimum_max_rank = find_minimum_feasible_max_rank(problem_data);

        weighted_consider_assignment_seed(
            problem_data,
            solve_rank_first_problem(problem_data,
                                     problem_data->lab_count + 1,
                                     FILL_TIE_AVERAGE_THEN_MINIMUM,
                                     NULL),
            weights,
            score_context,
            best_solution,
            best_score,
            best_score_is_set);
        weighted_consider_assignment_seed(
            problem_data,
            solve_rank_first_problem(problem_data,
                                     problem_data->lab_count + 1,
                                     FILL_TIE_MINIMUM_THEN_AVERAGE,
                                     NULL),
            weights,
            score_context,
            best_solution,
            best_score,
            best_score_is_set);
        weighted_consider_assignment_seed(
            problem_data,
            solve_problem(problem_data, minimum_max_rank),
            weights,
            score_context,
            best_solution,
            best_score,
            best_score_is_set);
        weighted_consider_assignment_seed(
            problem_data,
            solve_rank_first_problem(problem_data,
                                     minimum_max_rank,
                                     FILL_TIE_AVERAGE_THEN_MINIMUM,
                                     NULL),
            weights,
            score_context,
            best_solution,
            best_score,
            best_score_is_set);
        if (problem_data->rank_cost_model != NULL) {
            weighted_consider_assignment_seed(
                problem_data,
                solve_rank_first_problem(problem_data,
                                         problem_data->lab_count + 1,
                                         FILL_TIE_AVERAGE_THEN_MINIMUM,
                                         problem_data->rank_cost_model),
                weights,
                score_context,
                best_solution,
                best_score,
                best_score_is_set);
        }
    }
}

static int weighted_compute_box_lower_bound(
    const ProblemData *problem_data,
    const WeightedObjective *active_weights,
    const ExactAverageContext *score_context,
    const WeightedAverageFastPath *average_fast_path,
    const IntList *rank_candidates,
    const MinimumCountCandidateList *minimum_candidates,
    WeightedRelaxedCornerCache *relaxed_corner_cache,
    WeightedStudentGroupCache *student_group_cache,
    WeightedSearchBox *box)
{
    int q_low = rank_candidates->items[box->q_low_index];
    int q_high = rank_candidates->items[box->q_high_index];
    const MinimumCountCandidate *minimum_low =
        &minimum_candidates->items[box->u_low_index];
    WeightedRelaxedCornerCacheEntry *corner_entry =
        weighted_relaxed_corner_cache_entry(relaxed_corner_cache,
                                            box->q_high_index,
                                            box->u_low_index);
    if (!corner_entry->computed) {
        const StudentGroups *active_group_pointer =
            weighted_student_group_cache_get(student_group_cache,
                                             problem_data,
                                             rank_candidates,
                                             box->q_high_index);
        ExactAverageContext flow_context;
        const ExactAverageContext *flow_context_pointer = NULL;
        const WeightedObjective *flow_weights = active_weights;
        const long long *lab_average_rewards = NULL;
        int use_average_weight =
            active_weights->average_fill != 0LL && !average_fast_path->available;
        if (active_profile != NULL) {
            active_profile->weighted_corner_cache_misses++;
        }
        if (average_fast_path->available) {
            flow_weights = &average_fast_path->scaled_weights;
            lab_average_rewards = average_fast_path->lab_average_rewards;
        } else if (use_average_weight) {
            flow_context = exact_average_context_create(problem_data);
            flow_context.use_weighted_scalar = 1;
            flow_context.average_fill_weight = active_weights->average_fill;
            flow_context_pointer = &flow_context;
        }
        corner_entry->result =
            try_solve_with_minimum_counts(problem_data,
                                          q_high,
                                          minimum_low->minimum_counts,
                                          use_average_weight,
                                          0,
                                          flow_context_pointer,
                                          NULL,
                                          flow_weights,
                                          lab_average_rewards,
                                          NULL,
                                          active_group_pointer);
        if (flow_context_pointer != NULL) {
            exact_average_context_free(&flow_context);
        }
        signed_big_score_zero(&corner_entry->additive_score);
        if (corner_entry->result.feasible &&
            corner_entry->result.solution.total_cost.first ==
                -(long long)minimum_low->minimum_count_sum) {
            if (average_fast_path->available) {
                corner_entry->result.solution.total_cost.second =
                    weighted_additive_cost_for_assignment(
                        problem_data,
                        corner_entry->result.solution.assignment,
                        active_weights);
            }
            corner_entry->additive_score =
                weighted_additive_score_for_solution(problem_data,
                                                     &corner_entry->result.solution,
                                                     active_weights,
                                                     score_context);
        }
        corner_entry->computed = 1;
    } else if (active_profile != NULL) {
        active_profile->weighted_corner_cache_hits++;
    }
    if (!corner_entry->result.feasible ||
        corner_entry->result.solution.total_cost.first !=
            -(long long)minimum_low->minimum_count_sum) {
        return 0;
    }
    box->lower_bound =
        weighted_lower_bound_for_box(&corner_entry->additive_score,
                                     score_context,
                                     active_weights,
                                     q_low,
                                     maximum_ratio_in_minimum_candidate_range(
                                         minimum_candidates,
                                         box->u_low_index,
                                         box->u_high_index));
    return 1;
}

static int *solve_weighted_exact_problem(const ProblemData *problem_data,
                                         const WeightedObjective *weights)
{
    WeightedObjective effective_weights = *weights;
    const WeightedObjective *active_weights = &effective_weights;
    if (graph_capacity_sum_equals_student_count(problem_data)) {
        effective_weights.average_fill = 0LL;
        effective_weights.minimum_fill = 0LL;
    }
    if (active_weights->average_fill == 0LL &&
        active_weights->minimum_fill == 0LL) {
        return solve_weighted_exact_integer_only_problem(problem_data, active_weights);
    }
    ExactAverageContext score_context = exact_average_context_create(problem_data);
    WeightedAverageFastPath average_fast_path;
    MinimumCountCandidateList minimum_candidates =
        build_minimum_count_candidates(problem_data, active_weights->minimum_fill != 0LL);
    IntList rank_candidates = build_rank_threshold_candidates(problem_data);
    SolutionResult best_solution = empty_solution_result();
    SignedBigScore best_score;
    int best_score_is_set = 0;
    int first_q_index;
    int last_q_index;
    int q_start;
    int q_end = target_rank_upper_bound(problem_data, problem_data->lab_count + 1);
    long long weighted_grid_size;
    WeightedSearchBoxHeap search_boxes;
    WeightedRelaxedCornerCache relaxed_corner_cache;
    WeightedStudentGroupCache student_group_cache;

    average_fast_path.available = 0;
    average_fast_path.scaled_weights = *active_weights;
    average_fast_path.lab_average_rewards = NULL;

    q_start = active_weights->max_rank == 0LL ?
                  q_end :
                  find_minimum_feasible_max_rank(problem_data);
    if (q_end < 1) {
        int_list_free(&rank_candidates);
        minimum_count_candidate_list_free(&minimum_candidates);
        weighted_average_fast_path_free(&average_fast_path);
        exact_average_context_free(&score_context);
        fail_with_context("target constraints",
                          "No feasible solution: max-rank target leaves no allowable rank");
    }

    average_fast_path =
        weighted_average_fast_path_create(problem_data,
                                          &score_context,
                                          active_weights,
                                          q_end);

    first_q_index = int_list_first_index_at_least(&rank_candidates, q_start);
    last_q_index = int_list_last_index_at_most(&rank_candidates, q_end);
    if (first_q_index > last_q_index) {
        int_list_free(&rank_candidates);
        minimum_count_candidate_list_free(&minimum_candidates);
        weighted_average_fast_path_free(&average_fast_path);
        exact_average_context_free(&score_context);
        fail_with_context("weighted exact objective", "no rank threshold candidate found");
    }

    search_boxes.items = NULL;
    search_boxes.size = 0;
    search_boxes.capacity = 0;
    relaxed_corner_cache =
        weighted_relaxed_corner_cache_create(rank_candidates.size,
                                             minimum_candidates.size);
    student_group_cache =
        weighted_student_group_cache_create(
            rank_candidates.size,
            !(active_weights->change > 0LL &&
              problem_data->base_assignment != NULL));
    /*
     * The seed is only a pruning incumbent.  It is useful on sparse rank
     * threshold grids, but on full-preference inputs it can require a costly
     * rank-threshold scan before branch-and-bound has solved any corner.
     * The first relaxed corner is considered as an incumbent anyway, so skip
     * this optional seed when the rank axis itself is large.
     */
    weighted_grid_size =
        (long long)(last_q_index - first_q_index + 1) *
        (long long)minimum_candidates.size;
    if ((last_q_index - first_q_index + 1) <= WEIGHTED_SEED_MAX_Q_AXIS &&
        weighted_grid_size >= WEIGHTED_LIGHT_SEED_MIN_GRID_SIZE) {
        weighted_seed_initial_incumbent(problem_data,
                                        active_weights,
                                        &score_context,
                                        &best_solution,
                                        &best_score,
                                        &best_score_is_set,
                                        weighted_grid_size >=
                                            WEIGHTED_SEED_MIN_GRID_SIZE);
    }
    {
        WeightedSearchBox initial_box;
        initial_box.q_low_index = first_q_index;
        initial_box.q_high_index = last_q_index;
        initial_box.u_low_index = 0;
        initial_box.u_high_index = minimum_candidates.size - 1;
        signed_big_score_zero(&initial_box.lower_bound);
        if (weighted_compute_box_lower_bound(problem_data,
                                             active_weights,
                                             &score_context,
                                             &average_fast_path,
                                             &rank_candidates,
                                             &minimum_candidates,
                                             &relaxed_corner_cache,
                                             &student_group_cache,
                                             &initial_box) &&
            (!best_score_is_set ||
             weighted_score_less(&initial_box.lower_bound, &best_score))) {
            weighted_search_box_heap_push(&search_boxes, initial_box);
        }
    }

    while (search_boxes.size > 0) {
        WeightedSearchBox box = weighted_search_box_heap_pop(&search_boxes);
        const MinimumCountCandidate *minimum_low =
            &minimum_candidates.items[box.u_low_index];
        WeightedRelaxedCornerCacheEntry *corner_entry =
            weighted_relaxed_corner_cache_entry(&relaxed_corner_cache,
                                                box.q_high_index,
                                                box.u_low_index);
        OptionalSolutionResult relaxed_solution;

        if (active_profile != NULL) {
            active_profile->q_candidates_tested++;
            active_profile->minimum_candidates_tested++;
        }

        if (best_score_is_set && !weighted_score_less(&box.lower_bound, &best_score)) {
            if (active_profile != NULL) {
                active_profile->weighted_bound_prunes++;
            }
            continue;
        }

        relaxed_solution =
            copy_optional_solution_result(problem_data, &corner_entry->result);
        if (relaxed_solution.feasible &&
            relaxed_solution.solution.total_cost.first ==
                -(long long)minimum_low->minimum_count_sum) {
            weighted_consider_solution(problem_data,
                                       &relaxed_solution.solution,
                                       active_weights,
                                       &score_context,
                                       &best_solution,
                                       &best_score,
                                       &best_score_is_set);
        }
        free_solution_result(&relaxed_solution.solution);

        if (box.q_low_index == box.q_high_index &&
            box.u_low_index == box.u_high_index) {
            continue;
        }
        if (weighted_box_should_split_q(&box,
                                        active_weights,
                                        &rank_candidates,
                                        &minimum_candidates)) {
            int middle_index = box.q_low_index +
                               (box.q_high_index - box.q_low_index) / 2;
            WeightedSearchBox left_box;
            WeightedSearchBox right_box;
            left_box.q_low_index = box.q_low_index;
            left_box.q_high_index = middle_index;
            left_box.u_low_index = box.u_low_index;
            left_box.u_high_index = box.u_high_index;
            signed_big_score_zero(&left_box.lower_bound);
            right_box.q_low_index = middle_index + 1;
            right_box.q_high_index = box.q_high_index;
            right_box.u_low_index = box.u_low_index;
            right_box.u_high_index = box.u_high_index;
            signed_big_score_zero(&right_box.lower_bound);
            if (weighted_compute_box_lower_bound(problem_data,
                                                 active_weights,
                                                 &score_context,
                                                 &average_fast_path,
                                                 &rank_candidates,
                                                 &minimum_candidates,
                                                 &relaxed_corner_cache,
                                                 &student_group_cache,
                                                 &left_box) &&
                (!best_score_is_set ||
                 weighted_score_less(&left_box.lower_bound, &best_score))) {
                weighted_search_box_heap_push(&search_boxes, left_box);
            }
            if (weighted_compute_box_lower_bound(problem_data,
                                                 active_weights,
                                                 &score_context,
                                                 &average_fast_path,
                                                 &rank_candidates,
                                                 &minimum_candidates,
                                                 &relaxed_corner_cache,
                                                 &student_group_cache,
                                                 &right_box) &&
                (!best_score_is_set ||
                 weighted_score_less(&right_box.lower_bound, &best_score))) {
                weighted_search_box_heap_push(&search_boxes, right_box);
            }
        } else {
            int middle_index = box.u_low_index +
                               (box.u_high_index - box.u_low_index) / 2;
            WeightedSearchBox low_box;
            WeightedSearchBox high_box;
            low_box.q_low_index = box.q_low_index;
            low_box.q_high_index = box.q_high_index;
            low_box.u_low_index = box.u_low_index;
            low_box.u_high_index = middle_index;
            signed_big_score_zero(&low_box.lower_bound);
            high_box.q_low_index = box.q_low_index;
            high_box.q_high_index = box.q_high_index;
            high_box.u_low_index = middle_index + 1;
            high_box.u_high_index = box.u_high_index;
            signed_big_score_zero(&high_box.lower_bound);
            if (weighted_compute_box_lower_bound(problem_data,
                                                 active_weights,
                                                 &score_context,
                                                 &average_fast_path,
                                                 &rank_candidates,
                                                 &minimum_candidates,
                                                 &relaxed_corner_cache,
                                                 &student_group_cache,
                                                 &low_box) &&
                (!best_score_is_set ||
                 weighted_score_less(&low_box.lower_bound, &best_score))) {
                weighted_search_box_heap_push(&search_boxes, low_box);
            }
            if (weighted_compute_box_lower_bound(problem_data,
                                                 active_weights,
                                                 &score_context,
                                                 &average_fast_path,
                                                 &rank_candidates,
                                                 &minimum_candidates,
                                                 &relaxed_corner_cache,
                                                 &student_group_cache,
                                                 &high_box) &&
                (!best_score_is_set ||
                 weighted_score_less(&high_box.lower_bound, &best_score))) {
                weighted_search_box_heap_push(&search_boxes, high_box);
            }
        }
    }
    weighted_search_box_heap_free(&search_boxes);
    weighted_relaxed_corner_cache_free(&relaxed_corner_cache);
    weighted_student_group_cache_free(&student_group_cache);

    int_list_free(&rank_candidates);
    minimum_count_candidate_list_free(&minimum_candidates);
    weighted_average_fast_path_free(&average_fast_path);
    exact_average_context_free(&score_context);
    if (!best_score_is_set || best_solution.assignment == NULL) {
        fail_with_context("weighted exact objective", "no feasible assignment found");
    }
    {
        int *assignment = best_solution.assignment;
        best_solution.assignment = NULL;
        free_solution_result(&best_solution);
        return assignment;
    }
}

static int *solve_assignment_for_mode(const ProblemData *problem_data,
                                      ObjectiveMode objective_mode,
                                      const ProgramOptions *options)
{
    if (objective_mode == OBJECTIVE_WEIGHTED_EXACT) {
        WeightedObjective weights = options->weights;
        if (weights.change == 0LL && options->change_penalty > 0LL) {
            weights.change = options->change_penalty;
        }
        return solve_weighted_exact_problem(problem_data, &weights);
    }
    if (objective_mode == OBJECTIVE_FAIR) {
        return solve_fair_average_resource_bounded_problem(
            problem_data,
            problem_data->lab_count + 1);
    }
    if (objective_mode == OBJECTIVE_SATISFACTION) {
        return solve_rank_first_average_resource_bounded_problem(
            problem_data,
            problem_data->lab_count + 1,
            FILL_TIE_AVERAGE_THEN_MINIMUM,
            &options->rank_cost_model);
    }
    if (objective_mode == OBJECTIVE_BALANCED) {
        return solve_rank_first_average_resource_bounded_problem(
            problem_data,
            problem_data->lab_count + 1,
            FILL_TIE_MINIMUM_THEN_AVERAGE,
            NULL);
    }
    if (objective_mode == OBJECTIVE_GUARDED) {
        int minimum_max_rank;
        int guarded_max_rank;
        if (average_fill_target_needs_bounded_resource(problem_data)) {
            int *fair_assignment =
                solve_fair_average_resource_bounded_problem(
                    problem_data,
                    problem_data->lab_count + 1);
            minimum_max_rank =
                (int)max_rank_for_solution(problem_data, fair_assignment);
            free(fair_assignment);
        } else {
            minimum_max_rank =
                find_fair_max_rank_satisfying_rank_sum_targets(problem_data);
        }
        guarded_max_rank = minimum_max_rank + options->max_rank_slack;
        if (guarded_max_rank < minimum_max_rank ||
            guarded_max_rank > problem_data->lab_count + 1) {
            guarded_max_rank = problem_data->lab_count + 1;
        }
        return solve_rank_first_average_resource_bounded_problem(
            problem_data,
            guarded_max_rank,
            FILL_TIE_AVERAGE_THEN_MINIMUM,
            NULL);
    }
    if (objective_mode == OBJECTIVE_FILL_CONVEX) {
        return solve_convex_fill_problem(problem_data);
    }
    return solve_rank_first_average_resource_bounded_problem(
        problem_data,
        problem_data->lab_count + 1,
        FILL_TIE_AVERAGE_THEN_MINIMUM,
        NULL);
}

static EvaluationMetrics compute_evaluation_metrics(const ProblemData *problem_data,
                                                    const int *assignment)
{
    EvaluationMetrics metrics;
    int student_index;
    int lab_index;
    long double mean_square_rank;
    long double variance;
    int positive_lab_count = 0;

    metrics.rank_sum = 0LL;
    metrics.rank_square_sum = 0LL;
    metrics.dissatisfaction_sum = 0LL;
    metrics.dissatisfaction_square_sum = 0LL;
    metrics.max_dissatisfaction = 0LL;
    metrics.max_rank = 0;
    metrics.average_rank = 0.0L;
    metrics.rank_stddev = 0.0L;
    metrics.average_dissatisfaction = 0.0L;
    metrics.dissatisfaction_stddev = 0.0L;
    metrics.average_fill_rate = 0.0L;
    metrics.minimum_fill_rate = 0.0L;
    metrics.solver_cpu_seconds = 0.0L;
    metrics.counterfactual_cpu_seconds = 0.0L;
    metrics.program_cpu_seconds_before_metrics = 0.0L;
    metrics.lab_counts = checked_calloc((size_t)problem_data->lab_count, sizeof(int));

    for (student_index = 0; student_index < problem_data->student_count; student_index++) {
        int lab_index_for_student = assignment[student_index];
        int rank_value = rank_for_assignment(problem_data, student_index, lab_index_for_student);
        long long dissatisfaction =
            dissatisfaction_cost_for_rank(problem_data,
                                          problem_data->rank_cost_model,
                                          rank_value);
        long long dissatisfaction_square =
            checked_multiply_rank_cost(dissatisfaction,
                                       dissatisfaction,
                                       "rank satisfaction cost");
        metrics.lab_counts[lab_index_for_student]++;
        metrics.rank_sum += (long long)rank_value;
        metrics.rank_square_sum += (long long)rank_value * (long long)rank_value;
        metrics.dissatisfaction_sum += dissatisfaction;
        metrics.dissatisfaction_square_sum += dissatisfaction_square;
        if (rank_value > metrics.max_rank) {
            metrics.max_rank = rank_value;
        }
        if (dissatisfaction > metrics.max_dissatisfaction) {
            metrics.max_dissatisfaction = dissatisfaction;
        }
    }

    metrics.average_rank =
        (long double)metrics.rank_sum / (long double)problem_data->student_count;
    mean_square_rank =
        (long double)metrics.rank_square_sum / (long double)problem_data->student_count;
    variance = mean_square_rank - metrics.average_rank * metrics.average_rank;
    if (variance < 0.0L && variance > -0.000000000001L) {
        variance = 0.0L;
    }
    metrics.rank_stddev = square_root_long_double(variance);
    metrics.average_dissatisfaction =
        (long double)metrics.dissatisfaction_sum /
        (long double)problem_data->student_count;
    mean_square_rank =
        (long double)metrics.dissatisfaction_square_sum /
        (long double)problem_data->student_count;
    variance = mean_square_rank -
               metrics.average_dissatisfaction *
                   metrics.average_dissatisfaction;
    if (variance < 0.0L && variance > -0.000000000001L) {
        variance = 0.0L;
    }
    metrics.dissatisfaction_stddev = square_root_long_double(variance);

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        long long capacity_value = problem_data->labs[lab_index].capacity_value;
        if (capacity_value > 0LL) {
            long double fill_rate =
                (long double)metrics.lab_counts[lab_index] / (long double)capacity_value;
            if (positive_lab_count == 0 || fill_rate < metrics.minimum_fill_rate) {
                metrics.minimum_fill_rate = fill_rate;
            }
            metrics.average_fill_rate += fill_rate;
            positive_lab_count++;
        }
    }
    if (positive_lab_count > 0) {
        metrics.average_fill_rate /= (long double)positive_lab_count;
    }

    return metrics;
}

static void free_evaluation_metrics(EvaluationMetrics *metrics)
{
    free(metrics->lab_counts);
    metrics->lab_counts = NULL;
}

static char *metrics_output_path_for(const char *output_file_path)
{
    const char *suffix = ".metrics.txt";
    size_t output_length = strlen(output_file_path);
    size_t suffix_length = strlen(suffix);
    char *metrics_path;
    if (output_length > (size_t)-1 - suffix_length - 1U) {
        fail_with_context("metrics output path", "path is too long");
    }
    metrics_path = checked_malloc(output_length + suffix_length + 1U);
    memcpy(metrics_path, output_file_path, output_length);
    memcpy(metrics_path + output_length, suffix, suffix_length + 1U);
    return metrics_path;
}

static char *lab_report_output_path_for(const char *output_file_path)
{
    const char *suffix = ".by_lab.txt";
    size_t output_length = strlen(output_file_path);
    size_t suffix_length = strlen(suffix);
    char *report_path;
    if (output_length > (size_t)-1 - suffix_length - 1U) {
        fail_with_context("lab report output path", "path is too long");
    }
    report_path = checked_malloc(output_length + suffix_length + 1U);
    memcpy(report_path, output_file_path, output_length);
    memcpy(report_path + output_length, suffix, suffix_length + 1U);
    return report_path;
}

static char *student_report_output_path_for(const char *output_file_path)
{
    const char *suffix = ".by_student.tsv";
    size_t output_length = strlen(output_file_path);
    size_t suffix_length = strlen(suffix);
    char *report_path;
    if (output_length > (size_t)-1 - suffix_length - 1U) {
        fail_with_context("student report output path", "path is too long");
    }
    report_path = checked_malloc(output_length + suffix_length + 1U);
    memcpy(report_path, output_file_path, output_length);
    memcpy(report_path + output_length, suffix, suffix_length + 1U);
    return report_path;
}

static char *outside_report_output_path_for(const char *output_file_path)
{
    const char *suffix = ".outside_preferences.tsv";
    size_t output_length = strlen(output_file_path);
    size_t suffix_length = strlen(suffix);
    char *report_path;
    if (output_length > (size_t)-1 - suffix_length - 1U) {
        fail_with_context("outside report output path", "path is too long");
    }
    report_path = checked_malloc(output_length + suffix_length + 1U);
    memcpy(report_path, output_file_path, output_length);
    memcpy(report_path + output_length, suffix, suffix_length + 1U);
    return report_path;
}

static char *reasons_report_output_path_for(const char *output_file_path)
{
    const char *suffix = ".reasons.tsv";
    size_t output_length = strlen(output_file_path);
    size_t suffix_length = strlen(suffix);
    char *report_path;
    if (output_length > (size_t)-1 - suffix_length - 1U) {
        fail_with_context("reasons report output path", "path is too long");
    }
    report_path = checked_malloc(output_length + suffix_length + 1U);
    memcpy(report_path, output_file_path, output_length);
    memcpy(report_path + output_length, suffix, suffix_length + 1U);
    return report_path;
}

static char *adjustment_report_output_path_for(const char *output_file_path)
{
    const char *suffix = ".adjustment_delta.tsv";
    size_t output_length = strlen(output_file_path);
    size_t suffix_length = strlen(suffix);
    char *report_path;
    if (output_length > (size_t)-1 - suffix_length - 1U) {
        fail_with_context("adjustment report output path", "path is too long");
    }
    report_path = checked_malloc(output_length + suffix_length + 1U);
    memcpy(report_path, output_file_path, output_length);
    memcpy(report_path + output_length, suffix, suffix_length + 1U);
    return report_path;
}

static char *portfolio_report_output_path_for(const char *output_file_path)
{
    const char *suffix = ".portfolio.tsv";
    size_t output_length = strlen(output_file_path);
    size_t suffix_length = strlen(suffix);
    char *report_path;
    if (output_length > (size_t)-1 - suffix_length - 1U) {
        fail_with_context("portfolio report output path", "path is too long");
    }
    report_path = checked_malloc(output_length + suffix_length + 1U);
    memcpy(report_path, output_file_path, output_length);
    memcpy(report_path + output_length, suffix, suffix_length + 1U);
    return report_path;
}

static char *profile_output_path_for(const char *output_file_path)
{
    const char *suffix = ".profile.tsv";
    size_t output_length = strlen(output_file_path);
    size_t suffix_length = strlen(suffix);
    char *profile_path;
    if (output_length > (size_t)-1 - suffix_length - 1U) {
        fail_with_context("profile output path", "path is too long");
    }
    profile_path = checked_malloc(output_length + suffix_length + 1U);
    memcpy(profile_path, output_file_path, output_length);
    memcpy(profile_path + output_length, suffix, suffix_length + 1U);
    return profile_path;
}

static char *target_status_output_path_for(const char *output_file_path)
{
    const char *suffix = ".target_status.tsv";
    size_t output_length = strlen(output_file_path);
    size_t suffix_length = strlen(suffix);
    char *target_status_path;
    if (output_length > (size_t)-1 - suffix_length - 1U) {
        fail_with_context("target status output path", "path is too long");
    }
    target_status_path = checked_malloc(output_length + suffix_length + 1U);
    memcpy(target_status_path, output_file_path, output_length);
    memcpy(target_status_path + output_length, suffix, suffix_length + 1U);
    return target_status_path;
}

static char *explanation_output_path_for(const char *output_file_path)
{
    const char *suffix = ".explain.tsv";
    size_t output_length = strlen(output_file_path);
    size_t suffix_length = strlen(suffix);
    char *explanation_path;
    if (output_length > (size_t)-1 - suffix_length - 1U) {
        fail_with_context("explanation output path", "path is too long");
    }
    explanation_path = checked_malloc(output_length + suffix_length + 1U);
    memcpy(explanation_path, output_file_path, output_length);
    memcpy(explanation_path + output_length, suffix, suffix_length + 1U);
    return explanation_path;
}

static void print_success_summary(const char *output_file_path,
                                  const ProgramOptions *options)
{
    char *metrics_path = NULL;
    char *lab_report_path = NULL;
    char *student_report_path = NULL;
    char *outside_report_path = NULL;
    char *reasons_report_path = NULL;
    char *portfolio_path = NULL;
    char *profile_path = NULL;
    char *explanation_path = NULL;
    char *adjustment_path = NULL;
    char *target_status_path = NULL;

    if (options->quiet ||
        !(options->write_reports ||
          options->write_profile ||
          options->portfolio_mode ||
          options->explain_student_id != NULL ||
          options->try_lock_text != NULL ||
          options->base_assignment_path != NULL ||
          !target_constraints_are_empty(&options->targets))) {
        return;
    }

    printf("output=%s\n", output_file_path);
    if (options->write_reports) {
        metrics_path = metrics_output_path_for(output_file_path);
        lab_report_path = lab_report_output_path_for(output_file_path);
        student_report_path = student_report_output_path_for(output_file_path);
        outside_report_path = outside_report_output_path_for(output_file_path);
        reasons_report_path = reasons_report_output_path_for(output_file_path);
        printf("reports=%s,%s,%s,%s,%s\n",
               metrics_path,
               lab_report_path,
               student_report_path,
               outside_report_path,
               reasons_report_path);
        if (!target_constraints_are_empty(&options->targets)) {
            target_status_path = target_status_output_path_for(output_file_path);
            printf("target_status=%s\n", target_status_path);
        }
    }
    if (options->portfolio_mode) {
        portfolio_path = portfolio_report_output_path_for(output_file_path);
        printf("portfolio=%s\n", portfolio_path);
    }
    if (options->write_profile) {
        profile_path = profile_output_path_for(output_file_path);
        printf("profile=%s\n", profile_path);
    }
    if (options->explain_student_id != NULL || options->try_lock_text != NULL) {
        explanation_path = explanation_output_path_for(output_file_path);
        printf("explain=%s\n", explanation_path);
    }
    if (options->base_assignment_path != NULL && options->write_reports) {
        adjustment_path = adjustment_report_output_path_for(output_file_path);
        printf("adjustment=%s\n", adjustment_path);
    }
    free(metrics_path);
    free(lab_report_path);
    free(student_report_path);
    free(outside_report_path);
    free(reasons_report_path);
    free(portfolio_path);
    free(profile_path);
    free(explanation_path);
    free(adjustment_path);
    free(target_status_path);
}

static char *portfolio_assignment_output_path_for(const char *output_file_path,
                                                  const char *candidate_name)
{
    size_t output_length = strlen(output_file_path);
    size_t name_length = strlen(candidate_name);
    size_t suffix_length = 5U;
    char *candidate_path;
    if (output_length > (size_t)-1 - name_length - suffix_length - 2U) {
        fail_with_context("portfolio assignment output path", "path is too long");
    }
    candidate_path =
        checked_malloc(output_length + name_length + suffix_length + 2U);
    memcpy(candidate_path, output_file_path, output_length);
    candidate_path[output_length] = '.';
    memcpy(candidate_path + output_length + 1U, candidate_name, name_length);
    memcpy(candidate_path + output_length + 1U + name_length, ".txt", suffix_length);
    return candidate_path;
}

static void remove_file_if_present(const char *path)
{
    if (remove(path) != 0 && errno != ENOENT) {
        fail_with_context(path, "cannot remove temporary report file");
    }
}

static void remove_portfolio_candidate_outputs(const char *candidate_output_path)
{
    char *metrics_path = metrics_output_path_for(candidate_output_path);
    char *lab_report_path = lab_report_output_path_for(candidate_output_path);
    char *student_report_path = student_report_output_path_for(candidate_output_path);
    char *outside_report_path = outside_report_output_path_for(candidate_output_path);
    char *reasons_report_path = reasons_report_output_path_for(candidate_output_path);
    char *adjustment_report_path =
        adjustment_report_output_path_for(candidate_output_path);
    char *portfolio_report_path =
        portfolio_report_output_path_for(candidate_output_path);
    char *profile_path = profile_output_path_for(candidate_output_path);
    char *explanation_path = explanation_output_path_for(candidate_output_path);
    char *target_status_path = target_status_output_path_for(candidate_output_path);

    remove_file_if_present(metrics_path);
    remove_file_if_present(lab_report_path);
    remove_file_if_present(student_report_path);
    remove_file_if_present(outside_report_path);
    remove_file_if_present(reasons_report_path);
    remove_file_if_present(adjustment_report_path);
    remove_file_if_present(portfolio_report_path);
    remove_file_if_present(profile_path);
    remove_file_if_present(explanation_path);
    remove_file_if_present(target_status_path);
    remove_file_if_present(candidate_output_path);

    free(metrics_path);
    free(lab_report_path);
    free(student_report_path);
    free(outside_report_path);
    free(reasons_report_path);
    free(adjustment_report_path);
    free(portfolio_report_path);
    free(profile_path);
    free(explanation_path);
    free(target_status_path);
}

static char *temporary_output_path_for(const char *output_file_path)
{
    size_t output_length = strlen(output_file_path);
    size_t capacity = output_length + 96U;
    char *temporary_path = checked_malloc(capacity);
    int written = snprintf(temporary_path,
                           capacity,
                           "%s.tmp.%ld.%ld.%u",
                           output_file_path,
                           (long)time(NULL),
                           (long)clock(),
                           ++temporary_file_counter);
    if (written < 0 || (size_t)written >= capacity) {
        free(temporary_path);
        fail_with_context("temporary output path", "path is too long");
    }
    return temporary_path;
}

static void replace_output_file(const char *temporary_path,
                                const char *output_file_path,
                                const char *context)
{
#ifdef _WIN32
    if (!MoveFileExA(temporary_path,
                     output_file_path,
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
        fail_with_context_format(context,
                                 "replace temporary output file '%s' failed",
                                 temporary_path);
    }
#else
    if (rename(temporary_path, output_file_path) != 0) {
        fail_with_context_format(context,
                                 "rename temporary output file '%s' failed",
                                 temporary_path);
    }
#endif
}

static int same_existing_file(const char *left_path, const char *right_path)
{
#ifdef _WIN32
    HANDLE left_handle;
    HANDLE right_handle;
    BY_HANDLE_FILE_INFORMATION left_info;
    BY_HANDLE_FILE_INFORMATION right_info;
    int same_file;

    left_handle = CreateFileA(left_path,
                              0,
                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
    if (left_handle == INVALID_HANDLE_VALUE) {
        return 0;
    }
    right_handle = CreateFileA(right_path,
                               0,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    if (right_handle == INVALID_HANDLE_VALUE) {
        CloseHandle(left_handle);
        return 0;
    }
    if (!GetFileInformationByHandle(left_handle, &left_info) ||
        !GetFileInformationByHandle(right_handle, &right_info)) {
        CloseHandle(right_handle);
        CloseHandle(left_handle);
        return 0;
    }
    same_file =
        left_info.dwVolumeSerialNumber == right_info.dwVolumeSerialNumber &&
        left_info.nFileIndexHigh == right_info.nFileIndexHigh &&
        left_info.nFileIndexLow == right_info.nFileIndexLow;
    CloseHandle(right_handle);
    CloseHandle(left_handle);
    return same_file;
#else
    struct stat left_stat;
    struct stat right_stat;

    if (stat(left_path, &left_stat) != 0) {
        return 0;
    }
    if (stat(right_path, &right_stat) != 0) {
        return 0;
    }
    return left_stat.st_dev == right_stat.st_dev &&
           left_stat.st_ino == right_stat.st_ino;
#endif
}

static int path_matches_existing_input_file(const char *candidate_path,
                                            const char *input_path)
{
    return strcmp(candidate_path, input_path) == 0 ||
           same_existing_file(candidate_path, input_path);
}

static void reject_path_equal_to_input_path(const char *context,
                                            const char *lab_file_path,
                                            const char *preference_file_path,
                                            const char *candidate_path)
{
    if (path_matches_existing_input_file(candidate_path, lab_file_path)) {
        fail_with_context(context,
                          "output/report path must be different from lab input file path");
    }
    if (path_matches_existing_input_file(candidate_path, preference_file_path)) {
        fail_with_context(
            context,
            "output/report path must be different from preference input file path");
    }
}

static void reject_path_equal_to_optional_input_path(const char *context,
                                                     const char *optional_input_path,
                                                     const char *candidate_path)
{
    if (optional_input_path != NULL &&
        path_matches_existing_input_file(candidate_path, optional_input_path)) {
        fail_with_context(context,
                          "output/report path must be different from rank cost input file path");
    }
}

static void reject_path_equal_to_named_optional_input_path(
    const char *context,
    const char *optional_input_path,
    const char *candidate_path,
    const char *input_description)
{
    if (optional_input_path != NULL &&
        path_matches_existing_input_file(candidate_path, optional_input_path)) {
        fail_with_context_format(context,
                                 "output/report path must be different from %s",
                                 input_description);
    }
}

static void reject_report_paths_equal_to_input_paths(const char *lab_file_path,
                                                     const char *preference_file_path,
                                                     const char *rank_costs_path,
                                                     const char *weights_path,
                                                     const char *targets_path,
                                                     const char *constraints_path,
                                                     const char *base_assignment_path,
                                                     const char *output_file_path,
                                                     int include_profile_path)
{
    char *metrics_output_path = metrics_output_path_for(output_file_path);
    char *lab_report_output_path = lab_report_output_path_for(output_file_path);
    char *student_report_output_path = student_report_output_path_for(output_file_path);
    char *outside_report_output_path = outside_report_output_path_for(output_file_path);
    char *reasons_report_output_path = reasons_report_output_path_for(output_file_path);
    char *adjustment_report_output_path = adjustment_report_output_path_for(output_file_path);
    char *portfolio_report_output_path = portfolio_report_output_path_for(output_file_path);
    char *profile_output_path = profile_output_path_for(output_file_path);
    char *explanation_output_path = explanation_output_path_for(output_file_path);
    char *target_status_output_path = target_status_output_path_for(output_file_path);
#define REJECT_OPTIONAL_REPORT_INPUT(context_text, path_value, input_text) \
    do { \
        reject_path_equal_to_named_optional_input_path((context_text), \
                                                       (path_value), \
                                                       metrics_output_path, \
                                                       (input_text)); \
        reject_path_equal_to_named_optional_input_path((context_text), \
                                                       (path_value), \
                                                       lab_report_output_path, \
                                                       (input_text)); \
        reject_path_equal_to_named_optional_input_path((context_text), \
                                                       (path_value), \
                                                       student_report_output_path, \
                                                       (input_text)); \
        reject_path_equal_to_named_optional_input_path((context_text), \
                                                       (path_value), \
                                                       outside_report_output_path, \
                                                       (input_text)); \
        reject_path_equal_to_named_optional_input_path((context_text), \
                                                       (path_value), \
                                                       reasons_report_output_path, \
                                                       (input_text)); \
        reject_path_equal_to_named_optional_input_path((context_text), \
                                                       (path_value), \
                                                       adjustment_report_output_path, \
                                                       (input_text)); \
        reject_path_equal_to_named_optional_input_path((context_text), \
                                                       (path_value), \
                                                       portfolio_report_output_path, \
                                                       (input_text)); \
        if (include_profile_path) { \
            reject_path_equal_to_named_optional_input_path((context_text), \
                                                           (path_value), \
                                                           profile_output_path, \
                                                           (input_text)); \
        } \
        reject_path_equal_to_named_optional_input_path((context_text), \
                                                       (path_value), \
                                                       explanation_output_path, \
                                                       (input_text)); \
        reject_path_equal_to_named_optional_input_path((context_text), \
                                                       (path_value), \
                                                       target_status_output_path, \
                                                       (input_text)); \
    } while (0)

    reject_path_equal_to_input_path("metrics output path",
                                    lab_file_path,
                                    preference_file_path,
                                    metrics_output_path);
    reject_path_equal_to_optional_input_path("metrics output path",
                                             rank_costs_path,
                                             metrics_output_path);
    reject_path_equal_to_input_path("lab report output path",
                                    lab_file_path,
                                    preference_file_path,
                                    lab_report_output_path);
    reject_path_equal_to_optional_input_path("lab report output path",
                                             rank_costs_path,
                                             lab_report_output_path);
    reject_path_equal_to_input_path("student report output path",
                                    lab_file_path,
                                    preference_file_path,
                                    student_report_output_path);
    reject_path_equal_to_optional_input_path("student report output path",
                                             rank_costs_path,
                                             student_report_output_path);
    reject_path_equal_to_input_path("outside report output path",
                                    lab_file_path,
                                    preference_file_path,
                                    outside_report_output_path);
    reject_path_equal_to_optional_input_path("outside report output path",
                                             rank_costs_path,
                                             outside_report_output_path);
    reject_path_equal_to_input_path("reasons report output path",
                                    lab_file_path,
                                    preference_file_path,
                                    reasons_report_output_path);
    reject_path_equal_to_optional_input_path("reasons report output path",
                                             rank_costs_path,
                                             reasons_report_output_path);
    reject_path_equal_to_input_path("adjustment report output path",
                                    lab_file_path,
                                    preference_file_path,
                                    adjustment_report_output_path);
    reject_path_equal_to_optional_input_path("adjustment report output path",
                                             rank_costs_path,
                                             adjustment_report_output_path);
    reject_path_equal_to_input_path("portfolio report output path",
                                    lab_file_path,
                                    preference_file_path,
                                    portfolio_report_output_path);
    reject_path_equal_to_optional_input_path("portfolio report output path",
                                             rank_costs_path,
                                             portfolio_report_output_path);
    if (include_profile_path) {
        reject_path_equal_to_input_path("profile output path",
                                        lab_file_path,
                                        preference_file_path,
                                        profile_output_path);
        reject_path_equal_to_optional_input_path("profile output path",
                                                 rank_costs_path,
                                                 profile_output_path);
    }
    reject_path_equal_to_input_path("explanation output path",
                                    lab_file_path,
                                    preference_file_path,
                                    explanation_output_path);
    reject_path_equal_to_optional_input_path("explanation output path",
                                             rank_costs_path,
                                             explanation_output_path);
    reject_path_equal_to_input_path("target status output path",
                                    lab_file_path,
                                    preference_file_path,
                                    target_status_output_path);
    reject_path_equal_to_optional_input_path("target status output path",
                                             rank_costs_path,
                                             target_status_output_path);
    REJECT_OPTIONAL_REPORT_INPUT("report output path",
                                 weights_path,
                                 "weights input file path");
    REJECT_OPTIONAL_REPORT_INPUT("report output path",
                                 targets_path,
                                 "targets input file path");
    REJECT_OPTIONAL_REPORT_INPUT("report output path",
                                 constraints_path,
                                 "constraints input file path");
    REJECT_OPTIONAL_REPORT_INPUT("report output path",
                                 base_assignment_path,
                                 "base assignment input file path");
#undef REJECT_OPTIONAL_REPORT_INPUT

    free(metrics_output_path);
    free(lab_report_output_path);
    free(student_report_output_path);
    free(outside_report_output_path);
    free(reasons_report_output_path);
    free(adjustment_report_output_path);
    free(portfolio_report_output_path);
    free(profile_output_path);
    free(explanation_output_path);
    free(target_status_output_path);
}

static void reject_portfolio_assignment_paths_equal_to_input_paths(
    const char *lab_file_path,
    const char *preference_file_path,
    const char *rank_costs_path,
    const char *weights_path,
    const char *targets_path,
    const char *constraints_path,
    const char *base_assignment_path,
    const char *output_file_path,
    int portfolio_mode)
{
    const char *candidate_names[] = {
        "rubric",
        "satisfaction",
        "fair",
        "balanced",
        "guarded",
        "fill_focused",
        "fill_convex",
        "weighted_exact"
    };
    int candidate_count =
        portfolio_mode >= 2 ? MAX_PORTFOLIO_CANDIDATES : LIGHT_PORTFOLIO_CANDIDATE_COUNT;
    int candidate_index;
    for (candidate_index = 0;
         candidate_index < candidate_count;
         candidate_index++) {
        char *candidate_path =
            portfolio_assignment_output_path_for(output_file_path,
                                                 candidate_names[candidate_index]);
        reject_path_equal_to_input_path("portfolio assignment output path",
                                        lab_file_path,
                                        preference_file_path,
                                        candidate_path);
        reject_path_equal_to_optional_input_path("portfolio assignment output path",
                                                 rank_costs_path,
                                                 candidate_path);
        reject_path_equal_to_named_optional_input_path(
            "portfolio assignment output path",
            weights_path,
            candidate_path,
            "weights input file path");
        reject_path_equal_to_named_optional_input_path(
            "portfolio assignment output path",
            targets_path,
            candidate_path,
            "targets input file path");
        reject_path_equal_to_named_optional_input_path(
            "portfolio assignment output path",
            constraints_path,
            candidate_path,
            "constraints input file path");
        reject_path_equal_to_named_optional_input_path(
            "portfolio assignment output path",
            base_assignment_path,
            candidate_path,
            "base assignment input file path");
        free(candidate_path);
    }
}

static int assignment_is_outside_preferences(const ProblemData *problem_data,
                                             int student_index,
                                             int lab_index)
{
    return rank_for_assignment(problem_data, student_index, lab_index) >
           problem_data->max_preferences;
}

static int outside_preference_count_for_assignment(const ProblemData *problem_data,
                                                   const int *assignment)
{
    int student_index;
    int outside_count = 0;
    for (student_index = 0; student_index < problem_data->student_count; student_index++) {
        if (assignment_is_outside_preferences(problem_data,
                                              student_index,
                                              assignment[student_index])) {
            outside_count++;
        }
    }
    return outside_count;
}

static int labs_at_minimum_fill_rate_count(const ProblemData *problem_data,
                                           const EvaluationMetrics *metrics)
{
    int lab_index;
    int lab_count_at_minimum = 0;
    const long double tolerance = 0.000000000001L;

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        long long capacity_value = problem_data->labs[lab_index].capacity_value;
        if (capacity_value > 0LL) {
            long double fill_rate =
                (long double)metrics->lab_counts[lab_index] /
                (long double)capacity_value;
            long double difference = fill_rate - metrics->minimum_fill_rate;
            if (difference < 0.0L) {
                difference = -difference;
            }
            if (difference <= tolerance) {
                lab_count_at_minimum++;
            }
        }
    }
    return lab_count_at_minimum;
}

static int target_constraints_count(const TargetConstraints *targets)
{
    int count = 0;
    if (targets->has_average_rank_max) {
        count++;
    }
    if (targets->has_average_fill_min) {
        count++;
    }
    if (targets->has_rank_sum_max) {
        count++;
    }
    if (targets->has_rank_square_max) {
        count++;
    }
    if (targets->has_max_rank_max) {
        count++;
    }
    if (targets->has_minimum_fill_min) {
        count++;
    }
    if (targets->has_outside_max) {
        count++;
    }
    return count;
}

static int target_minimum_fill_satisfied(const ProblemData *problem_data,
                                         const EvaluationMetrics *metrics)
{
    const TargetConstraints *targets = problem_data->targets;
    int lab_index;
    if (targets == NULL || !targets->has_minimum_fill_min) {
        return 1;
    }
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        int required_count = target_minimum_count_for_lab(problem_data, lab_index);
        if (metrics->lab_counts[lab_index] < required_count) {
            return 0;
        }
    }
    return 1;
}

static int required_targets_are_satisfied(const ProblemData *problem_data,
                                          const int *assignment,
                                          const EvaluationMetrics *metrics)
{
    const TargetConstraints *targets = problem_data->targets;
    if (targets == NULL || target_constraints_are_empty(targets)) {
        return 1;
    }
    if (!rank_sum_satisfies_targets(problem_data, metrics->rank_sum)) {
        return 0;
    }
    if (targets->has_average_fill_min &&
        !average_fill_counts_satisfy_bound(problem_data,
                                           metrics->lab_counts,
                                           targets->average_fill_min)) {
        return 0;
    }
    if (targets->has_rank_square_max &&
        metrics->rank_square_sum > targets->rank_square_max) {
        return 0;
    }
    if (targets->has_max_rank_max && metrics->max_rank > targets->max_rank_max) {
        return 0;
    }
    if (!target_minimum_fill_satisfied(problem_data, metrics)) {
        return 0;
    }
    if (targets->has_outside_max &&
        outside_preference_count_for_assignment(problem_data, assignment) >
            targets->outside_max) {
        return 0;
    }
    return 1;
}

static int target_constraints_pass_count(const ProblemData *problem_data,
                                         const int *assignment,
                                         const EvaluationMetrics *metrics)
{
    const TargetConstraints *targets = problem_data->targets;
    int pass_count = 0;
    long long rank_sum_limit = 0LL;
    if (targets == NULL) {
        return 0;
    }
    if (targets->has_average_rank_max &&
        target_average_rank_limit(problem_data, &rank_sum_limit) &&
        metrics->rank_sum <= rank_sum_limit) {
        pass_count++;
    }
    if (targets->has_average_fill_min &&
        average_fill_counts_satisfy_bound(problem_data,
                                          metrics->lab_counts,
                                          targets->average_fill_min)) {
        pass_count++;
    }
    if (targets->has_rank_sum_max && metrics->rank_sum <= targets->rank_sum_max) {
        pass_count++;
    }
    if (targets->has_rank_square_max &&
        metrics->rank_square_sum <= targets->rank_square_max) {
        pass_count++;
    }
    if (targets->has_max_rank_max && metrics->max_rank <= targets->max_rank_max) {
        pass_count++;
    }
    if (targets->has_minimum_fill_min &&
        target_minimum_fill_satisfied(problem_data, metrics)) {
        pass_count++;
    }
    if (targets->has_outside_max &&
        outside_preference_count_for_assignment(problem_data, assignment) <=
            targets->outside_max) {
        pass_count++;
    }
    return pass_count;
}

static void write_target_status_row(FILE *target_file,
                                    const char *target_name,
                                    const char *operator_text,
                                    const char *required_text,
                                    const char *actual_text,
                                    int passed,
                                    long double margin,
                                    const char *context)
{
    if (fprintf(target_file,
                "%s\t%s\t%s\t%s\t%s\t%.17Lg\n",
                target_name,
                operator_text,
                required_text,
                actual_text,
                passed ? "pass" : "fail",
                margin) < 0) {
        fail_with_context(context, "write failed");
    }
}

static void write_target_status_file(const char *target_status_path,
                                     const ProblemData *problem_data,
                                     const int *assignment,
                                     const EvaluationMetrics *metrics)
{
    const TargetConstraints *targets = problem_data->targets;
    char *temporary_path;
    FILE *target_file;
    int outside_count;
    if (targets == NULL || target_constraints_are_empty(targets)) {
        return;
    }

    temporary_path = temporary_output_path_for(target_status_path);
    cleanup_target_status_output_path = temporary_path;
    target_file = fopen(temporary_path, "w");
    if (target_file == NULL) {
        fail_with_context(target_status_path,
                          "cannot open temporary target status output file");
    }
    if (fprintf(target_file,
                "target\toperator\trequired\tactual\tstatus\tmargin\n") < 0) {
        fclose(target_file);
        fail_with_context(target_status_path, "write failed");
    }

    outside_count = outside_preference_count_for_assignment(problem_data, assignment);

    if (targets->has_average_rank_max) {
        char required_text[64];
        char actual_text[64];
        long long rank_sum_limit;
        int passed;
        snprintf(required_text,
                 sizeof(required_text),
                 "%.17Lg",
                 (long double)targets->average_rank_max.numerator /
                     (long double)targets->average_rank_max.denominator);
        snprintf(actual_text, sizeof(actual_text), "%.17Lg", metrics->average_rank);
        (void)target_average_rank_limit(problem_data, &rank_sum_limit);
        passed = metrics->rank_sum <= rank_sum_limit;
        write_target_status_row(target_file,
                                "average_rank",
                                "<=",
                                required_text,
                                actual_text,
                                passed,
                                ((long double)rank_sum_limit -
                                 (long double)metrics->rank_sum) /
                                    (long double)problem_data->student_count,
                                target_status_path);
    }
    if (targets->has_rank_sum_max) {
        char required_text[64];
        char actual_text[64];
        snprintf(required_text, sizeof(required_text), "%lld", targets->rank_sum_max);
        snprintf(actual_text, sizeof(actual_text), "%lld", metrics->rank_sum);
        write_target_status_row(target_file,
                                "rank_sum",
                                "<=",
                                required_text,
                                actual_text,
                                metrics->rank_sum <= targets->rank_sum_max,
                                (long double)targets->rank_sum_max -
                                    (long double)metrics->rank_sum,
                                target_status_path);
    }
    if (targets->has_average_fill_min) {
        char required_text[64];
        char actual_text[64];
        long double required_value =
            (long double)targets->average_fill_min.numerator /
            (long double)targets->average_fill_min.denominator;
        int passed =
            average_fill_counts_satisfy_bound(problem_data,
                                              metrics->lab_counts,
                                              targets->average_fill_min);
        snprintf(required_text, sizeof(required_text), "%.17Lg", required_value);
        snprintf(actual_text, sizeof(actual_text), "%.17Lg", metrics->average_fill_rate);
        write_target_status_row(target_file,
                                "average_fill_rate",
                                ">=",
                                required_text,
                                actual_text,
                                passed,
                                metrics->average_fill_rate - required_value,
                                target_status_path);
    }
    if (targets->has_rank_square_max) {
        char required_text[64];
        char actual_text[64];
        snprintf(required_text,
                 sizeof(required_text),
                 "%lld",
                 targets->rank_square_max);
        snprintf(actual_text, sizeof(actual_text), "%lld", metrics->rank_square_sum);
        write_target_status_row(target_file,
                                "rank_square_sum",
                                "<=",
                                required_text,
                                actual_text,
                                metrics->rank_square_sum <= targets->rank_square_max,
                                (long double)targets->rank_square_max -
                                    (long double)metrics->rank_square_sum,
                                target_status_path);
    }
    if (targets->has_max_rank_max) {
        char required_text[64];
        char actual_text[64];
        snprintf(required_text, sizeof(required_text), "%d", targets->max_rank_max);
        snprintf(actual_text, sizeof(actual_text), "%d", metrics->max_rank);
        write_target_status_row(target_file,
                                "max_rank",
                                "<=",
                                required_text,
                                actual_text,
                                metrics->max_rank <= targets->max_rank_max,
                                (long double)targets->max_rank_max -
                                    (long double)metrics->max_rank,
                                target_status_path);
    }
    if (targets->has_minimum_fill_min) {
        char required_text[64];
        char actual_text[64];
        snprintf(required_text,
                 sizeof(required_text),
                 "%.17Lg",
                 (long double)targets->minimum_fill_min.numerator /
                     (long double)targets->minimum_fill_min.denominator);
        snprintf(actual_text,
                 sizeof(actual_text),
                 "%.17Lg",
                 metrics->minimum_fill_rate);
        write_target_status_row(target_file,
                                "minimum_fill_rate",
                                ">=",
                                required_text,
                                actual_text,
                                target_minimum_fill_satisfied(problem_data, metrics),
                                metrics->minimum_fill_rate -
                                    ((long double)targets->minimum_fill_min.numerator /
                                     (long double)targets->minimum_fill_min.denominator),
                                target_status_path);
    }
    if (targets->has_outside_max) {
        char required_text[64];
        char actual_text[64];
        snprintf(required_text, sizeof(required_text), "%d", targets->outside_max);
        snprintf(actual_text, sizeof(actual_text), "%d", outside_count);
        write_target_status_row(target_file,
                                "outside_preference_count",
                                "<=",
                                required_text,
                                actual_text,
                                outside_count <= targets->outside_max,
                                (long double)targets->outside_max -
                                    (long double)outside_count,
                                target_status_path);
    }

    if (fclose(target_file) != 0) {
        fail_with_context(target_status_path, "close failed");
    }
    replace_output_file(temporary_path, target_status_path, target_status_path);
    cleanup_target_status_output_path = NULL;
    free(temporary_path);
}

static void write_assignment_file(const char *output_file_path,
                                  const ProblemData *problem_data,
                                  const int *assignment)
{
    char *temporary_path = temporary_output_path_for(output_file_path);
    FILE *output_file;
    int student_index;

    cleanup_output_path = temporary_path;
    output_file = fopen(temporary_path, "w");
    if (output_file == NULL) {
        fail_with_context(output_file_path, "cannot open temporary output file");
    }

    if (fprintf(output_file, "%d\n", problem_data->student_count) < 0) {
        fclose(output_file);
        fail_with_context(output_file_path, "write failed");
    }

    for (student_index = 0; student_index < problem_data->student_count; student_index++) {
        int lab_index = assignment[student_index];
        if (fprintf(output_file,
                    "%s %s\n",
                    problem_data->students[student_index].student_id,
                    problem_data->labs[lab_index].name) < 0) {
            fclose(output_file);
            fail_with_context(output_file_path, "write failed");
        }
    }

    if (fclose(output_file) != 0) {
        fail_with_context(output_file_path, "close failed");
    }
    replace_output_file(temporary_path, output_file_path, output_file_path);
    cleanup_output_path = NULL;
    free(temporary_path);
}

static void write_metrics_file(const char *metrics_path,
                               const ProblemData *problem_data,
                               const int *assignment,
                               const EvaluationMetrics *metrics)
{
    char *temporary_path = temporary_output_path_for(metrics_path);
    FILE *metrics_file;
    int lab_index;
    int student_index;
    int outside_count =
        outside_preference_count_for_assignment(problem_data, assignment);
    int labs_at_minimum =
        labs_at_minimum_fill_rate_count(problem_data, metrics);
    int target_count =
        problem_data->targets == NULL ?
        0 :
        target_constraints_count(problem_data->targets);
    int target_pass_count =
        problem_data->targets == NULL ?
        0 :
        target_constraints_pass_count(problem_data, assignment, metrics);
    int reason_first_choice_selected = 0;
    int reason_first_choice_full = 0;
    int reason_minimum_occupancy = 0;
    int reason_manual_lock = 0;
    int reason_constraint = 0;
    int reason_kept_base = 0;
    int reason_global_tradeoff = 0;

    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        const char *reason =
            reason_for_assignment(problem_data, assignment, metrics, student_index);
        if (strcmp(reason, "first choice selected") == 0) {
            reason_first_choice_selected++;
        } else if (strcmp(reason, "first choice lab was full") == 0) {
            reason_first_choice_full++;
        } else if (strcmp(reason,
                          "moving to first choice would violate minimum occupancy") == 0) {
            reason_minimum_occupancy++;
        } else if (strcmp(reason, "manual lock") == 0) {
            reason_manual_lock++;
        } else if (strcmp(reason, "forbid/allow constraint") == 0) {
            reason_constraint++;
        } else if (strcmp(reason, "kept base assignment by change penalty") == 0) {
            reason_kept_base++;
        } else {
            reason_global_tradeoff++;
        }
    }

    cleanup_metrics_output_path = temporary_path;
    metrics_file = fopen(temporary_path, "w");
    if (metrics_file == NULL) {
        fail_with_context(metrics_path, "cannot open temporary metrics output file");
    }

    if (fprintf(metrics_file, "students %d\n", problem_data->student_count) < 0 ||
        fprintf(metrics_file, "labs %d\n", problem_data->lab_count) < 0 ||
        fprintf(metrics_file, "average_rank %.17Lg\n", metrics->average_rank) < 0 ||
        fprintf(metrics_file, "rank_average %.17Lg\n", metrics->average_rank) < 0 ||
        fprintf(metrics_file, "rank_stddev %.17Lg\n", metrics->rank_stddev) < 0 ||
        fprintf(metrics_file, "rank_max %d\n", metrics->max_rank) < 0 ||
        fprintf(metrics_file, "rank_sum %lld\n", metrics->rank_sum) < 0 ||
        fprintf(metrics_file, "rank_square_sum %lld\n", metrics->rank_square_sum) < 0 ||
        fprintf(metrics_file,
                "average_dissatisfaction %.17Lg\n",
                metrics->average_dissatisfaction) < 0 ||
        fprintf(metrics_file,
                "dissatisfaction_stddev %.17Lg\n",
                metrics->dissatisfaction_stddev) < 0 ||
        fprintf(metrics_file,
                "max_dissatisfaction %lld\n",
                metrics->max_dissatisfaction) < 0 ||
        fprintf(metrics_file,
                "dissatisfaction_sum %lld\n",
                metrics->dissatisfaction_sum) < 0 ||
        fprintf(metrics_file,
                "dissatisfaction_square_sum %lld\n",
                metrics->dissatisfaction_square_sum) < 0 ||
        fprintf(metrics_file, "average_fill_rate %.17Lg\n", metrics->average_fill_rate) < 0 ||
        fprintf(metrics_file, "fill_rate_average %.17Lg\n", metrics->average_fill_rate) < 0 ||
        fprintf(metrics_file, "minimum_fill_rate %.17Lg\n", metrics->minimum_fill_rate) < 0 ||
        fprintf(metrics_file, "fill_rate_minimum %.17Lg\n", metrics->minimum_fill_rate) < 0 ||
        fprintf(metrics_file, "outside_preference_count %d\n", outside_count) < 0 ||
        fprintf(metrics_file, "labs_at_minimum_fill_rate %d\n", labs_at_minimum) < 0 ||
        fprintf(metrics_file, "target_count %d\n", target_count) < 0 ||
        fprintf(metrics_file, "target_pass_count %d\n", target_pass_count) < 0 ||
        fprintf(metrics_file,
                "target_all_passed %d\n",
                target_count == target_pass_count ? 1 : 0) < 0 ||
        fprintf(metrics_file,
                "reason_first_choice_selected %d\n",
                reason_first_choice_selected) < 0 ||
        fprintf(metrics_file,
                "reason_first_choice_lab_was_full %d\n",
                reason_first_choice_full) < 0 ||
        fprintf(metrics_file,
                "reason_minimum_occupancy %d\n",
                reason_minimum_occupancy) < 0 ||
        fprintf(metrics_file, "reason_manual_lock %d\n", reason_manual_lock) < 0 ||
        fprintf(metrics_file, "reason_constraint %d\n", reason_constraint) < 0 ||
        fprintf(metrics_file,
                "reason_kept_base_assignment %d\n",
                reason_kept_base) < 0 ||
        fprintf(metrics_file,
                "reason_global_optimum_tradeoff %d\n",
                reason_global_tradeoff) < 0 ||
        fprintf(metrics_file, "solver_cpu_seconds %.17Lg\n", metrics->solver_cpu_seconds) < 0 ||
        fprintf(metrics_file,
                "selected_solver_cpu_seconds %.17Lg\n",
                metrics->solver_cpu_seconds) < 0 ||
        fprintf(metrics_file,
                "counterfactual_cpu_seconds %.17Lg\n",
                metrics->counterfactual_cpu_seconds) < 0 ||
        fprintf(metrics_file,
                "program_cpu_seconds_before_reports %.17Lg\n",
                metrics->program_cpu_seconds_before_metrics) < 0 ||
        fprintf(metrics_file,
                "program_cpu_seconds_before_metrics %.17Lg\n",
                metrics->program_cpu_seconds_before_metrics) < 0 ||
        fprintf(metrics_file, "lab_counts\n") < 0) {
        fclose(metrics_file);
        fail_with_context(metrics_path, "write failed");
    }

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        long long capacity_value = problem_data->labs[lab_index].capacity_value;
        char fill_rate_buffer[64];
        const char *fill_rate_text = "NA";
        if (capacity_value > 0LL) {
            long double fill_rate =
                (long double)metrics->lab_counts[lab_index] /
                (long double)capacity_value;
            snprintf(fill_rate_buffer, sizeof(fill_rate_buffer), "%.17Lg", fill_rate);
            fill_rate_text = fill_rate_buffer;
        }
        if (fprintf(metrics_file,
                    "%s %d %lld %s\n",
                    problem_data->labs[lab_index].name,
                    metrics->lab_counts[lab_index],
                    capacity_value,
                    fill_rate_text) < 0) {
            fclose(metrics_file);
            fail_with_context(metrics_path, "write failed");
        }
    }

    if (fclose(metrics_file) != 0) {
        fail_with_context(metrics_path, "close failed");
    }
    replace_output_file(temporary_path, metrics_path, metrics_path);
    cleanup_metrics_output_path = NULL;
    free(temporary_path);
}

static void write_lab_report_file(const char *report_path,
                                  const ProblemData *problem_data,
                                  const int *assignment,
                                  const EvaluationMetrics *metrics)
{
    char *temporary_path = temporary_output_path_for(report_path);
    FILE *report_file;
    int lab_index;
    int student_index;
    int outside_count =
        outside_preference_count_for_assignment(problem_data, assignment);
    int labs_at_minimum =
        labs_at_minimum_fill_rate_count(problem_data, metrics);
    int *head_by_lab = checked_malloc((size_t)problem_data->lab_count * sizeof(int));
    int *tail_by_lab = checked_malloc((size_t)problem_data->lab_count * sizeof(int));
    int *next_student = checked_malloc((size_t)problem_data->student_count * sizeof(int));

    cleanup_lab_report_output_path = temporary_path;
    report_file = fopen(temporary_path, "w");
    if (report_file == NULL) {
        fail_with_context(report_path, "cannot open temporary lab report output file");
    }

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        head_by_lab[lab_index] = -1;
        tail_by_lab[lab_index] = -1;
    }
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        int assigned_lab = assignment[student_index];
        next_student[student_index] = -1;
        if (tail_by_lab[assigned_lab] < 0) {
            head_by_lab[assigned_lab] = student_index;
        } else {
            next_student[tail_by_lab[assigned_lab]] = student_index;
        }
        tail_by_lab[assigned_lab] = student_index;
    }

    if (fprintf(report_file, "students %d\n", problem_data->student_count) < 0 ||
        fprintf(report_file, "labs %d\n", problem_data->lab_count) < 0 ||
        fprintf(report_file, "average_rank %.17Lg\n", metrics->average_rank) < 0 ||
        fprintf(report_file, "rank_stddev %.17Lg\n", metrics->rank_stddev) < 0 ||
        fprintf(report_file, "rank_max %d\n", metrics->max_rank) < 0 ||
        fprintf(report_file, "minimum_fill_rate %.17Lg\n", metrics->minimum_fill_rate) < 0 ||
        fprintf(report_file, "average_fill_rate %.17Lg\n", metrics->average_fill_rate) < 0 ||
        fprintf(report_file, "outside_preference_count %d\n", outside_count) < 0 ||
        fprintf(report_file, "labs_at_minimum_fill_rate %d\n", labs_at_minimum) < 0 ||
        fprintf(report_file, "solver_cpu_seconds %.17Lg\n", metrics->solver_cpu_seconds) < 0 ||
        fprintf(report_file,
                "counterfactual_cpu_seconds %.17Lg\n",
                metrics->counterfactual_cpu_seconds) < 0 ||
        fprintf(report_file,
                "program_cpu_seconds_before_reports %.17Lg\n",
                metrics->program_cpu_seconds_before_metrics) < 0 ||
        fprintf(report_file,
                "lab_name\tassigned\tcapacity\tfill_rate\tfill_percent\tstudents\n") < 0) {
        fclose(report_file);
        fail_with_context(report_path, "write failed");
    }

    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        long long capacity_value = problem_data->labs[lab_index].capacity_value;
        long double fill_rate = 0.0L;
        student_index = head_by_lab[lab_index];
        const char *fill_rate_text = "NA";
        char fill_rate_buffer[64];
        char percent_buffer[64];
        int wrote_student = 0;
        if (capacity_value > 0LL) {
            fill_rate =
                (long double)metrics->lab_counts[lab_index] / (long double)capacity_value;
            snprintf(fill_rate_buffer, sizeof(fill_rate_buffer), "%.17Lg", fill_rate);
            snprintf(percent_buffer, sizeof(percent_buffer), "%.6Lf", fill_rate * 100.0L);
            fill_rate_text = fill_rate_buffer;
        } else {
            snprintf(percent_buffer, sizeof(percent_buffer), "NA");
        }
        if (fprintf(report_file,
                    "%s\t%d\t%lld\t%s\t%s\t",
                    problem_data->labs[lab_index].name,
                    metrics->lab_counts[lab_index],
                    capacity_value,
                    fill_rate_text,
                    percent_buffer) < 0) {
            fclose(report_file);
            fail_with_context(report_path, "write failed");
        }
        while (student_index >= 0) {
            int rank_value = rank_for_assignment(problem_data, student_index, lab_index);
            const char *outside_text =
                assignment_is_outside_preferences(problem_data, student_index, lab_index) ?
                ";outside" :
                "";
            if (wrote_student &&
                fprintf(report_file, ",") < 0) {
                fclose(report_file);
                fail_with_context(report_path, "write failed");
            }
            if (fprintf(report_file,
                        "%s(rank=%d%s)",
                        problem_data->students[student_index].student_id,
                        rank_value,
                        outside_text) < 0) {
                fclose(report_file);
                fail_with_context(report_path, "write failed");
            }
            wrote_student = 1;
            student_index = next_student[student_index];
        }
        if (fprintf(report_file, "\n") < 0) {
            fclose(report_file);
            fail_with_context(report_path, "write failed");
        }
    }

    if (fclose(report_file) != 0) {
        fail_with_context(report_path, "close failed");
    }
    replace_output_file(temporary_path, report_path, report_path);
    cleanup_lab_report_output_path = NULL;
    free(temporary_path);
    free(head_by_lab);
    free(tail_by_lab);
    free(next_student);
}

static void write_submitted_preferences(FILE *output_file,
                                        const char *output_path,
                                        const ProblemData *problem_data,
                                        int student_index)
{
    int rank_value;
    int wrote_lab = 0;

    for (rank_value = 1;
         rank_value <= problem_data->max_preferences;
         rank_value++) {
        int lab_index;
        for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
            if (rank_for_assignment(problem_data, student_index, lab_index) == rank_value) {
                if (wrote_lab && fprintf(output_file, ",") < 0) {
                    fail_with_context(output_path, "write failed");
                }
                if (fprintf(output_file, "%s", problem_data->labs[lab_index].name) < 0) {
                    fail_with_context(output_path, "write failed");
                }
                wrote_lab = 1;
                break;
            }
        }
    }
}

static void write_student_report_file(const char *report_path,
                                      const ProblemData *problem_data,
                                      const int *assignment)
{
    char *temporary_path = temporary_output_path_for(report_path);
    FILE *report_file;
    int student_index;

    cleanup_student_report_output_path = temporary_path;
    report_file = fopen(temporary_path, "w");
    if (report_file == NULL) {
        fail_with_context(report_path, "cannot open temporary student report output file");
    }

    if (fprintf(report_file,
                "student_id\tassigned_lab\tassigned_rank\tdissatisfaction\toutside_preference\n") < 0) {
        fclose(report_file);
        fail_with_context(report_path, "write failed");
    }

    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        int lab_index = assignment[student_index];
        int rank_value = rank_for_assignment(problem_data, student_index, lab_index);
        long long dissatisfaction =
            dissatisfaction_cost_for_rank(problem_data,
                                          problem_data->rank_cost_model,
                                          rank_value);
        const char *outside_text =
            assignment_is_outside_preferences(problem_data, student_index, lab_index) ?
            "yes" :
            "no";
        if (fprintf(report_file,
                    "%s\t%s\t%d\t%lld\t%s\n",
                    problem_data->students[student_index].student_id,
                    problem_data->labs[lab_index].name,
                    rank_value,
                    dissatisfaction,
                    outside_text) < 0) {
            fclose(report_file);
            fail_with_context(report_path, "write failed");
        }
    }

    if (fclose(report_file) != 0) {
        fail_with_context(report_path, "close failed");
    }
    replace_output_file(temporary_path, report_path, report_path);
    cleanup_student_report_output_path = NULL;
    free(temporary_path);
}

static void write_outside_preferences_report_file(const char *report_path,
                                                  const ProblemData *problem_data,
                                                  const int *assignment)
{
    char *temporary_path = temporary_output_path_for(report_path);
    FILE *report_file;
    int student_index;

    cleanup_outside_report_output_path = temporary_path;
    report_file = fopen(temporary_path, "w");
    if (report_file == NULL) {
        fail_with_context(report_path,
                          "cannot open temporary outside-preference report output file");
    }

    if (fprintf(report_file,
                "student_id\tassigned_lab\tassigned_rank\tsubmitted_preferences\n") < 0) {
        fclose(report_file);
        fail_with_context(report_path, "write failed");
    }

    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        int lab_index = assignment[student_index];
        int rank_value = rank_for_assignment(problem_data, student_index, lab_index);
        if (!assignment_is_outside_preferences(problem_data, student_index, lab_index)) {
            continue;
        }
        if (fprintf(report_file,
                    "%s\t%s\t%d\t",
                    problem_data->students[student_index].student_id,
                    problem_data->labs[lab_index].name,
                    rank_value) < 0) {
            fclose(report_file);
            fail_with_context(report_path, "write failed");
        }
        write_submitted_preferences(report_file,
                                    report_path,
                                    problem_data,
                                    student_index);
        if (fprintf(report_file, "\n") < 0) {
            fclose(report_file);
            fail_with_context(report_path, "write failed");
        }
    }

    if (fclose(report_file) != 0) {
        fail_with_context(report_path, "close failed");
    }
    replace_output_file(temporary_path, report_path, report_path);
    cleanup_outside_report_output_path = NULL;
    free(temporary_path);
}

static int first_choice_lab_for_student(const ProblemData *problem_data,
                                        int student_index)
{
    int lab_index;
    for (lab_index = 0; lab_index < problem_data->lab_count; lab_index++) {
        if (rank_for_assignment(problem_data, student_index, lab_index) == 1) {
            return lab_index;
        }
    }
    return -1;
}

static const char *reason_for_assignment(const ProblemData *problem_data,
                                         const int *assignment,
                                         const EvaluationMetrics *metrics,
                                         int student_index)
{
    int assigned_lab = assignment[student_index];
    int assigned_rank = rank_for_assignment(problem_data, student_index, assigned_lab);
    int first_choice_lab = first_choice_lab_for_student(problem_data, student_index);
    const ConstraintSet *constraints = problem_data->constraints;

    if (constraints != NULL &&
        constraints->locked_lab_by_student[student_index] >= 0) {
        return "manual lock";
    }
    if (assigned_rank == 1) {
        return "first choice selected";
    }
    if (constraints != NULL && first_choice_lab >= 0) {
        size_t first_choice_index =
            student_lab_matrix_index(problem_data, student_index, first_choice_lab);
        if (constraints->forbidden_matrix[first_choice_index] ||
            (constraints->has_allowed_set[student_index] &&
             !constraints->allowed_matrix[first_choice_index])) {
            return "forbid/allow constraint";
        }
    }
    if (first_choice_lab >= 0 &&
        metrics->lab_counts[first_choice_lab] >=
            problem_data->labs[first_choice_lab].graph_capacity) {
        return "first choice lab was full";
    }
    if (problem_data->labs[assigned_lab].capacity_value > 0LL &&
        metrics->lab_counts[assigned_lab] <= 1) {
        return "moving to first choice would violate minimum occupancy";
    }
    if (problem_data->base_assignment != NULL &&
        assigned_lab == problem_data->base_assignment[student_index] &&
        problem_data->change_penalty > 0LL) {
        return "kept base assignment by change penalty";
    }
    return "global optimum trade-off";
}

static void write_reasons_report_file(const char *report_path,
                                      const ProblemData *problem_data,
                                      const int *assignment,
                                      const EvaluationMetrics *metrics)
{
    char *temporary_path = temporary_output_path_for(report_path);
    FILE *report_file;
    int student_index;
    int *first_choice_demands =
        checked_calloc((size_t)problem_data->lab_count, sizeof(int));

    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        int first_choice_lab = first_choice_lab_for_student(problem_data, student_index);
        if (first_choice_lab >= 0) {
            first_choice_demands[first_choice_lab]++;
        }
    }

    cleanup_reasons_report_output_path = temporary_path;
    report_file = fopen(temporary_path, "w");
    if (report_file == NULL) {
        fail_with_context(report_path, "cannot open temporary reasons report output file");
    }

    if (fprintf(report_file,
                "student_id\tassigned_lab\tassigned_rank\tfirst_choice\tfirst_choice_demand\tfirst_choice_capacity\tfirst_choice_assigned\treason\n") < 0) {
        fclose(report_file);
        free(first_choice_demands);
        fail_with_context(report_path, "write failed");
    }
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        int assigned_lab = assignment[student_index];
        int first_choice_lab = first_choice_lab_for_student(problem_data, student_index);
        const char *first_choice_text =
            first_choice_lab >= 0 ? problem_data->labs[first_choice_lab].name : "NA";
        int first_choice_demand =
            first_choice_lab >= 0 ? first_choice_demands[first_choice_lab] : 0;
        long long first_choice_capacity =
            first_choice_lab >= 0 ? problem_data->labs[first_choice_lab].capacity_value : 0LL;
        int first_choice_assigned =
            first_choice_lab >= 0 ? metrics->lab_counts[first_choice_lab] : 0;
        if (fprintf(report_file,
                    "%s\t%s\t%d\t%s\t%d\t%lld\t%d\t%s\n",
                    problem_data->students[student_index].student_id,
                    problem_data->labs[assigned_lab].name,
                    rank_for_assignment(problem_data, student_index, assigned_lab),
                    first_choice_text,
                    first_choice_demand,
                    first_choice_capacity,
                    first_choice_assigned,
                    reason_for_assignment(problem_data,
                                          assignment,
                                          metrics,
                                          student_index)) < 0) {
            fclose(report_file);
            free(first_choice_demands);
            fail_with_context(report_path, "write failed");
        }
    }

    if (fclose(report_file) != 0) {
        free(first_choice_demands);
        fail_with_context(report_path, "close failed");
    }
    replace_output_file(temporary_path, report_path, report_path);
    cleanup_reasons_report_output_path = NULL;
    free(temporary_path);
    free(first_choice_demands);
}

static void write_adjustment_report_file(const char *report_path,
                                         const ProblemData *problem_data,
                                         const int *assignment,
                                         const EvaluationMetrics *metrics)
{
    char *temporary_path = temporary_output_path_for(report_path);
    FILE *report_file;
    EvaluationMetrics base_metrics =
        compute_evaluation_metrics(problem_data, problem_data->base_assignment);
    int student_index;
    int changed_students = 0;

    cleanup_adjustment_report_output_path = temporary_path;
    report_file = fopen(temporary_path, "w");
    if (report_file == NULL) {
        fail_with_context(report_path, "cannot open temporary adjustment report output file");
    }
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        if (assignment[student_index] != problem_data->base_assignment[student_index]) {
            changed_students++;
        }
    }
    if (fprintf(report_file, "metric\tbaseline\tadjusted\tdelta\n") < 0 ||
        fprintf(report_file,
                "average_rank\t%.17Lg\t%.17Lg\t%.17Lg\n",
                base_metrics.average_rank,
                metrics->average_rank,
                metrics->average_rank - base_metrics.average_rank) < 0 ||
        fprintf(report_file,
                "rank_stddev\t%.17Lg\t%.17Lg\t%.17Lg\n",
                base_metrics.rank_stddev,
                metrics->rank_stddev,
                metrics->rank_stddev - base_metrics.rank_stddev) < 0 ||
        fprintf(report_file,
                "max_rank\t%d\t%d\t%d\n",
                base_metrics.max_rank,
                metrics->max_rank,
                metrics->max_rank - base_metrics.max_rank) < 0 ||
        fprintf(report_file,
                "average_fill\t%.17Lg\t%.17Lg\t%.17Lg\n",
                base_metrics.average_fill_rate,
                metrics->average_fill_rate,
                metrics->average_fill_rate - base_metrics.average_fill_rate) < 0 ||
        fprintf(report_file,
                "minimum_fill\t%.17Lg\t%.17Lg\t%.17Lg\n",
                base_metrics.minimum_fill_rate,
                metrics->minimum_fill_rate,
                metrics->minimum_fill_rate - base_metrics.minimum_fill_rate) < 0 ||
        fprintf(report_file,
                "changed_students\t0\t%d\t%d\n",
                changed_students,
                changed_students) < 0 ||
        fprintf(report_file,
                "\nstudent_id\told_lab\tnew_lab\told_rank\tnew_rank\treason\n") < 0) {
        fclose(report_file);
        free_evaluation_metrics(&base_metrics);
        fail_with_context(report_path, "write failed");
    }
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        int old_lab = problem_data->base_assignment[student_index];
        int new_lab = assignment[student_index];
        if (old_lab == new_lab) {
            continue;
        }
        if (fprintf(report_file,
                    "%s\t%s\t%s\t%d\t%d\treoptimized after constraints/objective\n",
                    problem_data->students[student_index].student_id,
                    problem_data->labs[old_lab].name,
                    problem_data->labs[new_lab].name,
                    rank_for_assignment(problem_data, student_index, old_lab),
                    rank_for_assignment(problem_data, student_index, new_lab)) < 0) {
            fclose(report_file);
            free_evaluation_metrics(&base_metrics);
            fail_with_context(report_path, "write failed");
        }
    }

    if (fclose(report_file) != 0) {
        free_evaluation_metrics(&base_metrics);
        fail_with_context(report_path, "close failed");
    }
    replace_output_file(temporary_path, report_path, report_path);
    cleanup_adjustment_report_output_path = NULL;
    free(temporary_path);
    free_evaluation_metrics(&base_metrics);
}

static PortfolioScoreComponents portfolio_score_components(
    const ProblemData *problem_data,
    const EvaluationMetrics *metrics)
{
    PortfolioScoreComponents components;
    long double lab_count = (long double)problem_data->lab_count;
    components.average_rank_component =
        (metrics->average_rank - 1.0L) / lab_count;
    components.stddev_component = metrics->rank_stddev / lab_count;
    components.max_rank_component =
        ((long double)metrics->max_rank - 1.0L) / lab_count;
    components.average_fill_deficit = 1.0L - metrics->average_fill_rate;
    components.minimum_fill_deficit = 1.0L - metrics->minimum_fill_rate;
    components.total =
        components.average_rank_component +
        components.stddev_component +
        components.max_rank_component +
        components.average_fill_deficit +
        components.minimum_fill_deficit;
    return components;
}

static long double portfolio_recommendation_score(const ProblemData *problem_data,
                                                  const EvaluationMetrics *metrics)
{
    return portfolio_score_components(problem_data, metrics).total;
}

static void portfolio_candidate_init(PortfolioCandidate *candidate,
                                     const char *candidate_name,
                                     ObjectiveMode objective_mode,
                                     int max_rank_slack,
                                     const WeightedObjective *weights)
{
    snprintf(candidate->name,
             sizeof(candidate->name),
             "%s",
             candidate_name);
    candidate->objective_mode = objective_mode;
    candidate->max_rank_slack = max_rank_slack;
    candidate->weights = *weights;
    candidate->assignment = NULL;
    memset(&candidate->metrics, 0, sizeof(candidate->metrics));
    candidate->solver_cpu_seconds = 0.0L;
    candidate->recommendation_score = 0.0L;
    candidate->recommended = 0;
}

static const char *objective_mode_command_name(ObjectiveMode objective_mode)
{
    if (objective_mode == OBJECTIVE_RUBRIC) {
        return "rubric";
    }
    if (objective_mode == OBJECTIVE_SATISFACTION) {
        return "satisfaction";
    }
    if (objective_mode == OBJECTIVE_FAIR) {
        return "fair";
    }
    if (objective_mode == OBJECTIVE_BALANCED) {
        return "balanced";
    }
    if (objective_mode == OBJECTIVE_GUARDED) {
        return "guarded";
    }
    if (objective_mode == OBJECTIVE_FILL_CONVEX) {
        return "fill-convex";
    }
    if (objective_mode == OBJECTIVE_WEIGHTED_EXACT) {
        return "weighted-exact";
    }
    return "rubric";
}

static char *format_long_long_argument(long long value)
{
    char buffer[64];
    int length = snprintf(buffer, sizeof(buffer), "%lld", value);
    if (length < 0 || (size_t)length >= sizeof(buffer)) {
        fail_with_context("parallel portfolio", "integer formatting failed");
    }
    return duplicate_text(buffer);
}

static char *format_int_argument(int value)
{
    char buffer[64];
    int length = snprintf(buffer, sizeof(buffer), "%d", value);
    if (length < 0 || (size_t)length >= sizeof(buffer)) {
        fail_with_context("parallel portfolio", "integer formatting failed");
    }
    return duplicate_text(buffer);
}

static char *format_ratio_argument(RatioValue value)
{
    char buffer[128];
    int length = snprintf(buffer,
                          sizeof(buffer),
                          "%lld/%lld",
                          value.numerator,
                          value.denominator);
    if (length < 0 || (size_t)length >= sizeof(buffer)) {
        fail_with_context("parallel portfolio", "ratio formatting failed");
    }
    return duplicate_text(buffer);
}

static long double read_metric_long_double_or_zero(const char *metrics_path,
                                                   const char *metric_name)
{
    FastInput input;
    char *line;
    long double result = 0.0L;
    fast_input_open(&input, metrics_path);
    while ((line = fast_read_line_dynamic(&input)) != NULL) {
        Tokenizer tokenizer;
        char *name_token;
        char *value_token;
        tokenizer_init(&tokenizer, line);
        name_token = next_token(&tokenizer);
        value_token = next_token(&tokenizer);
        if (name_token != NULL &&
            value_token != NULL &&
            strcmp(name_token, metric_name) == 0) {
            char *end_pointer = NULL;
            errno = 0;
            result = strtold(value_token, &end_pointer);
            if (errno != 0 || end_pointer == value_token || *end_pointer != '\0') {
                free(line);
                fast_input_close(&input);
                fail_with_context(metrics_path, "invalid metric value");
            }
            free(line);
            fast_input_close(&input);
            return result;
        }
        free(line);
    }
    fast_input_close(&input);
    return result;
}

static long long read_profile_long_long_or_zero(const char *profile_path,
                                                const char *metric_name)
{
    FastInput input;
    char *line;
    long long result = 0LL;
    fast_input_open(&input, profile_path);
    while ((line = fast_read_line_dynamic(&input)) != NULL) {
        Tokenizer tokenizer;
        char *name_token;
        char *value_token;
        tokenizer_init(&tokenizer, line);
        name_token = next_token(&tokenizer);
        value_token = next_token(&tokenizer);
        if (name_token != NULL &&
            value_token != NULL &&
            strcmp(name_token, metric_name) == 0) {
            result = parse_long_long_range(value_token,
                                           0LL,
                                           LLONG_MAX,
                                           profile_path);
            free(line);
            fast_input_close(&input);
            return result;
        }
        free(line);
    }
    fast_input_close(&input);
    return result;
}

static long double read_profile_long_double_or_zero(const char *profile_path,
                                                    const char *metric_name)
{
    FastInput input;
    char *line;
    long double result = 0.0L;
    fast_input_open(&input, profile_path);
    while ((line = fast_read_line_dynamic(&input)) != NULL) {
        Tokenizer tokenizer;
        char *name_token;
        char *value_token;
        tokenizer_init(&tokenizer, line);
        name_token = next_token(&tokenizer);
        value_token = next_token(&tokenizer);
        if (name_token != NULL &&
            value_token != NULL &&
            strcmp(name_token, metric_name) == 0) {
            char *end_pointer = NULL;
            errno = 0;
            result = strtold(value_token, &end_pointer);
            if (errno != 0 || end_pointer == value_token || *end_pointer != '\0') {
                free(line);
                fast_input_close(&input);
                fail_with_context(profile_path, "invalid profile value");
            }
            free(line);
            fast_input_close(&input);
            return result;
        }
        free(line);
    }
    fast_input_close(&input);
    return result;
}

static void solver_profile_add_child_profile(SolverProfile *profile,
                                             const char *profile_path)
{
    long long max_dinic_nodes;
    long long max_mcf_nodes;
    if (profile == NULL) {
        return;
    }
    profile->dinic_calls +=
        read_profile_long_long_or_zero(profile_path, "dinic_calls");
    profile->min_cost_flow_calls +=
        read_profile_long_long_or_zero(profile_path, "min_cost_flow_calls");
    profile->exact_min_cost_flow_calls +=
        read_profile_long_long_or_zero(profile_path, "exact_min_cost_flow_calls");
    profile->student_group_builds +=
        read_profile_long_long_or_zero(profile_path, "student_group_builds");
    profile->rank_threshold_candidate_builds +=
        read_profile_long_long_or_zero(profile_path,
                                       "rank_threshold_candidate_builds");
    profile->rank_target_checks +=
        read_profile_long_long_or_zero(profile_path, "rank_target_checks");
    profile->average_target_checks +=
        read_profile_long_long_or_zero(profile_path, "average_target_checks");
    profile->q_candidates_tested +=
        read_profile_long_long_or_zero(profile_path, "q_candidates_tested");
    profile->minimum_candidates_tested +=
        read_profile_long_long_or_zero(profile_path, "minimum_candidates_tested");
    profile->exact_path_cost_comparisons +=
        read_profile_long_long_or_zero(profile_path, "exact_path_cost_comparisons");
    profile->biguint_score_comparisons +=
        read_profile_long_long_or_zero(profile_path, "biguint_score_comparisons");
    profile->layered_initial_potentials_used +=
        read_profile_long_long_or_zero(profile_path,
                                       "layered_initial_potentials_used");
    profile->ordinary_average_scalar_attempts +=
        read_profile_long_long_or_zero(profile_path,
                                       "ordinary_average_scalar_attempts");
    profile->ordinary_average_scalar_used +=
        read_profile_long_long_or_zero(profile_path,
                                       "ordinary_average_scalar_used");
    profile->ordinary_average_scalar_fallback_lcm +=
        read_profile_long_long_or_zero(profile_path,
                                       "ordinary_average_scalar_fallback_lcm");
    profile->ordinary_average_scalar_fallback_overflow +=
        read_profile_long_long_or_zero(profile_path,
                                       "ordinary_average_scalar_fallback_overflow");
    profile->ordinary_average_scalar_fallback_not_applicable +=
        read_profile_long_long_or_zero(
            profile_path,
            "ordinary_average_scalar_fallback_not_applicable");
    profile->active_arc_template_hits +=
        read_profile_long_long_or_zero(profile_path,
                                       "active_arc_template_hits");
    profile->active_arc_template_misses +=
        read_profile_long_long_or_zero(profile_path,
                                       "active_arc_template_misses");
    profile->radix_heap_attempts +=
        read_profile_long_long_or_zero(profile_path, "radix_heap_attempts");
    profile->radix_heap_used +=
        read_profile_long_long_or_zero(profile_path, "radix_heap_used");
    profile->radix_heap_fallbacks +=
        read_profile_long_long_or_zero(profile_path, "radix_heap_fallbacks");
    profile->average_fill_resource_vectors_tested +=
        read_profile_long_long_or_zero(
            profile_path,
            "average_fill_resource_vectors_tested");
    profile->average_fill_resource_vector_limit_hits +=
        read_profile_long_long_or_zero(
            profile_path,
            "average_fill_resource_vector_limit_hits");
    profile->weighted_bound_prunes +=
        read_profile_long_long_or_zero(profile_path, "weighted_bound_prunes");
    profile->weighted_corner_cache_hits +=
        read_profile_long_long_or_zero(profile_path, "weighted_corner_cache_hits");
    profile->weighted_corner_cache_misses +=
        read_profile_long_long_or_zero(profile_path, "weighted_corner_cache_misses");
    profile->try_solve_infeasible +=
        read_profile_long_long_or_zero(profile_path, "try_solve_infeasible");
    profile->dinic_edges_added +=
        read_profile_long_long_or_zero(profile_path, "dinic_edges_added");
    profile->mcf_edges_added +=
        read_profile_long_long_or_zero(profile_path, "mcf_edges_added");
    max_dinic_nodes =
        read_profile_long_long_or_zero(profile_path, "max_dinic_nodes");
    max_mcf_nodes =
        read_profile_long_long_or_zero(profile_path, "max_mcf_nodes");
    if (max_dinic_nodes > (long long)profile->max_dinic_nodes) {
        profile->max_dinic_nodes = (int)max_dinic_nodes;
    }
    if (max_mcf_nodes > (long long)profile->max_mcf_nodes) {
        profile->max_mcf_nodes = (int)max_mcf_nodes;
    }
    profile->counterfactual_cpu_seconds +=
        read_profile_long_double_or_zero(profile_path, "counterfactual_cpu_seconds");
}

static void portfolio_candidate_free(PortfolioCandidate *candidate)
{
    free(candidate->assignment);
    candidate->assignment = NULL;
    free_evaluation_metrics(&candidate->metrics);
}

static int portfolio_candidate_better(const PortfolioCandidate *left,
                                      const PortfolioCandidate *right)
{
    const long double tolerance = 0.000000000000001L;
    long double difference = left->recommendation_score - right->recommendation_score;
    if (difference < -tolerance) {
        return 1;
    }
    if (difference > tolerance) {
        return 0;
    }
    (void)left;
    (void)right;
    return 0;
}

static void portfolio_append_label(char *buffer,
                                   size_t buffer_size,
                                   const char *label)
{
    size_t current_length = strlen(buffer);
    size_t label_length = strlen(label);
    size_t extra_length = label_length + (current_length > 0U ? 1U : 0U) + 1U;
    if (current_length > buffer_size || extra_length > buffer_size - current_length) {
        fail_with_context("portfolio report", "strength label buffer is too small");
    }
    if (current_length > 0U) {
        buffer[current_length] = ',';
        current_length++;
        buffer[current_length] = '\0';
    }
    memcpy(buffer + current_length, label, label_length + 1U);
}

static int long_double_close(long double left, long double right)
{
    long double difference = left - right;
    const long double tolerance = 0.000000000001L;
    if (difference < 0.0L) {
        difference = -difference;
    }
    return difference <= tolerance;
}

static void portfolio_candidate_strengths_and_weaknesses(
    const PortfolioCandidate *candidate,
    long double best_average_rank,
    long double worst_average_rank,
    long double best_stddev,
    long double worst_stddev,
    int best_max_rank,
    int worst_max_rank,
    long double best_average_fill,
    long double worst_average_fill,
    long double best_minimum_fill,
    long double worst_minimum_fill,
    int best_outside_count,
    int worst_outside_count,
    int outside_count,
    char *strengths,
    size_t strengths_size,
    char *weaknesses,
    size_t weaknesses_size)
{
    strengths[0] = '\0';
    weaknesses[0] = '\0';
    if (long_double_close(candidate->metrics.average_rank, best_average_rank)) {
        portfolio_append_label(strengths, strengths_size, "avg_rank");
    }
    if (long_double_close(candidate->metrics.rank_stddev, best_stddev)) {
        portfolio_append_label(strengths, strengths_size, "stddev");
    }
    if (candidate->metrics.max_rank == best_max_rank) {
        portfolio_append_label(strengths, strengths_size, "max_rank");
    }
    if (long_double_close(candidate->metrics.average_fill_rate, best_average_fill)) {
        portfolio_append_label(strengths, strengths_size, "avg_fill");
    }
    if (long_double_close(candidate->metrics.minimum_fill_rate, best_minimum_fill)) {
        portfolio_append_label(strengths, strengths_size, "min_fill");
    }
    if (outside_count == best_outside_count) {
        portfolio_append_label(strengths, strengths_size, "outside");
    }

    if (long_double_close(candidate->metrics.average_rank, worst_average_rank)) {
        portfolio_append_label(weaknesses, weaknesses_size, "avg_rank");
    }
    if (long_double_close(candidate->metrics.rank_stddev, worst_stddev)) {
        portfolio_append_label(weaknesses, weaknesses_size, "stddev");
    }
    if (candidate->metrics.max_rank == worst_max_rank) {
        portfolio_append_label(weaknesses, weaknesses_size, "max_rank");
    }
    if (long_double_close(candidate->metrics.average_fill_rate, worst_average_fill)) {
        portfolio_append_label(weaknesses, weaknesses_size, "avg_fill");
    }
    if (long_double_close(candidate->metrics.minimum_fill_rate, worst_minimum_fill)) {
        portfolio_append_label(weaknesses, weaknesses_size, "min_fill");
    }
    if (outside_count == worst_outside_count) {
        portfolio_append_label(weaknesses, weaknesses_size, "outside");
    }
    if (strengths[0] == '\0') {
        portfolio_append_label(strengths, strengths_size, "none");
    }
    if (weaknesses[0] == '\0') {
        portfolio_append_label(weaknesses, weaknesses_size, "none");
    }
}

static void write_portfolio_report_file(const char *report_path,
                                        const ProblemData *problem_data,
                                        const PortfolioCandidate *candidates,
                                        int candidate_count)
{
    char *temporary_path = temporary_output_path_for(report_path);
    FILE *report_file;
    int candidate_index;
    long double best_average_rank = 0.0L;
    long double worst_average_rank = 0.0L;
    long double best_stddev = 0.0L;
    long double worst_stddev = 0.0L;
    int best_max_rank = 0;
    int worst_max_rank = 0;
    long double best_average_fill = 0.0L;
    long double worst_average_fill = 0.0L;
    long double best_minimum_fill = 0.0L;
    long double worst_minimum_fill = 0.0L;
    int best_outside_count = 0;
    int worst_outside_count = 0;

    cleanup_portfolio_report_output_path = temporary_path;
    report_file = fopen(temporary_path, "w");
    if (report_file == NULL) {
        fail_with_context(report_path, "cannot open temporary portfolio report output file");
    }
    for (candidate_index = 0;
         candidate_index < candidate_count;
         candidate_index++) {
        int outside_count =
            outside_preference_count_for_assignment(problem_data,
                                                    candidates[candidate_index].assignment);
        if (candidate_index == 0) {
            best_average_rank = candidates[candidate_index].metrics.average_rank;
            worst_average_rank = candidates[candidate_index].metrics.average_rank;
            best_stddev = candidates[candidate_index].metrics.rank_stddev;
            worst_stddev = candidates[candidate_index].metrics.rank_stddev;
            best_max_rank = candidates[candidate_index].metrics.max_rank;
            worst_max_rank = candidates[candidate_index].metrics.max_rank;
            best_average_fill = candidates[candidate_index].metrics.average_fill_rate;
            worst_average_fill = candidates[candidate_index].metrics.average_fill_rate;
            best_minimum_fill = candidates[candidate_index].metrics.minimum_fill_rate;
            worst_minimum_fill = candidates[candidate_index].metrics.minimum_fill_rate;
            best_outside_count = outside_count;
            worst_outside_count = outside_count;
        } else {
            if (candidates[candidate_index].metrics.average_rank < best_average_rank) {
                best_average_rank = candidates[candidate_index].metrics.average_rank;
            }
            if (candidates[candidate_index].metrics.average_rank > worst_average_rank) {
                worst_average_rank = candidates[candidate_index].metrics.average_rank;
            }
            if (candidates[candidate_index].metrics.rank_stddev < best_stddev) {
                best_stddev = candidates[candidate_index].metrics.rank_stddev;
            }
            if (candidates[candidate_index].metrics.rank_stddev > worst_stddev) {
                worst_stddev = candidates[candidate_index].metrics.rank_stddev;
            }
            if (candidates[candidate_index].metrics.max_rank < best_max_rank) {
                best_max_rank = candidates[candidate_index].metrics.max_rank;
            }
            if (candidates[candidate_index].metrics.max_rank > worst_max_rank) {
                worst_max_rank = candidates[candidate_index].metrics.max_rank;
            }
            if (candidates[candidate_index].metrics.average_fill_rate > best_average_fill) {
                best_average_fill = candidates[candidate_index].metrics.average_fill_rate;
            }
            if (candidates[candidate_index].metrics.average_fill_rate < worst_average_fill) {
                worst_average_fill = candidates[candidate_index].metrics.average_fill_rate;
            }
            if (candidates[candidate_index].metrics.minimum_fill_rate > best_minimum_fill) {
                best_minimum_fill = candidates[candidate_index].metrics.minimum_fill_rate;
            }
            if (candidates[candidate_index].metrics.minimum_fill_rate < worst_minimum_fill) {
                worst_minimum_fill = candidates[candidate_index].metrics.minimum_fill_rate;
            }
            if (outside_count < best_outside_count) {
                best_outside_count = outside_count;
            }
            if (outside_count > worst_outside_count) {
                worst_outside_count = outside_count;
            }
        }
    }
    if (fprintf(report_file,
                "candidate\tavg_rank\trank_stddev\tmax_rank\tavg_dissatisfaction\tdissatisfaction_stddev\tmax_dissatisfaction\tavg_fill\tmin_fill\toutside_count\tstrengths\tweaknesses\tavg_rank_component\tstddev_component\tmax_rank_component\tavg_fill_deficit\tmin_fill_deficit\tsolver_seconds\trecommendation_score\trecommended\ttie_break_order\tselection_reason\n") < 0) {
        fclose(report_file);
        fail_with_context(report_path, "write failed");
    }
    for (candidate_index = 0;
         candidate_index < candidate_count;
         candidate_index++) {
        int outside_count =
            outside_preference_count_for_assignment(problem_data,
                                                    candidates[candidate_index].assignment);
        PortfolioScoreComponents components =
            portfolio_score_components(problem_data,
                                       &candidates[candidate_index].metrics);
        char strengths[128];
        char weaknesses[128];
        portfolio_candidate_strengths_and_weaknesses(
            &candidates[candidate_index],
            best_average_rank,
            worst_average_rank,
            best_stddev,
            worst_stddev,
            best_max_rank,
            worst_max_rank,
            best_average_fill,
            worst_average_fill,
            best_minimum_fill,
            worst_minimum_fill,
            best_outside_count,
            worst_outside_count,
            outside_count,
            strengths,
            sizeof(strengths),
            weaknesses,
            sizeof(weaknesses));
        if (fprintf(report_file,
                    "%s\t%.17Lg\t%.17Lg\t%d\t%.17Lg\t%.17Lg\t%lld\t%.17Lg\t%.17Lg\t%d\t%s\t%s\t%.17Lg\t%.17Lg\t%.17Lg\t%.17Lg\t%.17Lg\t%.17Lg\t%.17Lg\t%s\t%d\t%s\n",
                    candidates[candidate_index].name,
                    candidates[candidate_index].metrics.average_rank,
                    candidates[candidate_index].metrics.rank_stddev,
                    candidates[candidate_index].metrics.max_rank,
                    candidates[candidate_index].metrics.average_dissatisfaction,
                    candidates[candidate_index].metrics.dissatisfaction_stddev,
                    candidates[candidate_index].metrics.max_dissatisfaction,
                    candidates[candidate_index].metrics.average_fill_rate,
                    candidates[candidate_index].metrics.minimum_fill_rate,
                    outside_count,
                    strengths,
                    weaknesses,
                    components.average_rank_component,
                    components.stddev_component,
                    components.max_rank_component,
                    components.average_fill_deficit,
                    components.minimum_fill_deficit,
                    candidates[candidate_index].solver_cpu_seconds,
                    candidates[candidate_index].recommendation_score,
                    candidates[candidate_index].recommended ? "yes" : "no",
                    candidate_index + 1,
                    candidates[candidate_index].recommended ?
                        "lowest_recommendation_score_then_candidate_order" :
                        "not_selected") < 0) {
            fclose(report_file);
            fail_with_context(report_path, "write failed");
        }
    }
    if (fclose(report_file) != 0) {
        fail_with_context(report_path, "close failed");
    }
    replace_output_file(temporary_path, report_path, report_path);
    cleanup_portfolio_report_output_path = NULL;
    free(temporary_path);
}

static void solver_profile_write(const char *profile_path,
                                 const SolverProfile *profile)
{
    char *temporary_path = temporary_output_path_for(profile_path);
    FILE *profile_file;

    cleanup_profile_output_path = temporary_path;
    profile_file = fopen(temporary_path, "w");
    if (profile_file == NULL) {
        fail_with_context(profile_path, "cannot open temporary profile output file");
    }
    if (fprintf(profile_file, "metric\tvalue\n") < 0 ||
        fprintf(profile_file, "dinic_calls\t%lld\n", profile->dinic_calls) < 0 ||
        fprintf(profile_file,
                "min_cost_flow_calls\t%lld\n",
                profile->min_cost_flow_calls) < 0 ||
        fprintf(profile_file,
                "exact_min_cost_flow_calls\t%lld\n",
                profile->exact_min_cost_flow_calls) < 0 ||
        fprintf(profile_file,
                "student_group_builds\t%lld\n",
                profile->student_group_builds) < 0 ||
        fprintf(profile_file,
                "rank_threshold_candidate_builds\t%lld\n",
                profile->rank_threshold_candidate_builds) < 0 ||
        fprintf(profile_file,
                "rank_target_checks\t%lld\n",
                profile->rank_target_checks) < 0 ||
        fprintf(profile_file,
                "average_target_checks\t%lld\n",
                profile->average_target_checks) < 0 ||
        fprintf(profile_file,
                "q_candidates_tested\t%lld\n",
                profile->q_candidates_tested) < 0 ||
        fprintf(profile_file,
                "minimum_candidates_tested\t%lld\n",
                profile->minimum_candidates_tested) < 0 ||
        fprintf(profile_file,
                "exact_path_cost_comparisons\t%lld\n",
                profile->exact_path_cost_comparisons) < 0 ||
        fprintf(profile_file,
                "weighted_score_comparisons\t%lld\n",
                profile->exact_path_cost_comparisons) < 0 ||
        fprintf(profile_file,
                "biguint_score_comparisons\t%lld\n",
                profile->biguint_score_comparisons) < 0 ||
        fprintf(profile_file,
                "layered_initial_potentials_used\t%lld\n",
                profile->layered_initial_potentials_used) < 0 ||
        fprintf(profile_file,
                "ordinary_average_scalar_attempts\t%lld\n",
                profile->ordinary_average_scalar_attempts) < 0 ||
        fprintf(profile_file,
                "ordinary_average_scalar_used\t%lld\n",
                profile->ordinary_average_scalar_used) < 0 ||
        fprintf(profile_file,
                "ordinary_average_scalar_fallback_lcm\t%lld\n",
                profile->ordinary_average_scalar_fallback_lcm) < 0 ||
        fprintf(profile_file,
                "ordinary_average_scalar_fallback_overflow\t%lld\n",
                profile->ordinary_average_scalar_fallback_overflow) < 0 ||
        fprintf(profile_file,
                "ordinary_average_scalar_fallback_not_applicable\t%lld\n",
                profile->ordinary_average_scalar_fallback_not_applicable) < 0 ||
        fprintf(profile_file,
                "active_arc_template_hits\t%lld\n",
                profile->active_arc_template_hits) < 0 ||
        fprintf(profile_file,
                "active_arc_template_misses\t%lld\n",
                profile->active_arc_template_misses) < 0 ||
        fprintf(profile_file,
                "radix_heap_attempts\t%lld\n",
                profile->radix_heap_attempts) < 0 ||
        fprintf(profile_file,
                "radix_heap_used\t%lld\n",
                profile->radix_heap_used) < 0 ||
        fprintf(profile_file,
                "radix_heap_fallbacks\t%lld\n",
                profile->radix_heap_fallbacks) < 0 ||
        fprintf(profile_file,
                "average_fill_resource_vectors_tested\t%lld\n",
                profile->average_fill_resource_vectors_tested) < 0 ||
        fprintf(profile_file,
                "average_fill_resource_vector_limit_hits\t%lld\n",
                profile->average_fill_resource_vector_limit_hits) < 0 ||
        fprintf(profile_file,
                "weighted_bound_prunes\t%lld\n",
                profile->weighted_bound_prunes) < 0 ||
        fprintf(profile_file,
                "weighted_corner_cache_hits\t%lld\n",
                profile->weighted_corner_cache_hits) < 0 ||
        fprintf(profile_file,
                "weighted_corner_cache_misses\t%lld\n",
                profile->weighted_corner_cache_misses) < 0 ||
        fprintf(profile_file,
                "try_solve_infeasible\t%lld\n",
                profile->try_solve_infeasible) < 0 ||
        fprintf(profile_file,
                "dinic_edges_added\t%lld\n",
                profile->dinic_edges_added) < 0 ||
        fprintf(profile_file,
                "mcf_edges_added\t%lld\n",
                profile->mcf_edges_added) < 0 ||
        fprintf(profile_file, "max_dinic_nodes\t%d\n", profile->max_dinic_nodes) < 0 ||
        fprintf(profile_file, "max_mcf_nodes\t%d\n", profile->max_mcf_nodes) < 0 ||
        fprintf(profile_file,
                "read_cpu_seconds\t%.17Lg\n",
                profile->read_cpu_seconds) < 0 ||
        fprintf(profile_file,
                "solver_cpu_seconds\t%.17Lg\n",
                profile->solver_cpu_seconds) < 0 ||
        fprintf(profile_file,
                "counterfactual_cpu_seconds\t%.17Lg\n",
                profile->counterfactual_cpu_seconds) < 0 ||
        fprintf(profile_file,
                "report_cpu_seconds\t%.17Lg\n",
                profile->report_cpu_seconds) < 0 ||
        fprintf(profile_file,
                "total_cpu_seconds\t%.17Lg\n",
                profile->total_cpu_seconds) < 0) {
        fclose(profile_file);
        fail_with_context(profile_path, "write failed");
    }
    if (fclose(profile_file) != 0) {
        fail_with_context(profile_path, "close failed");
    }
    replace_output_file(temporary_path, profile_path, profile_path);
    cleanup_profile_output_path = NULL;
    free(temporary_path);
}

static int counterfactual_lock_conflicts(const ProblemData *problem_data,
                                         const ConstraintSet *constraints,
                                         int student_index,
                                         int lab_index,
                                         const char **reason_out)
{
    size_t matrix_index =
        student_lab_matrix_index(problem_data, student_index, lab_index);
    if (constraints == NULL) {
        return 0;
    }
    if (constraints->locked_lab_by_student[student_index] >= 0 &&
        constraints->locked_lab_by_student[student_index] != lab_index) {
        *reason_out = "conflicts with existing lock";
        return 1;
    }
    if (constraints->forbidden_matrix[matrix_index]) {
        *reason_out = "target lab is forbidden";
        return 1;
    }
    if (constraints->has_allowed_set[student_index] &&
        !constraints->allowed_matrix[matrix_index]) {
        *reason_out = "target lab is outside allowed set";
        return 1;
    }
    return 0;
}

static int count_changed_students_between_assignments(const ProblemData *problem_data,
                                                      const int *left_assignment,
                                                      const int *right_assignment)
{
    int changed_count = 0;
    int student_index;
    for (student_index = 0;
         student_index < problem_data->student_count;
         student_index++) {
        if (left_assignment[student_index] != right_assignment[student_index]) {
            changed_count++;
        }
    }
    return changed_count;
}

static void parse_try_lock_text(const ProblemData *problem_data,
                                const ProgramOptions *options,
                                int *student_index_out,
                                int *lab_index_out)
{
    if (options->try_lock_text != NULL) {
        char *text = duplicate_text(options->try_lock_text);
        char *separator = strchr(text, ':');
        int student_index;
        int lab_index;
        if (separator == NULL) {
            free(text);
            fail_with_context("--try-lock", "expected STUDENT_ID:LAB_NAME");
        }
        *separator = '\0';
        student_index = parse_constraint_student(problem_data, text, "--try-lock");
        lab_index = parse_constraint_lab(problem_data, separator + 1, "--try-lock");
        if (options->explain_student_id != NULL) {
            int explained_student_index =
                parse_constraint_student(problem_data,
                                         options->explain_student_id,
                                         "--explain-student");
            if (explained_student_index != student_index) {
                free(text);
                fail_with_context("--try-lock", "student id differs from --explain-student");
            }
        }
        *student_index_out = student_index;
        *lab_index_out = lab_index;
        free(text);
        return;
    }

    if (options->explain_student_id == NULL) {
        fail_with_context("explanation", "missing --explain-student or --try-lock");
    }
    *student_index_out =
        parse_constraint_student(problem_data,
                                 options->explain_student_id,
                                 "--explain-student");
    *lab_index_out = first_choice_lab_for_student(problem_data, *student_index_out);
    if (*lab_index_out < 0) {
        fail_with_context("--explain-student", "student has no first-choice lab");
    }
}

static void validate_counterfactual_options(const ProblemData *problem_data,
                                            const ProgramOptions *options)
{
    int student_index;
    int lab_index;
    if (options->explain_student_id == NULL && options->try_lock_text == NULL) {
        return;
    }
    parse_try_lock_text(problem_data, options, &student_index, &lab_index);
    (void)student_index;
    (void)lab_index;
}

static void write_counterfactual_explanation_file(const char *report_path,
                                                  ProblemData *problem_data,
                                                  const ProgramOptions *options,
                                                  const int *selected_assignment)
{
    char *temporary_path = temporary_output_path_for(report_path);
    FILE *report_file;
    int student_index;
    int target_lab_index;
    int feasible = 0;
    int *base_minimum_counts;
    int *counterfactual_assignment = NULL;
    ConstraintSet counterfactual_constraints;
    const ConstraintSet *original_constraints = problem_data->constraints;
    const char *reason = "ok";
    EvaluationMetrics selected_metrics =
        compute_evaluation_metrics(problem_data, selected_assignment);

    parse_try_lock_text(problem_data, options, &student_index, &target_lab_index);
    cleanup_explanation_report_output_path = temporary_path;
    report_file = fopen(temporary_path, "w");
    if (report_file == NULL) {
        free_evaluation_metrics(&selected_metrics);
        fail_with_context(report_path, "cannot open temporary explanation output");
    }

    constraint_set_copy(&counterfactual_constraints,
                        problem_data,
                        original_constraints);
    if (!counterfactual_lock_conflicts(problem_data,
                                       &counterfactual_constraints,
                                       student_index,
                                       target_lab_index,
                                       &reason)) {
        counterfactual_constraints.locked_lab_by_student[student_index] =
            target_lab_index;
        problem_data->constraints = &counterfactual_constraints;
        base_minimum_counts = build_base_minimum_counts(problem_data);
        feasible = has_feasible_assignment_with_minimum_counts(
            problem_data,
            problem_data->lab_count + 1,
            base_minimum_counts);
        free(base_minimum_counts);
        if (feasible) {
            counterfactual_assignment =
                solve_assignment_for_mode(problem_data,
                                          options->objective_mode,
                                          options);
            reason = "exact re-solve succeeded";
        } else {
            reason = "no feasible assignment after try-lock";
        }
        problem_data->constraints = original_constraints;
    }

    if (fprintf(report_file,
                "student_id\ttry_lab\tfeasible\tcurrent_lab\tcurrent_rank\t"
                "try_rank\tavg_rank_delta\trank_stddev_delta\tmax_rank_delta\t"
                "average_fill_delta\tminimum_fill_delta\toutside_delta\t"
                "changed_students\treason\n") < 0) {
        fclose(report_file);
        constraint_set_free(&counterfactual_constraints);
        free(counterfactual_assignment);
        free_evaluation_metrics(&selected_metrics);
        fail_with_context(report_path, "write failed");
    }

    if (counterfactual_assignment != NULL) {
        EvaluationMetrics counterfactual_metrics =
            compute_evaluation_metrics(problem_data, counterfactual_assignment);
        int outside_delta =
            outside_preference_count_for_assignment(problem_data,
                                                    counterfactual_assignment) -
            outside_preference_count_for_assignment(problem_data,
                                                    selected_assignment);
        int changed_students =
            count_changed_students_between_assignments(problem_data,
                                                       selected_assignment,
                                                       counterfactual_assignment);
        if (fprintf(report_file,
                    "%s\t%s\tyes\t%s\t%d\t%d\t%.17Lg\t%.17Lg\t%d\t%.17Lg\t%.17Lg\t%d\t%d\t%s\n",
                    problem_data->students[student_index].student_id,
                    problem_data->labs[target_lab_index].name,
                    problem_data->labs[selected_assignment[student_index]].name,
                    rank_for_assignment(problem_data,
                                        student_index,
                                        selected_assignment[student_index]),
                    rank_for_assignment(problem_data, student_index, target_lab_index),
                    counterfactual_metrics.average_rank - selected_metrics.average_rank,
                    counterfactual_metrics.rank_stddev - selected_metrics.rank_stddev,
                    counterfactual_metrics.max_rank - selected_metrics.max_rank,
                    counterfactual_metrics.average_fill_rate -
                        selected_metrics.average_fill_rate,
                    counterfactual_metrics.minimum_fill_rate -
                        selected_metrics.minimum_fill_rate,
                    outside_delta,
                    changed_students,
                    reason) < 0) {
            fclose(report_file);
            constraint_set_free(&counterfactual_constraints);
            free(counterfactual_assignment);
            free_evaluation_metrics(&counterfactual_metrics);
            free_evaluation_metrics(&selected_metrics);
            fail_with_context(report_path, "write failed");
        }
        free_evaluation_metrics(&counterfactual_metrics);
    } else {
        if (fprintf(report_file,
                    "%s\t%s\tno\t%s\t%d\t%d\tNA\tNA\tNA\tNA\tNA\tNA\tNA\t%s\n",
                    problem_data->students[student_index].student_id,
                    problem_data->labs[target_lab_index].name,
                    problem_data->labs[selected_assignment[student_index]].name,
                    rank_for_assignment(problem_data,
                                        student_index,
                                        selected_assignment[student_index]),
                    rank_for_assignment(problem_data, student_index, target_lab_index),
                    reason) < 0) {
            fclose(report_file);
            constraint_set_free(&counterfactual_constraints);
            free_evaluation_metrics(&selected_metrics);
            fail_with_context(report_path, "write failed");
        }
    }

    if (fclose(report_file) != 0) {
        constraint_set_free(&counterfactual_constraints);
        free(counterfactual_assignment);
        free_evaluation_metrics(&selected_metrics);
        fail_with_context(report_path, "close failed");
    }
    replace_output_file(temporary_path, report_path, report_path);
    cleanup_explanation_report_output_path = NULL;
    free(temporary_path);
    constraint_set_free(&counterfactual_constraints);
    free(counterfactual_assignment);
    free_evaluation_metrics(&selected_metrics);
}

#ifndef _WIN32
static void free_child_argument_strings(char **strings, int string_count)
{
    int index_value;
    for (index_value = 0; index_value < string_count; index_value++) {
        free(strings[index_value]);
    }
}

static int spawn_portfolio_child(const ProgramOptions *options,
                                 const PortfolioCandidate *candidate,
                                 const char *candidate_output_path)
{
    char *allocated_strings[32];
    int allocated_count = 0;
    char *child_argv[96];
    int arg_index = 0;
    pid_t child_pid;

#define ADD_LITERAL(value) child_argv[arg_index++] = (char *)(value)
#define ADD_ALLOCATED(value) \
    do { \
        allocated_strings[allocated_count] = (value); \
        child_argv[arg_index++] = allocated_strings[allocated_count]; \
        allocated_count++; \
    } while (0)

    ADD_LITERAL(options->program_path);
    ADD_LITERAL(options->lab_file_path);
    ADD_LITERAL(options->preference_file_path);
    ADD_LITERAL(candidate_output_path);
    ADD_LITERAL("--objective");
    ADD_LITERAL(objective_mode_command_name(candidate->objective_mode));
    ADD_LITERAL("--max-rank-slack");
    ADD_ALLOCATED(format_int_argument(candidate->max_rank_slack));
    ADD_LITERAL("--id-policy");
    ADD_LITERAL(student_id_policy_name(options->id_policy));
    if (options->id_policy != ID_POLICY_TOKEN) {
        ADD_LITERAL("--student-id-width");
        if (options->student_id_width == AUTO_STUDENT_ID_WIDTH) {
            ADD_LITERAL("auto");
        } else {
            ADD_ALLOCATED(format_int_argument(options->student_id_width));
        }
    }
    ADD_LITERAL("--assume-yes");
    ADD_LITERAL("--first-choice-gap");
    ADD_ALLOCATED(format_long_long_argument(options->rank_cost_model.first_choice_gap));
    ADD_LITERAL("--rank-tail-linear");
    ADD_ALLOCATED(format_long_long_argument(options->rank_cost_model.tail_linear));
    ADD_LITERAL("--rank-tail-quadratic");
    ADD_ALLOCATED(format_long_long_argument(options->rank_cost_model.tail_quadratic));
    ADD_LITERAL("--outside-cost");
    ADD_ALLOCATED(format_long_long_argument(options->rank_cost_model.outside_cost));
    if (options->rank_costs_path != NULL) {
        ADD_LITERAL("--rank-costs");
        ADD_LITERAL(options->rank_costs_path);
    }
    if (options->targets_path != NULL) {
        ADD_LITERAL("--targets");
        ADD_LITERAL(options->targets_path);
    }
    if (options->targets.has_average_rank_max) {
        ADD_LITERAL("--require-average-rank-at-most");
        ADD_ALLOCATED(format_ratio_argument(options->targets.average_rank_max));
    }
    if (options->targets.has_average_fill_min) {
        ADD_LITERAL("--require-average-fill-at-least");
        ADD_ALLOCATED(format_ratio_argument(options->targets.average_fill_min));
    }
    if (options->targets.has_rank_sum_max) {
        ADD_LITERAL("--require-rank-sum-at-most");
        ADD_ALLOCATED(format_long_long_argument(options->targets.rank_sum_max));
    }
    if (options->targets.has_rank_square_max) {
        ADD_LITERAL("--require-rank-square-at-most");
        ADD_ALLOCATED(format_long_long_argument(options->targets.rank_square_max));
    }
    if (options->targets.has_max_rank_max) {
        ADD_LITERAL("--require-max-rank-at-most");
        ADD_ALLOCATED(format_int_argument(options->targets.max_rank_max));
    }
    if (options->targets.has_minimum_fill_min) {
        ADD_LITERAL("--require-minimum-fill-at-least");
        ADD_ALLOCATED(format_ratio_argument(options->targets.minimum_fill_min));
    }
    if (options->targets.has_outside_max) {
        ADD_LITERAL("--require-outside-at-most");
        ADD_ALLOCATED(format_int_argument(options->targets.outside_max));
    }
    ADD_LITERAL("--weight-rank-sum");
    ADD_ALLOCATED(format_long_long_argument(candidate->weights.rank_sum));
    ADD_LITERAL("--weight-rank-square");
    ADD_ALLOCATED(format_long_long_argument(candidate->weights.rank_square));
    ADD_LITERAL("--weight-max-rank");
    ADD_ALLOCATED(format_long_long_argument(candidate->weights.max_rank));
    ADD_LITERAL("--weight-average-fill");
    ADD_ALLOCATED(format_long_long_argument(candidate->weights.average_fill));
    ADD_LITERAL("--weight-minimum-fill");
    ADD_ALLOCATED(format_long_long_argument(candidate->weights.minimum_fill));
    ADD_LITERAL("--weight-outside");
    ADD_ALLOCATED(format_long_long_argument(candidate->weights.outside));
    ADD_LITERAL("--weight-change");
    ADD_ALLOCATED(format_long_long_argument(candidate->weights.change));
    if (options->constraints_path != NULL) {
        ADD_LITERAL("--constraints");
        ADD_LITERAL(options->constraints_path);
    }
    if (options->base_assignment_path != NULL) {
        ADD_LITERAL("--base-assignment");
        ADD_LITERAL(options->base_assignment_path);
        ADD_LITERAL("--change-penalty");
        ADD_ALLOCATED(format_long_long_argument(options->change_penalty));
    }
    ADD_LITERAL("--reports");
    ADD_LITERAL("--quiet");
    if (options->write_profile) {
        ADD_LITERAL("--profile");
    }
    child_argv[arg_index] = NULL;

    child_pid = fork();
    if (child_pid < 0) {
        free_child_argument_strings(allocated_strings, allocated_count);
        fail_with_context("parallel portfolio", "fork failed");
    }
    if (child_pid == 0) {
        execvp(options->program_path, child_argv);
        _exit(127);
    }
    free_child_argument_strings(allocated_strings, allocated_count);
#undef ADD_LITERAL
#undef ADD_ALLOCATED
    return (int)child_pid;
}

static void wait_for_portfolio_child(int *candidate_by_pid,
                                     int pid_count,
                                     int *remaining_children)
{
    int status_value;
    pid_t completed_pid = wait(&status_value);
    int candidate_index;
    if (completed_pid < 0) {
        fail_with_context("parallel portfolio", "wait failed");
    }
    (*remaining_children)--;
    for (candidate_index = 0; candidate_index < pid_count; candidate_index++) {
        if (candidate_by_pid[candidate_index] == (int)completed_pid) {
            candidate_by_pid[candidate_index] = 0;
            break;
        }
    }
    if (!WIFEXITED(status_value) || WEXITSTATUS(status_value) != 0) {
        fail_with_context("parallel portfolio", "candidate process failed");
    }
}
#endif

static int *solve_portfolio_problem(const ProblemData *problem_data,
                                    const ProgramOptions *options,
                                    const char *output_file_path,
                                    long double *solver_seconds_out)
{
    PortfolioCandidate candidates[MAX_PORTFOLIO_CANDIDATES];
    int candidate_count = 0;
    int candidate_index;
    int recommended_index = 0;
    int *assignment;
    ProgramOptions candidate_options = *options;
    WeightedObjective fill_focused_weights = fill_focused_weighted_objective();

    portfolio_candidate_init(&candidates[candidate_count++],
                             "rubric",
                             OBJECTIVE_RUBRIC,
                             options->max_rank_slack,
                             &options->weights);
    portfolio_candidate_init(&candidates[candidate_count++],
                             "satisfaction",
                             OBJECTIVE_SATISFACTION,
                             options->max_rank_slack,
                             &options->weights);
    portfolio_candidate_init(&candidates[candidate_count++],
                             "fair",
                             OBJECTIVE_FAIR,
                             options->max_rank_slack,
                             &options->weights);
    portfolio_candidate_init(&candidates[candidate_count++],
                             "balanced",
                             OBJECTIVE_BALANCED,
                             options->max_rank_slack,
                             &options->weights);
    portfolio_candidate_init(&candidates[candidate_count++],
                             "guarded",
                             OBJECTIVE_GUARDED,
                             options->max_rank_slack,
                             &options->weights);
    if (options->portfolio_mode >= 2) {
        portfolio_candidate_init(&candidates[candidate_count++],
                                 "fill_focused",
                                 OBJECTIVE_WEIGHTED_EXACT,
                                 options->max_rank_slack,
                                 &fill_focused_weights);
        portfolio_candidate_init(&candidates[candidate_count++],
                                 "fill_convex",
                                 OBJECTIVE_FILL_CONVEX,
                                 options->max_rank_slack,
                                 &options->weights);
        portfolio_candidate_init(&candidates[candidate_count++],
                                 "weighted_exact",
                                 OBJECTIVE_WEIGHTED_EXACT,
                                 options->max_rank_slack,
                                 &options->weights);
    }

    if (!options->keep_candidate_files) {
        for (candidate_index = 0;
             candidate_index < candidate_count;
             candidate_index++) {
            char *candidate_output_path =
                portfolio_assignment_output_path_for(output_file_path,
                                                     candidates[candidate_index].name);
            remove_portfolio_candidate_outputs(candidate_output_path);
            free(candidate_output_path);
        }
    }

    if (options->jobs > 1) {
#ifdef _WIN32
        /* Portable fallback: the exact candidates remain deterministic. */
#else
        char *candidate_output_paths[MAX_PORTFOLIO_CANDIDATES];
        int pid_by_candidate[MAX_PORTFOLIO_CANDIDATES] = {0};
        int running_children = 0;
        int max_jobs = options->jobs < candidate_count ? options->jobs : candidate_count;
        for (candidate_index = 0;
             candidate_index < candidate_count;
             candidate_index++) {
            while (running_children >= max_jobs) {
                wait_for_portfolio_child(pid_by_candidate,
                                         candidate_count,
                                         &running_children);
            }
            candidate_output_paths[candidate_index] =
                portfolio_assignment_output_path_for(output_file_path,
                                                     candidates[candidate_index].name);
            pid_by_candidate[candidate_index] =
                spawn_portfolio_child(options,
                                      &candidates[candidate_index],
                                      candidate_output_paths[candidate_index]);
            running_children++;
        }
        while (running_children > 0) {
            wait_for_portfolio_child(pid_by_candidate,
                                     candidate_count,
                                     &running_children);
        }
        for (candidate_index = 0;
             candidate_index < candidate_count;
             candidate_index++) {
            char *metrics_path =
                metrics_output_path_for(candidate_output_paths[candidate_index]);
            char *profile_path =
                profile_output_path_for(candidate_output_paths[candidate_index]);
            candidates[candidate_index].assignment =
                read_assignment_file_as_base(candidate_output_paths[candidate_index],
                                             problem_data);
            candidates[candidate_index].metrics =
                compute_evaluation_metrics(problem_data,
                                           candidates[candidate_index].assignment);
            candidates[candidate_index].solver_cpu_seconds =
                read_metric_long_double_or_zero(metrics_path,
                                                "solver_cpu_seconds");
            candidates[candidate_index].metrics.solver_cpu_seconds =
                candidates[candidate_index].solver_cpu_seconds;
            candidates[candidate_index].recommendation_score =
                portfolio_recommendation_score(problem_data,
                                               &candidates[candidate_index].metrics);
            if (candidate_index == 0 ||
                portfolio_candidate_better(&candidates[candidate_index],
                                           &candidates[recommended_index])) {
                recommended_index = candidate_index;
            }
            if (options->write_profile) {
                solver_profile_add_child_profile(active_profile, profile_path);
            }
            if (!options->keep_candidate_files) {
                remove_portfolio_candidate_outputs(candidate_output_paths[candidate_index]);
            }
            free(metrics_path);
            free(profile_path);
            free(candidate_output_paths[candidate_index]);
        }
        goto portfolio_candidates_ready;
#endif
    }

    for (candidate_index = 0;
         candidate_index < candidate_count;
         candidate_index++) {
        clock_t candidate_start_clock = clock();
        clock_t candidate_end_clock;
        char *candidate_output_path;
        candidate_options.objective_mode = candidates[candidate_index].objective_mode;
        candidate_options.max_rank_slack = candidates[candidate_index].max_rank_slack;
        candidate_options.weights = candidates[candidate_index].weights;
        candidates[candidate_index].assignment =
            solve_assignment_for_mode(problem_data,
                                      candidates[candidate_index].objective_mode,
                                      &candidate_options);
        candidate_end_clock = clock();
        candidates[candidate_index].solver_cpu_seconds =
            elapsed_cpu_seconds(candidate_start_clock, candidate_end_clock);
        candidates[candidate_index].metrics =
            compute_evaluation_metrics(problem_data, candidates[candidate_index].assignment);
        candidates[candidate_index].metrics.solver_cpu_seconds =
            candidates[candidate_index].solver_cpu_seconds;
        candidates[candidate_index].recommendation_score =
            portfolio_recommendation_score(problem_data,
                                           &candidates[candidate_index].metrics);
        if (candidate_index == 0 ||
            portfolio_candidate_better(&candidates[candidate_index],
                                       &candidates[recommended_index])) {
            recommended_index = candidate_index;
        }
        if (options->keep_candidate_files) {
            candidate_output_path =
                portfolio_assignment_output_path_for(output_file_path,
                                                     candidates[candidate_index].name);
            write_assignment_file(candidate_output_path,
                                  problem_data,
                                  candidates[candidate_index].assignment);
            free(candidate_output_path);
        }
    }

portfolio_candidates_ready:
    candidates[recommended_index].recommended = 1;
    if (options->portfolio_mode) {
        char *portfolio_report_path =
            portfolio_report_output_path_for(output_file_path);
        write_portfolio_report_file(portfolio_report_path,
                                    problem_data,
                                    candidates,
                                    candidate_count);
        free(portfolio_report_path);
    }

    assignment =
        checked_malloc((size_t)problem_data->student_count * sizeof(int));
    memcpy(assignment,
           candidates[recommended_index].assignment,
           (size_t)problem_data->student_count * sizeof(int));
    *solver_seconds_out = 0.0L;
    for (candidate_index = 0;
         candidate_index < candidate_count;
         candidate_index++) {
        *solver_seconds_out += candidates[candidate_index].solver_cpu_seconds;
        portfolio_candidate_free(&candidates[candidate_index]);
    }
    return assignment;
}

static ProgramOptions default_program_options(void)
{
    ProgramOptions options;
    options.write_reports = 0;
    options.write_profile = 0;
    options.portfolio_mode = 0;
    options.keep_candidate_files = 1;
    options.quiet = 0;
    options.id_policy = ID_POLICY_AUTO;
    options.student_id_width = AUTO_STUDENT_ID_WIDTH;
    options.interactive = 1;
    options.assume_yes = 0;
    options.jobs = 1;
    options.objective_mode = OBJECTIVE_RUBRIC;
    options.max_rank_slack = 1;
    options.weights = default_weighted_objective();
    rank_cost_model_init_default(&options.rank_cost_model);
    options.rank_costs_path = NULL;
    options.weights_path = NULL;
    options.targets_path = NULL;
    options.constraints_path = NULL;
    options.base_assignment_path = NULL;
    options.explain_student_id = NULL;
    options.try_lock_text = NULL;
    options.program_path = NULL;
    options.lab_file_path = NULL;
    options.preference_file_path = NULL;
    options.change_penalty = 0LL;
    target_constraints_init(&options.targets);
    return options;
}

static ObjectiveMode parse_objective_mode(const char *text)
{
    if (strcmp(text, "rubric") == 0) {
        return OBJECTIVE_RUBRIC;
    }
    if (strcmp(text, "satisfaction") == 0) {
        return OBJECTIVE_SATISFACTION;
    }
    if (strcmp(text, "fair") == 0) {
        return OBJECTIVE_FAIR;
    }
    if (strcmp(text, "balanced") == 0) {
        return OBJECTIVE_BALANCED;
    }
    if (strcmp(text, "guarded") == 0) {
        return OBJECTIVE_GUARDED;
    }
    if (strcmp(text, "fill-convex") == 0) {
        return OBJECTIVE_FILL_CONVEX;
    }
    if (strcmp(text, "weighted-exact") == 0) {
        return OBJECTIVE_WEIGHTED_EXACT;
    }
    fail_with_context_format_hint(
        "objective",
        "run ./assign_labs --print-objectives to list valid modes",
        "unknown objective mode '%s'",
        text);
    return OBJECTIVE_FAIR;
}

static long long parse_nonnegative_weight(const char *text, const char *context)
{
    return parse_long_long_range(text, 0LL, MAX_CONFIG_WEIGHT, context);
}

static void load_weights_file(const char *weights_path, WeightedObjective *objective)
{
    FastInput input;
    char *line;

    fast_input_open(&input, weights_path);
    while ((line = fast_read_line_dynamic(&input)) != NULL) {
        Tokenizer tokenizer;
        char *command;
        char *value_text;
        long long value;
        tokenizer_init(&tokenizer, line);
        command = next_token(&tokenizer);
        if (command == NULL || command[0] == '#') {
            free(line);
            continue;
        }
        value_text = next_token(&tokenizer);
        if (value_text == NULL) {
            free(line);
            fail_with_context(weights_path, "weight entry requires name and value");
        }
        value = parse_nonnegative_weight(value_text, weights_path);
        check_no_extra_token(&tokenizer, weights_path);

        if (strcmp(command, "rank_sum") == 0) {
            objective->rank_sum = value;
        } else if (strcmp(command, "rank_square") == 0) {
            objective->rank_square = value;
        } else if (strcmp(command, "max_rank") == 0) {
            objective->max_rank = value;
        } else if (strcmp(command, "average_fill") == 0) {
            objective->average_fill = value;
        } else if (strcmp(command, "minimum_fill") == 0) {
            objective->minimum_fill = value;
        } else if (strcmp(command, "outside") == 0) {
            objective->outside = value;
        } else if (strcmp(command, "change") == 0) {
            objective->change = value;
        } else {
            free(line);
            fail_with_context(weights_path, "unknown weight name");
        }
        free(line);
    }
    fast_input_close(&input);
}

static void load_targets_file(const char *targets_path, TargetConstraints *targets)
{
    FastInput input;
    char *line;

    fast_input_open(&input, targets_path);
    while ((line = fast_read_line_dynamic(&input)) != NULL) {
        Tokenizer tokenizer;
        char *metric_text;
        char *operator_text;
        char *value_text;
        tokenizer_init(&tokenizer, line);
        metric_text = next_token(&tokenizer);
        if (metric_text == NULL || metric_text[0] == '#') {
            free(line);
            continue;
        }
        operator_text = next_token(&tokenizer);
        value_text = next_token(&tokenizer);
        if (operator_text == NULL || value_text == NULL) {
            free(line);
            fail_with_context(targets_path,
                              "target entry requires metric, operator, and value");
        }
        check_no_extra_token(&tokenizer, targets_path);

        if (strcmp(metric_text, "average_rank") == 0 ||
            strcmp(metric_text, "rank_average") == 0) {
            if (strcmp(operator_text, "<=") != 0) {
                free(line);
                fail_with_context(targets_path,
                                  "average_rank target requires <= operator");
            }
            target_constraints_set_average_rank_max(
                targets,
                parse_ratio_bound(value_text, targets_path));
        } else if (strcmp(metric_text, "rank_sum") == 0) {
            if (strcmp(operator_text, "<=") != 0) {
                free(line);
                fail_with_context(targets_path, "rank_sum target requires <= operator");
            }
            target_constraints_set_rank_sum_max(
                targets,
                parse_long_long_range(value_text, 0LL, LLONG_MAX / 4LL, targets_path));
        } else if (strcmp(metric_text, "rank_square_sum") == 0) {
            if (strcmp(operator_text, "<=") != 0) {
                free(line);
                fail_with_context(targets_path,
                                  "rank_square_sum target requires <= operator");
            }
            target_constraints_set_rank_square_max(
                targets,
                parse_long_long_range(value_text, 0LL, LLONG_MAX / 4LL, targets_path));
        } else if (strcmp(metric_text, "max_rank") == 0 ||
                   strcmp(metric_text, "rank_max") == 0) {
            if (strcmp(operator_text, "<=") != 0) {
                free(line);
                fail_with_context(targets_path, "max_rank target requires <= operator");
            }
            target_constraints_set_max_rank_max(
                targets,
                parse_int_range(value_text, 1, INT_MAX / 4, targets_path));
        } else if (strcmp(metric_text, "minimum_fill_rate") == 0 ||
                   strcmp(metric_text, "fill_rate_minimum") == 0) {
            if (strcmp(operator_text, ">=") != 0) {
                free(line);
                fail_with_context(targets_path,
                                  "minimum_fill_rate target requires >= operator");
            }
            target_constraints_set_minimum_fill_min(
                targets,
                parse_ratio_bound(value_text, targets_path));
        } else if (strcmp(metric_text, "outside_preference_count") == 0 ||
                   strcmp(metric_text, "outside") == 0) {
            if (strcmp(operator_text, "<=") != 0) {
                free(line);
                fail_with_context(targets_path,
                                  "outside_preference_count target requires <= operator");
            }
            target_constraints_set_outside_max(
                targets,
                parse_int_range(value_text, 0, INT_MAX / 4, targets_path));
        } else if (strcmp(metric_text, "average_fill_rate") == 0 ||
                   strcmp(metric_text, "fill_rate_average") == 0) {
            if (strcmp(operator_text, ">=") != 0) {
                free(line);
                fail_with_context(targets_path,
                                  "average_fill_rate target requires >= operator");
            }
            target_constraints_set_average_fill_min(
                targets,
                parse_ratio_bound(value_text, targets_path));
        } else {
            free(line);
            fail_with_context(targets_path, "unknown target metric");
        }
        free(line);
    }
    fast_input_close(&input);
}

static void fill_rank_cost_table_from_formula(const ProblemData *problem_data,
                                              RankCostModel *model)
{
    int rank_value;
    int max_rank = problem_data->lab_count + 1;
    for (rank_value = 1; rank_value <= max_rank; rank_value++) {
        if (rank_value == 1) {
            model->rank_costs[rank_value] = 0LL;
        } else if (rank_value == max_rank) {
            model->rank_costs[rank_value] = model->outside_cost;
        } else {
            model->rank_costs[rank_value] = rank_cost_formula_value(model, rank_value);
        }
    }
}

static void load_rank_costs_file(const char *rank_costs_path,
                                 const ProblemData *problem_data,
                                 RankCostModel *model)
{
    int max_rank = problem_data->lab_count + 1;
    FastInput input;
    char *line;

    free(model->rank_costs);
    model->rank_costs =
        checked_calloc((size_t)max_rank + 1U, sizeof(long long));
    model->use_explicit_table = 1;
    fill_rank_cost_table_from_formula(problem_data, model);

    fast_input_open(&input, rank_costs_path);
    while ((line = fast_read_line_dynamic(&input)) != NULL) {
        Tokenizer tokenizer;
        char *command;
        tokenizer_init(&tokenizer, line);
        command = next_token(&tokenizer);
        if (command == NULL || command[0] == '#') {
            free(line);
            continue;
        }
        if (strcmp(command, "rank") == 0) {
            char *rank_text = next_token(&tokenizer);
            char *cost_text = next_token(&tokenizer);
            int rank_value;
            long long cost_value;
            if (rank_text == NULL || cost_text == NULL) {
                free(line);
                fail_with_context(rank_costs_path, "rank entry requires rank and cost");
            }
            rank_value = parse_int_range(rank_text, 1, max_rank, rank_costs_path);
            cost_value =
                parse_long_long_range(cost_text,
                                      0LL,
                                      MAX_CONFIG_WEIGHT,
                                      rank_costs_path);
            check_no_extra_token(&tokenizer, rank_costs_path);
            model->rank_costs[rank_value] = rank_value == 1 ? 0LL : cost_value;
        } else if (strcmp(command, "outside") == 0) {
            char *cost_text = next_token(&tokenizer);
            long long cost_value;
            if (cost_text == NULL) {
                free(line);
                fail_with_context(rank_costs_path, "outside entry requires cost");
            }
            cost_value =
                parse_long_long_range(cost_text,
                                      0LL,
                                      MAX_CONFIG_WEIGHT,
                                      rank_costs_path);
            check_no_extra_token(&tokenizer, rank_costs_path);
            model->outside_cost = cost_value;
            model->rank_costs[max_rank] = cost_value;
        } else {
            free(line);
            fail_with_context(rank_costs_path, "unknown rank cost command");
        }
        free(line);
    }
    fast_input_close(&input);
    validate_rank_cost_model(problem_data, model);
}

static const char *require_option_value(int argc,
                                        char **argv,
                                        int *argument_index,
                                        const char *option_name)
{
    if (*argument_index + 1 >= argc) {
        fail_with_context(option_name, "missing option value");
    }
    (*argument_index)++;
    return argv[*argument_index];
}

static int target_constraints_have_rank_sum_like_bound(
    const TargetConstraints *targets)
{
    return targets->has_average_rank_max || targets->has_rank_sum_max;
}

static void validate_target_constraints_supported_for_run(
    const ProgramOptions *options)
{
    const TargetConstraints *targets = &options->targets;
    if (target_constraints_are_empty(targets)) {
        return;
    }
    if (targets->has_rank_square_max) {
        fail_with_context(
            "target constraints",
            "--require-rank-square-at-most is parsed but not supported as an exact hard target yet");
    }
    if (targets->has_outside_max && targets->outside_max != 0) {
        fail_with_context(
            "target constraints",
            "only --require-outside-at-most 0 is currently supported exactly");
    }
    if (target_constraints_have_rank_sum_like_bound(targets)) {
        if (options->change_penalty > 0LL) {
            fail_with_context(
                "target constraints",
                "average-rank/rank-sum hard targets are not currently exact with --change-penalty");
        }
        if (options->portfolio_mode) {
            fail_with_context(
                "target constraints",
                "average-rank/rank-sum hard targets are not supported with --portfolio; run a supported single objective");
        }
        if (options->objective_mode != OBJECTIVE_RUBRIC &&
            options->objective_mode != OBJECTIVE_BALANCED &&
            options->objective_mode != OBJECTIVE_GUARDED &&
            options->objective_mode != OBJECTIVE_FAIR) {
            fail_with_context(
                "target constraints",
                "average-rank/rank-sum hard targets are exact only for rubric, balanced, guarded, or fair");
        }
    }
}

static ProgramOptions parse_program_options(int argc, char **argv)
{
    ProgramOptions options = default_program_options();
    int argument_index;
    for (argument_index = 4; argument_index < argc; argument_index++) {
        const char *argument = argv[argument_index];
        if (strcmp(argument, "--reports") == 0) {
            options.write_reports = 1;
        } else if (strcmp(argument, "--profile") == 0) {
            options.write_profile = 1;
        } else if (strcmp(argument, "--quiet") == 0) {
            options.quiet = 1;
        } else if (strcmp(argument, "--portfolio") == 0) {
            if (options.portfolio_mode == 0) {
                options.portfolio_mode = 1;
            }
        } else if (strcmp(argument, "--portfolio-deep") == 0) {
            options.portfolio_mode = 2;
        } else if (strcmp(argument, "--portfolio-summary-only") == 0) {
            options.keep_candidate_files = 0;
        } else if (strcmp(argument, "--keep-candidate-files") == 0) {
            options.keep_candidate_files = 1;
        } else if (strcmp(argument, "--jobs") == 0) {
            options.jobs =
                parse_int_range(require_option_value(argc,
                                                     argv,
                                                     &argument_index,
                                                     argument),
                                1,
                                64,
                                argument);
        } else if (strcmp(argument, "--id-policy") == 0) {
            options.id_policy =
                parse_student_id_policy(require_option_value(argc,
                                                             argv,
                                                             &argument_index,
                                                             argument));
        } else if (strcmp(argument, "--student-id-width") == 0) {
            options.student_id_width =
                parse_student_id_width(require_option_value(argc,
                                                            argv,
                                                            &argument_index,
                                                            argument));
        } else if (strcmp(argument, "--interactive") == 0) {
            options.interactive = 1;
        } else if (strcmp(argument, "--no-interactive") == 0) {
            options.interactive = 0;
        } else if (strcmp(argument, "--assume-yes") == 0) {
            options.assume_yes = 1;
        } else if (strcmp(argument, "--objective") == 0) {
            options.objective_mode =
                parse_objective_mode(require_option_value(argc,
                                                          argv,
                                                          &argument_index,
                                                          "--objective"));
        } else if (strcmp(argument, "--max-rank-slack") == 0) {
            options.max_rank_slack =
                parse_int_range(require_option_value(argc,
                                                     argv,
                                                     &argument_index,
                                                     argument),
                                0,
                                INT_MAX / 4,
                                argument);
        } else if (strcmp(argument, "--rank-costs") == 0) {
            options.rank_costs_path =
                require_option_value(argc, argv, &argument_index, argument);
        } else if (strcmp(argument, "--weights") == 0) {
            options.weights_path =
                require_option_value(argc, argv, &argument_index, argument);
            load_weights_file(options.weights_path, &options.weights);
        } else if (strcmp(argument, "--targets") == 0) {
            options.targets_path =
                require_option_value(argc, argv, &argument_index, argument);
            load_targets_file(options.targets_path, &options.targets);
        } else if (strcmp(argument, "--require-average-rank-at-most") == 0) {
            target_constraints_set_average_rank_max(
                &options.targets,
                parse_ratio_bound(require_option_value(argc,
                                                       argv,
                                                       &argument_index,
                                                       argument),
                                  argument));
        } else if (strcmp(argument, "--require-rank-sum-at-most") == 0) {
            target_constraints_set_rank_sum_max(
                &options.targets,
                parse_long_long_range(require_option_value(argc,
                                                           argv,
                                                           &argument_index,
                                                           argument),
                                      0LL,
                                      LLONG_MAX / 4LL,
                                      argument));
        } else if (strcmp(argument, "--require-rank-square-at-most") == 0) {
            target_constraints_set_rank_square_max(
                &options.targets,
                parse_long_long_range(require_option_value(argc,
                                                           argv,
                                                           &argument_index,
                                                           argument),
                                      0LL,
                                      LLONG_MAX / 4LL,
                                      argument));
        } else if (strcmp(argument, "--require-max-rank-at-most") == 0) {
            target_constraints_set_max_rank_max(
                &options.targets,
                parse_int_range(require_option_value(argc,
                                                     argv,
                                                     &argument_index,
                                                     argument),
                                1,
                                INT_MAX / 4,
                                argument));
        } else if (strcmp(argument, "--require-minimum-fill-at-least") == 0) {
            target_constraints_set_minimum_fill_min(
                &options.targets,
                parse_ratio_bound(require_option_value(argc,
                                                       argv,
                                                       &argument_index,
                                                       argument),
                                  argument));
        } else if (strcmp(argument, "--require-average-fill-at-least") == 0) {
            target_constraints_set_average_fill_min(
                &options.targets,
                parse_ratio_bound(require_option_value(argc,
                                                       argv,
                                                       &argument_index,
                                                       argument),
                                  argument));
        } else if (strcmp(argument, "--require-no-outside") == 0) {
            target_constraints_set_outside_max(&options.targets, 0);
        } else if (strcmp(argument, "--require-outside-at-most") == 0) {
            target_constraints_set_outside_max(
                &options.targets,
                parse_int_range(require_option_value(argc,
                                                     argv,
                                                     &argument_index,
                                                     argument),
                                0,
                                INT_MAX / 4,
                                argument));
        } else if (strcmp(argument, "--first-choice-gap") == 0) {
            options.rank_cost_model.first_choice_gap =
                parse_nonnegative_weight(require_option_value(argc,
                                                              argv,
                                                              &argument_index,
                                                              argument),
                                         argument);
        } else if (strcmp(argument, "--rank-tail-linear") == 0) {
            options.rank_cost_model.tail_linear =
                parse_nonnegative_weight(require_option_value(argc,
                                                              argv,
                                                              &argument_index,
                                                              argument),
                                         argument);
        } else if (strcmp(argument, "--rank-tail-quadratic") == 0) {
            options.rank_cost_model.tail_quadratic =
                parse_nonnegative_weight(require_option_value(argc,
                                                              argv,
                                                              &argument_index,
                                                              argument),
                                         argument);
        } else if (strcmp(argument, "--outside-cost") == 0) {
            options.rank_cost_model.outside_cost =
                parse_nonnegative_weight(require_option_value(argc,
                                                              argv,
                                                              &argument_index,
                                                              argument),
                                         argument);
        } else if (strcmp(argument, "--constraints") == 0) {
            options.constraints_path =
                require_option_value(argc, argv, &argument_index, argument);
        } else if (strcmp(argument, "--base-assignment") == 0) {
            options.base_assignment_path =
                require_option_value(argc, argv, &argument_index, argument);
        } else if (strcmp(argument, "--explain-student") == 0) {
            options.explain_student_id =
                require_option_value(argc, argv, &argument_index, argument);
        } else if (strcmp(argument, "--try-lock") == 0) {
            options.try_lock_text =
                require_option_value(argc, argv, &argument_index, argument);
        } else if (strcmp(argument, "--change-penalty") == 0) {
            options.change_penalty =
                parse_nonnegative_weight(require_option_value(argc,
                                                              argv,
                                                              &argument_index,
                                                              argument),
                                         argument);
        } else if (strcmp(argument, "--weight-rank-sum") == 0) {
            options.weights.rank_sum =
                parse_nonnegative_weight(require_option_value(argc,
                                                              argv,
                                                              &argument_index,
                                                              argument),
                                         argument);
        } else if (strcmp(argument, "--weight-rank-square") == 0) {
            options.weights.rank_square =
                parse_nonnegative_weight(require_option_value(argc,
                                                              argv,
                                                              &argument_index,
                                                              argument),
                                         argument);
        } else if (strcmp(argument, "--weight-max-rank") == 0) {
            options.weights.max_rank =
                parse_nonnegative_weight(require_option_value(argc,
                                                              argv,
                                                              &argument_index,
                                                              argument),
                                         argument);
        } else if (strcmp(argument, "--weight-average-fill") == 0) {
            options.weights.average_fill =
                parse_nonnegative_weight(require_option_value(argc,
                                                              argv,
                                                              &argument_index,
                                                              argument),
                                         argument);
        } else if (strcmp(argument, "--weight-minimum-fill") == 0) {
            options.weights.minimum_fill =
                parse_nonnegative_weight(require_option_value(argc,
                                                              argv,
                                                              &argument_index,
                                                              argument),
                                         argument);
        } else if (strcmp(argument, "--weight-outside") == 0) {
            options.weights.outside =
                parse_nonnegative_weight(require_option_value(argc,
                                                              argv,
                                                              &argument_index,
                                                              argument),
                                         argument);
        } else if (strcmp(argument, "--weight-change") == 0) {
            options.weights.change =
                parse_nonnegative_weight(require_option_value(argc,
                                                              argv,
                                                              &argument_index,
                                                              argument),
                                         argument);
        } else {
            fail_with_context_format_hint(argument,
                                          "run ./assign_labs --help to list options",
                                          "unknown option");
        }
    }
    return options;
}

int main(int argc, char **argv)
{
    ProblemData problem_data;
    ConstraintSet constraints;
    int constraints_are_loaded = 0;
    int *base_assignment = NULL;
    clock_t program_start_clock;
    clock_t read_end_clock;
    clock_t solver_start_clock;
    clock_t solver_end_clock;
    clock_t counterfactual_start_clock;
    clock_t counterfactual_end_clock;
    clock_t report_start_clock;
    clock_t report_end_clock;
    int *assignment;
    ProgramOptions options;
    SolverProfile profile;
    long double portfolio_solver_seconds = -1.0L;

    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }
    if (argc == 2 && strcmp(argv[1], "--print-objectives") == 0) {
        print_objectives();
        return EXIT_SUCCESS;
    }
    if (argc == 2 && strcmp(argv[1], "--print-presets") == 0) {
        print_presets();
        return EXIT_SUCCESS;
    }
    if (argc == 2 && strcmp(argv[1], "--print-rank-costs") == 0) {
        print_default_rank_costs();
        return EXIT_SUCCESS;
    }
    if (argc >= 2 && strcmp(argv[1], "--explain-weights") == 0) {
        WeightedObjective weights = default_weighted_objective();
        const char *weights_path = NULL;
        if (argc > 3) {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
        if (argc == 3) {
            weights_path = argv[2];
            load_weights_file(weights_path, &weights);
        }
        print_weighted_objective_explanation(&weights, weights_path);
        return EXIT_SUCCESS;
    }
    if (argc < 4) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    options = parse_program_options(argc, argv);
    options.program_path = argv[0];
    options.lab_file_path = argv[1];
    options.preference_file_path = argv[2];
    validate_target_constraints_supported_for_run(&options);
    if (options.portfolio_mode &&
        (options.explain_student_id != NULL || options.try_lock_text != NULL)) {
        fail_with_context("--explain-student",
                          "counterfactual explanation requires a single --objective mode, not --portfolio");
    }

    program_start_clock = clock();
    solver_profile_init(&profile);
    active_profile = options.write_profile ? &profile : NULL;
    reject_path_equal_to_input_path("output path", argv[1], argv[2], argv[3]);
    reject_path_equal_to_optional_input_path("output path",
                                             options.rank_costs_path,
                                             argv[3]);
    reject_path_equal_to_named_optional_input_path("output path",
                                                  options.weights_path,
                                                  argv[3],
                                                  "weights input file path");
    reject_path_equal_to_named_optional_input_path("output path",
                                                  options.targets_path,
                                                  argv[3],
                                                  "targets input file path");
    reject_path_equal_to_named_optional_input_path("output path",
                                                  options.constraints_path,
                                                  argv[3],
                                                  "constraints input file path");
    reject_path_equal_to_named_optional_input_path("output path",
                                                  options.base_assignment_path,
                                                  argv[3],
                                                  "base assignment input file path");
    if (options.write_reports ||
        options.write_profile ||
        options.portfolio_mode ||
        options.explain_student_id != NULL ||
        options.try_lock_text != NULL) {
        reject_report_paths_equal_to_input_paths(argv[1],
                                                argv[2],
                                                options.rank_costs_path,
                                                options.weights_path,
                                                options.targets_path,
                                                options.constraints_path,
                                                options.base_assignment_path,
                                                argv[3],
                                                options.write_profile);
    }
    if (options.portfolio_mode) {
        reject_portfolio_assignment_paths_equal_to_input_paths(argv[1],
                                                              argv[2],
                                                              options.rank_costs_path,
                                                              options.weights_path,
                                                              options.targets_path,
                                                              options.constraints_path,
                                                              options.base_assignment_path,
                                                              argv[3],
                                                              options.portfolio_mode);
    }
    problem_data = read_problem_data(argv[1], argv[2], &options);
    options.id_policy = problem_data.id_policy;
    options.student_id_width = problem_data.student_id_width;
    options.assume_yes = 1;
    if (options.rank_costs_path != NULL) {
        load_rank_costs_file(options.rank_costs_path,
                             &problem_data,
                             &options.rank_cost_model);
    } else {
        validate_rank_cost_model(&problem_data, &options.rank_cost_model);
    }
    problem_data.rank_cost_model = &options.rank_cost_model;
    problem_data.targets = &options.targets;
    if (options.constraints_path != NULL) {
        read_constraints_file(options.constraints_path, &problem_data, &constraints);
        problem_data.constraints = &constraints;
        constraints_are_loaded = 1;
    }
    finalize_problem_capacities(&problem_data);
    validate_target_constraints_against_problem(&problem_data, &options);
    precheck_structural_target_constraints(&problem_data);
    if (options.base_assignment_path != NULL) {
        base_assignment =
            read_assignment_file_as_base(options.base_assignment_path, &problem_data);
        problem_data.base_assignment = base_assignment;
        problem_data.change_penalty = options.change_penalty;
    } else if (options.change_penalty > 0LL) {
        free_problem_data(&problem_data);
        rank_cost_model_free(&options.rank_cost_model);
        if (constraints_are_loaded) {
            constraint_set_free(&constraints);
        }
        fail_with_context("--change-penalty", "requires --base-assignment");
    }
    validate_counterfactual_options(&problem_data, &options);
    read_end_clock = clock();
    profile.read_cpu_seconds = elapsed_cpu_seconds(program_start_clock, read_end_clock);
    solver_start_clock = clock();
    if (options.portfolio_mode) {
        assignment = solve_portfolio_problem(&problem_data,
                                             &options,
                                             argv[3],
                                             &portfolio_solver_seconds);
    } else {
        assignment = solve_assignment_for_mode(&problem_data,
                                               options.objective_mode,
                                               &options);
    }
    solver_end_clock = clock();
    profile.solver_cpu_seconds =
        portfolio_solver_seconds >= 0.0L ?
        portfolio_solver_seconds :
        elapsed_cpu_seconds(solver_start_clock, solver_end_clock);
    if (!target_constraints_are_empty(&options.targets)) {
        EvaluationMetrics target_metrics =
            compute_evaluation_metrics(&problem_data, assignment);
        if (!required_targets_are_satisfied(&problem_data,
                                            assignment,
                                            &target_metrics)) {
            free_evaluation_metrics(&target_metrics);
            free(assignment);
            free_problem_data(&problem_data);
            rank_cost_model_free(&options.rank_cost_model);
            if (constraints_are_loaded) {
                constraint_set_free(&constraints);
            }
            free(base_assignment);
            fail_with_context("target constraints",
                              "No feasible solution: selected objective has no feasible solution satisfying all required targets");
        }
        free_evaluation_metrics(&target_metrics);
    }
    write_assignment_file(argv[3], &problem_data, assignment);

    if (options.explain_student_id != NULL || options.try_lock_text != NULL) {
        char *explanation_output_path = explanation_output_path_for(argv[3]);
        counterfactual_start_clock = clock();
        write_counterfactual_explanation_file(explanation_output_path,
                                              &problem_data,
                                              &options,
                                              assignment);
        counterfactual_end_clock = clock();
        profile.counterfactual_cpu_seconds =
            elapsed_cpu_seconds(counterfactual_start_clock,
                                counterfactual_end_clock);
        free(explanation_output_path);
    }

    report_start_clock = clock();
    if (options.write_reports) {
        EvaluationMetrics metrics = compute_evaluation_metrics(&problem_data, assignment);
        char *metrics_output_path = metrics_output_path_for(argv[3]);
        char *lab_report_output_path = lab_report_output_path_for(argv[3]);
        char *student_report_output_path = student_report_output_path_for(argv[3]);
        char *outside_report_output_path = outside_report_output_path_for(argv[3]);
        char *reasons_report_output_path = reasons_report_output_path_for(argv[3]);
        char *adjustment_report_output_path = NULL;
        char *target_status_output_path = NULL;
        clock_t before_metrics_clock;
        metrics.solver_cpu_seconds =
            portfolio_solver_seconds >= 0.0L ?
            portfolio_solver_seconds :
            elapsed_cpu_seconds(solver_start_clock, solver_end_clock);
        metrics.counterfactual_cpu_seconds = profile.counterfactual_cpu_seconds;
        before_metrics_clock = clock();
        metrics.program_cpu_seconds_before_metrics =
            elapsed_cpu_seconds(program_start_clock, before_metrics_clock);
        write_lab_report_file(lab_report_output_path,
                              &problem_data,
                              assignment,
                              &metrics);
        write_student_report_file(student_report_output_path,
                                  &problem_data,
                                  assignment);
        write_outside_preferences_report_file(outside_report_output_path,
                                              &problem_data,
                                              assignment);
        write_reasons_report_file(reasons_report_output_path,
                                  &problem_data,
                                  assignment,
                                  &metrics);
        if (!target_constraints_are_empty(&options.targets)) {
            target_status_output_path = target_status_output_path_for(argv[3]);
            write_target_status_file(target_status_output_path,
                                     &problem_data,
                                     assignment,
                                     &metrics);
        }
        if (problem_data.base_assignment != NULL) {
            adjustment_report_output_path = adjustment_report_output_path_for(argv[3]);
            write_adjustment_report_file(adjustment_report_output_path,
                                         &problem_data,
                                         assignment,
                                         &metrics);
        }
        write_metrics_file(metrics_output_path, &problem_data, assignment, &metrics);
        free_evaluation_metrics(&metrics);
        free(metrics_output_path);
        free(lab_report_output_path);
        free(student_report_output_path);
        free(outside_report_output_path);
        free(reasons_report_output_path);
        free(adjustment_report_output_path);
        free(target_status_output_path);
    }
    report_end_clock = clock();
    profile.report_cpu_seconds = elapsed_cpu_seconds(report_start_clock, report_end_clock);
    profile.total_cpu_seconds = elapsed_cpu_seconds(program_start_clock, report_end_clock);
    if (options.write_profile) {
        char *profile_output_path = profile_output_path_for(argv[3]);
        solver_profile_write(profile_output_path, &profile);
        free(profile_output_path);
    }
    print_success_summary(argv[3], &options);

    free(assignment);
    free(base_assignment);
    if (constraints_are_loaded) {
        constraint_set_free(&constraints);
    }
    rank_cost_model_free(&options.rank_cost_model);
    ungrouped_active_arc_template_clear(&ungrouped_active_arc_template_cache);
    free_problem_data(&problem_data);
    active_profile = NULL;
    return EXIT_SUCCESS;
}
