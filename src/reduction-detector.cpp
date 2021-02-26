#include <memory>
#include <string>
#include <vector>

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
using namespace clang::ast_matchers;

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include "translation_unit_finder.h"
using reduction_detector::translation_unit_finder::expand_directories;

#include "loop_analysis/loop_analysis.h"

#include "constants.h"

#include "command_line.h"

using namespace reduction_detector;

int main(int argc, const char **argv) {
  clang::tooling::CommonOptionsParser optionsParser(
      argc, argv, command_line_options::reduction_detector_option_category);

  // Construct  a list of all source files reachable from this directory.
  // TODO: This whole thing is a hack, and almost certainly much less efficient
  // than it should be. We probably should be descending recursively into
  // directories (one process
  // per directory), or something similar. See how other programs (e.g. simple
  // GNU coreutils) that accept options like --recursive do this.
  std::vector<std::string> input_path_list = optionsParser.getSourcePathList();
  std::vector<std::string> expanded_path_list;
  expand_directories(input_path_list, expanded_path_list);

  clang::tooling::ClangTool clangTool(optionsParser.getCompilations(),
                                      expanded_path_list);

  loop_analysis::LoopAnalyser loopAnalyser;
  clang::ast_matchers::MatchFinder finder;
  finder.addMatcher(loop_analysis::loopMatcher,
                    &loopAnalyser);

  int statusCode =
      clangTool.run(clang::tooling::newFrontendActionFactory(&finder).get());

  llvm::outs() << loopAnalyser.loopCounts.likelyReductionLoops.all << " out of "
               << loopAnalyser.loopCounts.totals.all
               << " loops detected as likely reduction loops.\n"
               << INDENT
               << loopAnalyser.loopCounts.likelyReductionLoops.forLoops
               << " out of " << loopAnalyser.loopCounts.totals.forLoops
               << " for loops.\n"
               << INDENT
               << loopAnalyser.loopCounts.likelyReductionLoops.whileLoops
               << " out of " << loopAnalyser.loopCounts.totals.whileLoops
               << " while loops.\n"
               << INDENT
               << loopAnalyser.loopCounts.likelyReductionLoops.doWhileLoops
               << " out of " << loopAnalyser.loopCounts.totals.doWhileLoops
               << " do-while loops.\n";

  return statusCode;
}

/* TODO:
 * - An accumulator isn't necessarily a variable directly named by an
 *   identifier.
 *   It can be any recurring lvalue − for example, a member of a struct
 *   or an element of an array. We need to be able to recognize those
 *   accumulators too. (Noted by Gerson.) Make diverse test cases.
 *   See the last few paragraphs of
 *   https://clang.llvm.org/docs/LibASTMatchersTutorial.html.
 * - The output of --help should state the default value of
 *   --min-score. See whether the CommandLine library can help with that.
 * - Documment (in header-file comments) every function whose behavior
 *   isn't obvious.
 * - Recognize uses of the unary increment and decrement operators
 *   as if they were reduction assignments. NPB code does at least two
 *   reductions using those.
 * - Add a test case for a loop whose body is an expression statement,
 *   rather than a compound statement (i.e. a block).
 * - Do proper encapsulation. Make public only what needs to be public.
 *   Use getter methods.
 * - The program should explain its reasonings only when asked to
 *   (for example, through an option --verbose).
 * - How well do we deal with nested loops? Write some test cases for that.
 * - Make the program optionally write its output to a given file.
 * - Use deeper indentation to make reading easier. Use 4 spaces.
 *   (See how to make clang-format do that for you.)
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
 * - Run Valgrind. Make sure we're not leaking memory.
 */
