#!/bin/bash

TEST_CASE="test cases/reduction loops"

ADDITIONAL_ARGUMENTS="--only-non-trivial-reductions"

# CLANG_ARGUMENTS can be used to inform the tool of where to find system header
# files (for example, if you see errors like "fatal error: 'stddef.h' file
# not found").
CLANG_ARGUMENTS="-I /usr/lib/llvm-3.8/bin/../lib/clang/3.8.0/include"

OUTPUT_DIRECTORY="test output"

cd "$(dirname "$0")"
echo "Testing on $TEST_CASE."
# normal output
build/reduction-detector  --output "${OUTPUT_DIRECTORY}/test.out" \
                          $ADDITIONAL_ARGUMENTS "$TEST_CASE" \
                          -- $CLANG_ARGUMENTS
# verbose output
build/reduction-detector --verbose --output "${OUTPUT_DIRECTORY}/test_verbose.out" \
                         $ADDITIONAL_ARGUMENTS "$TEST_CASE" \
                         -- $CLANG_ARGUMENTS
