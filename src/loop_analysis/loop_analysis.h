#ifndef LOOP_ANALYSIS_H
#define LOOP_ANALYSIS_H

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
  unsigned int likelyReductionCount = 0;
  unsigned int totalLoopCount = 0;

  // run is called by MatchFinder for each for loop.
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result);
};

namespace internal {

/* * * *
 * Fundamental struct types. These are manipulated by most
 * analysis passes.
 * * * */

// One instance of PossibleAccumulatingAssignmentInfo describes each
// assignment whose left-hand side is a possible accumulator.
struct PossibleAccumulatingAssignmentInfo {
  bool rhs_also_references_possible_accumulator = false;
  bool references_iteration_variable = false;
};

// A possible accumulator is a variable whose declaration is outside of a
// loop, but is the left side of an assignment in that loop's body.
struct PossibleAccumulatorInfo {

  std::map<const clang::BinaryOperator *, PossibleAccumulatingAssignmentInfo>
      possible_accumulating_assignments;

  unsigned int
      number_of_possible_accumulating_assignments_whose_RHS_also_references_this =
          0;

  unsigned int
      number_of_possible_accumulating_assignments_that_reference_the_iteration_variable =
          0;

  // If not null, notableNameSubstring points to a string constant.
  std::string *notableNameSubstring = nullptr;

  /* The number of in-loop references to this variable outside of
   * its possible accumulating assignments.
   */
  unsigned int outside_references = 0;

  int likelyAccumulatorScore = 0;
  bool isLikelyAccumulator = false;
};

struct PossibleReductionLoopInfo {
  const clang::Stmt *loopStmt = nullptr;

  // iteration_variable might receive a non-null value only if this is a for
  // loop.
  const clang::VarDecl *iteration_variable = nullptr;
  int numberOfIterationVariableReferencesInArraySubscripts = 0;
  std::map<const clang::VarDecl *, int>
      numberOfArrayAccessesInvolvingIterationVariablePerArray;

  std::map<const clang::VarDecl *, PossibleAccumulatorInfo>
      possible_accumulators;

  bool hasALikelyAccumulator = false;

  PossibleReductionLoopInfo(const clang::Stmt *loopStmt) : loopStmt(loopStmt){};
  void dump(llvm::raw_ostream &outputStream, clang::ASTContext *context);
};

// Analysis passes.
void getPossibleAccumulatorsIn(PossibleReductionLoopInfo *loop_info,
                               clang::ASTContext *context);
void detectPossibleAccumulatorReferencesInRHSOfPossibleAccumulatingStatements(
    PossibleReductionLoopInfo &loop_info, clang::ASTContext *context);
void countOutsideReferencesIn(PossibleReductionLoopInfo &loop_info,
                              clang::ASTContext *context);
void analysePossibleAccumulatorNames(PossibleReductionLoopInfo &loop_info,
                                     clang::ASTContext *context);
void determineIterationVariable(PossibleReductionLoopInfo &loopInfo,
                                clang::ASTContext *context);
void detectIterationVariableReferencesInApparentAccumulatingStatements(
    PossibleReductionLoopInfo &loopInfo, clang::ASTContext *context);
void determineLikelyAccumulatorsIn(PossibleReductionLoopInfo &loop_info,
                                   clang::ASTContext *context);
void detectIterationVariableReferencesInArraySubscripts(
    PossibleReductionLoopInfo &loopInfo, clang::ASTContext *context);
}
}
}

#endif
