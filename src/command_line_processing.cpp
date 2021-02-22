#include "command_line_processing.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"

namespace reduction_detector {
namespace command_line_options {

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
llvm::cl::OptionCategory
    reduction_detector_option_category("reduction-detector options");

// A help message for this specific tool can be added afterwards.
// TODO: consider eliminating this, and honoring the instructions in
// CommandLine's user guide to do this instead. Those instructions allow your
// short help text to appear at the top of the help output, which is even
// better.
// static llvm::cl::extrahelp MoreHelp("\nMore help text...");
}
}

// TODO: change the name of this module (including the corresponding implementation
// file) to simply "command_line".

// TODO: make sure that are descriptions for every option in the output of
// --help.
