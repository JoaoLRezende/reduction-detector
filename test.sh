#!/bin/bash

TEST_CASE="test inputs/reduction loops"

EXTRA_ARGUMENTS="--only-non-trivial-reductions"

# CLANG_ARGUMENTS can be used to inform the tool of where to find system header
# files (for example, if you see errors like "fatal error: 'stddef.h' file
# not found").
CLANG_ARGUMENTS="-I /usr/lib/llvm-3.8/bin/../lib/clang/3.8.0/include"

cd "$(dirname "$0")"
echo "Testing on $TEST_CASE."
# normal output
build/reduction-detector  --output test.out \
                          $EXTRA_ARGUMENTS "$TEST_CASE" \
                          -- $CLANG_ARGUMENTS
# verbose output
build/reduction-detector --verbose --output test_verbose.out \
                         $EXTRA_ARGUMENTS "$TEST_CASE" \
                         -- $CLANG_ARGUMENTS
