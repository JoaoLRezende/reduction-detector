#include "reduction-detector/potential_accumulator_detection.h"

#include "reduction-detector/loop_analysis.h"

#include "reduction-detector/constants.h"

#include "reduction-detector/reduction_assignment_matchers.h"
using reduction_detector::reduction_assignment_matchers::
    reductionAssignmentMatcher;

#include "clang/AST/ASTContext.h"
using clang::ASTContext;

#include "clang/ASTMatchers/ASTMatchFinder.h"
using clang::ast_matchers::MatchFinder;

#include "clang/AST/Expr.h"
using clang::BinaryOperator;

#include "clang/AST/Decl.h"
using clang::VarDecl;

#include "llvm/Support/CommandLine.h"

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

llvm::cl::opt<bool>
    debugPotentialAccumulatorDetection("debug-potential-accumulator-detection");

/*
 * One instance of PotentialAccumulatorFinder is created for each for loop.
 * For each assignment that looks like a potential reduction assignment
 * (for example: sum += array[i]) in that loop, it stores that assignment's
 * left-hand side (in that example, sum) in the potential_accumulators
 * map of the struct PotentialReductionLoopInfo that describes that loop.
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

    if (debugPotentialAccumulatorDetection) {
      llvm::errs() << INDENT "Potential accumulating assignment: ";
      assignment->printPretty(llvm::errs(), nullptr,
                              clang::PrintingPolicy(clang::LangOptions()));
    }

    PotentialAccumulatorInfo potential_accumulator_info;

    potential_accumulator_info.potential_accumulating_assignments.insert(
        assignment);

    // Note that std::map.insert does nothing if there already is an element
    // with the given key.
    loop_info->potential_accumulators.insert(
        {potentialAccumulator, potential_accumulator_info});
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

  if (debugPotentialAccumulatorDetection) {
    llvm::errs() << loop_info->potential_accumulators.size()
                 << " potential accumulators were found in the "
                    "loop:";
    for (auto &potentialAccumulator : loop_info->potential_accumulators) {
      llvm::errs() << " " << potentialAccumulator.first->getName();
    }
    llvm::errs() << "\n";
  }
}
}
}
}
