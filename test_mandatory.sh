
#!/usr/bin/env bash
#
# test_mandatory.sh - localhost functional tester for ircserv, driven over raw
# TCP with nc. This script does NOT compile or launch the server: start it
# yourself first, then run this in another terminal.
#
#   make && ./ircserv 6667 pass     # terminal 1
#   ./test_mandatory.sh             # terminal 2
#
# Requires: bash, nc. Works on macOS and Linux.

# =====================  CONFIG (edit to match your server)  =================
PORT="${1:-6667}"          # server port         (or pass as 1st arg)
PASS="${2:-pass}"          # server password     (or pass as 2nd arg)
HOST="${HOST:-127.0.0.1}"  # localhost only by design
WAIT="${WAIT:-0.6}"        # seconds to wait for a reply (raise if flaky)
SERVER="${SERVER:-humble_server}"
# ===========================================================================

TMP="$(mktemp -d)"

# ----- pretty output -------------------------------------------------------
if [ -t 1 ]; then
	GRN=$'\033[32m'; RED=$'\033[31m'; DIM=$'\033[2m'; RST=$'\033[0m'
else
	GRN=; RED=; DIM=; RST=
fi

TOTAL=0; PASSED=0; FAILED=0
GRABBED=""
FAILED_LIST=""

pass() {
	TOTAL=$((TOTAL + 1)); PASSED=$((PASSED + 1))
	printf "  ${GRN}PASS${RST} %s\n" "$1"
}

fail() {
	TOTAL=$((TOTAL + 1)); FAILED=$((FAILED + 1))
	FAILED_LIST="$FAILED_LIST
  - $1"
	printf "  ${RED}FAIL${RST} %s\n" "$1"
	printf "       ${DIM}expected:${RST} %s\n" "$2"
	printf "       ${DIM}received:${RST} %s\n" "$(printf '%s' "$3" | tr '\n' '|')"
}

section() { printf "\n${DIM}== %s ==${RST}\n" "$1"; }

# ----- cleanup -------------------------------------------------------------
cleanup() {
	exec 3>&- 4>&- 5>&- 6>&- 2>/dev/null
	[ -n "$A_PID" ] && kill "$A_PID" 2>/dev/null
	[ -n "$B_PID" ] && kill "$B_PID" 2>/dev/null
	[ -n "$C_PID" ] && kill "$C_PID" 2>/dev/null
	[ -n "$Q_PID" ] && kill "$Q_PID" 2>/dev/null
	rm -rf "$TMP"
}
trap cleanup EXIT INT TERM

# ----- make sure the server is reachable (fail fast, don't hang) -----------
if ! (exec 9<>"/dev/tcp/$HOST/$PORT") 2>/dev/null; then
	printf "${RED}cannot reach %s:%s${RST}\n" "$HOST" "$PORT"
	printf "start your server first, e.g.:  make && ./ircserv %s %s\n" "$PORT" "$PASS"
	exit 1
fi
exec 9>&- 2>/dev/null
printf "testing server at %s:%s (password '%s')\n" "$HOST" "$PORT" "$PASS"

# ----- persistent client plumbing ------------------------------------------
# Each client is an nc process reading from a FIFO (so the connection stays
# open) and writing everything it receives to an output file. We keep the
# write end of the FIFO open on a fixed fd (3 for A, 4 for B).
A_IN="$TMP/a.in"; A_OUT="$TMP/a.out"; A_OFF=0
B_IN="$TMP/b.in"; B_OUT="$TMP/b.out"; B_OFF=0

: >"$A_OUT"; : >"$B_OUT"
mkfifo "$A_IN" "$B_IN"

nc "$HOST" "$PORT" <"$A_IN" >"$A_OUT" 2>/dev/null & A_PID=$!
exec 3>"$A_IN"
nc "$HOST" "$PORT" <"$B_IN" >"$B_OUT" 2>/dev/null & B_PID=$!
exec 4>"$B_IN"

sendA() { printf '%s\r\n' "$*" >&3; }
sendB() { printf '%s\r\n' "$*" >&4; }

# grab LABEL -> stores the bytes appended since the last grab of LABEL into
# $GRABBED and advances that client's offset. NOTE: this must be called
# directly (not inside $(...)) or the offset update is lost to a subshell.
grab() {
	local out off cur label="$1"
	case "$label" in
		A) out="$A_OUT"; off="$A_OFF" ;;
		B) out="$B_OUT"; off="$B_OFF" ;;
		Q) out="$Q_OUT"; off="$Q_OFF" ;;
	esac
	cur=$(wc -c <"$out"); cur=$((cur))
	GRABBED=$(tail -c "+$((off + 1))" "$out" | tr -d '\r')
	case "$label" in
		A) A_OFF=$cur ;;
		B) B_OFF=$cur ;;
		Q) Q_OFF=$cur ;;
	esac
}

# drain LABEL -> wait, then discard whatever arrived (keeps offsets clean)
drain() { sleep "$WAIT"; grab "$1"; }

# Return 0 if $line matches pattern $pat (full-line match only).
#   re:REGEX   -> line must match REGEX (^ and $ implied if omitted)
#   =TEXT      -> line must equal TEXT exactly
line_matches() {
	local line="$1" pat="$2"
	case "$pat" in
		re:*)
			pat="${pat#re:}"
			case "$pat" in
				^*) ;;
				*) pat="^${pat}" ;;
			esac
			case "$pat" in
				*\$) ;;
				*) pat="${pat}\$" ;;
			esac
			printf '%s' "$line" | grep -qE -- "$pat"
			;;
		=*)
			[ "$line" = "${pat#=}" ]
			;;
		*)
			[ "$line" = ":${SERVER} ${pat}" ]
			;;
	esac
}

# Return 0 if some line in $actual matches $pat.
any_line_matches() {
	local actual="$1" pat="$2" line
	while IFS= read -r line || [ -n "$line" ]; do
		[ -z "$line" ] && continue
		line_matches "$line" "$pat" && return 0
	done <<EOF
${actual}
EOF
	return 1
}

# verify DESC ACTUAL [PATTERN...]   (no PATTERN => expect silence)
# Each pattern must match one complete IRC line (not a substring):
#   re:REGEX   anchored full-line regex (^ and $ added if omitted)
#   =TEXT      exact line literal
#   TEXT       exact line ":SERVER TEXT"
verify() {
	local desc="$1" actual="$2"; shift 2
	if [ "$#" -eq 0 ]; then
		if [ -z "$actual" ]; then pass "$desc"; else fail "$desc" "<silence>" "$actual"; fi
		return
	fi
	local miss="" e
	for e in "$@"; do
		any_line_matches "$actual" "$e" || miss="$miss[$e]"
	done
	[ -z "$miss" ] && pass "$desc" || fail "$desc" "$miss" "$actual"
}

# expect LABEL DESC [EXPECTED...]  -> read from a live client after a wait
expect() {
	local label="$1" desc="$2"; shift 2
	sleep "$WAIT"
	grab "$label"
	verify "$desc" "$GRABBED" "$@"
}

# oneshot PAYLOAD  -> open a fresh connection, send PAYLOAD, print the reply
# PAYLOAD is fed to printf %b, so use \r\n between commands.
oneshot() {
	local out="$TMP/os.$RANDOM"
	{ printf '%b' "$1"; sleep "$WAIT"; } | nc "$HOST" "$PORT" >"$out" 2>/dev/null &
	local pid=$!
	( sleep 4; kill "$pid" 2>/dev/null ) 2>/dev/null &
	wait "$pid" 2>/dev/null
	tr -d '\r' <"$out"; rm -f "$out"
}

###########################################################################
# TESTS
###########################################################################

section "Registration (PASS / NICK / USER)"
sendA "PASS $PASS"; sendA "NICK aylmer"; sendA "USER aylmer 0 * :Aylmer"
sleep "$WAIT"; grab A
verify "A registers -> 001..005 greeting burst" "$GRABBED" \
	're:^:humble_server 001 aylmer :Welcome to this Helsinki based Internet Relay Chat server, aylmer!~aylme@127\.0\.0\.1$' \
	're:^:humble_server 002 aylmer :Your host is humble_server, running version 0\.042$' \
	're:^:humble_server 003 aylmer :This server was created Mon Jan 01 2024 at 00:00:00 EET$' \
	're:^:humble_server 004 aylmer humble_server 0\.042 o itkl$' \
	're:^:humble_server 005 aylmer CHANTYPES=#& PREFIX=\(o\)@ NETWORK=Hive CASEMAPPING=ascii :are supported by this server$'
verify "A registers -> MOTD 375/372/376" "$GRABBED" \
	're:^:humble_server 375 aylmer :- humble_server Message of the day -$' \
	're:^:humble_server 372 aylmer :- Welcome to the our humble server!$' \
	're:^:humble_server 376 aylmer :End of /MOTD command$'
sendB "PASS $PASS"; sendB "NICK bob"; sendB "USER bob 0 * :Bob"
expect B "B registers -> 001..004" \
	're:^:humble_server 001 bob :Welcome to this Helsinki based Internet Relay Chat server, bob!~bob@127\.0\.0\.1$' \
	're:^:humble_server 004 bob humble_server 0\.042 o itkl$'

sendA "PASS $PASS"
expect A "PASS after registration -> 462" \
	're:^:humble_server 462 aylmer :You may not reregister$'

section "JOIN"
sendA "JOIN #test"
expect A "JOIN #test -> echo, RPL_NAMREPLY 353, 366, op prefix" \
	're:^:[^!]+![^@]+@[^ ]+ JOIN #test$' \
	're:^:humble_server 353 aylmer = #test :@aylmer$' \
	're:^:humble_server 366 aylmer #test :End of /NAMES list$'
sendB "JOIN #test"
expect B "B JOIN #test -> echo + names" \
	're:^:[^!]+![^@]+@[^ ]+ JOIN #test$' \
	're:^:humble_server 353 bob = #test :(@aylmer bob|bob @aylmer)$' \
	're:^:humble_server 366 bob #test :End of /NAMES list$'
expect A "A sees B's JOIN broadcast" \
	're:^:[^!]+![^@]+@[^ ]+ JOIN #test$'
sendA "JOIN"
expect A "JOIN no params -> 461" \
	're:^:humble_server 461 aylmer JOIN :Not enough parameters$'
sendA "JOIN test"
expect A "JOIN bad channel mask (no #) -> 476" \
	're:^:humble_server 476 aylmer test :Bad Channel Mask$'

section "PRIVMSG"
sendA "PRIVMSG #test :hello all"
expect B "PRIVMSG to channel -> B receives" \
	're:^:[^!]+![^@]+@[^ ]+ PRIVMSG #test :hello all$'
sendA "PRIVMSG bob :hi bob"
expect B "PRIVMSG to nick -> B receives" \
	're:^:[^!]+![^@]+@[^ ]+ PRIVMSG bob :hi bob$'
sendA "PRIVMSG ghost :x"
expect A "PRIVMSG unknown nick -> 401" \
	're:^:humble_server 401 aylmer ghost :No such nick/channel$'
sendA "PRIVMSG #ghost :x"
expect A "PRIVMSG unknown channel -> 403" \
	're:^:humble_server 403 aylmer #ghost :No such channel$'
sendA "PRIVMSG"
expect A "PRIVMSG no params -> 461" \
	're:^:humble_server 461 aylmer PRIVMSG :Not enough parameters$'

section "MODE"
sendB "MODE #test +t"
expect B "MODE by non-operator -> 482" \
	're:^:humble_server 482 bob #test :You'\''re not channel operator$'
sendA "MODE #test +t";        expect A "MODE +t -> broadcast" 're:^:[^!]+![^@]+@[^ ]+ MODE #test \+t$'; drain B
sendA "MODE #test +k secret"; expect A "MODE +k -> broadcast" 're:^:[^!]+![^@]+@[^ ]+ MODE #test \+k secret$'; drain B
sendA "MODE #test +l 5";      expect A "MODE +l -> broadcast" 're:^:[^!]+![^@]+@[^ ]+ MODE #test \+l 5$'; drain B
sendA "MODE #test +o bob";    expect A "MODE +o bob -> broadcast" 're:^:[^!]+![^@]+@[^ ]+ MODE #test \+o bob$'; drain B
sendA "MODE #test"
expect A "MODE query -> RPL_CHANNELMODEIS 324" \
	're:^:humble_server 324 aylmer #test \+tkl secret 5$'
sendA "MODE"
expect A "MODE no params -> 461" \
	're:^:humble_server 461 aylmer MODE :Not enough parameters$'
sendA "MODE #ghost +t"
expect A "MODE unknown channel -> 403" \
	're:^:humble_server 403 aylmer #ghost :No such channel$'

section "TOPIC"
sendA "TOPIC #test :Hello World"
expect A "TOPIC set (op) -> broadcast" \
	're:^:[^!]+![^@]+@[^ ]+ TOPIC #test :Hello World$'; drain B
sendA "TOPIC #test"
expect A "TOPIC query -> 332" \
	're:^:humble_server 332 aylmer #test :Hello World$'
sendA "TOPIC"
expect A "TOPIC no params -> 461" \
	're:^:humble_server 461 aylmer TOPIC :Not enough parameters$'
sendA "TOPIC #ghost"
expect A "TOPIC unknown channel -> 403" \
	're:^:humble_server 403 aylmer #ghost :No such channel$'
sendA "JOIN #solo"; drain A
sendB "TOPIC #solo"
expect B "TOPIC while not on channel -> 442" \
	're:^:humble_server 442 bob #solo :You'\''re not on that channel$'

section "INVITE (+i channel)"
sendA "JOIN #priv"; drain A
sendA "MODE #priv +i"; drain A
sendA "INVITE bob #priv"
expect A "INVITE -> RPL_INVITING 341 to inviter" \
	're:^:humble_server 341 aylmer bob #priv$'
expect B "INVITE -> target receives INVITE" \
	're:^:[^!]+![^@]+@[^ ]+ INVITE bob :#priv$'
sendB "JOIN #priv"
expect B "invited user can JOIN the +i channel" \
	're:^:[^!]+![^@]+@[^ ]+ JOIN #priv$'; drain A
sendA "INVITE"
expect A "INVITE no params -> 461" \
	're:^:humble_server 461 aylmer INVITE :Not enough parameters$'
sendA "INVITE bob #ghost"
expect A "INVITE unknown channel -> 403" \
	're:^:humble_server 403 aylmer #ghost :No such channel$'
sendA "INVITE ghostuser #test"
expect A "INVITE unknown nick -> 401" \
	're:^:humble_server 401 aylmer ghostuser :No such nick/channel$'

section "KICK"
sendA "KICK #test bob :bye"
expect B "KICK -> victim notified" \
	're:^:[^!]+![^@]+@[^ ]+ KICK #test bob :bye$'; drain A
sendA "KICK #test"
expect A "KICK missing params -> 461" \
	're:^:humble_server 461 aylmer KICK :Not enough parameters$'
sendA "KICK #ghost bob"
expect A "KICK unknown channel -> 403" \
	're:^:humble_server 403 aylmer #ghost :No such channel$'
sendA "KICK #test ghostuser"
expect A "KICK victim not on channel -> 441" \
	're:^:humble_server 441 aylmer #test ghostuser :They aren'\''t on that channel$'

section "NAMES / LIST"
sendA "NAMES #test"
expect A "NAMES #test -> 353 + 366" \
	're:^:humble_server 353 aylmer = #test :@aylmer$' \
	're:^:humble_server 366 aylmer #test :End of /NAMES list$'
sendA "NAMES"
expect A "NAMES no params -> 461" \
	're:^:humble_server 461 aylmer NAMES :Not enough parameters$'
sendA "NAMES #ghost"
expect A "NAMES unknown channel -> 403" \
	're:^:humble_server 403 aylmer #ghost :No such channel$'
sendA "LIST"
expect A "LIST -> RPL_LIST 322 + RPL_LISTEND 323" \
	're:^:humble_server 322 aylmer #test 1 :Hello World$' \
	're:^:humble_server 323 aylmer :End of /LIST$'

section "PING"
sendA "PING :tok123"
expect A "PING -> PONG echoes the token" \
	're:^:humble_server PONG humble_server :tok123$'

section "JOIN edge cases (keys / invite / limit)"
sendA "JOIN #dup"; drain A
sendA "JOIN #dup"
expect A "JOIN same channel twice -> silent"
sendA "JOIN #key"; drain A
sendA "MODE #key +k s3cret"; drain A
sendB "JOIN #key"
expect B "JOIN +k without key -> 475" \
	're:^:humble_server 475 bob #key :Cannot join channel \(\+k\)$'
sendB "JOIN #key wrongkey"
expect B "JOIN +k wrong key -> 475" \
	're:^:humble_server 475 bob #key :Cannot join channel \(\+k\)$'
sendB "JOIN #key s3cret"
expect B "JOIN +k correct key -> success" \
	're:^:[^!]+![^@]+@[^ ]+ JOIN #key$' \
	're:^:humble_server 366 bob #key :End of /NAMES list$'; drain A
sendA "JOIN #inv"; drain A
sendA "MODE #inv +i"; drain A
sendB "JOIN #inv"
expect B "JOIN +i without invite -> 473" \
	're:^:humble_server 473 bob #inv :Cannot join channel \(\+i\)$'
sendA "JOIN #lim"; drain A
sendA "MODE #lim +l 1"; drain A
sendB "JOIN #lim"
expect B "JOIN +l full channel -> 471" \
	're:^:humble_server 471 bob #lim :Cannot join channel \(\+l\)$'

section "PART edge cases"
sendA "PART #ghost"
expect A "PART unknown channel -> 403" \
	're:^:humble_server 403 aylmer #ghost :No such channel$'
sendB "JOIN #bpart"; drain B
sendA "PART #bpart"
expect A "PART channel you are not on -> 442" \
	're:^:humble_server 442 aylmer #bpart :You'\''re not on that channel$'

section "PRIVMSG membership"
sendA "PRIVMSG #bpart :hi"
expect A "PRIVMSG to channel without membership -> 404" \
	're:^:humble_server 404 aylmer #bpart :Cannot send to channel$'
sendB "JOIN #leave2"; drain B
sendA "JOIN #leave2"; drain A; drain B
sendA "PART #leave2"; drain A; drain B
sendA "PRIVMSG #leave2 :hi"
expect A "PRIVMSG after leaving channel -> 404" \
	're:^:humble_server 404 aylmer #leave2 :Cannot send to channel$'

section "TOPIC (no topic / non-op)"
sendA "JOIN #nt"; drain A
sendA "TOPIC #nt"
expect A "TOPIC query with no topic -> 331" \
	're:^:humble_server 331 aylmer #nt :No topic is set$'
sendB "JOIN #nt"; drain B; drain A
sendB "TOPIC #nt :hax"
expect B "non-operator TOPIC on +t -> 482" \
	're:^:humble_server 482 bob #nt :You'\''re not channel operator$'

section "Operator permissions"
sendB "KICK #nt aylmer :nope"
expect B "non-operator KICK -> 482" \
	're:^:humble_server 482 bob #nt :You'\''re not channel operator$'
sendA "MODE #nt +i"; drain A; drain B
sendB "INVITE somebody #nt"
expect B "non-operator INVITE on +i -> 482" \
	're:^:humble_server 482 bob #nt :You'\''re not channel operator$'

section "MODE removal & argument errors"
sendA "JOIN #rm"; drain A
sendA "MODE #rm +itl 5"; drain A
sendA "MODE #rm -i"; expect A "MODE -i -> broadcast" 're:^:[^!]+![^@]+@[^ ]+ MODE #rm -i$'
sendA "MODE #rm -t"; expect A "MODE -t -> broadcast" 're:^:[^!]+![^@]+@[^ ]+ MODE #rm -t$'
sendA "MODE #rm -l"; expect A "MODE -l -> broadcast" 're:^:[^!]+![^@]+@[^ ]+ MODE #rm -l$'
sendA "MODE #rm +k kk"; expect A "MODE +k -> broadcast" 're:^:[^!]+![^@]+@[^ ]+ MODE #rm \+k kk$'
sendA "MODE #rm -k"; expect A "MODE -k -> broadcast" 're:^:[^!]+![^@]+@[^ ]+ MODE #rm -k$'
sendB "JOIN #rm"; drain B; drain A
sendA "MODE #rm +o bob"; expect A "MODE +o -> broadcast" 're:^:[^!]+![^@]+@[^ ]+ MODE #rm \+o bob$'; drain B
sendA "MODE #rm -o bob"; expect A "MODE -o -> broadcast" 're:^:[^!]+![^@]+@[^ ]+ MODE #rm -o bob$'; drain B
sendB "MODE #rm +t"
expect B "ex-operator (-o) lost privileges -> 482" \
	're:^:humble_server 482 bob #rm :You'\''re not channel operator$'
sendA "MODE #rm +k"
expect A "MODE +k without key -> 461" \
	're:^:humble_server 461 aylmer MODE :Not enough parameters$'
sendA "MODE #rm +l"
expect A "MODE +l without limit -> 461" \
	're:^:humble_server 461 aylmer MODE :Not enough parameters$'
sendA "MODE #rm +o"
expect A "MODE +o without nick -> 461" \
	're:^:humble_server 461 aylmer MODE :Not enough parameters$'
sendA "MODE #rm +z"
expect A "MODE unknown flag +z -> 472" \
	're:^:humble_server 472 aylmer #rm z :is unknown mode char to me$'
sendA "MODE #rm +o ghostuser"
expect A "MODE +o nonexistent user -> 441" \
	're:^:humble_server 441 aylmer #rm ghostuser :They aren'\''t on that channel$'

section "Parser / protocol"
sendA "FOOBAR baz"
expect A "unknown command -> 421" \
	're:^:humble_server 421 aylmer FOOBAR :Unknown command$'
sendA "PRIVMSG   bob   :multi space"
expect B "multiple spaces between params handled" \
	're:^:[^!]+![^@]+@[^ ]+ PRIVMSG bob :multi space$'
sendA "JOIN #ts   "
expect A "trailing spaces after param still parses" \
	're:^:[^!]+![^@]+@[^ ]+ JOIN #ts$'
printf 'PING :lftest\n' >&3
expect A "bare LF line ending handled" \
	're:^:humble_server PONG humble_server :lftest$'
LONGMSG=$(printf 'x%.0s' $(seq 1 400))
sendA "PRIVMSG bob :$LONGMSG"
sleep "$WAIT"; grab B
verify "long message (~400B) delivered" "$GRABBED" \
	"=:aylmer2!~aylme@127.0.0.1 PRIVMSG bob :${LONGMSG}"

section "Disconnect cleanup / nick reuse"
C_IN="$TMP/c.in"; C_OUT="$TMP/c.out"; C_OFF=0
: >"$C_OUT"; mkfifo "$C_IN"
nc "$HOST" "$PORT" <"$C_IN" >"$C_OUT" 2>/dev/null & C_PID=$!
exec 5>"$C_IN"
sendC() { printf '%s\r\n' "$*" >&5; }
sendC "PASS $PASS"; sendC "NICK carol"; sendC "USER carol 0 * :Carol"
sleep "$WAIT"
verify "nick in use while connected -> 433" \
	"$(oneshot 'PASS '"$PASS"'\r\nNICK carol\r\nUSER carol 0 * :C\r\n')" \
	're:^:humble_server 433 \* carol :Nickname is already in use$'
exec 5>&-; kill "$C_PID" 2>/dev/null; C_PID=""
sleep 1.5
verify "nick reusable after abrupt disconnect -> 001" \
	"$(oneshot 'PASS '"$PASS"'\r\nNICK carol\r\nUSER carol 0 * :C\r\n')" \
	're:^:humble_server 001 carol :Welcome to this Helsinki based Internet Relay Chat server, carol!~carol@127\.0\.0\.1$'

section "NICK changes / errors"
sendA "NICK aylmer2"
expect A "NICK change -> broadcast to self/channels" \
	're:^:[^!]+![^@]+@[^ ]+ NICK aylmer2$'
sendB "NICK aylmer2"
expect B "NICK already in use -> 433" \
	're:^:humble_server 433 bob aylmer2 :Nickname is already in use$'
sendB "NICK 3invalid"
expect B "NICK erroneous -> 432" \
	're:^:humble_server 432 bob 3invalid :Erroneous nickname\..*$'
sendB "NICK"
expect B "NICK no params -> 431" \
	're:^:humble_server 431 bob :No nickname given$'

section "PART"
# bob is still a member of #priv (joined earlier via INVITE), so part that one.
sendB "PART #priv"
expect B "PART -> broadcast" \
	're:^:[^!]+![^@]+@[^ ]+ PART #priv$'

section "QUIT"
Q_IN="$TMP/q.in"; Q_OUT="$TMP/q.out"; Q_OFF=0
: >"$Q_OUT"; mkfifo "$Q_IN"
nc "$HOST" "$PORT" <"$Q_IN" >"$Q_OUT" 2>/dev/null & Q_PID=$!
exec 6>"$Q_IN"
sendQ() { printf '%s\r\n' "$*" >&6; }
sendQ "PASS $PASS"; sendQ "NICK quitter"; sendQ "USER quitter 0 * :Quitter"
sleep "$WAIT"; grab Q; drain Q
sendA "JOIN #qtest"; drain A
sendQ "JOIN #qtest"; drain Q; drain A
sendQ "QUIT :goodbye"
expect A "QUIT -> broadcast to channel members" \
	're:^:[^!]+![^@]+@[^ ]+ QUIT :goodbye$'
sleep "$WAIT"; grab Q
verify "QUIT command recognized" "$GRABBED"
exec 6>&-; kill "$Q_PID" 2>/dev/null; Q_PID=""
sleep 1.5
verify "QUIT -> nick available after disconnect" \
	"$(oneshot 'PASS '"$PASS"'\r\nNICK quitter\r\nUSER quitter 0 * :Q\r\n')" \
	're:^:humble_server 001 quitter :Welcome to this Helsinki based Internet Relay Chat server, quitter!~quitt@127\.0\.0\.1$'

section "One-shot negatives (fresh connections)"
verify "wrong password -> 464" \
	"$(oneshot 'PASS wrong\r\nNICK zz\r\nUSER zz 0 * :zz\r\n')" \
	're:^:humble_server 464 zz :Password incorrect$'
verify "command before registration -> 451" \
	"$(oneshot 'JOIN #x\r\n')" \
	're:^:humble_server 451 \* :You have not registered$'

###########################################################################
# SUMMARY
###########################################################################
printf "\n${DIM}--------------------------------------------${RST}\n"
if [ "$FAILED" -gt 0 ]; then
	printf "${RED}failed tests:${RST}%s\n\n" "$FAILED_LIST"
fi
printf "result: ${GRN}%s${RST}/%s passed" "$PASSED" "$TOTAL"
[ "$FAILED" -gt 0 ] && printf "   (${RED}%s failed${RST})" "$FAILED"
printf "\n"
[ "$FAILED" -eq 0 ] && exit 0 || exit 1
