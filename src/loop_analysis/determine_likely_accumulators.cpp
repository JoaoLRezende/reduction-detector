#include "../constants.h"
#include "loop_analysis.h"
#include "internal.h"

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

  score += std::lround(
      ACCUMULATOR_REFERENCE_IN_RHS_BONUS *
      (double)possibleAccumulator
          .number_of_possible_accumulating_assignments_whose_RHS_also_references_this /
      possibleAccumulator.possible_accumulating_assignments.size());

  score -= possibleAccumulator.outside_references * OUTSIDE_REFERENCE_PENALTY;

  if (possibleAccumulator.notableNameSubstring)
    score += COMMON_ACCUMULATOR_NAME_BONUS;

  return score;
}

void determineLikelyAccumulatorsIn(PossibleReductionLoopInfo &loopInfo,
                                   clang::ASTContext *context) {
  for (auto &possibleAccumulator : loopInfo.possible_accumulators) {
    int score = computeLikelyAccumulatorScore(possibleAccumulator.second);
    possibleAccumulator.second.likelyAccumulatorScore = score;

    bool isLikelyAccumulator = score >= accumulatorScoreThreshold;
    possibleAccumulator.second.isLikelyAccumulator = isLikelyAccumulator;
    if (isLikelyAccumulator)
      loopInfo.hasALikelyAccumulator = true;
  }
}
}
}
}
