#include "internal.h"
#include "loop_analysis.h"

#include "clang/AST/ASTContext.h"

#include "clang/ASTMatchers/ASTMatchers.h"
using namespace clang::ast_matchers;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

// TODO: this struct type, and the code that uses it below, represent a
// recurrent pattern that almost certainly could be encapsulated by a function
// template. Make that, then use it here and in the other modules that do
// something like this. It could be called something like isASTNodeMatchedBy.
// That would simplify this module's code immensely. There also is another
// similar template function to be made:
// doesASTNodeMatchOrHaveAMatchingDescendant.
struct SimpleBooleanMatchCallback : public MatchFinder::MatchCallback {
  bool wasMatched = false;
  virtual void run(const MatchFinder::MatchResult &result) {
    wasMatched = true;
  }
};

void detectIterationVariableReferencesInPossibleAccumulatingStatements(
    PossibleReductionLoopInfo &loopInfo, clang::ASTContext *context) {
  if (loopInfo.iteration_variable == nullptr)
    return;

  // Construct a matcher that will match assignments that contain a
  // reference to the loop's iteration variable.
  StatementMatcher iterationVariableReferenceMatcher =
      binaryOperator(hasDescendant(
          declRefExpr(to(varDecl(equalsNode(loopInfo.iteration_variable))))));

  for (auto &possibleAccumulatorInfo : loopInfo.possible_accumulators) {
    for (auto &possibleAccumulatingAssignment :
         possibleAccumulatorInfo.second.possible_accumulating_operations) {
      SimpleBooleanMatchCallback simpleBooleanMatchCallback;
      MatchFinder iterationVariableReferenceFinder;
      iterationVariableReferenceFinder.addMatcher(
          iterationVariableReferenceMatcher, &simpleBooleanMatchCallback);
      iterationVariableReferenceFinder.match(
          *possibleAccumulatingAssignment.first, *context);
      if (simpleBooleanMatchCallback.wasMatched) {
        possibleAccumulatingAssignment.second.references_iteration_variable =
            true;
        possibleAccumulatorInfo.second
            .number_of_possible_accumulating_assignments_that_reference_the_iteration_variable +=
            1;
      }
    }
  }
}

void detectIterationVariableReferencesInArraySubscripts(
    PossibleReductionLoopInfo &loopInfo, clang::ASTContext *context) {
  if (loopInfo.iteration_variable == nullptr)
    return;

  // arraySubscriptExpressionMatcher matches any array-subscript expression
  // whose RHS involves the loop's iteration variable, and makes a crude attempt
  // to identify the base array.
  StatementMatcher arraySubscriptExpressionMatcher = arraySubscriptExpr(
      hasRHS(hasDescendant(
          declRefExpr(to(varDecl(equalsNode(loopInfo.iteration_variable)))))),
      hasLHS(hasDescendant(declRefExpr(
          to(varDecl(anyOf(hasType(arrayType()), hasType(pointerType()),
                           hasType(decayedType(hasDecayedType(pointerType())))))
                 .bind("subscriptedArray"))))));

  // Instantiate a struct with a callback function for matching
  // array-subscript expressions. It accumulates information on them.
  struct MatcherCallBack : public MatchFinder::MatchCallback {
    int totalNumberOfArrayAccessesWhoseRHSInvolvesTheIterationVariable = 0;
    std::map<const clang::VarDecl *, int> numberOfAccessesPerArray;

    virtual void run(const MatchFinder::MatchResult &result) {
      totalNumberOfArrayAccessesWhoseRHSInvolvesTheIterationVariable += 1;

      const clang::VarDecl *subscriptedArray =
          result.Nodes.getNodeAs<clang::VarDecl>("subscriptedArray");
      auto numberOfArrayAccesses =
          numberOfAccessesPerArray.find(subscriptedArray);
      if (numberOfArrayAccesses != numberOfAccessesPerArray.end()) {
        numberOfArrayAccesses->second += 1;
      } else {
        numberOfAccessesPerArray.insert({subscriptedArray, 1});
      }
    }
  } callbackStruct;

  MatchFinder arraySubscriptExpressionFinder;
  arraySubscriptExpressionFinder.addMatcher(
      clang::ast_matchers::findAll(arraySubscriptExpressionMatcher),
      &callbackStruct);

  arraySubscriptExpressionFinder.match(*loopInfo.loop_stmt, *context);

  loopInfo.number_of_iteration_variable_references_in_array_subscripts =
      callbackStruct
          .totalNumberOfArrayAccessesWhoseRHSInvolvesTheIterationVariable;
  // TODO: the following assignment copies a map. Probably not ideal.
  // callbackStruct should have modified
  // loopInfo.number_of_array_accesses_involving_iteration_variable_per_array in place.
  loopInfo.number_of_array_accesses_involving_iteration_variable_per_array =
      callbackStruct.numberOfAccessesPerArray;
}
}
}
}
