#include "loop_analysis.h"
#include "internal.h"

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

/*
 * Update the statistics stored in LoopAnalyser to account for a
 * newly analyzed loop.
 */
void registerAnalyzedLoop(LoopAnalyser &loopAnalyser,
                                 PossibleReductionLoopInfo &loopInfo) {
  loopAnalyser.loopCounts.totals.all += 1;
  if (loopInfo.hasALikelyAccumulator) {
    loopAnalyser.loopCounts.likelyReductionLoops.all += 1;
  }

  if (clang::isa<clang::ForStmt>(loopInfo.loopStmt)) {
    loopAnalyser.loopCounts.totals.forLoops += 1;
    if (loopInfo.hasALikelyAccumulator) {
      loopAnalyser.loopCounts.likelyReductionLoops.forLoops += 1;
    }
  } else if (clang::isa<clang::WhileStmt>(loopInfo.loopStmt)) {
    loopAnalyser.loopCounts.totals.whileLoops += 1;
    if (loopInfo.hasALikelyAccumulator) {
      loopAnalyser.loopCounts.likelyReductionLoops.whileLoops += 1;
    }
  } else if (clang::isa<clang::DoStmt>(loopInfo.loopStmt)) {
    loopAnalyser.loopCounts.totals.doWhileLoops += 1;
    if (loopInfo.hasALikelyAccumulator) {
      loopAnalyser.loopCounts.likelyReductionLoops.doWhileLoops += 1;
    }
  }
}

}
}
}
