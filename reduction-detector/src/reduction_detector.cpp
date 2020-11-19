#include <memory>
#include <string>
#include <vector>

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

#include "reduction-detector/translation_unit_finder.h"
using reduction_detector::translation_unit_finder::expand_directories;

#include "reduction-detector/loop_analysis.h"
using reduction_detector::loop_analysis::LoopChecker;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory myToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static llvm::cl::extrahelp
    commonHelp(clang::tooling::CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static llvm::cl::extrahelp MoreHelp("\nMore help text...");

int main(int argc, const char **argv) {
  clang::tooling::CommonOptionsParser optionsParser(argc, argv, myToolCategory);

  std::vector<std::string> input_path_list = optionsParser.getSourcePathList();

  std::vector<std::string> expanded_path_list;
  expand_directories(input_path_list, expanded_path_list);

  clang::tooling::ClangTool clangTool(optionsParser.getCompilations(),
                                      expanded_path_list);

  LoopChecker loopChecker;
  clang::ast_matchers::MatchFinder finder;
  finder.addMatcher(clang::ast_matchers::forStmt().bind("forLoop"),
                    &loopChecker);

  int statusCode =
      clangTool.run(clang::tooling::newFrontendActionFactory(&finder).get());

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
