#ifndef LOOP_ANALYSIS_H
#define LOOP_ANALYSIS_H

#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

namespace reduction_detector {
namespace loop_analysis {

/*
 * LoopAnalyser is the entry point into the loop-analysis subsystem.
 * It is to be used with clang::ast_matchers::MatchFinder.
 * Only one instance of LoopAnalyser is to be created per execution.
 * It checks each for loop it is called upon, accumulating statistics.
 */
class LoopAnalyser : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  unsigned int likelyReductionCount = 0;
  unsigned int totalLoopCount = 0;

  // run is called by MatchFinder for each for loop.
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result);
};

namespace internal {

// Fundamental struct types. These are manipulated by most
// analysis passes.
struct PotentialAccumulatorInfo {
  std::set<const clang::BinaryOperator *> potential_accumulating_assignments;

  /* The number of references to this variable outside of assignments
   * that change its value.
   */
  unsigned int outside_references = 0;

  int likelyAccumulatorScore = 0;
  bool isLikelyAccumulator = false;
};

struct PotentialReductionLoopInfo {
  const clang::ForStmt *forStmt = nullptr;
  clang::VarDecl *iteration_variable = nullptr;
  std::map<const clang::VarDecl *, PotentialAccumulatorInfo>
      potential_accumulators;
  bool hasALikelyAccumulator = false;

  PotentialReductionLoopInfo(const clang::ForStmt *forStmt)
      : forStmt(forStmt){};
  void dump(llvm::raw_ostream &outputStream, clang::ASTContext *context);
};

// Analysis passes.
void countOutsideReferencesIn(PotentialReductionLoopInfo *loop_info,
                              clang::ASTContext *context);
void getPotentialAccumulatorsIn(PotentialReductionLoopInfo *loop_info,
                                clang::ASTContext *context);
void determineLikelyAccumulatorsIn(PotentialReductionLoopInfo &loop_info,
                                   clang::ASTContext *context);
}
}
}

#endif
