#include "loop_analysis.h"

#include "clang/ASTMatchers/ASTMatchers.h"
using namespace clang::ast_matchers;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

StatementMatcher iterationVariableMatcher =
    forStmt(
        anyOf(hasLoopInit( // The loop initialization can declare our iteration
                           // variable...
                  declStmt(hasDescendant(varDecl().bind("iterationVariable")))),
              hasLoopInit( // ... or assign a value to a iteration variable
                           // declared beforehand.
                  stmt(binaryOperator(hasOperatorName("="),
                                      hasLHS(declRefExpr(to(varDecl().bind(
                                          "iterationVariable")))))))))
        .bind("forLoop");

struct IterationVariableCallback : public MatchFinder::MatchCallback {
  const clang::VarDecl *iterationVariable = nullptr;
  virtual void run(const MatchFinder::MatchResult &result) {
    iterationVariable =
        result.Nodes.getNodeAs<clang::VarDecl>("iterationVariable");
  };
};

void determineIterationVariable(PossibleReductionLoopInfo &loopInfo,
                                clang::ASTContext *context) {
  IterationVariableCallback iterationVariableCallback;
  MatchFinder iterationVariableFinder;
  iterationVariableFinder.addMatcher(iterationVariableMatcher,
                                     &iterationVariableCallback);
  iterationVariableFinder.match(*loopInfo.forStmt, *context);
  loopInfo.iteration_variable = iterationVariableCallback.iterationVariable;
}
}
}
}
