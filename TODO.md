# IRC Server — Refactoring TODO

## Phase 1 — Update Fatstruct

- [ ] Add headers to `lib/irc_fatstruct.hpp`:
  ```cpp
  #include <vector>
  #include <poll.h>
  ```
- [ ] Replace `t_IRC_Client clients[MAX_CLIENTS]` + `int client_count` with:
  ```cpp
  #include <unordered_map>
  std::unordered_map<int, t_IRC_Client>  clients;
  ```
- [ ] Add pollfd vector:
  ```cpp
  std::vector<pollfd>  poll_fds;
  ```

## Phase 2 — Fix Makefile

- [ ] Add `src/ultra_simple_server.cpp` to `SRC`
- [ ] Bump `-std=c++98` → `-std=c++11` (required for `unordered_map`)

## Phase 3 — Rewrite main.cpp

- [ ] Bootstrap flow:
  1. Parse args (`port`, `password`)
  2. Call `setup_socket(port)` → returns `listen_fd`
  3. Init `t_IRC_Server` struct:
     - `listen_fd`, `port`, `password`
     - Push `{listen_fd, POLLIN, 0}` onto `poll_fds`
  4. Call `server_loop(server)`

## Phase 4 — Refactor ultra_simple_server.cpp

- [ ] **`server_loop(t_IRC_Server& server)`**
  - Owns `while(1) { poll(); poll_loop(); }`
  - No local buffers or vectors

- [ ] **`poll_loop(t_IRC_Server& server)`**
  - Iterates `server.poll_fds`
  - Dispatches:
    | Event | Action |
    |---|---|
    | `listen_fd` revents & POLLIN | `accept_new_client(server)` |
    | client fd revents & POLLIN | `handle_client_input(server, fd)` |
    | client fd revents & POLLHUP/POLLERR | `disconnect_client(server, fd)` |
    | client fd revents & POLLOUT | `flush_client_output(server, fd)` |

- [ ] **`accept_new_client(t_IRC_Server& server)`**
  - `accept()` → get `client_fd`
  - Insert into `server.clients[client_fd]`
  - Push `{client_fd, POLLIN, 0}` onto `server.poll_fds`

- [ ] **`handle_client_input(t_IRC_Server& server, int fd)`**
  - `recv()` into `server.clients[fd].received_message_buffer`
  - Extract complete `\r\n` lines
  - Echo or dispatch commands (WIP)

- [ ] **`disconnect_client(t_IRC_Server& server, int fd)`**
  - `close(fd)`
  - `server.clients.erase(fd)`
  - Find fd in `server.poll_fds` and erase it

- [ ] **Remove `echo_buf`** — no longer needed, per-client buffer lives in fatstruct

- [ ] **Fix broken `poll_loop()` call on line 136** (missing semicolon, wrong signature)

## Phase 5 — Cleanup (optional)

- [ ] Rename `ultra_simple_server.cpp` (e.g. `server.cpp`)
- [ ] Remove unused `#include <array>` from the old echo code
- [ ] Verify compile: `make re`

---

### Notes

- The `clients` map (`unordered_map<int, t_IRC_Client>`) gives O(1) fd→client lookup and eliminates manual slot management
- `poll_fds` stays as a `vector` because `poll()` consumes a flat array — don't overthink it
- Channel array stays fixed (`MAX_CHANNELS`) — channel count doesn't churn the way clients do
- **Watch out:** don't stash `t_IRC_Client*` pointers in channel membership lists — the map invalidates them on rehash. Use fd as the stable identifier instead
