#include "reduction-detector/reduction_assignment_matchers.h"

#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace reduction_detector {
namespace reduction_assignment_matchers {

/* potentialReductionAssignment matches any simple assignment whose assignee
 * also appears in its right-hand side once (and only once).
 * For example: sum = sum + array[i]
 */
StatementMatcher potentialReductionSimpleAssignmentMatcher = binaryOperator(
    hasOperatorName("="),
    hasLHS(declRefExpr(to(varDecl().bind("potentialAccumulator")))),
    hasRHS(hasDescendant(
        declRefExpr(to(varDecl(equalsBoundNode("potentialAccumulator"))))
            .bind("referenceTopotentialAccumulatorInRHS"))),
    unless(hasRHS(hasDescendant(declRefExpr(
        to(varDecl(equalsBoundNode("potentialAccumulator"))),
        unless(equalsBoundNode("referenceTopotentialAccumulatorInRHS")))))));

/* A matcher that matches a compound assignment whose left-hand side is a
 * variable that does not appear in its right-hand side.
 */
StatementMatcher potentialReductionCompoundAssignmentMatcher = binaryOperator(
    anyOf(hasOperatorName("+="), hasOperatorName("-="), hasOperatorName("*="),
          hasOperatorName("/=")),
    hasLHS(declRefExpr(to(varDecl().bind("potentialAccumulator")))),
    unless(hasRHS(hasDescendant(
        declRefExpr(to(varDecl(equalsBoundNode("potentialAccumulator"))))))));

StatementMatcher reductionAssignmentMatcher =
    binaryOperator(anyOf(potentialReductionSimpleAssignmentMatcher,
                         potentialReductionCompoundAssignmentMatcher))
        .bind("possibleReductionAssignment");
}
}
