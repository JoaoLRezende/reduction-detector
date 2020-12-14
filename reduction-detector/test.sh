#!/bin/bash

DEFAULT_TEST_CASE="test_programs/test.c"
EXTRA_ARGUMENTS="-min-score 7"

cd "$(dirname "$0")"

if [ -z "$1" ]
  then
    echo "testing on $DEFAULT"
    ../build/bin/reduction-detector $EXTRA_ARGUMENTS $DEFAULT_TEST_CASE --
  else
    ../build/bin/reduction-detector $EXTRA_ARGUMENTS $1 --
fi
