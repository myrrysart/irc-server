
# IRC Server ↔ Irssi Compatibility Checklist

This checklist focuses on protocol details that a raw client (`nc`) may tolerate but that **Irssi expects to be RFC-correct**.

---

# 1. Numeric Replies Must Have a Server Prefix

## Correct

```text
:irc.local 403 bob #ghost :No such channel
```

## Incorrect

```text
403 bob #ghost :No such channel
```

or

```text
:bob 403 bob #ghost :No such channel
```

### Verify

* [ ] Every numeric reply (`001`, `324`, `403`, etc.) begins with a **server name**.
* [ ] Never use a user's nickname as the prefix for a numeric.

---

# 2. Numerics Must Target the Client Nick First

Most numerics begin with the receiving nickname.

Example:

```text
:irc.local 403 bob #ghost :No such channel
```

where

```
403
bob        <- target client
#ghost
:message
```

### Verify

* [ ] Every numeric places the destination nickname immediately after the numeric.
* [ ] Parameter ordering matches the RFC.

---

# 3. Registration Burst

Expected order:

```text
001
002
003
004
005
375
372
376
```

### Verify

* [ ] Registration numerics are sent in this order.
* [ ] No replies are omitted.
* [ ] They are only sent after successful registration.

---

# 4. JOIN Replies

Joining a channel should produce

```text
JOIN
353
366
```

### Verify

* [ ] JOIN echo is sent.
* [ ] `353` (NAMES) is sent.
* [ ] `366` (End of NAMES) is sent.

---

# 5. RPL_NAMREPLY (353)

Correct format:

```text
:server 353 nick = #channel :@alice bob
```

### Verify

* [ ] First parameter is the destination nick.
* [ ] Channel type (`=`) exists.
* [ ] Channel name is correct.
* [ ] Nick list is the trailing parameter.

---

# 6. MODE Query (324)

Correct:

```text
:server 324 nick #channel +nt
```

Optional but common:

```text
:server 329 nick #channel <creation time>
```

### Verify

* [ ] `324` parameter ordering is correct.
* [ ] Channel appears before mode string.
* [ ] Optional `329` if implemented.

---

# 7. User-Originated Commands Must NOT Use the Server Prefix

These commands should originate from the user:

* JOIN
* PART
* QUIT
* NICK
* MODE
* TOPIC
* INVITE
* KICK

Example:

```text
:alice!user@host JOIN #chat
```

NOT

```text
:server JOIN #chat
```

### Verify

* [ ] JOIN prefix is `nick!user@host`
* [ ] PART prefix is `nick!user@host`
* [ ] MODE prefix is `nick!user@host`
* [ ] TOPIC prefix is `nick!user@host`
* [ ] KICK prefix is `nick!user@host`
* [ ] INVITE prefix is `nick!user@host`
* [ ] NICK prefix is `nick!user@host`
* [ ] QUIT prefix is `nick!user@host`

---

# 8. TOPIC Replies

Query:

```text
331
```

or

```text
332
```

Topic change broadcast:

```text
:nick!user@host TOPIC #chan :new topic
```

### Verify

* [ ] Query numerics come from the server.
* [ ] Topic change comes from the user.

---

# 9. KICK

Correct:

```text
:nick!user@host KICK #chan victim :reason
```

### Verify

* [ ] Prefix is the kicking user.
* [ ] Victim parameter ordering is correct.
* [ ] Optional reason is trailing.

---

# 10. INVITE

Numeric:

```text
341
```

Invite message:

```text
:nick!user@host INVITE bob :#private
```

### Verify

* [ ] INVITE command comes from the inviter.
* [ ] `341` comes from the server.

---

# 11. PART

Correct:

```text
:nick!user@host PART #channel
```

or

```text
:nick!user@host PART #channel :reason
```

### Verify

* [ ] Prefix is the leaving user.

---

# 12. QUIT

Correct:

```text
:nick!user@host QUIT :Leaving
```

### Verify

* [ ] Prefix is the quitting user.
* [ ] Reason is a trailing parameter.

---

# 13. NICK Changes

Correct:

```text
:oldnick!user@host NICK :newnick
```

### Verify

* [ ] Prefix uses the old nickname.
* [ ] New nickname is a trailing parameter (`:newnick`).

---

# 14. CRLF

Every IRC message must end with

```text
\r\n
```

### Verify

* [ ] Never send only `\n`.

---

# 15. Trailing Parameters

Human-readable text should be a trailing parameter.

Correct:

```text
:server 403 bob #ghost :No such channel
```

Incorrect:

```text
:server 403 bob #ghost No such channel
```

### Verify

* [ ] Every free-form message begins with `:`.

---

# 16. Extra Spaces

Avoid producing

```text
403  bob
```

or

```text
403 bob  #chan
```

### Verify

* [ ] Exactly one space separates parameters.

---

# 17. Server Name Consistency

Use a single server name everywhere.

Examples:

```text
irc.local
localhost
ircserv
```

### Verify

* [ ] All numerics use the same server name.
* [ ] Do not alternate between `localhost`, `127.0.0.1`, etc.

---

# 18. Test Script Weaknesses

The current test script **does not verify**:

* [ ] Prefix correctness.
* [ ] Parameter ordering.
* [ ] Destination nick position.
* [ ] User vs. server prefixes.
* [ ] Exact RFC formatting.
* [ ] CRLF correctness.
* [ ] Presence of required trailing parameters.

Because of this, a server can pass the script while still sending messages that Irssi ignores or parses incorrectly.

---

# Highest-Priority Areas to Audit

1. Numeric formatting (`001`–`005`, `324`, `331`, `332`, `341`, `353`, `366`, `401`, `403`, `442`, `461`, `482`, etc.).
2. Prefix generation (server vs. `nick!user@host`).
3. Parameter ordering.
4. Trailing parameters (`:message`).
5. `353` (NAMES) formatting.
6. `324` (MODE) formatting.
7. `NICK`, `QUIT`, `TOPIC`, `MODE`, `JOIN`, `PART`, `KICK`, and `INVITE` prefixes.
