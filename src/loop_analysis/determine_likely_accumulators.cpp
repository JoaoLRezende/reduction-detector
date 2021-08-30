#include "../constants.h"
#include "internal.h"
#include "loop_analysis.h"

#include "../command_line.h"

#include "llvm/Support/CommandLine.h"

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

// accumulatorScoreThreshold stores the minimum score threshold set by the
// user in the command line.
static llvm::cl::opt<int> accumulatorScoreThreshold(
    "min-score",
    llvm::cl::desc("Specify minimum accumulator-likelihood score for a "
                   "variable to be considered a likely accumulator"),
    llvm::cl::value_desc("integer"),
    llvm::cl::init(DEFAULT_LIKELY_ACCUMULATOR_THRESHOLD),
    llvm::cl::cat(command_line_options::reduction_detector_option_category));

static int
computeLikelyAccumulatorScore(PossibleAccumulatorInfo &possibleAccumulator) {
  int score = INITIAL_ACCUMULATOR_LIKELIHOOD_SCORE;

  if (possibleAccumulator.declaration_distance_from_loop_in_lines <=
      MAX_DISTANCE_IN_LINES_BETWEEN_DECLARATION_AND_LOOP_FOR_LOCALITY_BONUS) {
    score += DECLARATION_LOCALITY_BONUS;
  }

  score += std::lround(
      ACCUMULATOR_REFERENCE_IN_RHS_BONUS *
      (double)possibleAccumulator
          .number_of_possible_accumulating_assignments_whose_RHS_also_references_this /
      possibleAccumulator.possible_accumulating_assignments.size());

  score -= possibleAccumulator.outside_references * OUTSIDE_REFERENCE_PENALTY;

  if (possibleAccumulator.notable_name_substring)
    score += COMMON_ACCUMULATOR_NAME_BONUS;

  if (possibleAccumulator.is_apparently_unused_after_loop) {
    score -= UNREAD_POSSIBLE_ACCUMULATOR_PENALTY;
  }

  return score;
}

void determineLikelyAccumulatorsIn(PossibleReductionLoopInfo &loopInfo,
                                   clang::ASTContext *context) {
  for (auto &possibleAccumulator : loopInfo.possible_accumulators) {
    int score = computeLikelyAccumulatorScore(possibleAccumulator.second);
    possibleAccumulator.second.likely_accumulator_score = score;

    bool isLikelyAccumulator = score >= accumulatorScoreThreshold;
    possibleAccumulator.second.is_likely_accumulator = isLikelyAccumulator;
    if (isLikelyAccumulator)
      loopInfo.has_a_likely_accumulator = true;
  }
}
}
}
}
