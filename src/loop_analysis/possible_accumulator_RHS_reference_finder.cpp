#include "internal.h"
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

static bool doesSubtreeHaveAnExpressionWithFoldingSetNodeID(
    const clang::Expr &subtree,
    const llvm::FoldingSetNodeID &expressionFoldingSetNodeID,
    ASTContext *context) {

  // Instantiate a struct to be used as a MatchFinder callback.
  // We'll match every expression and compare its folding-set node ID
  // with our target folding-set node ID.
  struct MatcherCallback : public MatchFinder::MatchCallback {
    const llvm::FoldingSetNodeID &targetExpressionFoldingSetNodeID;

    MatcherCallback(
        const llvm::FoldingSetNodeID &targetExpressionFoldingSetNodeID)
        : targetExpressionFoldingSetNodeID(targetExpressionFoldingSetNodeID){};

    bool foundAnExpressionWithTargetFoldingSetNodeID = false;

    virtual void run(const MatchFinder::MatchResult &result) {
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
  matchFinder.addMatcher(findAll(expr().bind("expression")), &matcherCallback);
  matchFinder.match(subtree, *context);
  return matcherCallback.foundAnExpressionWithTargetFoldingSetNodeID;
}

void detectPossibleAccumulatorReferencesInRHSOfPossibleAccumulatingStatements(
    PossibleReductionLoopInfo &loop_info, ASTContext *context) {
  for (auto &possibleAccumulator : loop_info.possible_accumulators) {
    for (auto &possibleAccumulatingAssignment :
         possibleAccumulator.second.possible_accumulating_assignments) {
      // we consider possibleAccumulatingAssignment to have a reference to
      // possibleAccumulator in its right-hand side if either ...
      if ( // possibleAccumulatingAssignment is an application of a compound
           // assignment operator (like +=) ...
          possibleAccumulatingAssignment.first->isCompoundAssignmentOp() ||
          // or possibleAccumulatingAssignment really, explicitly has a
          // reference to possibleAccumulator in its right-hand side.
          doesSubtreeHaveAnExpressionWithFoldingSetNodeID(
              *possibleAccumulatingAssignment.first->getRHS(),
              possibleAccumulator.first, context)) {
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
