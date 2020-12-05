#ifndef POTENTIAL_ACCUMULATOR_DETECTION_H
#define POTENTIAL_ACCUMULATOR_DETECTION_H

#include "reduction-detector/loop_analysis.h"

#include "clang/AST/ASTContext.h"

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

void getPotentialAccumulatorsIn(PotentialReductionLoopInfo *loop_info,
                                clang::ASTContext *context);
}
}
}
#endif
