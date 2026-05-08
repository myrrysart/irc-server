# IRC Server Notes

##  How an IRC server works

An IRC server accepts TCP clients, parses line-based commands, updates client/channel state, and queues replies or routed messages.

High-level flow:

1. Start listening on a TCP port.
2. Use an event loop to watch all sockets.
3. Accept clients and store their state.
4. Read bytes into per-client buffers.
5. Extract complete `\r\n` IRC lines.
6. Parse lines into commands.
7. Update clients/channels.
8. Queue replies and routed messages.
9. Send queued data without blocking.
10. Clean up disconnected clients.

---

##  Building blocks

### 1. Server startup

**Fundamentals**

* IRC runs over a TCP listening socket bound to an IP/port.
* The listening socket accepts incoming client connections.

**Architecture**

* `Server` owns the listening socket, poll list, clients, channels, and config: port, password, hostname, and server name.

**Libraries / functions**

* `socket()`
* `setsockopt()`
* `bind()`
* `listen()`
* `close()`
* `sockaddr_in`
* `htons()`
* `std::string`
* RAII wrapper for socket cleanup

---

### 2. Event loop and non-blocking I/O

**Fundamentals**

* One slow client must not freeze the server.
* Use I/O multiplexing with non-blocking sockets to detect reads, writes, disconnects, and errors.

**Architecture**

* `Server` keeps `std::vector<pollfd>` for all watched sockets.
* The listening socket handles new connections.
* Client sockets handle incoming and outgoing IRC data.

**Libraries / functions**

* `poll()`
* `pollfd`
* `POLLIN`
* `POLLOUT`
* `POLLHUP`
* `POLLERR`
* `fcntl()`
* `O_NONBLOCK`
* `std::vector<pollfd>`

---

### 3. Client connections

**Fundamentals**

* Every connection starts as an unregistered client.
* A client becomes registered after valid `NICK` and `USER`.

**Architecture**

* `Client` stores fd, nickname, username, registration state, joined channels, input buffer, and output buffer.
* `Server` maps socket FDs to `Client` objects.
* A nickname lookup map helps route direct messages.

**Libraries / functions**

* `accept()`
* `sockaddr_storage`
* `inet_ntop()`
* `std::unordered_map<int, Client>`
* `std::unordered_map<std::string, int>`
* `std::string`
* `std::optional` or explicit `hasNick` / `hasUser` flags
* `enum class` for registration state

---

### 4. Receiving IRC data

**Fundamentals**

* IRC messages are line-based and end with `\r\n`.
* TCP is a byte stream, so one `recv()` may contain partial, full, or multiple messages.
* Incomplete messages stay in the client input buffer.

**Architecture**

* `Client` owns an input buffer; reads append bytes, and extraction pulls complete IRC lines from it.

**Libraries / functions**

* `recv()`
* `errno`
* `EAGAIN`
* `EWOULDBLOCK`
* `std::string`
* `std::string::find()`
* `std::string::substr()`
* `std::string::erase()`

---

### 5. Parsing and validation

**Fundamentals**

* Example command: `PRIVMSG #general :Hello everyone!\r\n`.
* General format: `[:prefix] COMMAND param1 param2 :trailing param`.
* Spaces matter, and the trailing parameter may contain spaces.
* Bad input should produce numeric error replies.

**Architecture**

* `Command` represents one parsed IRC line.
* Parsing should produce commands without modifying server state.

**Libraries / functions**

* `Command` struct
* `std::string`
* `std::vector<std::string>`
* `std::optional<std::string>`
* `std::transform()` for command normalization
* `std::stringstream` if useful

---

### 6. Command handling and IRC state

**Fundamentals**

* `NICK` and `USER` register a client.
* `JOIN` and `PART` modify channel membership.
* `PRIVMSG` creates direct or channel messages for routing.
* `PING` receives `PONG`.
* `QUIT` disconnects cleanly.
* Commands before registration may need error replies.

**Architecture**

* `Server` dispatches commands to handler functions.
* `Client` stores user identity and registration state.
* `Channel` stores name, members, operators, and modes.
* Reply builders format standard IRC responses like `001` and `433` with the correct prefix, target nick, message text, and `\r\n`.

**Libraries / functions**

* `std::unordered_map<std::string, Channel>`
* `std::set` / `std::unordered_set` for channel members
* handler functions like `handleNick()`, `handleUser()`, `handleJoin()`
* reply helpers for `001`, `433`, errors, joins, parts, and messages

---

### 7. Message routing

**Fundamentals**

* Direct messages target one nickname; channel messages target members except the sender.
* Server messages include joins, parts, errors, and numeric replies.
* Queue messages instead of sending directly from command handlers.

**Architecture**

* Nickname and channel maps find recipients.
* Routing appends formatted IRC messages to each target client's output buffer.

**Libraries / functions**

* `std::unordered_map<std::string, int>`
* `std::unordered_map<std::string, Channel>`
* `std::set` / `std::unordered_set`
* output buffer helpers like `queueMessage()`

---

### 8. Sending data and backpressure

**Fundamentals**

* `send()` may write only part of a message, so remaining bytes stay queued.
* Watch for `POLLOUT` only when a client has pending data.

**Architecture**

* The event loop flushes each client's output buffer when the socket is writable.
* Partial writes remove only the bytes successfully sent.

**Libraries / functions**

* `send()`
* `EAGAIN`
* `EWOULDBLOCK`
* `POLLOUT`
* `std::string::erase()`
* `std::deque<std::string>` for message queue

---

### 9. Disconnects and edge cases

**Fundamentals**

* `recv()` returning `0` means the client closed the connection.
* Cleanup must remove the client from all server state.
* Common edge cases: nickname collisions, split messages, duplicate joins, invalid commands, and commands before registration.

**Architecture**

* `Server` should use one cleanup path for normal quits, socket errors, and closed connections.
* Cleanup removes the client from channels, nickname maps, client maps, and the poll list, then notifies affected members.

**Libraries / functions**

* `close()`
* `POLLHUP`
* `POLLERR`
* `std::unordered_map::erase()`
* `std::vector::erase()`
* `std::remove_if()`

---
