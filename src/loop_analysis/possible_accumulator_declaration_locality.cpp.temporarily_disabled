#include "internal.h"

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

void getDistanceOfDeclarationOfPossibleAccumulators(
    PossibleReductionLoopInfo &loop_info, clang::ASTContext *context) {
  int loop_line = context->getSourceManager().getPresumedLineNumber(
      loop_info.loopStmt->getBeginLoc());

  for (auto &possible_accumulator : loop_info.possible_accumulators) {
    int possible_accumulator_declaration_line =
        context->getSourceManager().getPresumedLineNumber(
            possible_accumulator.first->getBeginLoc());
    possible_accumulator.second.declarationDistanceFromLoopInLines =
        loop_line - possible_accumulator_declaration_line;
  }
}
}
}
}