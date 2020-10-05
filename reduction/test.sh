#!/bin/bash

DEFAULT="test_programs/NPB/NPB-serial-IS.c"

if [ -z "$1" ]
  then
    echo "testing on $DEFAULT"
    ../build/bin/reduction $DEFAULT --
  else
    ../build/bin/reduction $1 --
fi
