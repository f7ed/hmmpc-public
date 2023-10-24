#!/bin/bash

# Usage: ./test_unit.sh [test_name]
    # Trunc: test for the pure truncation [Protocol 3.1 Trunc]
    # Fixed-Mult: test for the fixed-point multiplication [Protocol 3.2 Fixed-Mult]
    # PreOR: test for computing the prefix-OR of shared bits [Protocol 4.1 PreOR]
    # Bitwise-LT: test for computing the result of bitwise less-than [Protocol 4.2 Bitwise-LT]
    # DReLU: test for computing the derivative of ReLU of shared numbers, i.e. the sign bit [Protocol 5.1 DReLU]
    # ReLU: test for computing the ReLU of shared numbers [Protocol 5.3 ReLU]
    # Maxpool: test for computing the Maxpool and its derivative of a shared vector [Protocol 5.4 Maxpool]

which_test=$1

EXECUTION=test_units.x

make -j8 $EXECUTION

./test_units.x 2 $which_test >> /dev/null &
./test_units.x 1 $which_test >> /dev/null &
./test_units.x 0 $which_test

echo "Execution completed"