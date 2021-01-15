#!/bin/bash

DEFAULT_TEST_CASE="test inputs/test.c"
EXTRA_ARGUMENTS="-min-score 7"

cd "$(dirname "$0")"

if [ -z "$1" ]
  then
    echo "Testing on $DEFAULT_TEST_CASE."
    build/reduction-detector $EXTRA_ARGUMENTS "$DEFAULT_TEST_CASE"  --
  else
    build/reduction-detector $EXTRA_ARGUMENTS "$1"                  --
fi