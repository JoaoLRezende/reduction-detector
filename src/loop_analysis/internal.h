#ifndef LOOP_ANALYSIS_INTERNAL_H
#define LOOP_ANALYSIS_INTERNAL_H

#include "clang/AST/Stmt.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <string>

namespace reduction_detector {
namespace loop_analysis {
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

/* * * *
 * Analysis passes
 * * * */

// Determine the loop's iteration variable, if there is one, and set
// loopInfo.iteration_variable.
void determineIterationVariable(PossibleReductionLoopInfo &loopInfo,
                                clang::ASTContext *context);

// If we have identified the loop's iteration variable, then determine how many
// times it is referenced in array-subscript expressions
// (for example, array[i]). Also, for each array, determine how many
// times that array is subscripted by an expression that involves the iteration
// variable. Update the corresponding fields of loopInfo.
void detectIterationVariableReferencesInArraySubscripts(
    PossibleReductionLoopInfo &loopInfo, clang::ASTContext *context);

// Find possible accumulators, populating loopInfo.possible_accumulators.
void getPossibleAccumulatorsIn(PossibleReductionLoopInfo *loop_info,
                               clang::ASTContext *context);

// For each possible accumulating assignment in the loop, determine whether the
// variable in its left-hand side (which is a possible accumulator) also appears
// in its right-hand side (like in acc = acc + array[i]) or the
// assignment is a compound assignment (like acc += array[i]).
// Update the instances of PossibleAccumulatingAssignmentInfo that describe
// those assignments accordingly.
void detectPossibleAccumulatorReferencesInRHSOfPossibleAccumulatingStatements(
    PossibleReductionLoopInfo &loop_info, clang::ASTContext *context);

// For each possible accumulator, count the number of times it is referenced
// in that loop outside of its possible accumulating assignments.
// Store that number in the instance of PossibleAccumulatorInfo that
// describes that possible accumulator.
void countOutsideReferencesIn(PossibleReductionLoopInfo &loop_info,
                              clang::ASTContext *context);

// If we have identified the loop's iteration variable, then, for each possible
// accumulating assignment in the loop, determine whether the loop's iteration
// variable is referenced in it.
void detectIterationVariableReferencesInPossibleAccumulatingStatements(
    PossibleReductionLoopInfo &loopInfo, clang::ASTContext *context);

// Check whether each possible accumulator's name contains one of the
// strings in COMMON_ACCUMULATOR_NAME_SUBSTRINGS as a substring.
void analysePossibleAccumulatorNames(PossibleReductionLoopInfo &loop_info,
                                     clang::ASTContext *context);


/* Decide whether each possible accumulator is a likely accumulator.
 * Set each possible accumulator's likelyAccumulatorScore and
 * isLikelyAccumulator.
 */
void determineLikelyAccumulatorsIn(PossibleReductionLoopInfo &loop_info,
                                   clang::ASTContext *context);

/*
 * Update the statistics stored in LoopAnalyser to account for a
 * newly analyzed loop.
 */
void registerAnalyzedLoop(LoopAnalyser &loopAnalyser,
                          PossibleReductionLoopInfo &loopInfo);
}
}
}

#endif