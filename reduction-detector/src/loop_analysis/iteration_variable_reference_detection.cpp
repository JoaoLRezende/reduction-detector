#include "reduction-detector/loop_analysis.h"

#include "clang/AST/ASTContext.h"

#include "clang/ASTMatchers/ASTMatchers.h"
using namespace clang::ast_matchers;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

struct IterationVariableReferenceCallback : public MatchFinder::MatchCallback {
  bool foundReference = false;
  virtual void run(const MatchFinder::MatchResult &result) {
    foundReference = true;
  }
};

void detectIterationVariableReferencesInApparentAccumulatingStatements(
    PotentialReductionLoopInfo &loopInfo, clang::ASTContext *context) {
  if (loopInfo.iteration_variable == nullptr)
    return;

  // Construct a matcher that will match assignments that contain a
  // reference to the loop's iteration variable.
  StatementMatcher iterationVariableReferenceMatcher =
      binaryOperator(hasDescendant(
          declRefExpr(to(varDecl(equalsNode(loopInfo.iteration_variable))))));

  for (auto &potentialAccumulatorInfo : loopInfo.potential_accumulators) {
    for (const clang::BinaryOperator *const potentialAccumulatingAssignment :
         potentialAccumulatorInfo.second.potential_accumulating_assignments) {
      IterationVariableReferenceCallback iterationVariableReferenceCallback;
      MatchFinder iterationVariableReferenceFinder;
      iterationVariableReferenceFinder.addMatcher(
          iterationVariableReferenceMatcher,
          &iterationVariableReferenceCallback);
      iterationVariableReferenceFinder.match(*potentialAccumulatingAssignment,
                                             *context);
      if (iterationVariableReferenceCallback.foundReference) {
        potentialAccumulatorInfo.second
            .number_of_potential_accumulating_assignments_that_reference_the_iteration_variable +=
            1;
      }
    }
  }
}
}
}
}
