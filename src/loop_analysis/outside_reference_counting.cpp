#include "loop_analysis.h"

#include "clang/AST/ASTContext.h"

#include "clang/ASTMatchers/ASTMatchers.h"
using namespace clang::ast_matchers;

#include "clang/ASTMatchers/ASTMatchFinder.h"
using clang::ast_matchers::MatchFinder;

#include "../accumulating_assignment_matchers.h"
using reduction_detector::reduction_assignment_matchers::
    reductionAssignmentMatcher;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

static bool isDeclRefInExpression(const clang::DeclRefExpr *declRef,
                                  const clang::Expr *expression,
                                  clang::ASTContext *context) {
  /* Construct a matcher that will match declRef only if it is a descendant
   * of expression.
   */
  StatementMatcher declRefMatcher =
      declRefExpr(hasAncestor(expr(equalsNode(expression))));

  // Instantiate a struct to be used as a MatchFinder callback.
  struct MatcherCallback : public MatchFinder::MatchCallback {
    bool wasCalled = false;
    virtual void run(const MatchFinder::MatchResult &result) {
      wasCalled = true;
    }
  } matcherCallback;

  MatchFinder matchFinder;
  matchFinder.addMatcher(declRefMatcher, &matcherCallback);
  matchFinder.match(*declRef, *context);
  return matcherCallback.wasCalled;
}

static bool
isReferenceInOneOfPossibleAccumulatingAssignmentsOfPossibleAccumulator(
    const clang::DeclRefExpr *declRef,
    PossibleAccumulatorInfo &possibleAccumulator, clang::ASTContext *context) {
  for (auto &possibleAccumulatingAssignment :
       possibleAccumulator.possible_accumulating_assignments) {
    if (isDeclRefInExpression(declRef, possibleAccumulatingAssignment.first,
                              context)) {
      return true;
    }
  }
  return false;
}

/*
 * For each possible accumulator, count the number of references to it
 * that exist in the body of the loop but outside of the assignments
 * that seem to accumulate a value in that possible accumulator.
 */
void countOutsideReferencesIn(PossibleReductionLoopInfo &loop_info,
                              clang::ASTContext *context) {
  for (auto &possibleAccumulator : loop_info.possible_accumulators) {
    // Instantiate a struct to be used as a MatchFinder callback for all in-loop
    // references to possibleAccumulator.
    struct MatcherCallback : public MatchFinder::MatchCallback {
      decltype(possibleAccumulator) possibleAccumulator_;
      unsigned int outsideReferences = 0;

      MatcherCallback(decltype(possibleAccumulator) possibleAccumulator_)
          : possibleAccumulator_(possibleAccumulator_) {}

      virtual void run(const MatchFinder::MatchResult &result) {
        if (!isReferenceInOneOfPossibleAccumulatingAssignmentsOfPossibleAccumulator(
                result.Nodes.getNodeAs<clang::DeclRefExpr>(
                    "possibleAccumulatorReference"),
                possibleAccumulator_.second, result.Context)) {
          outsideReferences += 1;
        }
      }

    } matcherCallback(possibleAccumulator);

    // Count number of outside references.
    MatchFinder referenceFinder;
    referenceFinder.addMatcher(
        findAll(declRefExpr(to(varDecl(equalsNode(possibleAccumulator.first))))
            .bind("possibleAccumulatorReference")),
        &matcherCallback);
    referenceFinder.match(*loop_info.loopStmt, *context);

    // Get the resulting count of outside references.
    possibleAccumulator.second.outside_references =
        matcherCallback.outsideReferences;
  }
}
}
}
}
