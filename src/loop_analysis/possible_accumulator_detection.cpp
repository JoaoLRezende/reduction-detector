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

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

/*
 * One instance of PossibleAccumulatorFinder is created for each for loop.
 * For each assignment whose left-hand side is a possible accumulator, it stores
 * that left-hand side in the map possible_accumulators of
 * the struct PossibleReductionLoopInfo that describes that loop.
 */
class PossibleAccumulatorFinder : public MatchFinder::MatchCallback {

  /*
   * The address of the structure describing
   * the for loop this instance was created to search.
   */
  PossibleReductionLoopInfo *loop_info;

public:
  PossibleAccumulatorFinder(PossibleReductionLoopInfo *loop_info)
      : loop_info(loop_info) {}

  // run is called by MatchFinder for each assignment whose left-hand side is a
  // possible accumulator.
  virtual void run(const MatchFinder::MatchResult &result) {

    // Get the matched assignment.
    const clang::BinaryOperator *possibleAccumulatingAssignment =
        result.Nodes.getNodeAs<clang::BinaryOperator>(
            "possibleAccumulatingAssignment");

    // getNodeAs returns nullptr "if there was no node bound to ID or if there
    // is a node but it cannot be converted to the specified type".
    assert(possibleAccumulatingAssignment != nullptr);

    // Get the assignment's assignee.
    const clang::Expr *possibleAccumulator =
        possibleAccumulatingAssignment->getLHS();

    // TODO: experiment with passing different values for the third parameter of
    // the Profile method. What does that change?
    llvm::FoldingSetNodeID possibleAccumulatorFoldingSetID;
    possibleAccumulator->Profile(possibleAccumulatorFoldingSetID,
                                 *result.Context, true);

    // Note that std::map.insert does nothing if there already is an element
    // with the given key.
    loop_info->possible_accumulators.insert(
        {possibleAccumulatorFoldingSetID,
         PossibleAccumulatorInfo(possibleAccumulator)});

    loop_info->possible_accumulators.find(possibleAccumulatorFoldingSetID)
        ->second.possible_accumulating_assignments.insert(
            {possibleAccumulatingAssignment,
             PossibleAccumulatingAssignmentInfo()});
  };
};

void getPossibleAccumulatorsIn(PossibleReductionLoopInfo *loop_info,
                               ASTContext *context) {
  PossibleAccumulatorFinder possibleAccumulatorFinder(loop_info);

  // Construct a matcher that will match assignments whose LHS
  // contains a reference to a variable declared outside of the loop.
  StatementMatcher possibleAccumulatingAssignmentMatcher =
      binaryOperator(
          anyOf(hasOperatorName("="), hasOperatorName("+="),
                hasOperatorName("-="), hasOperatorName("*="),
                hasOperatorName("/="), hasOperatorName("%="),
                hasOperatorName("&="), hasOperatorName("|="),
                hasOperatorName("^="), hasOperatorName("<<="),
                hasOperatorName(">>=")),
          hasLHS(anyOf(
              // The assignment's left-hand side can be a reference to a
              // variable declared outside of the loop ...
              declRefExpr(to(varDecl(
                  unless(hasAncestor(stmt(equalsNode(loop_info->loopStmt))))))),
              // ... or it can be a larger sub-tree that contains such a
              // reference.
              hasDescendant(declRefExpr(to(varDecl(unless(
                  hasAncestor(stmt(equalsNode(loop_info->loopStmt)))))))))))
          .bind("possibleAccumulatingAssignment");

  MatchFinder possibleAccumulatingAssignmentFinder;
  possibleAccumulatingAssignmentFinder.addMatcher(
      clang::ast_matchers::findAll(possibleAccumulatingAssignmentMatcher),
      &possibleAccumulatorFinder);

  possibleAccumulatingAssignmentFinder.match(*loop_info->loopStmt, *context);
}
}
}
}
