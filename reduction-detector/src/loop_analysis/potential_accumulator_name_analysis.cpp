#include "reduction-detector/constants.h"
#include "reduction-detector/loop_analysis.h"
#include <string>
#include <vector>

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

static std::vector<std::string> COMMON_ACCUMULATOR_NAME_SUBSTRINGS_VECTOR = {
    COMMON_ACCUMULATOR_NAME_SUBSTRINGS};

void analysePotentialAccumulatorNames(PotentialReductionLoopInfo &loopInfo,
                                      clang::ASTContext *context) {
  for (auto &potentialAccumulator : loopInfo.potential_accumulators) {
    // TODO: creating an std::string object for every analysed name probably
    // isn't very efficient. do something better.
    std::string potentialAccumulatorName(potentialAccumulator.first->getName());

    for (std::string &commonAccumulatorNameSubstring :
         COMMON_ACCUMULATOR_NAME_SUBSTRINGS_VECTOR)
      if (potentialAccumulatorName.find(commonAccumulatorNameSubstring) !=
          std::string::npos) {
        potentialAccumulator.second.notableNameSubstring =
            &commonAccumulatorNameSubstring;
        break;
      };
  }
}
}
}
}
