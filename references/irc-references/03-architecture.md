# IRC Architecture TL;DR

## Core Model (Think: Post Office)
- **Server**: Central hub that routes messages (like a post office)
- **Client**: Users connecting to server (like people sending mail)
- Clients never talk directly to each other - all messages go through server

## Channels (Chat Rooms)
- Named starting with `#` (e.g. `#general`)
- Users join channels to participate in group chat
- Server forwards all channel messages to every joined client
- Channels can have passwords, user limits, operator-only settings

## Channel Operators (Moderators)
- Have special privileges (kick users, change topic, set modes)
- Mode `o` grants/takes operator status
- Regular users can become operators if granted

## Communication Types
1. **One-to-one**: Private messages between two clients
2. **One-to-many**: Messages to a channel (all joined clients receive)
3. **Client-to-server**: Commands like NICK, JOIN sent to server
4. **Server-to-client**: Replies, messages, notifications from server

## Key Notes
- No server-to-server communication required (we only build 1 server)
- TCP ensures reliable message delivery (no lost packets)
- Messages are text-only, sent as ASCII strings
