Architecture (per-client buffer) | Good |
| `queue` + `flush` split | Good |
| Poll integration | Needs work |
| `poll_fds` lookup | Bug |
| Non-blocking setup | Missing |
| End-to-end POLLOUT retry | Not wired yet

Install a signal handler** for SIGINT/SIGTERM (even a no-op handler would work, since it would prevent the default termination)
2. **Change the EINTR path** in `server_loop()` (`src/server.cpp:83`) from `continue` to something that exits the loop

The `poll()` call is already set to return -1 with `EINTR` when a signal arrives. Currently you just `continue` on EINTR. If instead you break out of the loop (or clear `SERVER_RUNNING`), the server will fall through to `shutdown_server()` in `main()`.
