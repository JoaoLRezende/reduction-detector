#include "internal.h"
using namespace reduction_detector::loop_analysis::internal;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

void determineTrivialAccumulators(PossibleReductionLoopInfo &loop_info) {
  loop_info.is_trivial_reduction = true;
  for (auto &possible_accumulator : loop_info.possible_accumulators) {
    possible_accumulator.second.is_trivial_accumulator = true;
    if ((possible_accumulator.second.possible_accumulating_assignments.size() !=
         possible_accumulator.second
             .number_of_possible_accumulating_assignments_whose_RHS_also_references_this) ||
        possible_accumulator.second.outside_references > 0) {
      possible_accumulator.second.is_trivial_accumulator = false;
      loop_info.is_trivial_reduction = false;
      continue;
    }
  }
}
}
}
}
