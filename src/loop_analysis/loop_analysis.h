#ifndef LOOP_ANALYSIS_H
#define LOOP_ANALYSIS_H

#include "internal.h"

#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include <string>

namespace reduction_detector {
namespace loop_analysis {

/*
 * LoopAnalyser is the entry point into the loop-analysis subsystem.
 * It is to be used as a callback with clang::ast_matchers::MatchFinder.
 * Only one instance of LoopAnalyser is to be created per execution.
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
    };
    LoopCounts totals;
    LoopCounts likelyReductionLoops;
  } loopCounts;

  // run is called by MatchFinder for each for loop.
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result);

private:
  void registerAnalyzedLoop(internal::PossibleReductionLoopInfo &loopInfo);
};
}
}

#endif
