#!/bin/bash
if [ -z "$1" ]
  then
    build/bin/reduction teste2.cpp --
  else
    build/bin/reduction $1 --
fi
