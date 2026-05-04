# IRC Server Scrumban Issues

## Must have

### Core Infrastructure
- [ ] Set up project structure (server, client, channel, parser, commands modules)
- [ ] Create TCP listening socket with proper error handling
- [ ] Configure socket for non-blocking I/O using fcntl O_NONBLOCK
- [ ] Implement single poll() event loop for all I/O operations
- [ ] Handle new client connections (accept) in event loop
- [ ] Implement per-client input buffering for partial TCP data
- [ ] Handle client disconnection detection (recv returns 0)
- [ ] Implement proper memory cleanup on client disconnect

### IRC Message Parsing
- [ ] Parse IRC message format: `[:prefix] COMMAND [params] [:trailing]`
- [ ] Extract command name and parameters from raw message
- [ ] Handle CRLF (`\r\n`) message termination
- [ ] Support multi-line message processing from buffer

### Client Registration
- [ ] Implement PASS command (server password authentication)
- [ ] Implement NICK command (set/change nickname)
- [ ] Implement USER command (set username and realname)
- [ ] Validate registration state before allowing other commands
- [ ] Send ERR_NOTREGISTERED when unregistered client sends commands
- [ ] Send RPL_WELCOME (001) on successful registration

### Nickname Management
- [ ] Validate nickname format (IRC naming rules)
- [ ] Check nickname uniqueness across server
- [ ] Handle nick collision (ERR_NICKNAMEINUSE 433)
- [ ] Allow nickname changes after registration
- [ ] Notify channels on nickname change

### Channel Management
- [ ] Implement JOIN command (join/create channel)
- [ ] Implement PART command (leave channel with optional reason)
- [ ] Track channel membership (clients per channel)
- [ ] Track client's joined channels
- [ ] Implement LIST command (list all visible channels)
- [ ] Implement NAMES command (list users in channel)
- [ ] Handle channel name validation (# prefix, valid characters)
- [ ] Send channel join/part notifications to members

### Messaging
- [ ] Implement PRIVMSG command (private messages)
- [ ] Route PRIVMSG to users (direct messages)
- [ ] Route PRIVMSG to channels (broadcast to members)
- [ ] Handle messages from unregistered clients
- [ ] Send ERR_NOSUCHNICK when target user not found
- [ ] Send ERR_CANNOTSENDTOCHAN when cannot send to channel
- [ ] Implement NOTICE command (similar to PRIVMSG, no auto-reply)

### Channel Modes (Mandatory 5)
- [ ] Implement MODE command parser for channel modes
- [ ] Mode `i` - Invite-only channel (boolean)
- [ ] Mode `t` - Restrict TOPIC to operators only (boolean)
- [ ] Mode `k` - Set/remove channel key (password) with parameter
- [ ] Mode `o` - Grant/remove operator privilege with parameter
- [ ] Mode `l` - Set/remove user limit with parameter
- [ ] Send ERR_CHANOPRIVSNEEDED when non-op tries operator actions
- [ ] Send ERR_UNKNOWNMODE for invalid mode characters

### Channel Operator System
- [ ] Track channel operators list per channel
- [ ] Grant operator status on channel creation (creator becomes op)
- [ ] Implement KICK command (eject client from channel)
- [ ] Implement INVITE command (invite client to channel)
- [ ] Implement TOPIC command (view/change channel topic)
- [ ] Restrict TOPIC changes per mode `t` setting
- [ ] Send RPL_TOPIC/RPL_NOTOPIC responses

### Connection Management
- [ ] Implement PING/PONG commands for keep-alive
- [ ] Implement QUIT command with optional quit message
- [ ] Notify channels when client quits
- [ ] Handle unexpected client disconnection
- [ ] Clean up client from all channels on disconnect

### Numeric Replies
- [ ] Define standard numeric reply codes (001-999 range)
- [ ] Implement RPL_WELCOME (001)
- [ ] Implement ERR_NOSUCHNICK (401)
- [ ] Implement ERR_NOSUCHCHANNEL (403)
- [ ] Implement ERR_CANNOTSENDTOCHAN (404)
- [ ] Implement ERR_NICKNAMEINUSE (433)
- [ ] Implement ERR_NOTREGISTERED (451)
- [ ] Implement ERR_NEEDMOREPARAMS (461)
- [ ] Implement ERR_ALREADYREGISTERED (462)
- [ ] Implement ERR_PASSWDMISMATCH (464)
- [ ] Implement ERR_UNKNOWNMODE (472)
- [ ] Implement ERR_INVITEONLYCHAN (473)
- [ ] Implement ERR_BADCHANNELKEY (475)
- [ ] Implement ERR_CHANOPRIVSNEEDED (482)
- [ ] Implement RPL_LIST (322) and RPL_LISTEND (323)
- [ ] Implement RPL_NAMREPLY (353) and RPL_ENDOFNAMES (366)
- [ ] Implement RPL_TOPIC (332) and RPL_NOTOPIC (331)

### Testing & Validation
- [ ] Test with actual IRC client (irssi, HexChat, WeeChat)
- [ ] Verify non-blocking I/O behavior
- [ ] Test partial message handling (split TCP packets)
- [ ] Test multiple simultaneous clients
- [ ] Verify single poll() usage (no other polling calls)
- [ ] Test all mandatory commands work correctly
- [ ] Test error responses match IRC spec

---

## Bonus

### File Transfer (DCC)
- [ ] Research DCC (Direct Client-to-Client) protocol
- [ ] Implement DCC SEND (file transfer initiation)
- [ ] Implement DCC RESUME (resume interrupted transfers)
- [ ] Implement DCC ACCEPT (accept incoming transfer)
- [ ] Handle DCC connection establishment between clients
- [ ] Test file transfer with IRC client

### IRC Bot
- [ ] Design bot architecture (automated client)
- [ ] Implement bot connection and registration
- [ ] Add Magic 8-Ball command (!8ball)
- [ ] Make bot join channels and respond to triggers
- [ ] Add additional bot features (trivia, quotes, etc.)
- [ ] Test bot interaction with human users

---

## Nice to have

### Additional Commands
- [ ] Implement WHOIS command (query user information)
- [ ] Implement WHOWAS command (query past user information)
- [ ] Implement AWAY command (set away status with message)
- [ ] Implement MOTD command (Message of the Day)
- [ ] Implement KILL command (operator disconnects user)
- [ ] Implement WALLOPS command (send to all operators)
- [ ] Implement TIME command (server local time)
- [ ] Implement VERSION command (server version info)
- [ ] Implement ADMIN command (admin info)
- [ ] Implement TRACE command (trace server path)
- [ ] Implement LINKS command (list servers)
- [ ] Implement STATS command (server statistics)

### Additional Channel Modes
- [ ] Mode `b` - Ban mask (ban specific users/hosts)
- [ ] Mode `e` - Ban exception (exception to ban list)
- [ ] Mode `I` - Invite exception (always allowed to join)
- [ ] Mode `m` - Moderated channel (only ops/voices can speak)
- [ ] Mode `n` - No external messages (no outside PRIVMSG)
- [ ] Mode `s` - Secret channel (hidden from LIST)
- [ ] Mode `p` - Private channel (like secret but different)
- [ ] Implement ban list management (RPL_BANLIST 367)

### Channel Features
- [ ] Support multiple channels per client
- [ ] Implement channel ban lists with persistence
- [ ] Implement invite lists for channels
- [ ] Support channel keys in JOIN command
- [ ] Implement user limit enforcement in JOIN
- [ ] Track channel creation time
- [ ] Implement channel URL setting

### User Modes
- [ ] Implement user mode `i` - Invisible (not in WHO unless in same channel)
- [ ] Implement user mode `o` - Operator (IRC operator, not channel op)
- [ ] Implement user mode `w` - Wallops (receive WALLOPS messages)
- [ ] Implement UMODE command (user mode changes)

### Enhanced Messaging
- [ ] Support CTCP (Client-to-Client Protocol) messages
- [ ] Implement CTCP VERSION, TIME, PING
- [ ] Support ACTION (IRC /me command)
- [ ] Add color/formatting code handling
- [ ] Implement message throttling/anti-flood

### Server Features
- [ ] Implement logging system (connections, messages, errors)
- [ ] Support CAP command (capability negotiation)
- [ ] Implement SASL authentication framework
- [ ] Add connection registration timeout
- [ ] Implement idle time tracking per client
- [ ] Support server passwords per-operator

### Multi-Server (Advanced)
- [ ] Design server-to-server protocol structure
- [ ] Implement basic server linking
- [ ] Handle user propagation across servers
- [ ] Implement netjoin/netpart notifications

---

## Won't have

- [ ] GUI interface for server administration
- [ ] Web-based chat client
- [ ] Database persistence for user data/channels
- [ ] SSL/TLS encrypted connections
- [ ] IPv6 support (project requires v4 or v6, choosing v4)
- [ ] REST API for server management
- [ ] JSON configuration files (using command-line args only)
- [ ] Docker containerization
- [ ] Metrics/monitoring dashboard
- [ ] Chat logging to files per channel
- [ ] Bouncer functionality (keeping user online when disconnected)
- [ ] Services (NickServ, ChanServ, etc.)
- [ ] Channel relay between different IRC networks
- [ ] File upload to server (only DCC client-to-client)
- [ ] Voice/video chat features
- [ ] Mobile app companion
