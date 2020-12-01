#include "reduction-detector/loop_analysis.h"
using namespace reduction_detector::loop_analysis::internal;

#include "reduction-detector/potential_accumulator_detection.h"

#include <map>

#include "clang/AST/ASTContext.h"
using clang::ASTContext;

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "clang/AST/Stmt.h"
using clang::ForStmt;

#include "clang/AST/Expr.h"
using clang::BinaryOperator;

#include "clang/AST/Decl.h"
using clang::VarDecl;

#include "clang/Basic/LangOptions.h"
using clang::LangOptions;

#include "clang/AST/PrettyPrinter.h"
using clang::PrintingPolicy;

#include "reduction-detector/reduction_assignment_matchers.h"
using reduction_detector::reduction_assignment_matchers::
    reductionAssignmentMatcher;

#include "reduction-detector/constants.h"

using namespace clang::ast_matchers;

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

  // TODO: get iteration variable, if there is one.

  // Find potential accumulators (populating loop_info.potential_accumulators).
  // TODO: review that class's definition. Make sure it does that (and only
  // that).
  // TODO: take this boilerplate code out into a static method of that class.
  // Also do analogously to the other classes used here.
  getPotentialAccumulatorsIn(&loop_info, context);

  // TODO: review PotentialAccumulatorOutsideReferenceCounter and use it here
  // here.

  // TODO:
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

  if (loop_info.potential_accumulators.size() > 0) {
    likelyReductionCount += 1;
  }
  totalLoopCount += 1;
}
}
}
