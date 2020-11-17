#ifndef LOOP_ANALYSIS_H
#define LOOP_ANALYSIS_H

#include "clang/ASTMatchers/ASTMatchFinder.h"

namespace reduction_detector {
namespace loop_analysis {

/*
 * One instance of LoopChecker is created for each translation unit.
 * It checks every for loop in that file, accumulating statistics.
 * TODO: is this true?
 */
class LoopChecker : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  std::string loop_analysis_report_buffer;

  unsigned int likelyReductionCount = 0;
  unsigned int totalLoopCount = 0;

  LoopChecker();

  // run is called by MatchFinder for each for loop.
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result);

};
}
}

#endif

