#!/bin/bash

cd "$(dirname "$0")"

cp src/* ../llvm-project/clang-tools-extra/finding-parallelizable-code-syntactically/ &&

# Check the program for syntactic errors before starting the build process.
#../build/bin/clang-check ../llvm-project/llvm-project/clang-tools-extra/finding-parallelizable-code-syntactically/reduction-detector.cpp &&    # segfaults, for some reason

cd ../build &&
../ninja/ninja

echo "Done"

