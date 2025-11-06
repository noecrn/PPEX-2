#!/bin/sh

my_out="my_out"
ref_out="ref_out"

RED='\033[0;31m'
GRN='\033[0;32m'
NC='\033[0m'

rm -f "$my_out"
rm -f "$ref_out"

set -- "true" \
       "ls" \
       "ls -la" \
       "tar -cf malloc.tar libmalloc.so" \
       "find /" \
       "tree /" \
       "od libmalloc.so" \
       "git status" \
       "less Makefile" \

for cmd in "$@"; do
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

