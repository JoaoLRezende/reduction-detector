#!/bin/bash

JOB_COUNT="5"

cd "$(dirname "$0")"
shopt -s globstar

# Copy C source files into LLVM's source tree.
# TODO: use recursive glob patterns to allow modules within subdirectoriess.
rm -f                                ../llvm-project/clang-tools-extra/reduction-detector/**/*.cpp &&
cp --preserve --parents src/**/*.cpp ../llvm-project/clang-tools-extra/reduction-detector/ &&

# Copy header files into LLVM's source tree's include directory.
rm                  ../llvm-project/clang/include/reduction-detector/* &&
cp -p src/include/* ../llvm-project/clang/include/reduction-detector/ &&

# Check the program for syntactic errors before starting the build process.
#../build/bin/clang-check ../llvm-project/llvm-project/clang-tools-extra/finding-parallelizable-code-syntactically/reduction-detector.cpp &&    # segfaults, for some reason

cd ../build &&
ninja -j $JOB_COUNT &&

echo "Done"

