#ifndef REDUCTION_ASSIGNMENT_MATCHERS_H
#define REDUCTION_ASSIGNMENT_MATCHERS_H

#include "clang/ASTMatchers/ASTMatchers.h"

// todo: wrap these in a namespace.
// TODO: use an include guard.

namespace reduction_detector {
namespace reduction_assignment_matchers {

    extern clang::ast_matchers::StatementMatcher potentialReductionSimpleAssignmentMatcher;

    extern clang::ast_matchers::StatementMatcher potentialReductionCompoundAssignmentMatcher;

    extern clang::ast_matchers::StatementMatcher reductionAssignmentMatcher;

}
}

#endif
