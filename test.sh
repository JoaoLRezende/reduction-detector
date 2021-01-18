#!/bin/bash

DEFAULT_TEST_CASE="test inputs/test.c"
EXTRA_ARGUMENTS="-min-score 7"
CLANG_ARGUMENTS="-I /usr/lib/llvm-3.8/bin/../lib/clang/3.8.0/include"

cd "$(dirname "$0")"

if [ -z "$1" ]
  then
    echo "Testing on $DEFAULT_TEST_CASE."
    build/reduction-detector $EXTRA_ARGUMENTS "$DEFAULT_TEST_CASE"  -- $CLANG_ARGUMENTS
  else
    build/reduction-detector $EXTRA_ARGUMENTS "$1"                  -- $CLANG_ARGUMENTS
fi
