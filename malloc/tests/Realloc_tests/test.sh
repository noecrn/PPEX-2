#!/bin/sh

my_out="my_out"
ref_out="ref_out"
expected_code=0

RED='\033[0;31m'
NC='\033[0m'

rm -f "$my_out"
rm -f "$ref_out"

make --no-print-directory -f Makefile > "$ref_out"
"../../minimake" -f Makefile > "$my_out"
exit_code=$?

# --- CHECK EXIT CODE ---
if [ "$exit_code" -ne "$expected_code" ]; then
	echo Expected: $expected_code
	echo Got: $exit_code
	exit 1
fi

diff -u "$ref_out" "$my_out"
