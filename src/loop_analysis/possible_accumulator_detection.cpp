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

#include "clang/AST/Stmt.h"
using clang::ForStmt;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

/*
 * One instance of PossibleAccumulatingAssignmentFinder is created
 * for each for loop.
 * For each assignment whose left side is a variable declared outside
 * of the loop, it stores that variable in the map possible_accumulators of
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

  // run is called by MatchFinder for each reduction-looking assignment.
  virtual void run(const MatchFinder::MatchResult &result) {

    // Get the matched assignment.
    const BinaryOperator *possibleAccumulatingAssignment =
        result.Nodes.getNodeAs<BinaryOperator>(
            "possibleAccumulatingAssignment");

    // Get the assignment's assignee.
    const VarDecl *possibleAccumulator =
        result.Nodes.getNodeAs<VarDecl>("possibleAccumulator");

    // Note that std::map.insert does nothing if there already is an element
    // with the given key.
    loop_info->possible_accumulators.insert(
        {possibleAccumulator, PossibleAccumulatorInfo()});

    loop_info->possible_accumulators.find(possibleAccumulator)
        ->second.possible_accumulating_assignments.insert(
            {possibleAccumulatingAssignment,
             PossibleAccumulatingAssignmentInfo()});
  };
};

void getPossibleAccumulatorsIn(PossibleReductionLoopInfo *loop_info,
                               ASTContext *context) {
  PossibleAccumulatorFinder possibleAccumulatorFinder(loop_info);

  // Construct a matcher that will match assignments whose LHS
  // contains a variable declared outside of the loop.
  StatementMatcher possibleAccumulatingAssignmentMatcher =
      binaryOperator(
          anyOf(hasOperatorName("="), hasOperatorName("+="),
                hasOperatorName("-="), hasOperatorName("*="),
                hasOperatorName("/="), hasOperatorName("%="),
                hasOperatorName("&="), hasOperatorName("|="),
                hasOperatorName("^="), hasOperatorName("<<="),
                hasOperatorName(">>=")),
          hasLHS(declRefExpr(to(varDecl(unless(hasAncestor(stmt(
                                            equalsNode(loop_info->loopStmt)))))
                                    .bind("possibleAccumulator")))))
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
