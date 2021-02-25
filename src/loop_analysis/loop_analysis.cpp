#include "loop_analysis.h"
#include "internal.h"
using namespace reduction_detector::loop_analysis::internal;

#include "../command_line.h"

#include "llvm/Support/CommandLine.h"

using namespace clang::ast_matchers;

namespace reduction_detector {
namespace loop_analysis {

// Define command-line option --print-non-reduction-loops.
llvm::cl::opt<bool> print_non_reduction_loops(
    "print-non-reduction-loops",
    llvm::cl::desc(
        "Instead of likely reduction loops, print loops that were not "
        "recognized as likely reduction loops"),
    llvm::cl::cat(reduction_detector::command_line_options::
                      reduction_detector_option_category));

clang::ast_matchers::StatementMatcher loopMatcher =
    stmt(anyOf(forStmt(), whileStmt(), doStmt())).bind("loop");

// run is called by MatchFinder for each loop matched by loopMatcher.
void LoopAnalyser::run(const MatchFinder::MatchResult &result) {
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

  determineIterationVariable(loop_info, context);

  detectIterationVariableReferencesInArraySubscripts(loop_info, context);

  getPossibleAccumulatorsIn(&loop_info, context);

  detectPossibleAccumulatorReferencesInRHSOfPossibleAccumulatingStatements(
      loop_info, context);

  countOutsideReferencesIn(loop_info, context);

  detectIterationVariableReferencesInApparentAccumulatingStatements(loop_info,
                                                                    context);

  analysePossibleAccumulatorNames(loop_info, context);

  determineLikelyAccumulatorsIn(loop_info, context);

  if ((!print_non_reduction_loops && loop_info.hasALikelyAccumulator) ||
      (print_non_reduction_loops && !loop_info.hasALikelyAccumulator)) {
    loop_info.dump(llvm::outs(), context);
  }

  registerAnalyzedLoop(*this, loop_info);
}
}
}
