#!/bin/sh

RED='\033[0;31m'
GRN='\033[0;32m'
NC='\033[0m'

set -- "true" \
       "ls" \
       "ls -la" \
       "tar -cf malloc.tar libmalloc.so" \
       "od libmalloc.so" \
       "git status" \
       "less Makefile" \

for cmd in "$@"; do
	# Execute command with our Malloc
	LD_PRELOAD=./libmalloc.so $cmd > /dev/null
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

