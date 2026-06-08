
### `src/server.cpp`
- `setup_client()`: add `fcntl(client_fd, F_SETFL, O_NONBLOCK)` after `setsockopt`
- `handle_poll_event()`: add `if (rev & POLLOUT)` branch → `flush_client(server, fd)` **before** POLLIN/PollErr handling (drain queue first on error)
- `queue_message(server, client, msg)`: new function — appends msg to `client.send_buffer`, sets `POLLOUT` on the client's `poll_fds` entry

### `src/client.cpp`
- `flush_client(server, fd)`:
  - Get `client = server.clients[fd]`
  - while `send_buffer` not empty:
    - `sent = send(fd, send_buffer.data(), send_buffer.size(), 0)`
    - if `sent > 0`: `send_buffer.erase(0, sent)`
    - if `sent == -1 && (EAGAIN || EWOULDBLOCK)`: break
    - if `sent == -1 && other error`: disconnect
  - if `send_buffer` empty: clear `POLLOUT` from that fd's `poll_fds[i].events`
- `handle_client_message()`: replace `send(client.fd, ...)` with `queue_message(server, client, response)`

### `lib/server.hpp`
- Add declarations for `queue_message()` and `flush_client()`

---

Key differences from your original:
- **No deque at all** — single `std::string` per client, `erase(0, sent)` handles partial sends cleanly
- **`queue_message()` is a new function**, not part of `flush_client()` — one pushes, one drains
- **POLLOUT is set when queueing**, cleared when drained — the pollfd's `events` field becomes the authoritative "is there data to send" signal
