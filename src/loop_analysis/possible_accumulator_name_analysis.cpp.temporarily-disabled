#include "../constants.h"
#include "loop_analysis.h"
#include "internal.h"

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
    // TODO: creating an std::string object for every analysed name probably
    // isn't very efficient. do something better.
    std::string possibleAccumulatorName(possibleAccumulator.first->getName());

    for (std::string &commonAccumulatorNameSubstring :
         COMMON_ACCUMULATOR_NAME_SUBSTRINGS_VECTOR)
      if (possibleAccumulatorName.find(commonAccumulatorNameSubstring) !=
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
