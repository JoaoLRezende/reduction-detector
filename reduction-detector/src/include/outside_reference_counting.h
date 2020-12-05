#ifndef OUTSIDE_REFERENCE_COUNTING_H
#define OUTSIDE_REFERENCE_COUNTING_H

#include "reduction-detector/outside_reference_counting.h"

#include "reduction-detector/loop_analysis.h"

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

void countOutsideReferencesIn(PotentialReductionLoopInfo *loop_info,
                              clang::ASTContext *context);
}
}
}
#endif
