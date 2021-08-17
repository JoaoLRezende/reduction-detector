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

static void
write_loop_analyser_statistics(loop_analysis::LoopAnalyser &loopAnalyser,
                               llvm::raw_ostream &output_stream) {
  output_stream << loopAnalyser.loopCounts.likelyReductionLoops.all
                << " out of " << loopAnalyser.loopCounts.totals.all
                << " loops detected as likely reduction loops.";
  output_stream << " " << loopAnalyser.loopCounts.trivialReductionLoops.all
                << " of them are trivial reductions.";

  if (command_line_options::only_non_trivial_reductions) {
    output_stream << " (Trivial reductions not shown.)";
  }
  if (command_line_options::only_trivial_reductions) {
    output_stream << " (Only trivial reductions shown.)";
  }

  output_stream << "\n" INDENT
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
}

int main(int argc, const char **argv) {
  clang::tooling::CommonOptionsParser optionsParser(
      argc, argv, command_line_options::reduction_detector_option_category);

  reduction_detector::command_line_options::open_output_file();

  // Construct  a list of all source files reachable from this directory.
  // TODO: This whole thing is a hack, and almost certainly much less efficient
  // than it should be. We probably should be descending recursively into
  // directories (one process per directory), or something similar. See how
  // other programs (e.g. simple GNU coreutils) that accept options like
  // --recursive do this.
  std::vector<std::string> input_path_list = optionsParser.getSourcePathList();
  std::vector<std::string> expanded_path_list;
  expand_directories(input_path_list, expanded_path_list);

  clang::tooling::ClangTool clangTool(optionsParser.getCompilations(),
                                      expanded_path_list);

  // Invoke loop_analysis::LoopAnalyser on loops matched by
  // loop_analysis::loopMatcher. These are described in
  // loop_analysis/loop_analysis.h.
  loop_analysis::LoopAnalyser loop_analyser;
  clang::ast_matchers::MatchFinder match_finder;
  match_finder.addMatcher(loop_analysis::loopMatcher, &loop_analyser);
  int status_code = clangTool.run(
      clang::tooling::newFrontendActionFactory(&match_finder).get());

  write_loop_analyser_statistics(loop_analyser,
                                 *command_line_options::output_file);

  llvm::errs() << "Detected likely reductions were written to "
               << command_line_options::output_file_name << ".\n";

  return status_code;
}

/* TODO:
 * - Recognize uses of the unary increment and decrement operators
 *   as if they were reduction assignments. NPB code does at least two
 *   reductions using those. Also, note that Clang also recognizes those
 *   reductions, and thus they should be considered trivial reductions.
 * - Add a test case for a loop whose body is an expression statement,
 *   rather than a compound statement (i.e. a block).
 * - How well do we deal with nested loops? Write some test cases for that.
 * - Use deeper indentation to make reading easier. Use 4 spaces.
 *   (See how to make clang-format do that for you.)
 * - Should we recognize loops like the following as reductions?
 *      for (int i = 0; i < arr_length; i++)
 *          if (max < arr[i])
 *              max = arr[i];
 * - Run Valgrind. Make sure we're not leaking memory.
 * - Parece que o Cetus não reconhece como possíveis acumuladores expressões de
 *   subscrito de array cujo RHS é mais complexo que uma expressão de referência
 *   a declaração. Impedir esses possíveis acumuladores de serem considerados
 *   triviais. Eu já tentei fazer isso uma vez. See the piece of code at
 *   https://pastebin.com/XMmMsESG.
 */
