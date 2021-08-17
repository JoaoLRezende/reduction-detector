#include "internal.h"
#include "loop_analysis.h"

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
  if (loopInfo.has_a_likely_accumulator) {
    loopAnalyser.loopCounts.likelyReductionLoops.all += 1;
  }
  if (loopInfo.has_a_trivial_accumulator) {
    loopAnalyser.loopCounts.trivialReductionLoops.all += 1;
  }

  if (clang::isa<clang::ForStmt>(loopInfo.loop_stmt)) {
    loopAnalyser.loopCounts.totals.forLoops += 1;
    if (loopInfo.has_a_likely_accumulator) {
      loopAnalyser.loopCounts.likelyReductionLoops.forLoops += 1;
    }
    if (loopInfo.has_a_trivial_accumulator) {
      loopAnalyser.loopCounts.trivialReductionLoops.forLoops += 1;
    }

  } else if (clang::isa<clang::WhileStmt>(loopInfo.loop_stmt)) {
    loopAnalyser.loopCounts.totals.whileLoops += 1;
    if (loopInfo.has_a_likely_accumulator) {
      loopAnalyser.loopCounts.likelyReductionLoops.whileLoops += 1;
    }
    if (loopInfo.has_a_trivial_accumulator) {
      loopAnalyser.loopCounts.trivialReductionLoops.whileLoops += 1;
    }

  } else if (clang::isa<clang::DoStmt>(loopInfo.loop_stmt)) {
    loopAnalyser.loopCounts.totals.doWhileLoops += 1;
    if (loopInfo.has_a_likely_accumulator) {
      loopAnalyser.loopCounts.likelyReductionLoops.doWhileLoops += 1;
    }
    if (loopInfo.has_a_trivial_accumulator) {
      loopAnalyser.loopCounts.trivialReductionLoops.doWhileLoops += 1;
    }
  }
}
}
}
}
