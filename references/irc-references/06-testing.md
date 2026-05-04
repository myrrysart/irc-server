# Testing TL;DR

## Quick TCP Test (Partial Data)
Test if server handles split TCP packets (from project requirements):
```bash
nc -C 127.0.0.1 6667
# Type: NICK bob (but send in parts with ctrl+D after each part)
NIC  # ctrl+D
K bo  # ctrl+D
b    # ctrl+D (server should process full NICK bob command)
```

## Reference Client Test
1. Pick a client (HexChat, irssi, WeeChat)
2. Connect: `/connect 127.0.0.1 6667 password`
3. Test all mandatory commands:
   - `/nick testuser`
   - `/join #test`
   - `/msg #test hello`
   - `/mode #test +i` (invite-only)
   - `/mode #test +k secret` (set key)
   - `/invite anotheruser #test`
   - `/kick #test baduser`

## Valgrind Leak Test
```bash
valgrind --leak-check=full --show-leak-kinds=all ./ircserv 6667 password
# Connect client, run commands, disconnect. Check for "no leaks are possible"
```

## Multi-Client Test
1. Open 2 terminal windows with nc/reference clients
2. Both join same channel
3. Send message from client 1, verify client 2 receives it
4. Test operator commands (KICK, MODE) from operator client

## Edge Case Tests
- [ ] Wrong password on connect
- [ ] Duplicate nickname
- [ ] Join full channel (after setting MODE +l)
- [ ] Send message to non-existent channel
- [ ] Client disconnects without QUIT
