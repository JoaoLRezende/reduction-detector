#!/bin/bash

arguments="--print-non-reduction-loops --report-unlikely-accumulators
           --detect-increment-accumulations"

OUTPUT_DIRECTORY="test output"

cd "$(dirname "$0")"

if [ -z "$(type -p reduction-detector)" ]; then
    echo You don\'t seem to have reduction-detector in your PATH. Aborting.
    exit
fi

function execute_on_file {
    local file="$1"
    local normal_output_file="$2"
    local verbose_output_file="$3"

    echo "Testing on $file."
    # normal output
    reduction-detector  --output "${OUTPUT_DIRECTORY}/$normal_output_file" \
                        $arguments "$file"
    # verbose output
    reduction-detector --verbose --output "${OUTPUT_DIRECTORY}/$verbose_output_file" \
                       $arguments "$file"
}

execute_on_file "test cases/reduction loops/artificial examples.c" \
                "positive_test_cases.out" "positive_test_cases_verbose.out"

execute_on_file "test cases/non-reduction loops/artificial non-examples.c" \
                "negative_test_cases.out" "negative_test_cases_verbose.out"
                