# Mandatory Requirements (TL;DR)

## Program Basics
- Name: `ircserv`
- Run command: `./ircserv <port> <password>`
- Language: C++98 (compile with `-std=c++98 -Wall -Wextra -Werror`)
- Makefile rules: `NAME`, `all`, `clean`, `fclean`, `re`
- No external libraries / Boost allowed

## Core Rules
- Handle multiple clients simultaneously (no hanging)
- **No forking** - all I/O must be non-blocking
- Use **1 poll()** (or select/epoll/kqueue) for all operations
  - Reading/writing without poll = grade 0
  - Non-blocking file descriptors required
- Communication via TCP/IP (v4 or v6)
- Must work with your chosen reference IRC client

## Mandatory Features
1. User authentication (password, NICK, USER)
2. Join channels, send/receive channel messages (forward to all joined clients)
3. Private messages between users
4. Channel operator system with these commands:
   - `KICK`: Eject client from channel
   - `INVITE`: Invite client to channel
   - `TOPIC`: Change/view channel topic
   - `MODE`: Channel mode changes:
     - `i`: Invite-only channel
     - `t`: Restrict TOPIC to operators only
     - `k`: Set/remove channel key (password)
     - `o`: Grant/remove operator privilege
     - `l`: Set/remove user limit

## Bonus (Only if mandatory is perfect)
- File transfer
- Bot
