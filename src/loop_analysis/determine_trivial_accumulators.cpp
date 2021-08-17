#include "internal.h"
using namespace reduction_detector::loop_analysis::internal;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

void determineTrivialAccumulators(PossibleReductionLoopInfo &loop_info) {
  for (auto &possible_accumulator : loop_info.possible_accumulators) {
    if ((possible_accumulator.second.possible_accumulating_assignments.size() ==
         possible_accumulator.second
             .number_of_possible_accumulating_assignments_whose_RHS_also_references_this) &&
        possible_accumulator.second.outside_references == 0 &&
        (llvm::isa<clang::DeclRefExpr>(
             possible_accumulator.second.possible_accumulator) ||
         llvm::isa<clang::ArraySubscriptExpr>(
             possible_accumulator.second.possible_accumulator)) &&
        (llvm::isa<clang::ForStmt>(loop_info.loop_stmt))) {
      possible_accumulator.second.is_trivial_accumulator = true;
      loop_info.has_a_trivial_accumulator = true;
    } else {
      possible_accumulator.second.is_trivial_accumulator = false;
      if (possible_accumulator.second.is_likely_accumulator) {
        loop_info.has_likely_but_non_trivial_accumulator = true;
      }
    }
  }
}
}
}
}
