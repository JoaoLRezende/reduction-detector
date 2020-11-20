#ifndef LOOP_ANALYSIS_H
#define LOOP_ANALYSIS_H

#include "clang/ASTMatchers/ASTMatchFinder.h"

namespace reduction_detector {
namespace loop_analysis {

/*
 * Only one instance of LoopAnalyser is created per execution.
 * It checks every for loop, accumulating statistics.
 */
class LoopAnalyser : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  unsigned int likelyReductionCount = 0;
  unsigned int totalLoopCount = 0;

  // run is called by MatchFinder for each for loop.
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result);

};
}
}

#endif

