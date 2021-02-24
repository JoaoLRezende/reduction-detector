/*
 * This header file describes the public interface to the loop-analysis
 * subsystem. For declarations of the data types and functions used by
 * the subsystem internally, see internal.h.
 */

#ifndef LOOP_ANALYSIS_H
#define LOOP_ANALYSIS_H

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <string>

namespace reduction_detector {
namespace loop_analysis {

// The AST matcher to be used with an instance of LoopAnaluser as its callback
// object.
extern clang::ast_matchers::StatementMatcher loopMatcher;

/*
 * LoopAnalyser is the entry point into the loop-analysis subsystem.
 * Only one instance of LoopAnalyser is to be created per execution.
 * It is to be used as the callback for loopMatcher
 * with clang::ast_matchers::MatchFinder.
 * It checks each for loop it is called upon, accumulating statistics.
 */
class LoopAnalyser : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  struct {
    struct LoopCounts {
      unsigned int all = 0;
      unsigned int forLoops = 0;
      unsigned int whileLoops = 0;
      unsigned int doWhileLoops = 0;
    } totals, likelyReductionLoops;
  } loopCounts;

  // run is called by MatchFinder for each for loop.
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result);
};
}
}

#endif
