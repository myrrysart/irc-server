# IRC Commands TL;DR

## Message Format
```
[@tags] [:source] COMMAND [params] [:trailing param]
```
- Tags: Optional key=value pairs prefixed with `@`
- Source: Optional sender info (nick!user@host)
- Command: 3-char uppercase or numeric (e.g. `NICK`, `PRIVMSG`, `001`)
- Params: Space-separated, trailing param starts with `:`

## Mandatory Commands (From Project Requirements)
| Command | Syntax | Purpose |
|---------|--------|---------|
| `PASS` | `PASS <password>` | Authenticate connection |
| `NICK` | `NICK <nickname>` | Set/change nickname |
| `USER` | `USER <username> <hostname> <servername> :<realname>` | Set user info |
| `JOIN` | `JOIN <channel> [<key>]` | Join channel (create if not exists) |
| `PART` | `PART <channel> [:message]` | Leave channel |
| `PRIVMSG` | `PRIVMSG <target> :<message>` | Send message to user/channel |
| `MODE` | `MODE <target> <mode changes> [params]` | Change user/channel modes |
| `KICK` | `KICK <channel> <user> [:reason]` | Eject user from channel |
| `INVITE` | `INVITE <nickname> <channel>` | Invite user to channel |
| `TOPIC` | `TOPIC <channel> [:new topic]` | View/change channel topic |
| `PING` | `PING :<server>` | Server checks client connection |
| `PONG` | `PONG :<server>` | Client responds to PING |
| `QUIT` | `QUIT [:message]` | Disconnect from server |
| `NAMES` | `NAMES [channel]` | List users in channel |
| `LIST` | `LIST [channels]` | List all/open channels |

## Channel MODE Types (Mandatory)
| Mode | Type | Purpose |
|------|------|---------|
| `i` | Boolean | Invite-only channel |
| `t` | Boolean | Restrict TOPIC to operators |
| `k` | Param | Channel key (password) |
| `o` | Param | Grant/remove operator privilege |
| `l` | Param | Set user limit |
