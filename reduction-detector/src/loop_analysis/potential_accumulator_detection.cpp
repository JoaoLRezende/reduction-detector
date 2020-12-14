#include "reduction-detector/loop_analysis.h"

#include "reduction-detector/constants.h"

#include "reduction-detector/reduction_assignment_matchers.h"
using reduction_detector::reduction_assignment_matchers::
    reductionAssignmentMatcher;

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

static bool isVariableDeclaredInLoop(const VarDecl *variable,
                                     const ForStmt *forStmt,
                                     ASTContext *context) {
  /* Construct a matcher that will match variable only if it is a descendant
   * of forStmt.
   */
  DeclarationMatcher variableMatcher =
      varDecl(hasAncestor(clang::ast_matchers::forStmt(equalsNode(forStmt))));

  // Instantiate a struct to be used as a MatchFinder callback.
  struct MatcherCallback : public MatchFinder::MatchCallback {
    bool wasCalled = false;
    virtual void run(const MatchFinder::MatchResult &result) {
      wasCalled = true;
    }
  } matcherCallback;

  MatchFinder matchFinder;
  matchFinder.addMatcher(variableMatcher, &matcherCallback);
  matchFinder.match(*variable, *context);
  return matcherCallback.wasCalled;
}

/*
 * One instance of PotentialAccumulatorFinder is created for each for loop.
 * For each assignment that looks like a potential accumulating assignment
 * (for example: sum += array[i]) in that loop, it stores that assignment's
 * left-hand side (in that example, sum) in the potential_accumulators
 * map of the struct PotentialReductionLoopInfo that describes that loop
 * (if it isn't a variable declared within the loop itself).
 */
class PotentialAccumulatorFinder : public MatchFinder::MatchCallback {

  /*
   * The address of the structure describing
   * the for loop this instance was created to search.
   */
  PotentialReductionLoopInfo *loop_info;

public:
  PotentialAccumulatorFinder(PotentialReductionLoopInfo *loop_info)
      : loop_info(loop_info) {}

  // run is called by MatchFinder for each reduction-looking assignment.
  virtual void run(const MatchFinder::MatchResult &result) {

    // Get the matched assignment.
    const BinaryOperator *assignment =
        result.Nodes.getNodeAs<BinaryOperator>("possibleReductionAssignment");

    // Get the assignment's assignee.
    const VarDecl *potentialAccumulator =
        result.Nodes.getNodeAs<VarDecl>("potentialAccumulator");

    if (isVariableDeclaredInLoop(potentialAccumulator, loop_info->forStmt,
                                 result.Context)) {
      return;
    }

    // Note that std::map.insert does nothing if there already is an element
    // with the given key.
    loop_info->potential_accumulators.insert(
        {potentialAccumulator, PotentialAccumulatorInfo()});

    (*loop_info->potential_accumulators.find(potentialAccumulator))
        .second.potential_accumulating_assignments.insert(assignment);
  };
};

void getPotentialAccumulatorsIn(PotentialReductionLoopInfo *loop_info,
                                ASTContext *context) {
  PotentialAccumulatorFinder potentialAccumulatorFinder(loop_info);

  MatchFinder potentialReductionAssignmentFinder;
  potentialReductionAssignmentFinder.addMatcher(
      clang::ast_matchers::findAll(reductionAssignmentMatcher),
      &potentialAccumulatorFinder);

  potentialReductionAssignmentFinder.match(*loop_info->forStmt, *context);
}
}
}
}
