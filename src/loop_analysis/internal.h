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
  const clang::Expr *possible_accumulator;

  // base is the earliest declaration-reference expression that exists in the
  // first occurrence of the possible accumulator.
  const clang::DeclRefExpr *base;

  // textual representation of the possible accumulator (unparsed from
  // possible_accumulator)
  std::string name;

  PossibleAccumulatorInfo(const clang::Expr *possible_accumulator,
                          const clang::DeclRefExpr *base)
      : possible_accumulator(possible_accumulator), base(base),
        is_local_variable(!this->base->getType()->isPointerType() &&
                          llvm::cast<clang::VarDecl>(this->base->getDecl())
                              ->isLocalVarDeclOrParm()) {
    // Unparse the possible accumulator into this->name.
    llvm::raw_string_ostream nameStringStream(this->name);
    this->possible_accumulator->printPretty(
        nameStringStream, nullptr, clang::PrintingPolicy(clang::LangOptions()));
    nameStringStream.flush();
  };

  std::map<const clang::BinaryOperator *, PossibleAccumulatingAssignmentInfo>
      possible_accumulating_assignments;

  int declaration_distance_from_loop_in_lines = -1;

  unsigned int
      number_of_possible_accumulating_assignments_whose_RHS_also_references_this =
          0;

  unsigned int
      number_of_possible_accumulating_assignments_that_reference_the_iteration_variable =
          0;

  // is_local_variable is true if the base of this possible
  // accumulator references a variable that is local to the function that
  // contains this loop and is not a pointer.
  bool is_local_variable;

  const clang::Expr *first_reference_after_loop = nullptr;

  // is_apparently_unused_after_loop is true either if this possible accumulator
  // is a local variable that is never read after the loop (which means that the
  // value acquired by the variable in the loop is never consulted, and thus
  // probably isn't meaningful at all) or if first_reference_after_loop is a
  // write reference. (Note that this is false when the possible accumulator is
  // not a local variable and is not referenced after the loop in the function.
  // In such a case, the variable still can be consulted outside of the
  // function.)
  bool is_apparently_unused_after_loop = false;

  /* is_dereference_of_inconstant_pointer is true if the base of this possible
   * accumulator references a pointer that is the left-hand side of an
   * assignment expression in the loop. This means that that pointer
   * might point to a different object in each iteration, and that thus this
   * possible accumulator actually represents a different object in each
   * iteration.
   */
  bool is_dereference_of_inconstant_pointer = false;

  // If not null, notable_name_substring points to a string constant.
  std::string *notable_name_substring = nullptr;

  /* The number of in-loop references to this variable outside of
   * its possible accumulating assignments.
   */
  unsigned int outside_references = 0;

  /* We call a "trivial accumulator" a possible accumulator for which all of the
   * following are true:
   * (1) It does not appear anywhere in the loop outside of its accumulating
   * assignments.
   * (2) It is referenced in the right-hand side of all of its accumulating
   * assignments.
   * (3) It is either a declaration-reference expression or an array-subscript
   * expression.
   * (4) It appears in a for loop (rather than in a while or do-while loop).
   * Thus: a trivial accumulator is one that would likely be identified as a
   * reduction accumulator by Cetus 1.4.4, as described in section 2.2.6 of "The
   * Cetus source-to-source compiler infrastructure: overview and evaluation"
   * (2013).
   */
  bool is_trivial_accumulator = false;

  int likely_accumulator_score = 0;
  bool is_likely_accumulator = false;
};

struct PossibleReductionLoopInfo {
  const clang::Stmt *loop_stmt = nullptr;

  // iteration_variable might receive a non-null value only if this is a for
  // loop.
  const clang::VarDecl *iteration_variable = nullptr;
  int number_of_iteration_variable_references_in_array_subscripts = 0;
  std::map<const clang::VarDecl *, int>
      number_of_array_accesses_involving_iteration_variable_per_array;

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

  bool has_a_likely_accumulator = false;
  bool has_a_trivial_accumulator = false;
  bool has_likely_but_non_trivial_accumulator = false;

  PossibleReductionLoopInfo(const clang::Stmt *loopStmt)
      : loop_stmt(loopStmt){};
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
// declaration_distance_from_loop_in_lines of the structure that descibes that
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

// Populate each possible accumulator's is_dereference_of_inconstant_pointer.
void seeWhetherPossibleAccumulatorsAreDereferencesOfInconstantPointers(
    PossibleReductionLoopInfo &loop_info, clang::ASTContext *context);

// For each possible accumulator, determine whether it is read after the loop.
// That is: determine whether the value it acquires in the loop is used after
// the loop at all. Populate the following members of each possible accumulator:
// first_reference_after_loop, is_read_after_loop.
void checkWhetherPossibleAccumulatorsAreReadAfterLoop(
    PossibleReductionLoopInfo &loop_info, clang::ASTContext &context);

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
 * Set each possible accumulator's likely_accumulator_score and
 * is_likely_accumulator.
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

} // namespace internal
} // namespace loop_analysis
} // namespace reduction_detector

#endif