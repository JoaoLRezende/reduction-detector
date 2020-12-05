#include "reduction-detector/loop_analysis.h"

#include "clang/AST/ASTContext.h"

#include "clang/ASTMatchers/ASTMatchers.h"
using namespace clang::ast_matchers;

#include "reduction-detector/reduction_assignment_matchers.h"
using reduction_detector::reduction_assignment_matchers::
    reductionAssignmentMatcher;

#include "llvm/Support/CommandLine.h"

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

llvm::cl::opt<bool>
    debugPotentialAccumulatorOutsideReferenceCounting("debug-potential-accumulator-reference-counting");

/* One instance of OutsideReferenceAccumulator is created for each
 * potential accumulator we check. Its sole purpose is to count
 * how many references to its assignee occur outside
 * of potential accumulation assignments (which is the number of times its run
 * method is called by MatchFinder).
 */
class OutsideReferenceAccumulator : public MatchFinder::MatchCallback {
public:
  unsigned int outsideReferences = 0;
  virtual void run(const MatchFinder::MatchResult &result) {
    outsideReferences += 1;
  }
};

/*
 * One instance of PotentialAccumulatorOutsideReferenceCounter is created for
 * each for loop.
 * For each potential accumulator, it counts the number of references to it
 * that exist in the body of the loop but outside of the assignments
 * that seem to accumulate a value in that potential accumulator.
 */
void countOutsideReferencesIn(PotentialReductionLoopInfo *loop_info,
                              clang::ASTContext *context) {
  for (auto &potentialAccumulator : loop_info->potential_accumulators) {
    // Construct a matcher that matches other references to the assignee.
    /*
     * If issues arise in detecting other references to potentialAccumulator
     * in this way, consider doing as described in
     * https://clang.llvm.org/docs/LibASTMatchersTutorial.html#:~:text=the%20%E2%80%9Ccanonical%E2%80%9D%20form%20of%20each%20declaration.
     */
    StatementMatcher outsideReferenceMatcher = findAll(
        declRefExpr(to(varDecl(equalsNode(potentialAccumulator.first))),
                    unless(hasAncestor(binaryOperator(allOf(
                        reductionAssignmentMatcher,
                        hasLHS(hasDescendant(declRefExpr(to(varDecl(
                            equalsNode(potentialAccumulator.first)))))))))))
            .bind("outsideReference"));

    // Count number of outside references.
    OutsideReferenceAccumulator outsideReferenceAccumulator;
    MatchFinder outsideReferenceFinder;
    outsideReferenceFinder.addMatcher(outsideReferenceMatcher,
                                      &outsideReferenceAccumulator);
    outsideReferenceFinder.match(*loop_info->forStmt, *context);

    // Get the resulting count of outside references.
    potentialAccumulator.second.outside_references =
        outsideReferenceAccumulator.outsideReferences;

    if (debugPotentialAccumulatorOutsideReferenceCounting) {
      llvm::errs() << potentialAccumulator.second.outside_references <<
      " references to " << potentialAccumulator.first->getName() << " found in"
      " the loop outside of its potential accumulating assignments.\n";
    }
  }
}
}
}
}
