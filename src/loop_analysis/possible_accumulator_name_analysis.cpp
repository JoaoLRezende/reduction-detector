#include "../constants.h"
#include "internal.h"
#include "loop_analysis.h"

#include <string>
#include <vector>

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

static std::vector<std::string> COMMON_ACCUMULATOR_NAME_SUBSTRINGS_VECTOR = {
    COMMON_ACCUMULATOR_NAME_SUBSTRINGS};

void analysePossibleAccumulatorNames(PossibleReductionLoopInfo &loopInfo,
                                     clang::ASTContext *context) {
  for (auto &possibleAccumulator : loopInfo.possible_accumulators) {
    for (std::string &commonAccumulatorNameSubstring :
         COMMON_ACCUMULATOR_NAME_SUBSTRINGS_VECTOR)
      if (possibleAccumulator.second.name.find(commonAccumulatorNameSubstring) !=
          std::string::npos) {
        possibleAccumulator.second.notableNameSubstring =
            &commonAccumulatorNameSubstring;
        break;
      };
  }
}
}
}
}
