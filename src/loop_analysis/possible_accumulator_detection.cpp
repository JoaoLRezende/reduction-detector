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

// Check whether a clang::ValueDecl (for example, a declaration of the base of a
// possible accumulator) is a descendant of a clang::Stmt (for example, a loop
// statement).
static bool isDeclarationInStatement(const clang::ValueDecl *declaration,
                                     const clang::Stmt *statement,
                                     clang::ASTContext *context) {
  /* Construct a matcher that will match declaration only if it is a descendant
   * of statement.
   */
  DeclarationMatcher declarationMatcher =
      valueDecl(hasAncestor(stmt(equalsNode(statement))));

  // Instantiate a struct to be used as a MatchFinder callback.
  struct MatcherCallback : public MatchFinder::MatchCallback {
    bool wasCalled = false;
    virtual void run(const MatchFinder::MatchResult &result) {
      wasCalled = true;
    }
  } matcherCallback;

  MatchFinder matchFinder;
  matchFinder.addMatcher(declarationMatcher, &matcherCallback);
  matchFinder.match(*declaration, *context);
  return matcherCallback.wasCalled;
}

/*
 * One instance of PossibleAccumulatorFinder is created for each loop.
 * For each assignment whose left-hand side is a possible accumulator, it stores
 * that left-hand side in the map possible_accumulators of the struct
 * PossibleReductionLoopInfo that describes that loop, together with its base.
 *
 * Here, we consider the _base_ of an expression to be the earliest identifier
 * that appears in it. For example, the base of structitty.member is structitty.
 * The base of *ptr is ptr. The base of array[i] is array.
 *
 * It seems to me that, in expressions that don't involve type names, there is a
 * one-to-one mapping between identifiers and declaration-reference expressions.
 * And there is no AST matcher for identifiers. Thus, we look for the earliest
 * declaration-reference expression in an l-value and take that as its base.
 *
 * To find the earliest declaration-reference expression in an expression, we
 * rely on the fact that it seems that Clang's AST-matchers framework
 * always traverses ASTs in pre-order when looking for matches, even though that
 * isn't formally guaranteed anywhere (I think).
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

  // run is called by MatchFinder for each assignment expression.
  virtual void run(const MatchFinder::MatchResult &result) {
    const clang::BinaryOperator *possible_possible_accumulating_assignment =
        result.Nodes.getNodeAs<clang::BinaryOperator>(
            "possible_possible_accumulating_assignment");
    const clang::DeclRefExpr *possible_possible_accumulator_base =
        result.Nodes.getNodeAs<clang::DeclRefExpr>(
            "possible_possible_accumulator_base");

    // getNodeAs returns nullptr "if there was no node bound to ID or if there
    // is a node but it cannot be converted to the specified type".
    assert(possible_possible_accumulating_assignment != nullptr);
    assert(possible_possible_accumulator_base != nullptr);

    // If possible_possible_accumulator_base is declared inside the loop we're
    // examining, then possible_possible_accumulator is not a possible
    // accumulator.
    if (isDeclarationInStatement(possible_possible_accumulator_base->getDecl(),
                                 loop_info->loop_stmt, result.Context)) {
      return;
    }

    const clang::Expr *possible_accumulator =
        possible_possible_accumulating_assignment->getLHS();

    // TODO: what does the third argument to the Profile method do?
    // Experiment.
    llvm::FoldingSetNodeID possibleAccumulatorFoldingSetID;
    possible_accumulator->Profile(possibleAccumulatorFoldingSetID,
                                  *result.Context, true);

    // Note that std::map's insert method, if there already is an element with
    // the given key, simply returns that element instead of inserting a new
    // one.
    auto inserted_map_node = loop_info->possible_accumulators.insert(
        {possibleAccumulatorFoldingSetID,
         PossibleAccumulatorInfo(possible_accumulator,
                                 possible_possible_accumulator_base)});

    inserted_map_node.first->second.possible_accumulating_assignments.insert(
        {possible_possible_accumulating_assignment,
         PossibleAccumulatingAssignmentInfo()});
  };
};

void getPossibleAccumulatorsIn(PossibleReductionLoopInfo *loop_info,
                               ASTContext *context) {
  PossibleAccumulatorFinder possibleAccumulatorFinder(loop_info);

  // Construct a matcher that will match assignment expressions.
  StatementMatcher possiblePossibleAccumulatingAssignmentMatcher =
      binaryOperator(
          anyOf(hasOperatorName("="), hasOperatorName("+="),
                hasOperatorName("-="), hasOperatorName("*="),
                hasOperatorName("/="), hasOperatorName("%="),
                hasOperatorName("&="), hasOperatorName("|="),
                hasOperatorName("^="), hasOperatorName("<<="),
                hasOperatorName(">>=")),
          hasLHS(anyOf(
              // The LHS can be either a naked declaration-reference
              // expression ...
              declRefExpr().bind("possible_possible_accumulator_base"),
              // ... or a larger subtree that includes a
              // declaration-reference expression.
              hasDescendant(
                  declRefExpr().bind("possible_possible_accumulator_base")))))
          .bind("possible_possible_accumulating_assignment");

  MatchFinder possibleAccumulatingAssignmentFinder;
  possibleAccumulatingAssignmentFinder.addMatcher(
      clang::ast_matchers::findAll(
          possiblePossibleAccumulatingAssignmentMatcher),
      &possibleAccumulatorFinder);

  possibleAccumulatingAssignmentFinder.match(*loop_info->loop_stmt, *context);
}
}
}
}
