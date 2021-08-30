#include "utilities.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"
using namespace clang::ast_matchers;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {
namespace utilities {

bool doesSubtreeHaveAnExpressionWithFoldingSetNodeID(
    const clang::Expr &subtree,
    const llvm::FoldingSetNodeID &expressionFoldingSetNodeID,
    clang::ASTContext *context) {

  // Instantiate a struct to be used as a MatchFinder callback.
  // We'll match every expression and compare its folding-set node ID
  // with our target folding-set node ID.
  struct MatcherCallback
      : public clang::ast_matchers::MatchFinder::MatchCallback {
    const llvm::FoldingSetNodeID &targetExpressionFoldingSetNodeID;

    MatcherCallback(
        const llvm::FoldingSetNodeID &targetExpressionFoldingSetNodeID)
        : targetExpressionFoldingSetNodeID(targetExpressionFoldingSetNodeID){};

    bool foundAnExpressionWithTargetFoldingSetNodeID = false;

    virtual void
    run(const clang::ast_matchers::MatchFinder::MatchResult &result) {
      const clang::Expr *thisExpression =
          result.Nodes.getNodeAs<clang::Expr>("expression");

      // getNodeAs returns nullptr "if there was no node bound to ID or if there
      // is a node but it cannot be converted to the specified type". This
      // shouldn't happen.
      assert(thisExpression != nullptr);

      // TODO: do anything only if foundAnExpressionWithTargetFoldingSetNodeID
      // isn't already true.
      llvm::FoldingSetNodeID thisExpressionFoldingSetID;
      thisExpression->Profile(thisExpressionFoldingSetID, *result.Context,
                              true);

      if (thisExpressionFoldingSetID == targetExpressionFoldingSetNodeID) {
        foundAnExpressionWithTargetFoldingSetNodeID = true;
      }
    }
  } matcherCallback(expressionFoldingSetNodeID);

  MatchFinder matchFinder;
  // TODO: here, we match, and compute the hash sum of, every expression under
  // the given
  // subtree. This is a waste, and might result in a lot of unnecessary
  // work in more complex subtrees. (Computing the hash sum of each node
  // involves visiting all of its descendants.) We could avoid a lot of
  // useless work by considering only expressions whose type is the same as
  // that of the expression we're looking for. For example: if the target
  // expression is an array-subscript expression, then match only
  // array-subscript expressions.
  // We've done something very similar in outside_reference_counting.cpp.
  matchFinder.addMatcher(findAll(expr().bind("expression")), &matcherCallback);
  matchFinder.match(subtree, *context);
  return matcherCallback.foundAnExpressionWithTargetFoldingSetNodeID;
}

} // namespace utilities
} // namespace internal
} // namespace loop_analysis
} // namespace reduction_detector