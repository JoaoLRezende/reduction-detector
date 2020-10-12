// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

#define INDENT "  " // a string with 2 spaces, for indenting debug ouput

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory myToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...");

/* potentialReductionAssignment matches any simple assignment whose assignee
 * also appears in its right-hand side once (and only once).
 * For example: sum = sum + array[i]
 */
StatementMatcher potentialReductionSimpleAssignmentMatcher = binaryOperator(
    hasOperatorName("="),
    hasLHS(declRefExpr(to(varDecl().bind("possibleAccumulator")))),
    hasRHS(hasDescendant(
        declRefExpr(to(varDecl(equalsBoundNode("possibleAccumulator"))))
            .bind("referenceToPossibleAccumulatorInRHS"))),
    unless(hasRHS(hasDescendant(declRefExpr(
        to(varDecl(equalsBoundNode("possibleAccumulator"))),
        unless(equalsBoundNode("referenceToPossibleAccumulatorInRHS")))))));

/* A matcher that matches a compound assignment whose left-hand side is a
 * variable that does not appear in its right-hand side.
 */
StatementMatcher potentialReductionCompoundAssignmentMatcher = binaryOperator(
    anyOf(hasOperatorName("+="), hasOperatorName("-="), hasOperatorName("*="),
          hasOperatorName("/=")),

    hasLHS(declRefExpr(to(varDecl().bind("possibleAccumulator")))),
    unless(hasRHS(hasDescendant(
        declRefExpr(to(varDecl(equalsBoundNode("possibleAccumulator"))))))));

StatementMatcher reduceAssignmentMatcher =
    findAll(binaryOperator(anyOf(potentialReductionSimpleAssignmentMatcher,
                                 potentialReductionCompoundAssignmentMatcher))
                .bind("possibleReductionAssignment"));

/*
 * One instance of LoopChecker is created for each source file.
 * It checks every for loop in that file, accumulating statistics.
 */
class LoopChecker : public MatchFinder::MatchCallback {
public:
  unsigned int likelyReductionCount = 0;
  unsigned int totalLoopCount = 0;

  // run is called by MatchFinder for each for loop.
  virtual void run(const MatchFinder::MatchResult &result) {
    ASTContext *context = result.Context;

    const ForStmt *forStmt = result.Nodes.getNodeAs<ForStmt>("forLoop");

    // If this loop is in an included header file, do nothing.
    if (!forStmt ||
        !context->getSourceManager().isWrittenInMainFile(forStmt->getForLoc()))
      return;

    llvm::errs() << "Found a for loop at ";
    forStmt->getForLoc().dump(context->getSourceManager());
    forStmt->dumpPretty(*context);

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
    AccumulatorChecker accumulatorChecker(forStmt);
    MatchFinder reductionAssignmentFinder;
    reductionAssignmentFinder.addMatcher(reduceAssignmentMatcher,
                                         &accumulatorChecker);
    reductionAssignmentFinder.match(*forStmt, *context);

    if (accumulatorChecker.likelyAccumulatorsFound) {
      llvm::errs() << INDENT "This might be a reduction loop.\n";
      likelyReductionCount += 1;
    } else {
      llvm::errs() << INDENT "This probably isn't a reduction loop.\n";
    }
    errs() << "\n";
    totalLoopCount += 1;
  }

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

  public:
    int likelyAccumulatorsFound = 0;

    AccumulatorChecker(const ForStmt *forStmt) { this->forStmt = forStmt; }

    // run is called by MatchFinder for each reduction-looking assignment.
    virtual void run(const MatchFinder::MatchResult &result) {
      // Get the matched assignment.
      const BinaryOperator *assignment =
          result.Nodes.getNodeAs<BinaryOperator>("possibleReductionAssignment");

      llvm::errs() << INDENT "Possible reduction assignment: ";
      assignment->dumpPretty(*result.Context);
      llvm::errs() << "\n";

      // Get the assignment's assignee.
      const VarDecl *possibleAccumulator =
          result.Nodes.getNodeAs<VarDecl>("possibleAccumulator");

      // Construct a matcher that matches other references to the assignee.
      /* To check whether two declaration references that reference
       * declarations a and b in fact reference the same declaration,
       * an earlier version of this code went
       * out of its way to compare the referenced declarations with
       * a->getCanonicalDecl() == b->getCanonicalDecl()
       * procedurally instead of directly comparing AST nodes in a matcher
       * as we do here.
       * I don't know why. I don't know what a "canonical declaration" is.
       * If issues arise, consider doing that here.
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

      llvm::errs() << INDENT INDENT "└ Found "
                   << outsideReferenceAccumulator.outsideReferences
                   << " other references to " << possibleAccumulator->getName()
                   << " in this for loop. ";
      if (!outsideReferenceAccumulator.outsideReferences) {
        llvm::errs() << "Thus, " << possibleAccumulator->getName()
                     << " might be a reduction accumulator.\n";
        likelyAccumulatorsFound += 1;
      } else {
        llvm::errs() << "Thus, " << possibleAccumulator->getName()
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
};

int main(int argc, const char **argv) {
  CommonOptionsParser optionsParser(argc, argv, myToolCategory);
  ClangTool Tool(optionsParser.getCompilations(),
                 optionsParser.getSourcePathList());

  LoopChecker loopChecker;
  MatchFinder finder;
  finder.addMatcher(forStmt().bind("forLoop"), &loopChecker);

  int statusCode = Tool.run(newFrontendActionFactory(&finder).get());

  llvm::errs() << loopChecker.likelyReductionCount << " out of "
               << loopChecker.totalLoopCount
               << " loops detected as likely reduction loops.\n";

  return statusCode;
}

/* TODO:
 * - Add a test case for a loop whose body is an expression statement,
 *   rather than a compount statement (i.e. a block).
 * - Do proper encapsulation. Make public only what needs to be public.
 *   Use getter methods.
 * - Be able to receive command-line arguments. Options like
 *   --print-unrecognized-loops (which shall default
 *   to false) and --verbose, which causes it to explain all its reasonings
 *   as
 *   precisely as possible (thus printing unrecognized loops too, of
 *   course).
 * - Get more example loops from real software systems. (See PARSEC,
 *   Cowichan.)
 * - How well do we deal with nested loops? Write some test cases for that.
 * - Be able to receive a directory as input (rather than only a single
 *   file).
 *   Then, test on larger software systems (such as Linux).
 * - Comment the code.
 * - Use deeper indentation to make reading easier. At least 4 spaces.
 * - If we plan to work with C++ too, we'll probably have to deal with other
 *   ways of iterating over an array (like range-based for loops)
 *   and with collections other than arrays.
 * - Make a basic testing framework that allows a good number of regression
 *   tests without requiring laborious output-checking effort.
 *   Each loop would go in a separate function with a unique name. Each
 *   function would be named either reduce<n> or notAReduce<n>,
 *   where <n> is an integer. The main function would then simply check
 *   whether each loop is or is not detected in accordance with
 *   the enclosing function's name. (It probably is trivial to get the name
 *   of the function in which a loop was detected.)
 *   Then, collect all tests in a single file.
 * - Should we recognize loops like the following as reductions?
 *      for (int i = 0; i < arr_length; i++)
 *          if (max < arr[i])
 *              max = arr[i];
 * - Separate the code into multiple source files.
 */
