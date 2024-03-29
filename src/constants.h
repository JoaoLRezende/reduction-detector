#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H

namespace reduction_detector {
namespace constants {
    // [cricket noises]
}
}

#define DEFAULT_OUTPUT_FILE_NAME    "detected_reductions.out"

#define INDENT "    " // a string with 4 spaces, for indenting debug ouput

#define INITIAL_ACCUMULATOR_LIKELIHOOD_SCORE        5
#define MAX_DISTANCE_IN_LINES_BETWEEN_DECLARATION_AND_LOOP_FOR_LOCALITY_BONUS \
                                                    2
#define DECLARATION_LOCALITY_BONUS                  2
#define ACCUMULATOR_REFERENCE_IN_RHS_BONUS          3
#define OUTSIDE_REFERENCE_PENALTY                   1
#define COMMON_ACCUMULATOR_NAME_SUBSTRINGS          "acc", "total", "sum"
#define COMMON_ACCUMULATOR_NAME_BONUS               5
#define UNREAD_POSSIBLE_ACCUMULATOR_PENALTY         5
#define DEREFERENCE_OF_INCONSTANT_POINTER_PENALTY   5
#define DEFAULT_LIKELY_ACCUMULATOR_THRESHOLD        7


#endif
