// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/ASTContext.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory myToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...");

static bool areSameVariable(const ValueDecl *First, const ValueDecl *Second) {
  return First && Second &&
         First->getCanonicalDecl() == Second->getCanonicalDecl();
}

/* A matcher that matches any assignment whose right-hand side
 * is a binary operation whose first operand contains the assignee,
 * while its second operand doesn't.
 * For example: sum = sum + array[i]
 */
StatementMatcher reduceSimpleAssignmentMatcher1 =
  binaryOperator(
    hasOperatorName("="),
    hasLHS(
      declRefExpr(to(varDecl().bind("accumulator")))), 
    
    hasRHS(
      binaryOperator(
        hasLHS(
          hasDescendant(
            declRefExpr(to(varDecl(equalsBoundNode("accumulator")))))), 
        unless(hasRHS(
          hasDescendant(
            declRefExpr(to(varDecl(equalsBoundNode("accumulator"))))))))) 
  ).bind("reduce");

/* A matcher that matches any assignment whose right-hand side
 * is a binary operation whose second operand contains the assignee,
 * while its first operand doesn't.
 * For example: sum = array[i] + sum
 */
StatementMatcher reduceSimpleAssignmentMatcher2 =
  binaryOperator(
    hasOperatorName("="),
    hasLHS(
      declRefExpr(to(varDecl().bind("accumulator")))), 
    
    hasRHS(
      binaryOperator(
        hasRHS(
            ignoringParenImpCasts(
             declRefExpr(to(varDecl(equalsBoundNode("accumulator")))))))), 
    unless(hasDescendant(binaryOperator(hasLHS(hasDescendant(declRefExpr(to(varDecl(equalsBoundNode("accumulator")))))))))).bind("reduce");

/* TODO: the two matchers above seem to be an attempt to exclude assignments
 * whose assignee appear in their right-hand side multiple times.
 * (For example: sum = sum + sum.)
 * But it doesn't always work. For example, it succeeds at excluding
 * sum = sum + (sum + x)
 * (since both operands of that addition involve sum)
 * but doesn't exclude
 * sum = (sum + sum) + x
 * (since only one of the operands of the outermost addition involves sum).
 * Thus, it also doesn't exclude
 * sum = sum + sum + x
 * since the plus operator is left-associative.
 * There should be a better way to do this.
 * Maybe we can bind the first reference to the potential accumulator we find in the right-hand
 * side to a name and then see if there is still any reference to that potential
 * accumulator in that same right-hand side that _isn't_ bound to that name.
 * (This might require some procedural code.)
 */

/* A matcher that matches a compound assignment whose left-hand side is a variable
 * that does not appear in its right-hand side.
 */
StatementMatcher reduceCompoundAssignmentMatcher = 
  binaryOperator(
    anyOf(hasOperatorName("+="), hasOperatorName("-="), hasOperatorName("*="), hasOperatorName("/=")), 
    
    hasLHS(
      declRefExpr(to(varDecl().bind("accumulator")))),
    unless(hasRHS(
      hasDescendant(
        declRefExpr(to(varDecl(equalsBoundNode("accumulator")))))))
  ).bind("reduce");

StatementMatcher reduceAssignmentMatcher = 
    findAll(stmt(
        anyOf(
            reduceSimpleAssignmentMatcher1,
            reduceSimpleAssignmentMatcher2,
            reduceCompoundAssignmentMatcher // TODO: bind the matching node to a name here (and use a more reasonable name) rather than in the inner matchers.
        )
    ));

// old. remove after recycling.
class LoopPrinter : public MatchFinder::MatchCallback {
public :
  virtual void run(const MatchFinder::MatchResult &Result) {
      ASTContext *Context = Result.Context;
      llvm::outs() << "Hello!!\n";
      const ForStmt *FS = Result.Nodes.getNodeAs<ForStmt>("forLoop");
      // We do not want to convert header files!
      if(!FS || !Context->getSourceManager().isWrittenInMainFile(FS->getForLoc()))
        return;
      const VarDecl *IncVar = Result.Nodes.getNodeAs<VarDecl>("incVarName");
      const VarDecl *CondVar = Result.Nodes.getNodeAs<VarDecl>("condVarName");
      //const VarDecl *InitVar = Result.Nodes.getNodeAs<VarDecl>("initVarName");

      if(!areSameVariable(IncVar, CondVar)/* || !areSameVariable(IncVar, InitVar)*/){
        llvm::outs() << "for variables didnt match\n";
        return;
      }
      FS->dump();
      llvm::outs() << "Potential Reduce pattern, in the next line you'll find the file and the suspect line.\n";
      FS->getForLoc().dump(Context->getSourceManager());
      llvm::outs() << "\n";
    }
};

StatementMatcher loopMatcher = forStmt().bind("forLoop");

/*
 * LoopChecker's run method is called for every for loop.
 * It checks whether a for loop looks like a reduction operation.
 */
class LoopChecker : public MatchFinder::MatchCallback {
  /*
   * In a for loop, for each assignment that looks like a possible
   * reduction assignment (e.g. sum += array[i]), we check whether
   * that assignment's assignee (which is then a possible accumulator)
   * appears in any other expression in the loop's body.
   * This is done by AccumulatorChecker.
   */
  class AccumulatorChecker : public MatchFinder::MatchCallback {

    const ForStmt *forStmt;

  public:

    AccumulatorChecker(const ForStmt *forStmt) {
      this->forStmt = forStmt;
    }

    virtual void run(const MatchFinder::MatchResult &result) {
        llvm::outs() << "Found a possible reduction assignment. AccumulatorChecker in action.\n";

      // Get the matched assignment.
      const BinaryOperator *assignment = result.Nodes.getNodeAs<BinaryOperator>("reduce");
      llvm::errs() << "Possible reduction assignment: ";
      assignment->dump();

      // Get the assignment's assignee.
      const VarDecl *possibleAccumulator = result.Nodes.getNodeAs<VarDecl>("accumulator");

      // Construct a matcher that matches other references to the assignee.
      StatementMatcher outsideReferenceMatcher =
        declRefExpr(
          to(varDecl(equalsNode(possibleAccumulator))),
          unless(hasAncestor(equalsNode(assignment)))
        );
    };
  };

  public:

    virtual void run(const MatchFinder::MatchResult &result) {
      ASTContext *context = result.Context;

      const ForStmt *forStmt = result.Nodes.getNodeAs<ForStmt>("forLoop");

      llvm::outs() << "Found a for loop in the following location:\n";
      forStmt->getForLoc().dump(context->getSourceManager());
      llvm::outs() << "\n";
      forStmt->dump();

      // We do not want to scan header files.
      if(!forStmt || !context->getSourceManager().isWrittenInMainFile(forStmt->getForLoc()))
        return;

      /*
       * TODO: for each assignment that looks like a reduction assignment,
       * check whether its lvalue
       * is referenced in any other expression of the loop.
       * If it isn't, then report it as a possible reduction accumulator.
       */
      AccumulatorChecker accumulatorChecker(forStmt);
      MatchFinder expressionFinder;
      expressionFinder.addMatcher(reduceAssignmentMatcher, &accumulatorChecker);
      expressionFinder.match(*forStmt, *context);
    }
};

int main(int argc, const char **argv) {
  CommonOptionsParser optionsParser(argc, argv, myToolCategory);
  ClangTool Tool(optionsParser.getCompilations(),
                 optionsParser.getSourcePathList());

  LoopChecker loopChecker;
  MatchFinder finder;
  finder.addMatcher(loopMatcher, &loopChecker);

  return Tool.run(newFrontendActionFactory(&finder).get());
}

/* TODO:
 * - PROBLEMA ATUAL EH BASICAMENTE: QUANDO ENCONTRO UM ACUMULADOR EM
 *   POTENCIAL, DEVO CONSEGUIR VERIFICAR SE APARECE APENAS UMA VEZ NO
 *   RESTO DA EQUACAO, E DEPOIS VERIFICAR SE APARECE NOVAMENTE EM ALGUMA
 *   EQUACAO DENTRO DO FOR. MAS COMO GUARDAR ONDE DA EQUACAO/FOR QUE
 *   ENCONTREI O ACUMULADOR EM QUESTAO?
 * - At the end of a run, print something like "__ out of __ loops recognized as possible reductions".
 *   That should facilitate checking whether all loops were recognized.
 * - Get more example loops from real software systems. (See PARSEC, Cowichan.)
 * - How well do we deal with nested loops? Write some test cases for that.
 * - Optionally print detected loops themselves, rather than simply their locations.
 * - Be able to receive a directory as input (rather than only a single file). Then, test on
 *   larger software systems (such as Linux).
 * - Comment the code.
 * - Fix indentation. Use deeper indentation to make reading easier. At least 4 spaces.
 * - Consider that a reduction assignment can involve a function rather than a raw arithmetic operator: sum = f(sum, array[i]).
 * - If we plan to work with C++ too, we'll probably have to deal with other ways of iterating over an array (like range-based for loops)
 *   and with collections other than arrays.
 * - What happens when we have multiple possible accumulators (i.e. multiple nodes bound to the name "accumulator"?) Add test cases. Deal with that.
 * - Make a basic testing framework that allows a good number of regression tests without requiring laborious output-checking effort.
     Each loop would go in a separate function with a unique name. Each function would be named either reduce<n> or notAReduce<n>,
     where <n> is an integer. The main function would then simply check whether each loop is or is not detected in accordance with
     the enclosing function's name. (It probably is trivial to get the name of the function in which a loop was detected.)
     Then, collect all tests in a single file.
   - Should we recognize loops like the following as reductions?
        for (int i = 0; i < arr_length; i++)
            if (max < arr[i])
                max = arr[i];

 */
