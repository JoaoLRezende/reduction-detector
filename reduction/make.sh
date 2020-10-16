#!/bin/bash

# copy and make

cp reduction.cpp ../llvm/tools/clang/tools/extra/reduction &&

# Check the program for syntactic errors before starting the build process.
#../build/bin/clang-check ../llvm/tools/clang/tools/extra/reduction/reduction.cpp &&    # segfaults, for some reason

make -j reduction -C ../build/ --no-print-directory

echo "Done"

