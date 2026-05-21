
 * TODO: *
 - state machine for error handling etc.
 1) status flag for main loop, when off-> out of the main loop
 2) errno and message to a struct. Printed at error handler after the main loop
 3) close fds etc at the end
- make the first parsing step with correct termination.
- try catch blocks where error handling is now, at atoi
- server.client.at(fd) throws
-  
 '''
 create_listener(server)
 server_loop(server)        ← only enters if state == RUNNING
                            
 if (server.state & FATAL)  ← post-loop error handler
     print_error(server)
 cleanup(server)             ← closes all fds, frees resources
 return server.state & FATAL ? 1 : 0
'''
Good call — consistent handling for all paths is much cleaner. Here's the concrete plan:

## Plan

### New/Changed State Constants (`irc_fatstruct.hpp`)
```
IN_SETUP  0      (no change)
RUNNING   BIT(0) (currently just `1` — functionally identical)
FATAL     BIT(1) (new)
```

### New Error Struct (`irc_fatstruct.hpp`)
```cpp
typedef struct s_ServerError {
    std::string msg;    // context (e.g. "socket", "bind", "poll")
    int         errnum; // saved errno
} t_ServerError;
```

Add `t_ServerError error;` to `t_IRC_Server`.

### Changed Functions

| Function | Now → After |
|---|---|
| `fatal_server_error(msg, fd)` | → `fatal_server_error(server, msg)` — saves errno, sets `state = FATAL`, returns |
| `create_listener()` | calls new `fatal_server_error` instead of `exit(1)`; listen_fd stays open for cleanup |
| `server_loop()` | guards entry with `if (state != RUNNING) return`; sets error on poll failure instead of `exit(1)` |
| `handle_poll_event()` | calls new `fatal_server_error` on listen socket error instead of `exit(1)` |
| `recv_from_client()` | preserves errno in error struct (optional — or just let it signal disconnect) |
| `cleanup()` (new) | closes listen_fd + all client fds, clears poll_fds/clients |

### What No Longer Happens
- `exit(1)` — nowhere in the code
- `perror()` — replaced by printing the stored errno+msg in the error handler
- `close()` inside `fatal_server_error` — one single cleanup path instead

### Files Touched
`irc_fatstruct.hpp` (+struct +constant), `error.cpp` (rewrite), `server.hpp` (signatures), `main.cpp` (reorder), `listener.cpp` (replace calls), `server.cpp` (replace calls), `client.cpp` (optional errno fix)

---

## Two Open Questions

1. **Cleanup ownership**: `t_IRC_Channel::members` is a raw `t_IRC_ChannelMembership*` that's never allocated yet. Should `cleanup()` free it with `free()`/`delete[]` (since channels could be allocated by then), or leave it as a TODO for when channels are actually implemented?

2. **`recv_from_client`**: Currently silently discards `errno` from `recv()` and just returns `true/false`. Want me to thread it into the error struct too, or leave it as a simple disconnect signal?
