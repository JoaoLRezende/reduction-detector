#include "reduction-detector/loop_analysis.h"
using namespace reduction_detector::loop_analysis::internal;

#include "reduction-detector/constants.h"

#include "reduction-detector/reduction_assignment_matchers.h"
using reduction_detector::reduction_assignment_matchers::
    reductionAssignmentMatcher;

#include <map>

#include "clang/AST/ASTContext.h"
using clang::ASTContext;

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "clang/AST/Stmt.h"
using clang::ForStmt;

#include "clang/Basic/LangOptions.h"
using clang::LangOptions;

#include "clang/AST/PrettyPrinter.h"
using clang::PrintingPolicy;

using namespace clang::ast_matchers;

#include "llvm/Support/CommandLine.h"

namespace reduction_detector {
namespace loop_analysis {

// run is called by MatchFinder for each for loop.
void LoopAnalyser::run(const MatchFinder::MatchResult &result) {
  PotentialReductionLoopInfo loop_info(
      result.Nodes.getNodeAs<ForStmt>("forLoop"));

  ASTContext *context = result.Context;

  // If this loop is in an included header file, do nothing.
  if (!loop_info.forStmt ||
      !context->getSourceManager().isWrittenInMainFile(
          loop_info.forStmt->getForLoc())) {
    return;
  }

  // Determine the loop's iteration variable, if there is one (and set
  // loop_info.iteration_variable).
  determineIterationVariable(loop_info, context);

  // Find potential accumulators (populating loop_info.potential_accumulators).
  getPotentialAccumulatorsIn(&loop_info, context);

  // For each potential accumulator, count the number of times it is referenced
  // in that loop outside of its potential accumulating assignments
  // (and store that number in the structure that describes that potential
  // accumulator).
  countOutsideReferencesIn(&loop_info, context);

  detectIterationVariableReferencesInApparentAccumulatingStatements(loop_info, context);

  analysePotentialAccumulatorNames(loop_info, context);

  // Decide whether each potential accumulator is a likely accumulator.
  // Set each potential accumulator's likelyAccumulatorScore and
  // isLikelyAccumulator.
  determineLikelyAccumulatorsIn(loop_info, context);

  if (loop_info.hasALikelyAccumulator) {
    loop_info.dump(llvm::errs(), context);
  }

  // TODO:
  // recycle the following stuff. use printPotentialAccumulatorsIn and
  // printOutsideReferenceCountsIn
  // if (we were given "--debug-loop-analysis" or something) {
  //   // TODO: report all the information we gathered on this for loop.
  //   llvm::errs() << "Found a for loop at ";
  //   forStmt->getForLoc().print(llvm::errs(), context->getSourceManager());
  //   llvm::errs() << ":\n";
  //   forStmt->printPretty(llvm::errs(), nullptr,
  //   PrintingPolicy(LangOptions()));

  //   if (accumulatorChecker.likelyAccumulatorsFound) {
  //     llvm::errs() << INDENT "This might be a reduction loop.\n";
  //   } else {
  //     llvm::errs() << INDENT "This probably isn't a reduction loop.\n";
  //   }
  //   loop_analysis_report_stream << "\n";
  // }

  if (loop_info.hasALikelyAccumulator) {
    likelyReductionCount += 1;
  }
  totalLoopCount += 1;
}
}
}
