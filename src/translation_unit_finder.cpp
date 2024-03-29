#include "translation_unit_finder.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include "command_line.h"

#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace reduction_detector {
namespace translation_unit_finder {

llvm::cl::opt<bool> debugInputFiles(
    "debug-input-files",
    llvm::cl::desc("Write list of all analyzed source files to stderr"),
    llvm::cl::cat(command_line_options::reduction_detector_option_category));

static bool string_has_suffix(std::string const &big_string,
                              std::string const &suffix) {
  if (suffix.size() > big_string.size())
    return false;
  return std::equal(suffix.rbegin(), suffix.rend(), big_string.rbegin());
}

/*
 * Decide whether a regular file is one we should analyze.
 */
static bool isProperInputFile(std::string file_name, struct stat *stat_struct) {
  return string_has_suffix(file_name, ".c") ||
         string_has_suffix(file_name, ".cpp");
}

static void dump_string_vector(std::vector<std::string> &vector) {
  for (std::string &string : vector) {
    llvm::errs() << "-- " << string << "\n";
  }
}

static void get_files_in_directory(std::string &directory_path,
                                   std::vector<std::string> &output_list) {
  if (directory_path.back() == '/')
    directory_path.pop_back();

  DIR *directory = opendir(directory_path.c_str());
  // for each file in the directory
  for (struct dirent *directory_entry = readdir(directory);
       directory_entry != nullptr; directory_entry = readdir(directory)) {
    std::string file_path = directory_path + "/" + directory_entry->d_name;
    struct stat stat_struct;
    if (stat(file_path.c_str(), &stat_struct)) {
      perror(("couldn't stat file " + file_path).c_str());
    }

    if ((stat_struct.st_mode & S_IFMT) == S_IFREG) { // if is a regular file
      if (isProperInputFile(file_path, &stat_struct)) {
        output_list.push_back(file_path);
      }
    } else if ((stat_struct.st_mode & S_IFMT) == S_IFDIR &&
               strcmp(".", directory_entry->d_name) &&
               strcmp("..", directory_entry->d_name)) { // if is a directory
      get_files_in_directory(file_path, output_list);
    }
  }
  closedir(directory);
}

void expand_directories(std::vector<std::string> &input_list,
                        std::vector<std::string> &output_list) {
  if (debugInputFiles) {
    llvm::errs() << "input_path_list:\n";
    dump_string_vector(input_list);
    llvm::errs() << "\n";
  }

  for (std::string &path : input_list) {
    struct stat stat_struct;
    if (stat(path.c_str(), &stat_struct)) {
      perror(("couldn't stat file " + path).c_str());
    }

    if ((stat_struct.st_mode & S_IFMT) == S_IFREG) { // if is a regular file
      output_list.push_back(path);
    } else if ((stat_struct.st_mode & S_IFMT) == S_IFDIR) { // if is a directory
      get_files_in_directory(path, output_list);
    }
  }

  if (debugInputFiles) {
    llvm::errs() << "expanded_path_list:\n";
    dump_string_vector(output_list);
    llvm::errs() << "\n";
  }
}
} // namespace translation_unit_finder
} // namespace reduction_detector
