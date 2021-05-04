#include "internal.h"

#include "clang/ASTMatchers/ASTMatchers.h"
using namespace clang::ast_matchers;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

/* Get the number of the line in which the base of a possible accumulator was
 * declared.

 * Here, we consider the _base_ of an expression to be the earliest identifier
 * that appears in it. For example, the base of structitty.member is structitty.
 * The base of *ptr is ptr. The base of array[i] is array.
 */
static int get__declaration_line_number_of_base_of_possible_accumulator(
    PossibleAccumulatorInfo &possible_accumulator, clang::ASTContext *context) {
  /* To find the earliest declaration-reference expression in an expression,
   * we're going to rely on the fact that it appears that Clang's AST-matchers
   * framework always traverses ASTs in pre-order when looking for matches,
   * even though that isn't guaranteed (I think).
   */
  // Create an instance of MatchFinder::MatchCallback to capture the
  // declaration-reference expression it finds.
  struct MatcherCallback
      : public clang::ast_matchers::MatchFinder::MatchCallback {
    const clang::DeclRefExpr *declaration_reference_expression = nullptr;

    virtual void
    run(const clang::ast_matchers::MatchFinder::MatchResult &result) {
      if (declaration_reference_expression != nullptr) {
        return;
      }

      const clang::DeclRefExpr *declRefExpr =
          result.Nodes.getNodeAs<clang::DeclRefExpr>("declRefExpr");

      // getNodeAs returns nullptr "if there was no node bound to ID or if there
      // is a node but it cannot be converted to the specified type". This
      // shouldn't happen.
      assert(declRefExpr != nullptr);

      declaration_reference_expression = declRefExpr;
    }
  } matcherCallback;

  clang::ast_matchers::MatchFinder matchFinder;
  matchFinder.addMatcher(declRefExpr().bind("declRefExpr"), &matcherCallback);
  matchFinder.addMatcher(expr(hasDescendant(declRefExpr().bind("declRefExpr"))),
                         &matcherCallback);
  matchFinder.match(*possible_accumulator.possibleAccumulator, *context);

  return context->getSourceManager().getPresumedLineNumber(
      matcherCallback.declaration_reference_expression->getDecl()->getBeginLoc());
}

void getDistanceOfDeclarationOfPossibleAccumulators(
    PossibleReductionLoopInfo &loop_info, clang::ASTContext *context) {
  int loop_line = context->getSourceManager().getPresumedLineNumber(
      loop_info.loopStmt->getBeginLoc());

  for (auto &possible_accumulator : loop_info.possible_accumulators) {
    int possible_accumulator_declaration_line =
        get__declaration_line_number_of_base_of_possible_accumulator(
            possible_accumulator.second, context);
    possible_accumulator.second.declarationDistanceFromLoopInLines =
        loop_line - possible_accumulator_declaration_line;
  }
}
}
}
}