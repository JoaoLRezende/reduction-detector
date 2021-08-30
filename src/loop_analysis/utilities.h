#include "clang/ASTMatchers/ASTMatchFinder.h"

namespace reduction_detector {
namespace loop_analysis {
namespace internal {
namespace utilities {

bool doesSubtreeHaveAnExpressionWithFoldingSetNodeID(
    const clang::Expr &subtree,
    const llvm::FoldingSetNodeID &expressionFoldingSetNodeID,
    clang::ASTContext *context);

}
} // namespace internal
} // namespace loop_analysis
} // namespace reduction_detector