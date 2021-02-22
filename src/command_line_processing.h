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

#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"

namespace reduction_detector {
namespace command_line_options {

/* The category to which our command-line options pertain. All options are to be
 * defined as members of this category. Forgetting to do so will prevent them
 * from appearing in the program's --help output.
 */
extern llvm::cl::OptionCategory reduction_detector_option_category;

}
}

#endif