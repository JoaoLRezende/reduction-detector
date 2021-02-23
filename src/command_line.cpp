#include "command_line.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"

namespace reduction_detector {
namespace command_line_options {

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
llvm::cl::OptionCategory
    reduction_detector_option_category("reduction-detector options");

}
}
