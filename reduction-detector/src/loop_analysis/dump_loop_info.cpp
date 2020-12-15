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
  if (this->iteration_variable) {
    outputStream << "Its iteration variable is "
                 << this->iteration_variable->getName() << ".\n";
  } else {
    outputStream << "I couldn't determine its iteration variable.\n";
  }

  outputStream << "\n";

  for (auto &potentialAccumulator : this->potential_accumulators) {
    outputStream << INDENT << potentialAccumulator.first->getName()
                 << " was detected as a "
                 << (potentialAccumulator.second.isLikelyAccumulator
                         ? "likely accumulator"
                         : "potential accumulator, but not as a likely one")
                 << ". Score: "
                 << potentialAccumulator.second.likelyAccumulatorScore << ".\n";

    if (potentialAccumulator.second.notableNameSubstring) {
      outputStream << INDENT "Its name has the substring \""
                   << *potentialAccumulator.second.notableNameSubstring
                   << "\".\n";
    } else {
      outputStream << INDENT "Its name doesn't have any of the common "
                             "accumulator-name substrings.\n";
    }

    outputStream
        << INDENT "It seems to be accumulated in the following assignments:\n";
    for (auto &accumulatingAssignment :
         potentialAccumulator.second.potential_accumulating_assignments) {
      outputStream << INDENT INDENT;
      accumulatingAssignment->printPretty(
          outputStream, nullptr, clang::PrintingPolicy(clang::LangOptions()));
      outputStream << "\n";
    }

    if (potentialAccumulator.second
            .number_of_potential_accumulating_assignments_that_reference_the_iteration_variable ==
        potentialAccumulator.second.potential_accumulating_assignments.size()) {
      outputStream << INDENT "All those assignments involve "
                             "the loop's iteration variable.\n";
    } else {
      outputStream
          << INDENT "Those apparent accumulating assigments do not all "
                    "involve the loop's iteration variable. Only "
          << potentialAccumulator.second
                 .number_of_potential_accumulating_assignments_that_reference_the_iteration_variable
          << " of them.\n";
    }

    outputStream << INDENT "There are "
                 << potentialAccumulator.second.outside_references
                 << " other in-loop references to "
                 << potentialAccumulator.first->getName()
                 << " outside of those "
                    "accumulating assignments.\n";
    outputStream << "\n";
  }
}
}
}
}
