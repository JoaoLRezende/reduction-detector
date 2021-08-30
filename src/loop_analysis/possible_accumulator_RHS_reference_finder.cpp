#include "internal.h"
#include "loop_analysis.h"

#include "../constants.h"

#include "utilities.h"

#include "clang/AST/ASTContext.h"
using clang::ASTContext;

#include "clang/ASTMatchers/ASTMatchers.h"
using namespace clang::ast_matchers;

#include "clang/ASTMatchers/ASTMatchFinder.h"
using clang::ast_matchers::MatchFinder;

#include "clang/AST/Expr.h"
using clang::BinaryOperator;

#include "clang/AST/Decl.h"
using clang::VarDecl;

namespace reduction_detector {
namespace loop_analysis {
namespace internal {

void detectPossibleAccumulatorReferencesInRHSOfPossibleAccumulatingStatements(
    PossibleReductionLoopInfo &loop_info, ASTContext *context) {
  for (auto &possibleAccumulator : loop_info.possible_accumulators) {
    for (auto &possibleAccumulatingAssignment :
         possibleAccumulator.second.possible_accumulating_assignments) {
      // we consider possibleAccumulatingAssignment to have a reference to
      // possibleAccumulator in its right-hand side if either ...
      if ( // possibleAccumulatingAssignment is an application of a compound
           // assignment operator (like +=) ...
          possibleAccumulatingAssignment.first->isCompoundAssignmentOp() ||
          // or possibleAccumulatingAssignment really, explicitly has a
          // reference to possibleAccumulator in its right-hand side.
          utilities::doesSubtreeHaveAnExpressionWithFoldingSetNodeID(
              *possibleAccumulatingAssignment.first->getRHS(),
              possibleAccumulator.first, context)) {
        possibleAccumulatingAssignment.second
            .rhs_also_references_possible_accumulator = true;
        possibleAccumulator.second
            .number_of_possible_accumulating_assignments_whose_RHS_also_references_this +=
            1;
      }
    }
  }
}
}
}
}
