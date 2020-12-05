#include "reduction-detector/loop_analysis.h"

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

// test stub. TODO.
static bool
isLikelyAccumulator(PotentialAccumulatorInfo &potentialAccumulator) {
  return true;
}

void determineLikelyAccumulatorsIn(PotentialReductionLoopInfo &loopInfo,
                                   clang::ASTContext *context) {
  for (auto &potentialAccumulator : loopInfo.potential_accumulators) {
    potentialAccumulator.second.isLikelyAccumulator =
        isLikelyAccumulator(potentialAccumulator.second);
    if (potentialAccumulator.second.isLikelyAccumulator)
    loopInfo.hasALikelyAccumulator = true;
  }
}
}
}
}
