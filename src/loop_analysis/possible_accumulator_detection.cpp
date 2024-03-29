#include "../command_line.h"
#include "../constants.h"
#include "internal.h"
#include "loop_analysis.h"

#include "llvm/Support/CommandLine.h"

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

// Define command-line option --detect-increment-accumulations.
static llvm::cl::opt<bool> detect_increment_accumulations(
    "detect-increment-accumulations",
    llvm::cl::desc("Recognize unary increment and decrement operations as "
                   "possibly accumulating operations "),
    llvm::cl::cat(reduction_detector::command_line_options::
                      reduction_detector_option_category));

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
 * For each assignment whose left-hand side is a possible accumulator or unary
 * increment or decrement operation whose operand is a possible accumulator, it
 * stores that possible accumulator in the map possible_accumulators of the
 * struct PossibleReductionLoopInfo that describes that loop, together with that
 * possible accumulator's base.
 *
 * Here, we consider the _base_ of an expression to be the earliest identifier
 * that appears in it. For example, the base of structitty.member is structitty.
 * The base of *ptr is ptr. The base of array[i] is array.
 *
 * It seems that, in expressions that don't involve type names, there is a
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

  // run is called by MatchFinder for each assignment expression
  // or unary increment/decrement expression.
  virtual void run(const MatchFinder::MatchResult &result) {
    // We receive either an assignment expression ...
    const clang::BinaryOperator *possible_possible_accumulating_assignment =
        result.Nodes.getNodeAs<clang::BinaryOperator>(
            "possible_possible_accumulating_assignment");
    // ... or a unary increment or decrement expression.
    const clang::UnaryOperator *possible_possible_accumulating_unary_operation =
        result.Nodes.getNodeAs<clang::UnaryOperator>(
            "possible_possible_accumulating_unary_operation");
    assert(possible_possible_accumulating_assignment != nullptr ||
           possible_possible_accumulating_unary_operation != nullptr);

    // In any case, we also receive the base of the modified LHS or unary
    // operand.
    const clang::DeclRefExpr *possible_possible_accumulator_base =
        result.Nodes.getNodeAs<clang::DeclRefExpr>(
            "possible_possible_accumulator_base");
    assert(possible_possible_accumulator_base != nullptr);

    // If possible_possible_accumulator_base is declared inside the loop we're
    // examining, then possible_possible_accumulator is not a possible
    // accumulator.
    if (isDeclarationInStatement(possible_possible_accumulator_base->getDecl(),
                                 loop_info->loop_stmt, result.Context)) {
      return;
    }

    const clang::Expr *possible_accumulator = nullptr;
    if (possible_possible_accumulating_assignment != nullptr) {
      possible_accumulator =
          possible_possible_accumulating_assignment->getLHS();
    } else if (possible_possible_accumulating_unary_operation != nullptr) {
      possible_accumulator =
          possible_possible_accumulating_unary_operation->getSubExpr();
    } else {
      assert(false);
    };

    // If possible_accumulator is actually a pointer, then discard it.
    // [Pointers can't be accumulators. This check prevents us from detecting
    // linked-list traversals (that have statements like aux = aux->next;) as
    // reductions.]
    if (possible_accumulator->getType()->isPointerType()) {
      return;
    }

    llvm::FoldingSetNodeID possibleAccumulatorFoldingSetID;
    possible_accumulator->Profile(possibleAccumulatorFoldingSetID,
                                  *result.Context, true);

    const clang::Expr *possible_accumulating_operation = nullptr;
    if (possible_possible_accumulating_assignment != nullptr) {
      possible_accumulating_operation =
          possible_possible_accumulating_assignment;
    } else if (possible_possible_accumulating_unary_operation != nullptr) {
      possible_accumulating_operation =
          possible_possible_accumulating_unary_operation;
    }

    // Note that std::map's insert method, if there already is an element with
    // the given key, simply returns that element instead of inserting a new
    // one.
    auto inserted_map_node = loop_info->possible_accumulators.insert(
        {possibleAccumulatorFoldingSetID,
         PossibleAccumulatorInfo(possible_accumulator,
                                 possible_possible_accumulator_base)});

    inserted_map_node.first->second.possible_accumulating_operations.insert(
        {possible_accumulating_operation,
         PossibleAccumulatingOperationInfo()});
  };
};

void getPossibleAccumulatorsIn(PossibleReductionLoopInfo *loop_info,
                               ASTContext *context) {
  PossibleAccumulatorFinder possibleAccumulatorFinder(loop_info);

  // Construct a matcher that will match assignment expressions.
  StatementMatcher possiblePossibleAccumulatingAssignmentMatcher =
      binaryOperator(
          isAssignmentOperator(),
          hasLHS(anyOf(
              // The LHS can be either a bare declaration-reference
              // expression ...
              declRefExpr().bind("possible_possible_accumulator_base"),
              // ... or a larger subtree that includes a
              // declaration-reference expression.
              hasDescendant(
                  declRefExpr().bind("possible_possible_accumulator_base")))))
          .bind("possible_possible_accumulating_assignment");

  // Construct a matcher that will match unary increment or decrement
  // operations.
  StatementMatcher possiblePossibleAccumulatingUnaryOperationMatcher =
      unaryOperator(
          anyOf(hasOperatorName("++"), hasOperatorName("--")),
          hasUnaryOperand(anyOf(
              // The operand can be either a naked declaration-reference
              // expression ...
              declRefExpr().bind("possible_possible_accumulator_base"),
              // ... or a larger subtree that includes a
              // declaration-reference expression.
              hasDescendant(
                  declRefExpr().bind("possible_possible_accumulator_base")))))
          .bind("possible_possible_accumulating_unary_operation");

  MatchFinder possibleAccumulatingAssignmentFinder;

  possibleAccumulatingAssignmentFinder.addMatcher(
      clang::ast_matchers::findAll(
          possiblePossibleAccumulatingAssignmentMatcher),
      &possibleAccumulatorFinder);
  if (detect_increment_accumulations) {
    possibleAccumulatingAssignmentFinder.addMatcher(
        clang::ast_matchers::findAll(
            possiblePossibleAccumulatingUnaryOperationMatcher),
        &possibleAccumulatorFinder);
  }

  possibleAccumulatingAssignmentFinder.match(*loop_info->loop_stmt, *context);
}
} // namespace internal
} // namespace loop_analysis
} // namespace reduction_detector
