#include "internal.h"
#include "loop_analysis.h"

#include "clang/AST/ASTContext.h"

#include "clang/ASTMatchers/ASTMatchers.h"
using namespace clang::ast_matchers;

#include "clang/ASTMatchers/ASTMatchFinder.h"
using clang::ast_matchers::MatchFinder;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

// Check whether expression1 is a descendant of expression2.
static bool isExpressionInExpression(const clang::Expr *expression1,
                                     const clang::Expr *expression2,
                                     clang::ASTContext *context) {
  /* Construct a matcher that will match expression1 only if it is a descendant
   * of expression2.
   */
  StatementMatcher expression1Matcher =
      expr(hasAncestor(expr(equalsNode(expression2))));

  // Instantiate a struct to be used as a MatchFinder callback.
  struct MatcherCallback : public MatchFinder::MatchCallback {
    bool wasCalled = false;
    virtual void run(const MatchFinder::MatchResult &result) {
      wasCalled = true;
    }
  } matcherCallback;

  MatchFinder matchFinder;
  matchFinder.addMatcher(expression1Matcher, &matcherCallback);
  matchFinder.match(*expression1, *context);
  return matcherCallback.wasCalled;
}

static bool
isExpressionInOneOfPossibleAccumulatingAssignmentsOfPossibleAccumulator(
    const clang::Expr *expression, PossibleAccumulatorInfo &possibleAccumulator,
    clang::ASTContext *context) {
  for (auto &possibleAccumulatingAssignment :
       possibleAccumulator.possible_accumulating_assignments) {
    if (isExpressionInExpression(
            expression, possibleAccumulatingAssignment.first, context)) {
      return true;
    }
  }
  return false;
}

static clang::ast_matchers::internal::BindableMatcher<clang::Stmt>
getMatcherForSpecificExpressionType(const clang::Expr *expression) {
  if (llvm::isa<clang::DeclRefExpr>(expression))
    return clang::ast_matchers::declRefExpr();

  if (llvm::isa<clang::ArraySubscriptExpr>(expression))
    return clang::ast_matchers::arraySubscriptExpr();

  if (llvm::isa<clang::MemberExpr>(expression))
    return clang::ast_matchers::memberExpr();

  if (llvm::isa<clang::UnaryOperator>(expression))
    return clang::ast_matchers::unaryOperator();

  // I can't think of any l-value in C that isn't of any of the types above, so
  // the matchers above should be enough. But I might be wrong. In that case,
  // we check all expressions anyway:
  return expr();
}

/*
 * For each possible accumulator, count the number of times it
 * appears in the body of the loop but outside of the assignments
 * that seem to accumulate a value in that possible accumulator.
 */
void countOutsideReferencesIn(PossibleReductionLoopInfo &loop_info,
                              clang::ASTContext *context) {
  for (auto &possibleAccumulator : loop_info.possible_accumulators) {
    // Instantiate a struct to be used as a MatchFinder callback.
    // We'll match every expression and compare its folding-set node ID
    // with the folding-set node ID of possibleAccumulator.
    struct MatcherCallback : public MatchFinder::MatchCallback {
      decltype(possibleAccumulator) _possibleAccumulator;
      unsigned int outsideReferences = 0;

      MatcherCallback(decltype(possibleAccumulator) _possibleAccumulator)
          : _possibleAccumulator(_possibleAccumulator) {}

      virtual void run(const MatchFinder::MatchResult &result) {
        const clang::Expr *thisExpression =
            result.Nodes.getNodeAs<clang::Expr>("expression");

        // getNodeAs returns nullptr "if there was no node bound to ID or if
        // there
        // is a node but it cannot be converted to the specified type". This
        // shouldn't happen.
        assert(thisExpression != nullptr);

        llvm::FoldingSetNodeID thisExpressionFoldingSetID;
        thisExpression->Profile(thisExpressionFoldingSetID, *result.Context,
                                true);

        if (thisExpressionFoldingSetID == _possibleAccumulator.first &&
            !isExpressionInOneOfPossibleAccumulatingAssignmentsOfPossibleAccumulator(
                thisExpression, _possibleAccumulator.second, result.Context)) {
          outsideReferences += 1;
        }
      }

    } matcherCallback(possibleAccumulator);

    // Count number of outside references.
    MatchFinder referenceFinder;
    referenceFinder.addMatcher(
        findAll(getMatcherForSpecificExpressionType(
                    possibleAccumulator.second.possible_accumulator)
                    .bind("expression")),
        &matcherCallback);
    referenceFinder.match(*loop_info.loop_stmt, *context);

    // Get the resulting count of outside references.
    possibleAccumulator.second.outside_references =
        matcherCallback.outsideReferences;
  }
}
}
}
}
