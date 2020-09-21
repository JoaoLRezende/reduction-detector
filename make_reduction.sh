#!/bin/bash

#copy and make
cp reduction.cpp llvm/tools/clang/tools/extra/reduction
make -j reduction -C build/ --no-print-directory
echo "Done"
