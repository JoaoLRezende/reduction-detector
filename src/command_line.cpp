#include "command_line.h"

#include "constants.h"

#include <fstream>

#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"

namespace reduction_detector {
namespace command_line_options {

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
llvm::cl::OptionCategory
    reduction_detector_option_category("reduction-detector options");

// The value of output_file_name is used only in opening output_file.
llvm::cl::opt<std::string> output_file_name(
    "output", llvm::cl::desc("Specify output file"),
    llvm::cl::value_desc("filename"), llvm::cl::init(DEFAULT_OUTPUT_FILE_NAME),
    llvm::cl::cat(command_line_options::reduction_detector_option_category));
static llvm::cl::alias output_file_name2("o",
                                         llvm::cl::desc("Alias for --output"),
                                         llvm::cl::aliasopt(output_file_name));

// We have output_file as a pointer (instead of as a static
// llvm::raw_fd_ostream) because we don't yet have the name of the file to be
// opened at the time of construction of static objects. We only process
// command-line arguments later.
std::unique_ptr<llvm::raw_fd_ostream> output_file;

// open_output_file needs to be called before any output is written.
void open_output_file() {
  std::error_code error_code;
  output_file =
      std::unique_ptr<llvm::raw_fd_ostream>(new llvm::raw_fd_ostream(output_file_name, error_code));
  if (error_code) {
    llvm::errs() << "Could not open output file " << output_file_name << ".\n";
    std::terminate(); // can't throw exceptions :/
  }
}
}
}
