#!/bin/bash

DEFAULT="test_programs/test.c"

if [ -z "$1" ]
  then
    echo "testing on $DEFAULT"
    ../build/bin/reduction $DEFAULT --
  else
    ../build/bin/reduction $1 --
fi
