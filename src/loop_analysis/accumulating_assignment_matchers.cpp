#include "../accumulating_assignment_matchers.h"

#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace reduction_detector {
namespace reduction_assignment_matchers {

/* possibleAccumulatingAssignment matches any simple assignment whose assignee
 * also appears in its right-hand side once (and only once).
 * For example: sum = sum + array[i]
 */
StatementMatcher possibleAccumulatingSimpleAssignmentMatcher = binaryOperator(
    hasOperatorName("="),
    hasLHS(declRefExpr(to(varDecl().bind("possibleAccumulator")))),
    hasRHS(hasDescendant(
        declRefExpr(to(varDecl(equalsBoundNode("possibleAccumulator"))))
            .bind("referenceTopossibleAccumulatorInRHS"))),
    unless(hasRHS(hasDescendant(declRefExpr(
        to(varDecl(equalsBoundNode("possibleAccumulator"))),
        unless(equalsBoundNode("referenceTopossibleAccumulatorInRHS")))))));

/* A matcher that matches a compound assignment whose left-hand side is a
 * variable that does not appear in its right-hand side.
 */
StatementMatcher possibleAccumulatingCompoundAssignmentMatcher = binaryOperator(
    anyOf(hasOperatorName("+="), hasOperatorName("-="), hasOperatorName("*="),
          hasOperatorName("/=")),
    hasLHS(declRefExpr(to(varDecl().bind("possibleAccumulator")))),
    unless(hasRHS(hasDescendant(
        declRefExpr(to(varDecl(equalsBoundNode("possibleAccumulator"))))))));

StatementMatcher accumulatingAssignmentMatcher =
    binaryOperator(anyOf(possibleAccumulatingSimpleAssignmentMatcher,
                         possibleAccumulatingCompoundAssignmentMatcher))
        .bind("possibleAccumulatingAssignment");
}
}
