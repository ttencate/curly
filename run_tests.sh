#!/bin/bash

function test_get_root() {
	send_request "GET / HTTP/1.1\r\n\r\n"
	assert_response_status 200
}

function test_http_1_0_supported() {
	send_request "GET / HTTP/1.0\r\n\r\n"
	assert_response_status 200
}

function test_http_2_0_unsupported() {
	send_request "GET / HTTP/2.0\r\n\r\n"
	assert_response_status 505
}

function test_empty_request() {
	send_request "\r\n\r\n"
	assert_response_status 400
}

function test_missing_http_version() {
	send_request "GET /\r\n\r\n"
	assert_response_status 400
}

function test_invalid_http_version() {
	send_request "GET / HTTP/1.x\r\n\r\n"
	assert_response_status 400
}

function test_incomplete_http_version() {
	send_request "GET / H\r\n\r\n"
	assert_response_status 400
}

function test_null_byte_in_request_line() {
	send_request "GET / HTTP/1.1\0\r\n\r\n"
	assert_response_status 400
}

function test_null_byte_in_blank_line() {
	send_request "GET / HTTP/1.1\r\n\0\r\n"
	assert_response_status 400
}

#-------------------------------------------------------------------------------
# TEST HELPER FUNCTIONS
#-------------------------------------------------------------------------------

function send_request() {
	echo -ne "$1" | nc localhost ${port} > $last_response || exit 1
}

function assert_response_status() {
	head -n1 $last_response | grep -q " $1 " || ( \
		echo "Expected first response line to contain $1, but was:" ; \
		head -n1 $last_response ) | fail
}

function fail() {
	cat
	return 1
}

#-------------------------------------------------------------------------------
# UNIT TEST SCAFFOLDING
#-------------------------------------------------------------------------------

function all_test_functions() {
	declare -F | cut -d' ' -f3 | egrep '^test_'
}

last_response=$(mktemp)
test_output=$(mktemp)

function atexit() {
	kill $curly_pid
	rm -f $last_response
	rm -f $test_output
}

red="\033[1;31m"
green="\033[1;32m"
normal="\033[0m"

cd $(dirname "$0")

port=8081

echo -n "Starting server... "
src/curly -r test_root -p ${port} &
curly_pid=$!
disown
echo "pid $curly_pid"
trap atexit EXIT

longest_test_function_name=$(all_test_functions | wc -L)

num_failures=0
while read test_function; do
	if ! kill -0 $curly_pid 2>/dev/null; then
		echo -e "${red}Server no longer running, exiting${normal}"
		exit 1
	fi

	echo -n "$(printf "%-$(( $longest_test_function_name + 12 ))s" "Running ${test_function}... ")"

	if $test_function > $test_output; then
		echo -e "[${green}PASS${normal}]"
	else
		echo -e "[${red}FAIL${normal}]"
		cat $test_output
		(( num_failures++ ))
	fi
done < <(all_test_functions)

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
