#!/bin/bash

function test_get_root() {
	return 0
}

function test_something_fails() {
	return 1
}

#-------------------------------------------------------------------------------

function atexit() {
	kill $curly_pid
}

red="\033[1;31m"
green="\033[1;32m"
normal="\033[0m"

cd $(dirname "$0")

echo -n "Starting server... "
src/curly -r test_root &
curly_pid=$!
echo "pid $curly_pid"
trap atexit EXIT

num_failures=0
while read test_func; do
	if ! kill -0 $curly_pid 2>/dev/null; then
		echo -e "${red}Server no longer running, exiting${normal}"
		exit 1
	fi
	echo -n "$(printf "%-40s" "Running ${test_func}... ")"
	if $test_func; then
		echo -e "[${green}PASS${normal}]"
	else
		echo -e "[${red}FAIL${normal}]"
		(( num_failures++ ))
	fi
done < <(declare -F | cut -d' ' -f3 | egrep '^test_')

if (( $num_failures != 0 )); then
	if (( $num_failures == 1 )); then
		verb=was
		plural=
	else
		verb=were
		plural=s
	fi
	echo -e "${red}There ${verb} ${num_failures} failure${plural}!${normal}"
else
	echo -e "${green}All tests pass!${normal}"
fi

exit $num_failures
