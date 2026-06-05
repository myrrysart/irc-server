#!/usr/bin/env bash
set -euo pipefail
cd /home/deploy/irc-server
make re
if [ -f ircserv.pid ]; then kill "$(cat ircserv.pid)" 2>/dev/null || true; sleep 1; fi
nohup /home/deploy/irc-server/ircserv 6668 > ircserv.log 2>&1 & echo $! > ircserv.pid
sleep 1; kill -0 "$(cat ircserv.pid)" || { cat ircserv.log; exit 1; }
ss -ltn | awk '{print $4}' | grep -Eq '(^|:)6668$' || { cat ircserv.log; exit 1; }
