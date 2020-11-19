#ifndef TRANSLATION_UNIT_FINDER_H
#define TRANSLATION_UNIT_FINDER_H

#include <vector>
#include <string>

namespace reduction_detector {
namespace translation_unit_finder {
    
/*
 * Takes a vector of directory paths, and appends to another vector
 * the paths of all regular files transitively contained by those directories.
 */
void expand_directories(std::vector<std::string> &input_list,
                        std::vector<std::string> &output_list);

}
}

#endif
