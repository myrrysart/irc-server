# Common Pitfalls TL;DR

## Bug #1: Not Handling Partial TCP Data
- **Problem**: TCP splits messages into packets. `recv()` may return half a command.
- **Result**: Commands like `NICK bob` sent as `NIC` + `K bob` fail.
- **Fix**: Buffer incoming data per client, only process when `\r\n` received.

## Bug #2: Client Disconnect Not Detected
- **Problem**: Client closes without sending QUIT. Server keeps stale connections.
- **Result**: Evaluators note "disconnect issues", memory leaks.
- **Fix**: Check `recv()` return value: 0 = client disconnected, -1 = error.

## Bug #3: Memory Leaks
- **Problem**: Not freeing client/channel structs on disconnect.
- **Result**: Valgrind errors, lower grade.
- **Fix**: Track all allocations, free in destructor/disconnect handler.

## Bug #4: Multiple poll() Calls
- **Problem**: Using poll() in multiple places (command handler + main loop).
- **Result**: Grade 0 (violates single poll rule).
- **Fix**: Only 1 poll() call in main event loop.

## Bug #5: Blocking I/O
- **Problem**: Using blocking sockets, not setting O_NONBLOCK.
- **Result**: Server hangs when client is slow.
- **Fix**: Set `fcntl(fd, F_SETFL, O_NONBLOCK)` for all sockets (MacOS too).

## Bug #6: Incomplete MODE Implementation
- **Problem**: Only implementing `o` mode, skipping `i`/`t`/`k`/`l`.
- **Result**: Evaluators fail mandatory requirements.
- **Fix**: Implement all 5 mandatory MODE types.

## Bug #7: Leftover Unused Code
- **Problem**: Dead code from testing/debugging left in repo.
- **Result**: Evaluators comment on "unused code", lower score.
- **Fix**: Clean up before submission, only keep necessary code.
