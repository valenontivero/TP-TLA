#!/bin/bash

# Parser Test Script
# Tests the complete grammar implementation

cd "/Users/mvella/Projects/TLA DSL/DrumMachineLanguajeTLA"

echo "=========================================="
echo "DRUM MACHINE DSL - PARSER TEST"
echo "=========================================="
echo ""

tests=(
    "src/test/c/accept/11-simple-program"
    "src/test/c/accept/12-complex-program"
)

for test in "${tests[@]}"; do
    echo "-------------------------------------------"
    echo "Test: $(basename $test)"
    echo "-------------------------------------------"
    echo "Input:"
    cat "$test"
    echo ""
    echo "Output:"
    docker compose run --rm -e LOGGING_LEVEL=INFORMATION compiler bash -c \
        "src/main/bash/run.sh $test" 2>&1
    echo ""
done

echo "=========================================="
echo "✓ Parser tests completed"
echo "=========================================="
echo ""
