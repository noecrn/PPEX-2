#!/bin/sh

RED='\033[0;31m'
GRN='\033[0;32m'
NC='\033[0m'

set -- "true" \
       "ls" \
       "factor 20 30 40 50 60 70 80 90" \
       "cat Makefile" \
       "ip a" \
       "sort Makefile" \
       "grep -r . src" \
       "cat libmalloc.so > /dev/null"

for cmd in "$@"; do
	# Execute command with our Malloc
	LD_PRELOAD=./libmalloc.so > /dev/null
	exit_code=$?

    echo $cmd

	# Check exit code
	if [ $exit_code -eq 0 ]; then
		echo $GRN "OK" $NC
	else
		echo $RED "KO"
		echo Expected: 0
		echo Got: $exit_code $NC
		exit 1
	fi
done

