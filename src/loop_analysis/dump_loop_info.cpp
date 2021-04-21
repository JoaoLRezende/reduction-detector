#include "internal.h"
#include "loop_analysis.h"

#include "../command_line.h"
#include "../constants.h"

#include "llvm/Support/CommandLine.h"

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

/* reportUnlikelyAccumulators holds whether we are to dump information
 * on possible accumulators that aren't likely accumulators.
 * Note that this has nothing to do with printing information
 * on the loop itself.
 */
static llvm::cl::opt<bool> reportUnlikelyAccumulators(
    "report-unlikely-accumulators",
    llvm::cl::desc(
        "For each printed loop, also report and explain the score of possible "
        "accumulators whose accumulator-likelihood score didn't "
        "reach the threshold"),
    llvm::cl::cat(command_line_options::reduction_detector_option_category));

static llvm::cl::opt<bool> verbose(
    "verbose", llvm::cl::desc("Print information acquired on each possible "
                              "accumulator and other internal info"),
    llvm::cl::cat(command_line_options::reduction_detector_option_category));

void PossibleReductionLoopInfo::dump(llvm::raw_ostream &outputStream,
                                     clang::ASTContext *context) {
  if (this->hasALikelyAccumulator) {
    outputStream << "Likely reduction loop at ";
  } else {
    outputStream << "Unlikely reduction loop at ";
  }
  this->loopStmt->getBeginLoc().print(outputStream,
                                      context->getSourceManager());
  outputStream << ":\n";
  this->loopStmt->printPretty(outputStream, nullptr,
                              clang::PrintingPolicy(clang::LangOptions()));

  if (verbose) {
    if (this->iteration_variable == nullptr) {
      outputStream << "I couldn't determine its iteration variable.\n";
    } else {
      outputStream << "Its iteration variable is "
                   << this->iteration_variable->getName() << ".";
      if (this->numberOfIterationVariableReferencesInArraySubscripts == 0) {
        outputStream << "\nNo arrays are subscripted by "
                     << this->iteration_variable->getName() << ".";
      } else {
        outputStream << "\nArrays subscripted by "
                     << this->iteration_variable->getName() << ": ";
        for (auto &array_accessCount_pair :
             this->numberOfArrayAccessesInvolvingIterationVariablePerArray) {
          outputStream << array_accessCount_pair.first->getName() << " ("
                       << array_accessCount_pair.second << " accesses)";
          if (array_accessCount_pair.first !=
              std::prev(
                  this->numberOfArrayAccessesInvolvingIterationVariablePerArray
                      .end())
                  ->first) { // if this is not the last element. TODO: there
                             // definitely is a better way to do this.
            outputStream << ", ";
          }
        }
        outputStream << ".\n";
      }
    }
  }

  outputStream << "Likely accumulators:";
  for (auto &possible_accumulator : this->possible_accumulators) {
    if (possible_accumulator.second.isLikelyAccumulator) {
      outputStream << " " << possible_accumulator.first->getName();
    }
  }

  outputStream << "\n";

  if (verbose) {
    for (auto &possibleAccumulator : this->possible_accumulators) {
      if (possibleAccumulator.second.isLikelyAccumulator ||
          reportUnlikelyAccumulators) {
        outputStream << "\n" INDENT << possibleAccumulator.first->getName()
                     << " was detected as a "
                     << (possibleAccumulator.second.isLikelyAccumulator
                             ? "likely accumulator"
                             : "possible accumulator, but not as a likely one")
                     << " (score: "
                     << possibleAccumulator.second.likelyAccumulatorScore
                     << ").";
        if (possibleAccumulator.second.is_trivial_accumulator) {
          outputStream << " But it is a trivial accumulator.\n";
        } else {
          outputStream << " It is not a trivial accumulator.\n";
        }

        outputStream
            << INDENT "It was declared "
            << possibleAccumulator.second.declarationDistanceFromLoopInLines
            << " lines above the loop.\n";

        if (possibleAccumulator.second.notableNameSubstring) {
          outputStream << INDENT "Its name has the substring \""
                       << *possibleAccumulator.second.notableNameSubstring
                       << "\".\n";
        } else {
          outputStream << INDENT "Its name doesn't have any of the common "
                                 "accumulator-name substrings.\n";
        }

        outputStream << INDENT
            "It might be accumulated in the following assignments:\n";
        for (auto &accumulatingAssignment :
             possibleAccumulator.second.possible_accumulating_assignments) {
          if (accumulatingAssignment.second
                  .rhs_also_references_possible_accumulator) {
            outputStream << INDENT INDENT;
            accumulatingAssignment.first->printPretty(
                outputStream, nullptr,
                clang::PrintingPolicy(clang::LangOptions()));
            outputStream << "\n";
          }
        }

        outputStream
            << INDENT
            << possibleAccumulator.second
                   .number_of_possible_accumulating_assignments_that_reference_the_iteration_variable
            << " of those assignments involve "
               "the loop's iteration variable.\n";

        outputStream << INDENT "There are "
                     << possibleAccumulator.second.outside_references
                     << " other in-loop references to "
                     << possibleAccumulator.first->getName()
                     << " outside of those "
                        "possible accumulating assignments.\n";
      }
    }
  }
  outputStream << "\n";
}
}
}
}
