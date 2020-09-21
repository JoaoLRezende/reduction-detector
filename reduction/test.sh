#!/bin/bash
if [ -z "$1" ]
  then
    ../build/bin/reduction test_programs/test_program_2.cpp --
  else
    ../build/bin/reduction "test_programs/$1" --
fi
