#!/bin/bash

DEFAULT="test_programs/test.c"

if [ -z "$1" ]
  then
    echo "testing on $DEFAULT"
    ../build/bin/reduction-detector $DEFAULT --
  else
    ../build/bin/reduction-detector $1 --
fi
