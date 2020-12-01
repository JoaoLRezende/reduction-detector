#!/bin/bash

DEFAULT="test_programs/test.c"

cd "$(dirname "$0")"

if [ -z "$1" ]
  then
    echo "testing on $DEFAULT"
    ../build/bin/reduction-detector -debug-potential-accumulator-detection $DEFAULT --
  else
    ../build/bin/reduction-detector -debug-potential-accumulator-detection $1 --
fi
