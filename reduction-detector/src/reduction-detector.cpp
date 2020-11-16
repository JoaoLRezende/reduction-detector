#include <memory>
#include <string>
#include <vector>

#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include "llvm/Support/raw_ostream.h"

#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

#include "reduction-detector/reduction-assignment-matchers.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

using namespace reduction_detector::reduction_assignment_matchers;

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


/*
 * One instance of LoopChecker is created for each source file.
 * It checks every for loop in that file, accumulating statistics.
 */
class LoopChecker : public MatchFinder::MatchCallback {
public:
  std::string loop_analysis_report_buffer;

  unsigned int likelyReductionCount = 0;
  unsigned int totalLoopCount = 0;

  LoopChecker() { loop_analysis_report_buffer = std::string(); }

  // run is called by MatchFinder for each for loop.
  virtual void run(const MatchFinder::MatchResult &result) {

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
    forStmt->getForLoc().print(loop_analysis_report_stream, context->getSourceManager());
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
      assignment->printPretty(loop_analysis_report_stream, nullptr, PrintingPolicy(LangOptions()));
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

      loop_analysis_report_stream
          << INDENT INDENT "└ Found "
          << outsideReferenceAccumulator.outsideReferences
          << " other references to " << possibleAccumulator->getName()
          << " in this for loop. ";
      if (!outsideReferenceAccumulator.outsideReferences) {
        loop_analysis_report_stream << "Thus, "
                                    << possibleAccumulator->getName()
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
};

/*
 * todo: description.
 * Concatenates those paths to output_list.
 */
static void get_files_in_directory(std::string &directory_path,
                                   std::vector<std::string> &output_list) {
  if (directory_path.back() == '/')
    directory_path.pop_back();

  DIR *directory = opendir(directory_path.c_str());
  struct stat stat_struct;
  // for each file in the directory
  for (struct dirent *directory_entry = readdir(directory);
       directory_entry != nullptr; directory_entry = readdir(directory)) {
    std::string path = directory_path + "/" + directory_entry->d_name;
    if (stat(path.c_str(), &stat_struct)) {
      perror(("couldn't stat file " + path).c_str());
    }

    if ((stat_struct.st_mode & S_IFMT) == S_IFREG) { // if is a regular file
      output_list.push_back(path);
    } else if ((stat_struct.st_mode & S_IFMT) == S_IFDIR &&
               strcmp(".", directory_entry->d_name) &&
               strcmp("..", directory_entry->d_name)) { // if is a directory
      get_files_in_directory(path, output_list);
    }
  }
  closedir(directory);
}

void expand_directories(std::vector<std::string> &input_list,
                        std::vector<std::string> &output_list) {
  for (std::string &path : input_list) {
    struct stat stat_struct;
    if (stat(path.c_str(), &stat_struct)) {
      perror(("couldn't stat file " + path).c_str());
    }

    if ((stat_struct.st_mode & S_IFMT) == S_IFREG) { // if is a regular file
      output_list.push_back(path);
    } else if ((stat_struct.st_mode & S_IFMT) == S_IFDIR) { // if is a directory
      get_files_in_directory(path, output_list);
    }
  }
}

void dump_string_vector(std::vector<std::string> &vector) {
  for (std::string &string : vector) {
    llvm::errs() << "-- " << string << "\n";
  }
}

int main(int argc, const char **argv) {
  CommonOptionsParser optionsParser(argc, argv, myToolCategory);

  std::vector<std::string> input_path_list = optionsParser.getSourcePathList();
  llvm::errs() << "input_path_list:\n";
  dump_string_vector(input_path_list);
  llvm::errs() << "\n";

  std::vector<std::string> expanded_path_list;
  expand_directories(input_path_list, expanded_path_list);

  llvm::errs() << "expanded_path_list:\n";
  dump_string_vector(expanded_path_list);
  llvm::errs() << "\n";

  ClangTool clangTool(optionsParser.getCompilations(), expanded_path_list);

  LoopChecker loopChecker;
  MatchFinder finder;
  finder.addMatcher(forStmt().bind("forLoop"), &loopChecker);

  int statusCode = clangTool.run(newFrontendActionFactory(&finder).get());

  llvm::errs() << loopChecker.likelyReductionCount << " out of "
               << loopChecker.totalLoopCount
               << " loops detected as likely reduction loops.\n";

  return statusCode;
}

/* TODO:
 * - Recognize uses of the unary increment and decrement operators
 *   as if they were reduction assignments. NPB code does at least two
 *   reductions using those.
 * - Check whether each potential accumulator is declared outside
 *   of the loop.
 *   (That should change nothing now, but it will be a necessary
 *   check after we stop
 *   requiring that an accumulator is referenced in only one
 *   assignemnt in the loop.)
 * - Why do we check whether a potential accumulator is referenced
 *   only once
 *   in the right-hand side of its assignment? Is there any good
 *   reason for that?
 *   Make tests. If nothing changes, we should probably stop doing that.
 *   (Noted by Gerson.)
 * - Add a test case for a loop whose body is an expression statement,
 *   rather than a compound statement (i.e. a block).
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
 * - An accumulator isn't necessarily a variable directly named by an
 *   identifier.
 *   It can be any recurring lvalue − for example, a member of a struct
 *   or an element of an array. We need to be able to recognize those
 *   accumulators too. (Noted by Gerson.) Make diverse test cases.
 * - Be able to receive a directory as input (rather than only a single
 *   file).
 *   Then, test on larger software systems (such as Linux).
 * - Make the program optionally write its output to a given file, with
 *   detailed information about the location of each potential reduction loop
 *   and what the potential reduction accumulators are.
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
 * - Run Valgrind. Make sure we're not leaking memory.
 */
