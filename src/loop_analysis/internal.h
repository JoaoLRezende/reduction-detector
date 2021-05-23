#ifndef LOOP_ANALYSIS_INTERNAL_H
#define LOOP_ANALYSIS_INTERNAL_H

#include "loop_analysis.h"

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

// A _possible accumulator_ is an l-value that is the left-hand operand of an
// assignment in a loop's body and whose _base_ (the earliest identifier that
// appears in it) is declared outside of that loop. For example, a possible
// accumulator can be an array-subscript expression like arr[i], or a
// member-access expression like stats.sum (where stats is a structure with a
// field named "sum").
struct PossibleAccumulatorInfo {
  // A pointer to the first encountered occurrence of the possible
  // accumulator in the left-hand side of an assignment.
  const clang::Expr *possibleAccumulator;

  // base is the earliest declaration-reference expression that exists in the
  // first occurrence of the possible accumulator.
  const clang::DeclRefExpr *base;

  // textual representation of the possible accumulator (unparsed from
  // possibleAccumulator)
  std::string name;

  PossibleAccumulatorInfo(const clang::Expr *possibleAccumulator,
                          const clang::DeclRefExpr *base)
      : possibleAccumulator(possibleAccumulator), base(base) {
    // Unparse the possible accumulator into this->name.
    llvm::raw_string_ostream nameStringStream(this->name);
    this->possibleAccumulator->printPretty(
        nameStringStream, nullptr, clang::PrintingPolicy(clang::LangOptions()));
    nameStringStream.flush();
  };

  std::map<const clang::BinaryOperator *, PossibleAccumulatingAssignmentInfo>
      possible_accumulating_assignments;

  int declarationDistanceFromLoopInLines = -1;

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

  /* We call a "trivial accumulator" an accumulator that does not appear
   * anywhere in the loop outside of its accumulating assignments, and is
   * referenced in the right-hand side of all of its accumulating assignments.
   * Thus: a trivial accumulator is one that would likely be identified as a
   * reduction accumulator by Cetus as described in section 2.2.6 of "The Cetus
   * source-to-source compiler infrastructure: overview and evaluation" (2013).
   */
  bool is_trivial_accumulator = false;

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

  /* Each possible accumulator is identified by its llvm::FoldingSetNodeID
   * (something like a hash code). Note that a possible accumulator, which
   * is an expression, can be, for example, a bare declaration-reference
   * expression (i.e. a reference to a bare variable), an array-subscript
   * expression, a member-access expression, or a pointer-dereference
   * expression. Every occurrence of an expression has a same
   * llvm::FoldingSetNodeID. Thus, multiple occurrences of a possible
   * accumulator will be mapped to one same PossibleAccumulatorInfo as long as
   * they are identical. (But, for example, array[0] will not be detected as the
   * same as array[1-1], and *(&ptr) will not be detected as the
   * same as ptr.) TODO: detail here how that hash value is acquired for
   * an expression.
   */
  std::map<llvm::FoldingSetNodeID, PossibleAccumulatorInfo>
      possible_accumulators;

  bool hasALikelyAccumulator = false;
  bool has_a_trivial_accumulator = false;
  bool has_likely_but_non_trivial_accumulator = false;

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

// If we have identified the loop's iteration variable, then determine how
// many
// times it is referenced in array-subscript expressions
// (for example, array[i]). Also, for each array, determine how many
// times that array is subscripted by an expression that involves the
// iteration
// variable. Update the corresponding fields of loopInfo.
void detectIterationVariableReferencesInArraySubscripts(
    PossibleReductionLoopInfo &loopInfo, clang::ASTContext *context);

// Find possible accumulators, populating loopInfo.possible_accumulators.
void getPossibleAccumulatorsIn(PossibleReductionLoopInfo *loop_info,
                               clang::ASTContext *context);

// For each possible accumulator, populate the member
// declarationDistanceFromLoopInLines of the structure that descibes that
// possible accumulator.
void getDistanceOfDeclarationOfPossibleAccumulators(
    PossibleReductionLoopInfo &loop_info, clang::ASTContext *context);

// For each possible accumulating assignment in the loop, determine whether
// the
// variable in its left-hand side (which is a possible accumulator) also
// appears
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

// If we have identified the loop's iteration variable, then, for each
// possible
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

/* Determine which possible accumulators are trivial accumulators, and update
 * the corresponding fields of loop_info and of the relevant
 * PossibleAccumulatorInfos.
*/
void determineTrivialAccumulators(PossibleReductionLoopInfo &loop_info);
}
}
}

#endif