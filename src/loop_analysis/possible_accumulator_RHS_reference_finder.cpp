#include "loop_analysis.h"

#include "../constants.h"

#include "clang/AST/ASTContext.h"
using clang::ASTContext;

#include "clang/ASTMatchers/ASTMatchers.h"
using namespace clang::ast_matchers;

#include "clang/ASTMatchers/ASTMatchFinder.h"
using clang::ast_matchers::MatchFinder;

#include "clang/AST/Expr.h"
using clang::BinaryOperator;

#include "clang/AST/Decl.h"
using clang::VarDecl;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

static bool isVariableReferencedInRHSOfBinaryOperation(
    const VarDecl *variable, const BinaryOperator *binaryOperation,
    ASTContext *context) {
  /* Construct a matcher that will match binaryOperator only if it either has a
   * reference to variable in its RHS or it is an application of a compound
   * assignment operator
   * and its LHS is variable (which means the assignment is equivalent to
   * a simple assignment whose RHS has a reference to variable).
   */
  StatementMatcher referencingAssignmentMatcher = binaryOperator(anyOf(
      hasRHS(hasDescendant(declRefExpr(to(varDecl(equalsNode(variable)))))),
      allOf(hasLHS(declRefExpr(to(varDecl(equalsNode(variable))))),
            anyOf(hasOperatorName("+="), hasOperatorName("-="),
                  hasOperatorName("*="), hasOperatorName("/="),
                  hasOperatorName("%="), hasOperatorName("&="),
                  hasOperatorName("|="), hasOperatorName("^="),
                  hasOperatorName("<<="), hasOperatorName(">>=")))));

  // Instantiate a struct to be used as a MatchFinder callback.
  struct MatcherCallback : public MatchFinder::MatchCallback {
    bool wasCalled = false;
    virtual void run(const MatchFinder::MatchResult &result) {
      wasCalled = true;
    }
  } matcherCallback;

  MatchFinder matchFinder;
  matchFinder.addMatcher(referencingAssignmentMatcher, &matcherCallback);
  matchFinder.match(*binaryOperation, *context);
  return matcherCallback.wasCalled;
}

void detectPossibleAccumulatorReferencesInRHSOfPossibleAccumulatingStatements(
    PossibleReductionLoopInfo &loop_info, ASTContext *context) {
  for (auto &possibleAccumulator : loop_info.possible_accumulators) {
    for (auto &possibleAccumulatingAssignment :
         possibleAccumulator.second.possible_accumulating_assignments) {
      if (isVariableReferencedInRHSOfBinaryOperation(
              possibleAccumulator.first, possibleAccumulatingAssignment.first,
              context)) {
        possibleAccumulatingAssignment.second
            .rhs_also_references_possible_accumulator = true;
        possibleAccumulator.second
            .number_of_possible_accumulating_assignments_whose_RHS_also_references_this +=
            1;
      }
    }
  }
}
}
}
}
