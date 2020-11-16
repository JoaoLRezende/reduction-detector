#include "clang/ASTMatchers/ASTMatchers.h"

// todo: wrap these in a namespace.
// TODO: use an include guard.

extern clang::ast_matchers::StatementMatcher potentialReductionSimpleAssignmentMatcher;

extern clang::ast_matchers::StatementMatcher potentialReductionCompoundAssignmentMatcher;

extern clang::ast_matchers::StatementMatcher reductionAssignmentMatcher;
