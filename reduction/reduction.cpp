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
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

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
StatementMatcher reduceAssignmentMatcher1 =
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
StatementMatcher reduceAssignmentMatcher2 =
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
 * But there should be a better way to do this.
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

/* A matcher that matches a for loop whose body has an assignment that is
 * matched by any of the assignment matchers defined above.
 */
StatementMatcher LoopMatcher = 
forStmt(
  /*hasLoopInit(
    declStmt(hasSingleDecl(varDecl(
      hasInitializer(
        integerLiteral(equals(0)))).bind("initVarName")))),*/
  hasIncrement(
    unaryOperator(
      hasOperatorName("++"),
      hasUnaryOperand(
        declRefExpr(to(varDecl(hasType(isInteger())).bind("incVarName")))))),
  hasCondition(
    binaryOperator(
      anyOf(hasOperatorName("<"), hasOperatorName("<=")),
      hasLHS(
        ignoringParenImpCasts(declRefExpr(to(varDecl(hasType(isInteger())).bind("condVarName"))))),
      hasRHS(
        expr(hasType(isInteger()))))),
  hasBody(
    // The loop's body needs to have an assignment that matches one of the assignment matchers.
    compoundStmt(   // TODO: consider that the body of a for loop isn't necessarily a compound statement (i.e. a block). It can be simply an expression statement.
      hasAnySubstatement(
        anyOf(reduceAssignmentMatcher1, reduceAssignmentMatcher2, reduceCompoundAssignmentMatcher)
      ),
      // The loop's body can't have another assignment that involves the accumulator.
      unless(anyOf(
        hasAnySubstatement(
          binaryOperator(
            hasOperatorName("="),
            hasRHS(
              binaryOperator( // TODO: why do we need a binary operator here? Why not simply check whether the assignment's RHS has the accumulator as a descendant? Try that and test.
                hasLHS(
                  hasDescendant(
                    declRefExpr(to(varDecl(equalsBoundNode("accumulator"))))
                  )
                ),
                hasRHS(
                  hasDescendant(
                    declRefExpr(to(varDecl(equalsBoundNode("accumulator"))))
                  )
                )
              )
            )/*,
          unless(equalsBoundNode("reduce"))*/
          )
        ),
        /* An accumulator does nothing other than accumulate. It doesn't affect
         * the computation in other ways.
         * Thus, no other variable receives a value that depends on the value of the accumulator.
         * TODO: this matcher looks for _any_ binary operator that has the accumulator only in
         * its right-hand side, rather than only for assignments. Is this desired?
         */
        hasAnySubstatement(
          binaryOperator(//esse binaryoperator precisa ser igual os dos matchers // TODO: huh? Why?
            hasRHS(
              hasDescendant(
                declRefExpr(to(varDecl(equalsBoundNode("accumulator"))))
              )
            ),
            unless(
              hasLHS(
                declRefExpr(to(varDecl(equalsBoundNode("accumulator"))))
              )
            )
          )
        ),
        /* An accumulator doesn't receive a value that doesn't depend on its previous value.
         * Thus, in a reduce loop, there is no assignment that has the accumulator in its left-hand side
         * but not in its right-hand side.
         * TODO: this matcher looks for _any_ binary operator that has the accumulator only in
         * its left-hand side, rather than only for assignments. Is this desired?
         */
        hasAnySubstatement(
          binaryOperator(
            hasLHS(
              declRefExpr(to(varDecl(equalsBoundNode("accumulator"))))
            ),
            unless(
              hasRHS(
                hasDescendant(
                  declRefExpr(to(varDecl(equalsBoundNode("accumulator"))))
                )
              )
            )
          )
        )
      ))
    )
  )
).bind("forLoop");

StatementMatcher LoopMatcher2 = 
forStmt(
  /*hasLoopInit(
    declStmt(hasSingleDecl(varDecl(
      hasInitializer(
        integerLiteral(equals(0)))).bind("initVarName")))),*/
  hasIncrement(
    unaryOperator(
      hasOperatorName("--"),
      hasUnaryOperand(
        declRefExpr(to(varDecl(hasType(isInteger())).bind("incVarName")))))),
  hasCondition(
    binaryOperator(
      anyOf(hasOperatorName(">"), hasOperatorName(">=")),
      hasLHS(
        ignoringParenImpCasts(declRefExpr(to(varDecl(hasType(isInteger())).bind("condVarName"))))),
      hasRHS(
        expr(hasType(isInteger()))))),
  hasBody(
    compoundStmt(
      hasAnySubstatement(
        anyOf(reduceAssignmentMatcher1, reduceAssignmentMatcher2, reduceCompoundAssignmentMatcher)
      ),
      unless(anyOf(
        hasAnySubstatement(
          binaryOperator(
            hasOperatorName("="),
            hasRHS(
              binaryOperator(
                hasLHS(
                  hasDescendant(
                    declRefExpr(to(varDecl(equalsBoundNode("accumulator"))))
                  )
                ),
                hasRHS(
                  hasDescendant(
                    declRefExpr(to(varDecl(equalsBoundNode("accumulator"))))
                  )
                )
              )
            )/*,
          unless(equalsBoundNode("reduce"))*/
          )
        ),
        hasAnySubstatement(
          binaryOperator(//esse binaryoperator precisa ser igual os dos matchers
            hasRHS(
              hasDescendant(
                declRefExpr(to(varDecl(equalsBoundNode("accumulator"))))
              )
            ),
            unless(
              hasLHS(
                declRefExpr(to(varDecl(equalsBoundNode("accumulator"))))
              )
            )
          )
        ),
        hasAnySubstatement(
          binaryOperator(
            hasLHS(
              declRefExpr(to(varDecl(equalsBoundNode("accumulator"))))
            ),
            unless(
              hasRHS(
                hasDescendant(
                  declRefExpr(to(varDecl(equalsBoundNode("accumulator"))))
                )
              )
            )
          )
        )
      ))
    )
  )
).bind("forLoop");


class LoopPrinter : public MatchFinder::MatchCallback {
public :
  static int loops_found;
    
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
    
      loops_found += 1;
    }
};
int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  LoopPrinter Printer;
  MatchFinder Finder;
  Finder.addMatcher(LoopMatcher, &Printer);
  Finder.addMatcher(LoopMatcher2, &Printer);

  int status_code = Tool.run(newFrontendActionFactory(&Finder).get());
  
  llvm::outs() << "Potential reduce loops: " << LoopPrinter::loops_found << "\n";
  
  return status_code;
}

/* TODO:
 * - To facilitate regression testing, use a static variable to store the number of loops recognized as potential
 *   reduce loops, and print that number at the end of execution. Then, separate all test loops into two
     files, one including only actual reduce loops and, the other, none.
 * - Don't capitalize the first letter of variable names.
 * - Run this on NAS benchmark code, and evaluate the results.
 * - Comment the code.
 * - Use deeper indentation to make reading easier. At least 4 spaces.
 * - Fix indentation.
 * - Consider that a reduction assignment can involve a function rather than a raw arithmetic operator: sum = f(sum, array[i]).
 * - If we plan to work with C++ too, we'll probably have to deal with other ways of iterating over an array (like range-based for loops)
 *   and with collections other than arrays.
 * - What happens when we have multiple possible accumulators (i.e. multiple nodes bound to the name "accumulator"?) Test. Deal with that.
 * - Make a basic testing framework that allows a good number of regression tests without requiring laborious output-checking effort.
     Each loop would go in a separate function with a unique name. Each function would be named either reduce<n> or notAReduce<n>,
     where <n> is an integer. The main function would then simply check whether each loop is or is not detected in accordance with
     the enclosing function's name. (It probably is trivial to get the name of the function in which a loop was detected.)
     Then, collect all tests in a single file.
 */
