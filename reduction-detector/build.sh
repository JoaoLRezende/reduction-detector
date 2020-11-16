#!/bin/bash

cd "$(dirname "$0")"

#TODO: use recursive glob patterns to allow modules within subdirectoriess.
cp src/*.cpp ../llvm-project/clang-tools-extra/reduction-detector/ &&
cp src/include/* ../llvm-project/clang/include/reduction-detector/ &&

# Check the program for syntactic errors before starting the build process.
#../build/bin/clang-check ../llvm-project/llvm-project/clang-tools-extra/finding-parallelizable-code-syntactically/reduction-detector.cpp &&    # segfaults, for some reason

cd ../build &&
../ninja/ninja &&

echo "Done"

