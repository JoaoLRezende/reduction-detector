#ifndef TRANSLATION_UNIT_FINDER_H
#define TRANSLATION_UNIT_FINDER_H

#include <vector>
#include <string>

namespace reduction_detector {
namespace translation_unit_finder {
    
/*
 * todo: description.
 * Concatenates those paths to output_list.
 * todo: this shouldn't be static. the _other_ functions should, and shouldn't be declared in this header file.
 */
static void get_files_in_directory(std::string &directory_path,
                                   std::vector<std::string> &output_list);

void expand_directories(std::vector<std::string> &input_list,
                        std::vector<std::string> &output_list);

void dump_string_vector(std::vector<std::string> &vector);

}
}

#endif
