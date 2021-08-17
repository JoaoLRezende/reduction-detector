#include "internal.h"

#include "clang/ASTMatchers/ASTMatchers.h"
using namespace clang::ast_matchers;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

/* Get the number of the line in which the base of a possible accumulator was
 * declared.
 */
static int get__declaration_line_number_of_base_of_possible_accumulator(
    PossibleAccumulatorInfo &possible_accumulator, clang::ASTContext *context) {
  return context->getSourceManager().getPresumedLineNumber(
      possible_accumulator.base->getDecl()->getBeginLoc());
}

void getDistanceOfDeclarationOfPossibleAccumulators(
    PossibleReductionLoopInfo &loop_info, clang::ASTContext *context) {
  int loop_line = context->getSourceManager().getPresumedLineNumber(
      loop_info.loop_stmt->getBeginLoc());

  for (auto &possible_accumulator : loop_info.possible_accumulators) {
    int possible_accumulator_declaration_line =
        get__declaration_line_number_of_base_of_possible_accumulator(
            possible_accumulator.second, context);
    possible_accumulator.second.declaration_distance_from_loop_in_lines =
        loop_line - possible_accumulator_declaration_line;
  }
}
}
}
}