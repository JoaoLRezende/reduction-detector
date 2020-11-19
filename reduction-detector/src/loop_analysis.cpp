#include "reduction-detector/loop_analysis.h"

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

#include "llvm/Support/raw_ostream.h"
using llvm::raw_string_ostream;

#include "reduction-detector/reduction_assignment_matchers.h"
using reduction_detector::reduction_assignment_matchers::
    reductionAssignmentMatcher;

#include "reduction-detector/constants.h"

using namespace clang::ast_matchers;

namespace reduction_detector {
namespace loop_analysis {

LoopAnalyser::LoopAnalyser() { loop_analysis_report_buffer = std::string(); }

/*
 * One instance of AccumulatorChecker is created for each for loop.
 * For each assignment that looks like a potential reduction assignment
 * (for example: sum += array[i]) in that loop, it checks whether
 * that assignment's assignee (which is then a potential accumulator)
 * appears in any other expression in the loop's body.
 */
class AccumulatorChecker : public MatchFinder::MatchCallback {

  /*
   * A reference to the for loop this instance was created to search
   * is kept in forStmt.
   */
  const ForStmt *forStmt;

  raw_string_ostream &loop_analysis_report_stream;

public:
  int likelyAccumulatorsFound = 0;

  AccumulatorChecker(const ForStmt *forStmt,
                     raw_string_ostream &loop_analysis_report_stream)
      : forStmt(forStmt),
        loop_analysis_report_stream(loop_analysis_report_stream) {}

  // run is called by MatchFinder for each reduction-looking assignment.
  virtual void run(const MatchFinder::MatchResult &result) {

    // Get the matched assignment.
    const BinaryOperator *assignment =
        result.Nodes.getNodeAs<BinaryOperator>("possibleReductionAssignment");

    loop_analysis_report_stream << INDENT "Possible reduction assignment: ";
    assignment->printPretty(loop_analysis_report_stream, nullptr,
                            PrintingPolicy(LangOptions()));
    loop_analysis_report_stream << "\n";

    // Get the assignment's assignee.
    const VarDecl *possibleAccumulator =
        result.Nodes.getNodeAs<VarDecl>("possibleAccumulator");

    // Construct a matcher that matches other references to the assignee.
    /*
     * If issues arise in detecting other references to possibleAccumulator
     * in this way, consider doing as described in
     * https://clang.llvm.org/docs/LibASTMatchersTutorial.html#:~:text=the%20%E2%80%9Ccanonical%E2%80%9D%20form%20of%20each%20declaration.
     */
    StatementMatcher outsideReferenceMatcher =
        findAll(declRefExpr(to(varDecl(equalsNode(possibleAccumulator))),
                            unless(hasAncestor(equalsNode(assignment))))
                    .bind("outsideReference"));

    // Count number of outside references.
    OutsideReferenceAccumulator outsideReferenceAccumulator;
    MatchFinder referenceFinder;
    referenceFinder.addMatcher(outsideReferenceMatcher,
                               &outsideReferenceAccumulator);
    referenceFinder.match(*forStmt, *result.Context);

    loop_analysis_report_stream << INDENT INDENT "└ Found "
                                << outsideReferenceAccumulator.outsideReferences
                                << " other references to "
                                << possibleAccumulator->getName()
                                << " in this for loop. ";
    if (!outsideReferenceAccumulator.outsideReferences) {
      loop_analysis_report_stream << "Thus, " << possibleAccumulator->getName()
                                  << " might be a reduction accumulator.\n";
      likelyAccumulatorsFound += 1;
    } else {
      loop_analysis_report_stream
          << "Thus, " << possibleAccumulator->getName()
          << " probably isn't a reduction accumulator.\n";
    }
  };

  /* One instance of OutsideReferenceAccumulator is created for each
   * assignment we check. Its sole purpose is to count
   * how many references to that assignment's assignee occur outside
   * of the assignment (which is the number of times its run method is
   * called by MatchFinder).
   */
  class OutsideReferenceAccumulator : public MatchFinder::MatchCallback {
  public:
    unsigned int outsideReferences = 0;
    virtual void run(const MatchFinder::MatchResult &result) {
      outsideReferences += 1;
    }
  };
};

// run is called by MatchFinder for each for loop.
void LoopAnalyser::run(const MatchFinder::MatchResult &result) {

  loop_analysis_report_buffer.clear();
  llvm::raw_string_ostream loop_analysis_report_stream(
      loop_analysis_report_buffer);

  ASTContext *context = result.Context;

  const ForStmt *forStmt = result.Nodes.getNodeAs<ForStmt>("forLoop");

  // If this loop is in an included header file, do nothing.
  if (!forStmt ||
      !context->getSourceManager().isWrittenInMainFile(forStmt->getForLoc()))
    return;

  loop_analysis_report_stream << "Found a for loop at ";
  forStmt->getForLoc().print(loop_analysis_report_stream,
                             context->getSourceManager());
  loop_analysis_report_stream << ":\n";
  forStmt->printPretty(loop_analysis_report_stream, nullptr,
                       PrintingPolicy(LangOptions()));

  /*
   * A reduction accumulator does nothing other than accumulate.
   * It doesn't affect the computation in other ways. If a
   * potential accumulator appears in multiple places in the loop,
   * then it probably isn't really an accumulator.
   * Thus, for each assignment that looks like a reduction assignment,
   * we check whether its lvalue
   * is referenced in any other expression of the loop.
   * If it isn't, then report it as a possible reduction accumulator.
   */
  AccumulatorChecker accumulatorChecker(forStmt, loop_analysis_report_stream);
  MatchFinder reductionAssignmentFinder;
  reductionAssignmentFinder.addMatcher(reductionAssignmentMatcher,
                                       &accumulatorChecker);
  reductionAssignmentFinder.match(*forStmt, *context);

  if (accumulatorChecker.likelyAccumulatorsFound) {
    loop_analysis_report_stream << INDENT "This might be a reduction loop.\n";
    likelyReductionCount += 1;
  } else {
    loop_analysis_report_stream
        << INDENT "This probably isn't a reduction loop.\n";
  }
  loop_analysis_report_stream << "\n";
  totalLoopCount += 1;

  loop_analysis_report_stream.flush(); // write buffered analysis results to
                                       // loop_analysis_report_buffer
  llvm::outs() << loop_analysis_report_buffer;
}
}
}
