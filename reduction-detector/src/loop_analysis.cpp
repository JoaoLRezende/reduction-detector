#include "reduction-detector/loop_analysis.h"

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

struct PotentialAccumulatorInfo {
  /* The number of references to this variable outside of assignments
   * that change its value.
   */
  unsigned int outside_references = 0;
};

struct PotentialReductionLoopInfo {
  ForStmt *forStmt = nullptr;
  VarDecl *iteration_variable = nullptr;
  std::map<const VarDecl *, PotentialAccumulatorInfo> potential_accumulators;
};

/*
 * One instance of PotentialAccumulatorAnalyser is created for each for loop.
 * For each assignment that looks like a potential reduction assignment
 * (for example: sum += array[i]) in that loop, it determines how many times
 * that assignment's assignee (which is then a potential accumulator)
 * appears in expressions other than that assignment and other assignments
 * to that variable.
 */
class PotentialAccumulatorAnalyser : public MatchFinder::MatchCallback {

  /*
   * The address of a structure describing
   * the for loop this instance was created to search.
   */
  PotentialReductionLoopInfo *loop_info;

public:
  PotentialAccumulatorAnalyser(PotentialReductionLoopInfo *loop_info)
      : loop_info(loop_info) {}

  // run is called by MatchFinder for each reduction-looking assignment.
  virtual void run(const MatchFinder::MatchResult &result) {

    // Get the matched assignment.
    const BinaryOperator *assignment =
        result.Nodes.getNodeAs<BinaryOperator>("possibleReductionAssignment");

    // Get the assignment's assignee.
    const VarDecl *potentialAccumulator =
        result.Nodes.getNodeAs<VarDecl>("potentialAccumulator");

    // TODO: check in loop_info whether we have already analysed this potential
    // accumulator. If we have, do nothing and return.
    auto maybePotentialAccumulatorInfo =
        loop_info->potential_accumulators.find(potentialAccumulator);
    if (maybePotentialAccumulatorInfo ==
        loop_info->potential_accumulators.end()) {
      return;
    }

    PotentialAccumulatorInfo potential_accumulator_info;

    // Construct a matcher that matches other references to the assignee.
    /*
     * If issues arise in detecting other references to potentialAccumulator
     * in this way, consider doing as described in
     * https://clang.llvm.org/docs/LibASTMatchersTutorial.html#:~:text=the%20%E2%80%9Ccanonical%E2%80%9D%20form%20of%20each%20declaration.
     */
    /* TODO: this matcher also matches references to potentialAccumulator
     * in other assignments that change its value.
     * It shouldn't. It should ignore all of those assignments.
     */
    StatementMatcher outsideReferenceMatcher =
        findAll(declRefExpr(to(varDecl(equalsNode(potentialAccumulator))),
                            unless(hasAncestor(equalsNode(assignment))))
                    .bind("outsideReference"));

    // Count number of outside references.
    OutsideReferenceAccumulator outsideReferenceAccumulator;
    MatchFinder referenceFinder;
    referenceFinder.addMatcher(outsideReferenceMatcher,
                               &outsideReferenceAccumulator);
    referenceFinder.match(loop_info->forStmt, *result.Context);

    // Get the resuling count of outside references.
    potential_accumulator_info.outside_references =
        outsideReferenceAccumulator.outsideReferences;

    loop_info->potential_accumulators.insert(
        {potentialAccumulator, potential_accumulator_info});
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
  ASTContext *context = result.Context;

  const ForStmt *forStmt = result.Nodes.getNodeAs<ForStmt>("forLoop");

  // If this loop is in an included header file, do nothing.
  if (!forStmt ||
      !context->getSourceManager().isWrittenInMainFile(forStmt->getForLoc()))
    return;

  // TODO: instantiate the relevant structs (defined above) and populate them
  PotentialReductionLoopInfo loop_info;
  // TODO: get iteration variable, if there is one.

  PotentialAccumulatorAnalyser potentialAccumulatorAnalyser(&loop_info);

  MatchFinder reductionAssignmentFinder;
  reductionAssignmentFinder.addMatcher(reductionAssignmentMatcher,
                                       &potentialAccumulatorAnalyser);
  reductionAssignmentFinder.match(*forStmt, *context);

  // TODO:
  // if (we were given "--debug-loop-analysis" or something) {
  //   // TODO: report all the information we gathered on this for loop.
  //   llvm::errs() << "Found a for loop at ";
  //   forStmt->getForLoc().print(llvm::errs(), context->getSourceManager());
  //   llvm::errs() << ":\n";
  //   forStmt->printPretty(llvm::errs(), nullptr, PrintingPolicy(LangOptions()));

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
