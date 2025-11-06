#!/bin/sh

my_out="my_out"
ref_out="ref_out"

RED='\033[0;31m'
GRN='\033[0;32m'
NC='\033[0m'

rm -f "$my_out"
rm -f "$ref_out"

COMMANDS=(
	"ls"
	"factor 20 30 40 50 60 70 80 90"
	"cat ../Makefile"
	"ip a"
	"ls not_a_file"
)

for cmd in "${COMMANDS}"; do
	# Execute command with our Malloc
	LD_PRELOAD=../libmalloc.so $cmd > /dev/null 2>&1
	exit_code=$?

	# Check exit code
	if [ $exit_code -eq 0 ]; then
		echo $GRN "OK" $NC
	else
		echo $RED "KO"
		echo Expected: $expected_code
		echo Got: $exit_code $NC
		exit 1
	fi
done

diff -u "$ref_out" "$my_out"
