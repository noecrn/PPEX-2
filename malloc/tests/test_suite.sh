#!/bin/sh

ref_out="ref_out"
my_out="my_out"

total_tests=0
passed_tests=0

RED='\033[0;31m'
GRN='\033[0;32m'
YEL='\033[0;33m'
NC='\033[0m'

echo $YEL "Running test suite..." $NC

func_dir () {
	for dir in tests/*; do
		if [ -d "$dir" ]; then
            func_file $dir
		fi
	done
}

func_file () {
    path="$1/test.sh"
    if [ -f "$path" ] && [ -x "$path" ]; then
        echo $YEL "--- $1 ---" $NC
        total_tests=$((total_tests + 1))

        # Execute the test
        ./$path
        if [ $? -eq 0 ]; then
            passed_tests=$((passed_tests + 1))
        fi
    fi
}

func_dir
echo ""
echo $YEL "Passed $passed_tests/$total_tests tests" $NC

exit 0;
