/*
 * This header file contains declarations of all objects related to LLVM's
 * CommandLine library that are to be accessible from multiple modules (for
 * example, objects that represent command-line options that influence multiple
 * modules). Objects that are used in only one module don't go here; those are
 * to be declared in their respective modules instead. The set of all options
 * can be seen by invoking the program with --help.
 */

#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <fstream>

#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"

namespace reduction_detector {
namespace command_line_options {

/* The category to which our command-line options pertain. All options are to be
 * defined as members of this category. Forgetting to do so will prevent them
 * from appearing in the program's --help output.
 */
extern llvm::cl::OptionCategory reduction_detector_option_category;

/* only_non_trivial_reductions holds whether we are to show only loops that
 * would not usually be detected by other reduction-detecting tools.
 */
extern llvm::cl::opt<bool> only_non_trivial_reductions;

extern llvm::cl::opt<bool> only_trivial_reductions;

extern llvm::cl::opt<std::string> output_file_name;
extern std::unique_ptr<llvm::raw_fd_ostream> output_file;

// open_output_file needs to be called before anything is written to
// output_file.
void open_output_file();
}
}

#endif