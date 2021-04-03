#!/bin/bash

DEFAULT_TEST_CASE="test inputs/test.c"

EXTRA_ARGUMENTS="--min-score 7 --only-non-trivial-reductions"

# CLANG_ARGUMENTS can be used to inform the tool of where to find system header
# files (for example, if you see errors like "fatal error: 'stddef.h' file
# not found").
CLANG_ARGUMENTS="-I /usr/lib/llvm-3.8/bin/../lib/clang/3.8.0/include"

cd "$(dirname "$0")"

if [ -z "$1" ]
  then
    echo "Testing on $DEFAULT_TEST_CASE."
    build/reduction-detector $EXTRA_ARGUMENTS "$DEFAULT_TEST_CASE"  -- $CLANG_ARGUMENTS
  else
    build/reduction-detector $EXTRA_ARGUMENTS "$1"                  -- $CLANG_ARGUMENTS
fi
