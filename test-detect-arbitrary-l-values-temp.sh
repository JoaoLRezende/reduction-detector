#!/bin/bash

# This is a terrible, temporary replacement for temp.sh to be used while we have a bunch of modules temporarily disabled. (The original test.sh fails because it passes command-line arguments that are defined in the disabled modules.)


cd "$(dirname "$0")"

./build/reduction-detector test\ inputs/test.c --print-non-reduction-loops --report-unlikely-accumulators -- -I /usr/lib/llvm-3.8/bin/../lib/clang/3.8.0/include