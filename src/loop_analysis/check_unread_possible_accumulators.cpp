#include "internal.h"
#include "utilities.h"
using namespace reduction_detector::loop_analysis::internal;

#include "clang/ASTMatchers/ASTMatchFinder.h"
using namespace clang::ast_matchers;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

static bool expression_contains_subexpression(const clang::Expr &haystack,
                                              const clang::Expr &needle,
                                              clang::ASTContext &context) {
  llvm::FoldingSetNodeID needle_folding_set_note_ID;
  needle.Profile(needle_folding_set_note_ID, context, true);
  return utilities::doesSubtreeHaveAnExpressionWithFoldingSetNodeID(
      haystack, needle_folding_set_note_ID, &context);
}

// Check whether a given expression is the left side of an assignment
// expression whose right side does not also reference it. (If the right side
// also references the variable that is the left side, then
// that variable is read before it is written again.)
static bool is_write_reference(const clang::Expr *expression,
                               clang::ASTContext *context) {
  /* Construct a matcher that will match expression only if it is the left-hand
   * side of an assignment. */
  StatementMatcher left_side_matcher =
      expr(hasAncestor(binaryOperator(isAssignmentOperator(),
                                      hasLHS(expr(equalsNode(expression))))
                           .bind("assignment_expression")))
          .bind("LHS");

  // Instantiate a struct to be used as a MatchFinder callback.
  struct MatcherCallback : public MatchFinder::MatchCallback {
    bool is_write_reference = false;
    virtual void run(const MatchFinder::MatchResult &result) {
      const clang::BinaryOperator *assignment_expression =
          result.Nodes.getNodeAs<clang::BinaryOperator>(
              "assignment_expression");
      const clang::Expr *LHS = result.Nodes.getNodeAs<clang::Expr>("LHS");

      assert(assignment_expression != nullptr);
      assert(LHS != nullptr);

      if (assignment_expression->isCompoundAssignmentOp() ||
          expression_contains_subexpression(*assignment_expression->getRHS(),
                                            *LHS, *result.Context)) {
        is_write_reference = false;
      } else {
        is_write_reference = true;
      }
    }
  } matcherCallback;

  MatchFinder matchFinder;
  matchFinder.addMatcher(left_side_matcher, &matcherCallback);
  matchFinder.match(*expression, *context);
  return matcherCallback.is_write_reference;
}

const clang::FunctionDecl &
get_enclosing_function(const clang::Stmt &enclosed_statement,
                       clang::ASTContext &context) {
  const clang::Stmt *walker = &enclosed_statement;
  while (true) {
    auto walker_parent = context.getParentMapContext().getParents(*walker)[0];
    if (const clang::Stmt *parent_statement =
            walker_parent.get<clang::Stmt>()) {
      walker = parent_statement;
      continue;
    } else if (const clang::FunctionDecl *function_definition =
                   walker_parent.get<clang::FunctionDecl>()) {
      return *function_definition;
    }
  }
  assert(false);
}

void checkWhetherPossibleAccumulatorsAreReadAfterLoop(
    PossibleReductionLoopInfo &loop_info, clang::ASTContext &context) {
  for (auto &possibleAccumulator : loop_info.possible_accumulators) {
    // Instantiate a struct to be used as a MatchFinder callback.
    // We'll match expressions outside the loop that are similar to
    // possibleAccumulator. Our goal is to find which one of them has the same
    // folding-set node ID as possibleAccumulator and is the first one that
    // appears after the loop.
    struct MatcherCallback : public MatchFinder::MatchCallback {
      decltype(loop_info) _loop;
      decltype(possibleAccumulator) _possibleAccumulator;
      bool done = false;

      MatcherCallback(decltype(loop_info) _loop,
                      decltype(possibleAccumulator) _possibleAccumulator)
          : _loop(_loop), _possibleAccumulator(_possibleAccumulator) {}

      virtual void run(const MatchFinder::MatchResult &result) {
        if (done) {
          return;
        }

        const clang::Expr *thisExpression = result.Nodes.getNodeAs<clang::Expr>(
            "possible_reference_to_possible_accumulator");

        // getNodeAs returns nullptr "if there was no node bound to ID or if
        // there is a node but it cannot be converted to the specified type".
        // This shouldn't happen.
        assert(thisExpression != nullptr);

        bool is_this_expression_after_the_loop =
            result.SourceManager->isBeforeInTranslationUnit(
                _loop.loop_stmt->getBeginLoc(), thisExpression->getBeginLoc());
        if (is_this_expression_after_the_loop) {
          llvm::FoldingSetNodeID thisExpressionFoldingSetID;
          thisExpression->Profile(thisExpressionFoldingSetID, *result.Context,
                                  true);
          if (thisExpressionFoldingSetID == _possibleAccumulator.first) {
            _possibleAccumulator.second.first_reference_after_loop =
                thisExpression;
            done = true;
          }
        }
      }

    } matcherCallback(loop_info, possibleAccumulator);

    const clang::Expr *pa = possibleAccumulator.second.possible_accumulator;
    StatementMatcher
        matcher_of_expressions_similar_to_possible_accumulator_outside_loop(
            clang::ast_matchers::anything());
    // Obtain a matcher that is specific to the expression type of the possible
    // accumulator.
    // TODO: make this code less repetitive by storing the condition
    // unless(hasAncestor(stmt(equalsNode(loop_info.loop_stmt))))
    // in a variable.
    if (llvm::isa<clang::DeclRefExpr>(pa)) {
      matcher_of_expressions_similar_to_possible_accumulator_outside_loop =
          clang::ast_matchers::declRefExpr(
              unless(hasAncestor(stmt(equalsNode(loop_info.loop_stmt)))))
              .bind("possible_reference_to_possible_accumulator");
    } else if (llvm::isa<clang::ArraySubscriptExpr>(pa)) {
      matcher_of_expressions_similar_to_possible_accumulator_outside_loop =
          clang::ast_matchers::arraySubscriptExpr(
              unless(hasAncestor(stmt(equalsNode(loop_info.loop_stmt)))))
              .bind("possible_reference_to_possible_accumulator");
    } else if (llvm::isa<clang::MemberExpr>(pa)) {
      matcher_of_expressions_similar_to_possible_accumulator_outside_loop =
          clang::ast_matchers::memberExpr(
              unless(hasAncestor(stmt(equalsNode(loop_info.loop_stmt)))))
              .bind("possible_reference_to_possible_accumulator");
    } else if (llvm::isa<clang::UnaryOperator>(pa)) {
      matcher_of_expressions_similar_to_possible_accumulator_outside_loop =
          clang::ast_matchers::unaryOperator(
              unless(hasAncestor(stmt(equalsNode(loop_info.loop_stmt)))))
              .bind("possible_reference_to_possible_accumulator");
    } else {
      matcher_of_expressions_similar_to_possible_accumulator_outside_loop =
          clang::ast_matchers::expr(
              unless(hasAncestor(stmt(equalsNode(loop_info.loop_stmt)))))
              .bind("possible_reference_to_possible_accumulator");
    }

    MatchFinder referenceFinder;
    referenceFinder.addMatcher(
        functionDecl(forEachDescendant(
            matcher_of_expressions_similar_to_possible_accumulator_outside_loop)),
        &matcherCallback);
    referenceFinder.match(get_enclosing_function(*loop_info.loop_stmt, context),
                          context);

    possibleAccumulator.second.is_apparently_unused_after_loop =
        (possibleAccumulator.second.is_local_variable &&
         possibleAccumulator.second.first_reference_after_loop == nullptr) ||
        (possibleAccumulator.second.first_reference_after_loop != nullptr &&
         is_write_reference(
             possibleAccumulator.second.first_reference_after_loop, &context));
  }
}
} // namespace internal
} // namespace loop_analysis
} // namespace reduction_detector