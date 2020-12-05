#include "reduction-detector/loop_analysis.h"

#include "reduction-detector/constants.h"

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

void PotentialReductionLoopInfo::dump(llvm::raw_ostream &outputStream,
                                      clang::ASTContext *context) {
  if (this->hasALikelyAccumulator) {
    outputStream << "The following loop seems to perform a reduction, at ";
  } else {
    outputStream
        << "The following loop was not detected as a reduction loop, at ";
  }
  this->forStmt->getForLoc().print(outputStream, context->getSourceManager());
  outputStream << ":\n";
  this->forStmt->printPretty(outputStream, nullptr,
                             clang::PrintingPolicy(clang::LangOptions()));

  for (auto &potentialAccumulator : this->potential_accumulators) {
    outputStream << INDENT << potentialAccumulator.first->getName()
                 << " was detected as a "
                 << (potentialAccumulator.second.isLikelyAccumulator
                         ? "likely accumulator"
                         : "potential accumulator, but not as a likely one")
                 << ". Score: "
                 << potentialAccumulator.second.likelyAccumulatorScore << ".\n";

    outputStream << INDENT
        "It seems to be accumulated in the following assignments:\n";
    for (auto &accumulatingAssignment :
         potentialAccumulator.second.potential_accumulating_assignments) {
      outputStream << INDENT INDENT;
      accumulatingAssignment->printPretty(
          outputStream, nullptr, clang::PrintingPolicy(clang::LangOptions()));
      outputStream << "\n";
    }

    outputStream
        << INDENT "There are " << potentialAccumulator.second.outside_references
        << " references to it outside of those accumulating assignments.\n";
  }
}
}
}
}
