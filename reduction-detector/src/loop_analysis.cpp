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
  std::set<const BinaryOperator *> potential_accumulating_assignments;
  unsigned int outside_references = 0;
};

struct PotentialReductionLoopInfo {
  const ForStmt *forStmt = nullptr;
  VarDecl *iteration_variable = nullptr;
  std::map<const VarDecl *, PotentialAccumulatorInfo> potential_accumulators;

  PotentialReductionLoopInfo(const ForStmt *forStmt) : forStmt(forStmt){};
};

/*
 * One instance of PotentialAccumulatorFinder is created for each for loop.
 * For each assignment that looks like a potential reduction assignment
 * (for example: sum += array[i]) in that loop, it stores that assignment's
 * left-hand side (in that example, sum) in the potential_accumulators
 * list of the struct PotentialReductionLoopInfo that describes that loop.
 */
class PotentialAccumulatorFinder : public MatchFinder::MatchCallback {

  /*
   * The address of the structure describing
   * the for loop this instance was created to search.
   */
  PotentialReductionLoopInfo *loop_info;

  PotentialAccumulatorFinder(PotentialReductionLoopInfo *loop_info)
      : loop_info(loop_info) {}

public:
  static void getPotentialAccumulatorsIn(PotentialReductionLoopInfo *loop_info,
                                         ASTContext *context) {
    PotentialAccumulatorFinder potentialAccumulatorFinder(loop_info);

    MatchFinder potentialReductionAssignmentFinder;
    potentialReductionAssignmentFinder.addMatcher(
        findAll(reductionAssignmentMatcher), &potentialAccumulatorFinder);

    potentialReductionAssignmentFinder.match(*loop_info->forStmt, *context);
  }

  // run is called by MatchFinder for each reduction-looking assignment.
  virtual void run(const MatchFinder::MatchResult &result) {

    // Get the matched assignment.
    const BinaryOperator *assignment =
        result.Nodes.getNodeAs<BinaryOperator>("possibleReductionAssignment");

    // Get the assignment's assignee.
    const VarDecl *potentialAccumulator =
        result.Nodes.getNodeAs<VarDecl>("potentialAccumulator");

    PotentialAccumulatorInfo potential_accumulator_info;

    potential_accumulator_info.potential_accumulating_assignments.insert(
        assignment);

    // Note that std::map.insert does nothing if there already is an element
    // with the given key.
    loop_info->potential_accumulators.insert(
        {potentialAccumulator, potential_accumulator_info});
  };
};

/*
 * One instance of PotentialAccumulatorOutsideReferenceCounter is created for
 * each for loop.
 * For each potential accumulator, it counts the number of references to it
 * that exist in the body of the loop but outside of assignments whose
 * left side is that potential accumulator.
 */
class PotentialAccumulatorOutsideReferenceCounter {
public:
  static void countOutsideReferencesIn(PotentialReductionLoopInfo *loop_info,
                                       ASTContext *context) {
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
    }
  }

  /* One instance of OutsideReferenceAccumulator is created for each
   * potential accumulator we check. Its sole purpose is to count
   * how many references to it assignee occur outside
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
};

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
  PotentialAccumulatorFinder.getPotentialAccumulatorsIn(&loop_info, context);

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
