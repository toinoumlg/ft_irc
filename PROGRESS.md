# ft_irc

## 0. Status
- [x] Done
- [ ] To do
- [ ] **In progress:**

## 1. Network / Server
- [x] `socket`
- [x] `bind`
- [x] `listen`
- [x] `poll`
- [x] `accept`
- [x] Multiple clients
- [x] `recv`
- [x] Basic buffer handling

## 2. IRC Connection
### PASS
- [x] Parse `PASS`
- [x] Validate number of parameters
- [x] Compare password with server password
- [x] Reject incorrect password
- [x] Reject `PASS` if client is already registered
- [ ] Send correct IRC errors

### NICK
- [x] Parse `NICK`
- [x] Store nickname
- [x] Reject missing nickname
- [x] Reject invalid nickname
- [x] Detect nickname already in use
- [x] Allow nickname changes after registration
- [ ] Notify relevant users when nickname changes

### USER
- [x] Parse `USER`
- [x] Validate parameters
- [x] Store username
- [x] Store real name
- [x] Reject `USER` if client is already registered

### Registration State
- [ ] Track password validation
- [ ] Track nickname registration
- [ ] Track user registration
- [ ] Detect when registration becomes complete
- [ ] **In progress: (toinou)** `001 RPL_WELCOME`
- [ ] Ensure `001` is sent only after successful registration
- [ ] Ensure welcome replies are not sent twice

## 3. IRC Replies / Errors
- [ ] **In progress:** IRC reply handling
- [ ] Implement reusable numeric reply system
- [ ] Implement command-specific errors
- [ ] Check reply format
- [ ] Check server prefix
- [ ] Check nickname used in replies

## 4. Messaging
### Client → Client
- [x] Parse `PRIVMSG`
- [x] Find destination client by nickname
- [x] Forward message to destination
- [x] Handle unknown nickname
- [x] Handle missing recipient
- [ ] Handle missing message

### Client → Channel
- [x] Detect channel target
- [x] Find channel
- [x] Verify sender permissions
- [x] Broadcast message to channel members
- [x] Do not broadcast the message back to the sender
- [x] Handle unknown channel

## 5. Channels
### Channel Class
- [ ] **In progress:** `Channel` class
- [x] Channel name
- [x] Members
- [x] Channel operators
- [x] Topic
- [x] Invitations
- [ ] Password/key
- [ ] User limit
- [ ] Channel modes

### JOIN
- [x] Parse `JOIN`
- [x] Find existing channel
- [x] Create channel when necessary
- [x] Add user to channel
- [x] Make channel creator an operator
- [x] Broadcast `JOIN` to channel members
- [x] Send topic information
- [x] Send channel member list
- [ ] Check channel modes before allowing entry

### User Removal / Channel Lifetime
- [x] Remove users from channels correctly
- [x] Remove disconnected clients from every channel
- [x] Remove empty channels when appropriate
- [x] Keep operator lists consistent

## 6. Channel Operators
- [x] Assign operator status to channel creator
- [x] Store operator status
- [x] Check operator permissions before privileged commands
- [ ] Support operator promotion/removal with `MODE +/-o`

## 7. Operator Commands
### KICK
- [x] Parse `KICK`
- [x] Check that channel exists
- [x] Check that sender is in channel
- [x] Check operator privileges
- [x] Check that target user exists
- [x] Check that target user belongs to channel
- [x] Broadcast `KICK`
- [x] Remove user from channel

### INVITE
- [x] Parse `INVITE`
- [x] Check target user
- [x] Check channel
- [x] Check permissions
- [x] Store invitation when required
- [x] Notify invited user

### TOPIC
- [x] Read current topic
- [x] Set new topic
- [x] Clear topic
- [x] Enforce `+t`
- [x] Broadcast topic changes

## 8. Channel Modes
### `MODE i` — Invite Only
- [x] `+i`
- [x] `-i`
- [x] Reject non-invited users when `+i`

### `MODE t` — Topic Restriction
- [x] `+t`
- [x] `-t`
- [x] Restrict topic modification to operators when enabled

### `MODE k` — Channel Key
- [ ] `+k <password>`
- [ ] `-k`
- [ ] Store channel key
- [ ] Require correct key during `JOIN`

### `MODE o` — Channel Operator
- [ ] `+o <nickname>`
- [ ] `-o <nickname>`
- [ ] Validate target user
- [ ] Update operator status

### `MODE l` — User Limit
- [ ] `+l <limit>`
- [ ] `-l`
- [ ] Validate limit
- [ ] Reject `JOIN` when channel is full

## 9. Buffering / Fragmented Messages
- [x] Fragmented command reception
- [x] Keep incomplete data between `recv()` calls
- [x] Detect `\r\n`
- [x] Extract one complete command at a time
- [x] Handle several commands in one `recv()`
- [x] Keep remaining incomplete data in the client buffer

## 10. Non-Blocking Output
- [x] Proper outgoing-message buffering
- [x] Handle partial `send()`
- [x] Use `POLLOUT` when data is waiting to be sent
- [x] Remove sent data from output buffer
- [x] Avoid blocking writes

## 11. Client Disconnection / Cleanup
- [x] Detect closed connection
- [x] Handle `recv()` returning `0`
- [x] Remove fd from `poll`
- [x] Close client fd
- [x] Remove client from client container
- [x] Remove client from every joined channel
- [x] Update operator/member lists
- [ ] Avoid invalid references after removal

## 12. Final Tests
- [x] Multiple clients
- [ ] Fragmented / multiple commands
- [ ] Client disconnection
- [ ] All required commands and modes
- [x] Reference IRC client

## 13. C++98 / Project Requirements
- [ ] Check C++98 compatibility
- [x] Check flags
- [ ] No forbidden functions
- [ ] No blocking server operations
- [ ] Check file descriptor cleanup
- [ ] Check memory leaks
- [ ] Check unexpected exceptions/crashes
