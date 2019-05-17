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

StatementMatcher LoopMatcher = forStmt(hasLoopInit(declStmt(
                                hasSingleDecl(varDecl(hasInitializer(integerLiteral(equals(0)))).bind("initVarName")))),
                                hasIncrement(unaryOperator(hasOperatorName("++"),
                                  hasUnaryOperand(declRefExpr(to(varDecl(hasType(isInteger())).bind("incVarName")))))),
                                hasCondition(
                                  binaryOperator(hasOperatorName("<"),
                                  hasLHS(ignoringParenImpCasts(declRefExpr(to(varDecl(hasType(isInteger())).bind("condVarName"))))),
                                  hasRHS(expr(hasType(isInteger())))))
                                ).bind("forLoop");
StatementMatcher arrayClause = arraySubscriptExpr(unless(hasDescendant(binaryOperator())),
                                hasDescendant(declRefExpr(to(varDecl(hasType(isInteger())).bind("arrayIndex")))));
StatementMatcher opClause[4] = {binaryOperator(hasOperatorName("+"),
                                  hasDescendant(arrayClause), hasDescendant(declRefExpr(to(varDecl().bind("accOperator")))), unless(hasDescendant(binaryOperator()))) ,
                                binaryOperator(hasOperatorName("-") ,
                                  hasDescendant(arrayClause), hasDescendant(declRefExpr(to(varDecl().bind("accOperator")))), unless(hasDescendant(binaryOperator()))) ,
                                binaryOperator(hasOperatorName("&") ,
                                  hasDescendant(arrayClause), hasDescendant(declRefExpr(to(varDecl().bind("accOperator")))), unless(hasDescendant(binaryOperator()))) ,
                                binaryOperator(hasOperatorName("|") ,
                                  hasDescendant(arrayClause), hasDescendant(declRefExpr(to(varDecl().bind("accOperator")))), unless(hasDescendant(binaryOperator())))
};
StatementMatcher ReduceMatcher =
    binaryOperator(hasOperatorName("="), hasParent(compoundStmt(hasParent(LoopMatcher))) , hasRHS(anyOf(
      opClause[0], opClause[1], opClause[2], opClause[3])), hasLHS(declRefExpr(to(varDecl().bind("accumulator"))))
    ).bind("reduce");

class LoopPrinter : public MatchFinder::MatchCallback {
public :
  virtual void run(const MatchFinder::MatchResult &Result) {
      ASTContext *Context = Result.Context;
      const BinaryOperator *BO = Result.Nodes.getNodeAs<BinaryOperator>("reduce");
      // We do not want to convert header files!
      if(!BO || !Context->getSourceManager().isWrittenInMainFile(BO->getOperatorLoc()))
        return;
      const VarDecl *IncVar = Result.Nodes.getNodeAs<VarDecl>("incVarName");
      const VarDecl *CondVar = Result.Nodes.getNodeAs<VarDecl>("condVarName");
      const VarDecl *InitVar = Result.Nodes.getNodeAs<VarDecl>("initVarName");
      const VarDecl *ArrayIndex = Result.Nodes.getNodeAs<VarDecl>("arrayIndex");
      const VarDecl *AccOperator = Result.Nodes.getNodeAs<VarDecl>("accOperator");
      const VarDecl *Accumulator = Result.Nodes.getNodeAs<VarDecl>("accumulator");

      if(!areSameVariable(IncVar, CondVar) || !areSameVariable(IncVar, InitVar)){
        return;
      }else if(!areSameVariable(IncVar, ArrayIndex)){
        return;
      }else if(!areSameVariable(AccOperator, Accumulator)){
        return;
      }
    //  BO->dump();
      llvm::outs() << "Potential Reduce pattern, in the next line you'll find the file and the suspect line.\n";
      BO->getExprLoc().dump(Context->getSourceManager());
      llvm::outs() << "\n";
    }
};
int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  LoopPrinter Printer;
  MatchFinder Finder;
  Finder.addMatcher(ReduceMatcher, &Printer);

  return Tool.run(newFrontendActionFactory(&Finder).get());
}
