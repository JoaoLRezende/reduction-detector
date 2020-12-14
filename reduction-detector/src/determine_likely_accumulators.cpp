#include "reduction-detector/constants.h"
#include "reduction-detector/loop_analysis.h"

#include "llvm/Support/CommandLine.h"

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

// accumulatorScoreThreshold stores the minimum score threshold set by the
// user in the command line.
static llvm::cl::opt<int> accumulatorScoreThreshold(
    "min-score", llvm::cl::desc("Specify minimum \"likely-accumulator score\" for a "
                          "variable to be considered a potential accumulator"),
    llvm::cl::value_desc("integer"), llvm::cl::init(DEFAULT_LIKELY_ACCUMULATOR_THRESHOLD));

static int
computeLikelyAccumulatorScore(PotentialAccumulatorInfo &potentialAccumulator) {
  int score = INITIAL_ACCUMULATOR_SCORE;

  score -= potentialAccumulator.outside_references * OUTSIDE_REFERENCE_PENALTY;

  if (potentialAccumulator.notableNameSubstring)
    score += COMMON_ACCUMULATOR_NAME_BONUS;

  return score;
}

void determineLikelyAccumulatorsIn(PotentialReductionLoopInfo &loopInfo,
                                   clang::ASTContext *context) {
  for (auto &potentialAccumulator : loopInfo.potential_accumulators) {
    int score = computeLikelyAccumulatorScore(potentialAccumulator.second);
    potentialAccumulator.second.likelyAccumulatorScore = score;

    bool isLikelyAccumulator = score >= accumulatorScoreThreshold;
    potentialAccumulator.second.isLikelyAccumulator = isLikelyAccumulator;
    if (isLikelyAccumulator)
      loopInfo.hasALikelyAccumulator = true;
  }
}
}
}
}
