#include "internal.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

// Check whether a statement's AST has any node that is matched by a given
// matcher.
static bool stmtContainsNodeMatchedByMatcher(const clang::Stmt *haystack,
                                             StatementMatcher matcher,
                                             clang::ASTContext *context) {
  // Instantiate a struct to be used as a MatchFinder callback.
  struct MatcherCallback : public MatchFinder::MatchCallback {
    bool wasCalled = false;
    virtual void run(const MatchFinder::MatchResult &result) {
      wasCalled = true;
    }
  } matcherCallback;

  MatchFinder matchFinder;
  matchFinder.addMatcher(findAll(matcher), &matcherCallback);
  matchFinder.match(*haystack, *context);
  return matcherCallback.wasCalled;
}

void seeWhetherPossibleAccumulatorsAreDereferencesOfInconstantPointers(
    PossibleReductionLoopInfo &loop_info, clang::ASTContext *context) {
  for (auto &possible_accumulator : loop_info.possible_accumulators) {
    const clang::DeclRefExpr &possible_accumulator_base =
        *possible_accumulator.second.base;
    if (possible_accumulator_base.getType()->isPointerType()) {
      // Check whether the pointer referenced by possible_accumulator_base
      // is the left-hand side of any assignments in the loop.
      StatementMatcher matcher_of_base_as_LHS_of_assignment = declRefExpr(
          to(varDecl(equalsNode(possible_accumulator_base.getDecl()))),
          hasAncestor(
              binaryOperator(isAssignmentOperator(),
                             hasLHS(declRefExpr(to(varDecl(equalsNode(
                                 possible_accumulator_base.getDecl()))))))));

      possible_accumulator.second.is_dereference_of_inconstant_pointer =
          stmtContainsNodeMatchedByMatcher(loop_info.loop_stmt,
                                           matcher_of_base_as_LHS_of_assignment,
                                           context);
    }
  }
}

} // namespace internal
} // namespace loop_analysis
} // namespace reduction_detector
