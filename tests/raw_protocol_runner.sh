#!/bin/bash
set -e

OUR_HOST="127.0.0.1"
OUR_PORT="6668"
OUR_PASS="password"

REF_HOST="37.27.33.34"
REF_PORT="6667"
REF_PASS=""

ALICE_NICK="rawalice"
BOB_NICK="rawbob"
TEST_CHANNEL="#rawtest"

OUR_READ_FIRST_TIMEOUT="2"
REF_READ_FIRST_TIMEOUT="25"

OUTPUT_DIR="$(dirname "$0")/output/raw"
rm -rf "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

send_line() {
	local fd="$1" line="$2"
	printf '%s\r\n' "$line" >&"$fd"
	sleep 0.15
}

read_some() {
	local fd="$1" out="$2" chunk idle
	local got=0 timeout
	idle=0

	while [ "$idle" -lt 5 ]; do
		if [ "$got" -eq 0 ]; then
			timeout="$READ_FIRST_TIMEOUT"
		else
			timeout="0.2"
		fi

		if IFS= read -r -t "$timeout" -u "$fd" chunk; then
			printf '%s\n' "$chunk" >> "$out"
			got=1
			idle=0
		else
			if [ "$got" -eq 0 ]; then
				return 0
			fi
			idle=$((idle + 1))
		fi
	done
}

register_client() {
	local fd="$1" pass="$2" nick="$3"

	if [ -n "$pass" ]; then
		send_line "$fd" "PASS $pass"
	fi
	send_line "$fd" "NICK $nick"
	send_line "$fd" "USER $nick 0 * :$nick"
}

normalize_file() {
	sed -E \
		-e 's/\r$//' \
		-e "s/$OUR_HOST/<server>/g" \
		-e "s/$REF_HOST/<server>/g" \
		-e "s/$OUR_PORT/<port>/g" \
		-e "s/$REF_PORT/<port>/g" \
		-e "s/$OUR_PASS/<pass>/g" \
		-e 's/:[^ !]+![^ ]+ /:nick!user@host /g' \
		"$1" > "$2"
}

client_fd() {
	case "$1" in
		alice) echo 3 ;;
		bob) echo 4 ;;
		*) echo "Unknown client: $1"; return 1 ;;
	esac
}

client_nick() {
	case "$1" in
		alice) echo "$ALICE_NICK" ;;
		bob) echo "$BOB_NICK" ;;
		*) echo "Unknown client: $1"; return 1 ;;
	esac
}

connect() {
	local client="$1" fd
	fd="$(client_fd "$client")"
	if ! eval "exec $fd<>\"/dev/tcp/$CURRENT_HOST/$CURRENT_PORT\""; then
		echo "	failed to connect $client to $CURRENT_HOST:$CURRENT_PORT"
		SIDE_FAILED=1
	fi
}

register() {
	local client="$1" fd nick
	fd="$(client_fd "$client")"
	nick="$(client_nick "$client")"
	register_client "$fd" "$CURRENT_PASS" "$nick" || SIDE_FAILED=1
}

send() {
	local client="$1" line="$2" fd
	fd="$(client_fd "$client")"
	send_line "$fd" "$line" || SIDE_FAILED=1
}

read_into() {
	local client="$1" fd
	fd="$(client_fd "$client")"
	read_some "$fd" "$CURRENT_RAW" || SIDE_FAILED=1
}

close_clients() {
	exec 3>&- 2>/dev/null || true
	exec 4>&- 2>/dev/null || true
}

test_register() {
	connect alice
	register alice
	read_into alice
}

test_join() {
	connect alice
	register alice
	read_into alice

	send alice "JOIN $TEST_CHANNEL"
	read_into alice
}

test_channel_privmsg() {
	connect alice
	connect bob
	register alice
	register bob
	read_into alice
	read_into bob

	send alice "JOIN $TEST_CHANNEL"
	send bob "JOIN $TEST_CHANNEL"
	read_into alice
	read_into bob

	send alice "PRIVMSG $TEST_CHANNEL :hello raw channel"
	read_into bob
}

test_direct_privmsg() {
	connect alice
	connect bob
	register alice
	register bob
	read_into alice
	read_into bob

	send alice "PRIVMSG $BOB_NICK :hello raw direct"
	read_into bob
}

test_unknown() {
	connect alice
	register alice
	read_into alice

	send alice "BOGUSCMD arg"
	read_into alice
}

run_side() {
	local test="$1" host="$2" port="$3" pass="$4" side="$5" timeout="$6"

	READ_FIRST_TIMEOUT="$timeout"
	CURRENT_HOST="$host"
	CURRENT_PORT="$port"
	CURRENT_PASS="$pass"
	CURRENT_RAW="$OUTPUT_DIR/output-$side-${test#test_}-raw.txt"
	CURRENT_OUT="$OUTPUT_DIR/output-$side-${test#test_}.txt"
	SIDE_FAILED=0

	: > "$CURRENT_RAW"
	"$test"
	close_clients

	normalize_file "$CURRENT_RAW" "$CURRENT_OUT"
	[ "$SIDE_FAILED" -eq 0 ]
}

run_test() {
	local test="$1"
	local name="${test#test_}"
	local our_file="$OUTPUT_DIR/output-our-$name.txt"
	local ref_file="$OUTPUT_DIR/output-reference-$name.txt"
	local diff_file="$OUTPUT_DIR/output-diff-$name.txt"
	local our_ok=0 ref_ok=0

	echo
	echo "===== $name ====="

	if ! run_side "$test" "$OUR_HOST" "$OUR_PORT" "$OUR_PASS" our "$OUR_READ_FIRST_TIMEOUT"; then
		our_ok=1
	fi

	if ! run_side "$test" "$REF_HOST" "$REF_PORT" "$REF_PASS" reference "$REF_READ_FIRST_TIMEOUT"; then
		ref_ok=1
	fi

	if [ "$our_ok" -ne 0 ]; then
		echo "	warning: project server run had errors"
		failures=$((failures + 1))
	fi
	if [ "$ref_ok" -ne 0 ]; then
		echo "	warning: reference server run had errors"
	fi

	echo "--- diff: reference vs project ---"
	diff -u "$ref_file" "$our_file" | tee "$diff_file" || true

	echo "	our: $our_file"
	echo "	ref: $ref_file"
	echo "	diff: $diff_file"
}

failures=0
echo "Testing raw protocol against $OUR_HOST:$OUR_PORT (reference $REF_HOST:$REF_PORT)"
for test in $(compgen -A function test_); do
	run_test "$test"
done
exit "$failures"
