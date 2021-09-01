#!/bin/bash

ADDITIONAL_ARGUMENTS=""

# CLANG_ARGUMENTS can be used to inform the tool of where to find system header
# files (for example, if you see errors like "fatal error: 'stddef.h' file
# not found").
CLANG_ARGUMENTS="-I /usr/lib/gcc/x86_64-linux-gnu/9/include
                 -I /usr/lib/llvm-3.8/bin/../lib/clang/3.8.0/include
                 -I /home/jplrez/intel/oneapi/compiler/2021.1.1/linux/compiler/include"

OUTPUT_DIRECTORY="test output"

cd "$(dirname "$0")"

function execute_on_file {
    local file="$1"
    local normal_output_file="$2"
    local verbose_output_file="$3"

    echo "Testing on $file."
    # normal output
    build/reduction-detector  --output "${OUTPUT_DIRECTORY}/$normal_output_file" \
                            $ADDITIONAL_ARGUMENTS "$file" \
                            -- $CLANG_ARGUMENTS
    # verbose output
    build/reduction-detector --verbose --output "${OUTPUT_DIRECTORY}/$verbose_output_file" \
                            $ADDITIONAL_ARGUMENTS "$file" \
                            -- $CLANG_ARGUMENTS
}

execute_on_file "test cases/reduction loops/artificial examples.c" test.out test_verbose.out