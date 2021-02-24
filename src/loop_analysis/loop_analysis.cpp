#include "loop_analysis.h"
#include "internal.h"
using namespace reduction_detector::loop_analysis::internal;

#include "../command_line.h"

#include "llvm/Support/CommandLine.h"

llvm::cl::opt<bool> print_non_reduction_loops(
    "print-non-reduction-loops",
    llvm::cl::desc(
        "Instead of likely reduction loops, print loops that were not "
        "recognized as likely reduction loops"),
    llvm::cl::cat(reduction_detector::command_line_options::
                      reduction_detector_option_category));

namespace reduction_detector {
namespace loop_analysis {

/*
 * Update the statistics stored in LoopAnalyser to account for a
 * newly analyzed loop.
 */
static void registerAnalyzedLoop(LoopAnalyser &loopAnalyser,
                                 PossibleReductionLoopInfo &loopInfo) {
  loopAnalyser.loopCounts.totals.all += 1;
  if (loopInfo.hasALikelyAccumulator) {
    loopAnalyser.loopCounts.likelyReductionLoops.all += 1;
  }

  if (clang::isa<clang::ForStmt>(loopInfo.loopStmt)) {
    loopAnalyser.loopCounts.totals.forLoops += 1;
    if (loopInfo.hasALikelyAccumulator) {
      loopAnalyser.loopCounts.likelyReductionLoops.forLoops += 1;
    }
  } else if (clang::isa<clang::WhileStmt>(loopInfo.loopStmt)) {
    loopAnalyser.loopCounts.totals.whileLoops += 1;
    if (loopInfo.hasALikelyAccumulator) {
      loopAnalyser.loopCounts.likelyReductionLoops.whileLoops += 1;
    }
  } else if (clang::isa<clang::DoStmt>(loopInfo.loopStmt)) {
    loopAnalyser.loopCounts.totals.doWhileLoops += 1;
    if (loopInfo.hasALikelyAccumulator) {
      loopAnalyser.loopCounts.likelyReductionLoops.doWhileLoops += 1;
    }
  }
}

// run is called by MatchFinder for each for loop.
void LoopAnalyser::run(
    const clang::ast_matchers::MatchFinder::MatchResult &result) {
  PossibleReductionLoopInfo loop_info(
      result.Nodes.getNodeAs<clang::Stmt>("loop"));

  // getNodeAs returns nullptr "if there was no node bound to ID or if there is
  // a node but it cannot be converted to the specified type". This shouldn't
  // happen.
  assert(loop_info.loopStmt != nullptr);

  clang::ASTContext *context = result.Context;

  // If this loop is in an included header file, do nothing.
  if (!context->getSourceManager().isWrittenInMainFile(
          loop_info.loopStmt->getBeginLoc())) {
    return;
  }

  // Determine the loop's iteration variable, if there is one (and set
  // loop_info.iteration_variable).
  determineIterationVariable(loop_info, context);

  detectIterationVariableReferencesInArraySubscripts(loop_info, context);

  // Find possible accumulators (populating loop_info.possible_accumulators).
  getPossibleAccumulatorsIn(&loop_info, context);

  detectPossibleAccumulatorReferencesInRHSOfPossibleAccumulatingStatements(
      loop_info, context);

  // For each possible accumulator, count the number of times it is referenced
  // in that loop outside of its possible accumulating assignments
  // (and store that number in the structure that describes that possible
  // accumulator).
  countOutsideReferencesIn(loop_info, context);

  detectIterationVariableReferencesInApparentAccumulatingStatements(loop_info,
                                                                    context);

  analysePossibleAccumulatorNames(loop_info, context);

  // Decide whether each possible accumulator is a likely accumulator.
  // Set each possible accumulator's likelyAccumulatorScore and
  // isLikelyAccumulator.
  determineLikelyAccumulatorsIn(loop_info, context);

  if ((!print_non_reduction_loops && loop_info.hasALikelyAccumulator) ||
      (print_non_reduction_loops && !loop_info.hasALikelyAccumulator)) {
    loop_info.dump(llvm::outs(), context);
  }

  // TODO:
  // recycle the following stuff. use printPossibleAccumulatorsIn and
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

  registerAnalyzedLoop(*this, loop_info);
}

}
}
