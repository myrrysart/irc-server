# Evaluation Tips TL;DR

## What Evaluators Test (From 100+ Peer Reviews)
1. **All mandatory commands**: NICK, USER, JOIN, PRIVMSG, MODE, KICK, INVITE, TOPIC
2. **Edge cases**: Wrong password, invalid nick, full channel, partial TCP data
3. **Memory leaks**: Run with `valgrind` during eval
4. **Code explanation**: They will ask how specific parts work (poll, command handling)
5. **Reference client**: Must connect and work without errors
6. **Non-blocking I/O**: Verify no blocking calls, single poll used

## High-Scoring Team Patterns (115/115)
- Strong understanding of *why* they implemented things a certain way
- Clean, no leftover unused code
- All MODE types work correctly
- Can explain poll()/select() choice and I/O handling
- Tested with multiple reference clients

## Common Evaluator Questions
- "How does your server handle partial TCP packets?"
- "Why did you choose poll/select/epoll?"
- "How do you track channel operators?"
- "What happens when a client disconnects unexpectedly?"
- "Show me how MODE k (channel key) works"

## Prep Checklist
- [ ] Test all commands with reference client
- [ ] Run `valgrind --leak-check=full ./ircserv 6667 password`
- [ ] Practice explaining your code out loud
- [ ] Test with partial data (nc -C 127.0.0.1 6667 + ctrl+D)
