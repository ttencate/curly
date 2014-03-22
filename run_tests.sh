#!/bin/bash

function test_get_root() {
	send_request "GET / HTTP/1.1\r\n\r\n"
	assert_response_status 200
}

function test_http_1_0_supported() {
	send_request "GET / HTTP/1.0\r\n\r\n"
	assert_response_status 200
}

function test_http_1_2_unsupported() {
	send_request "GET / HTTP/1.2\r\n\r\n"
	assert_response_status 505
}

function test_http_2_0_unsupported() {
	send_request "GET / HTTP/2.0\r\n\r\n"
	assert_response_status 505
}

function test_post_unsupported() {
	send_request "POST / HTTP/1.1\r\n\r\n"
	assert_response_status 405
	assert_response_header "Allow: GET, HEAD"
}

function test_empty_request() {
	send_request "\r\n\r\n"
	assert_response_status 400
}

function test_get_hello() {
	send_request "GET /hello.txt HTTP/1.1\r\n\r\n"
	assert_response_body "Hello world!"
}

function test_get_goodbye() {
	send_request "GET /goodbye.txt HTTP/1.1\r\n\r\n"
	assert_response_body "Goodbye world!"
}

function test_get_nonexistent() {
	send_request "GET /does_not_exist.txt HTTP/1.1\r\n\r\n"
	assert_response_status 404
}

#-------------------------------------------------------------------------------
# TEST HELPER FUNCTIONS
#-------------------------------------------------------------------------------

function send_request() {
	echo -ne "$1" | nc localhost ${port} > $last_response || exit 1
}

function extract_body() {
	local in_body=0
	local line
	while read -r; do
		line="${REPLY%[$'\r']}"
		if (( $in_body )); then
			echo "$line"
			continue
		fi
		if [[ -z "$line" ]]; then
			in_body=1
		fi
	done
}

function assert_response_status() {
	head -n1 $last_response | grep -q " $1 " || ( \
		echo "Expected first response line to contain $1, but was:" ; \
		head -n1 $last_response ) | fail
}

function assert_response_header() {
	grep -q -f <(echo "$1") $last_response || ( \
		echo "Expected response header '$1', but got:" ; \
		cat $last_response ) | fail
}

function assert_response_body() {
	od -tc $last_response
	cp $last_response /tmp/foo
	echo "FULL RESPONSE"
	cat $last_response
	echo "END"
	echo "BEGIN BODY"
	extract_body < $last_response
	echo "END BODY"
	diff <(echo "$1") <(extract_body < $last_response) >/dev/null || ( \
		echo "Mismatch in response body:" ; \
		diff -u <(echo "$1") <(extract_body < $last_response) ) | fail
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
