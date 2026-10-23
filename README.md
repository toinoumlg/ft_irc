*This project has been created as part of the 42 curriculum by lmaria, amalangu.*

# ft_irc

## Description

ft_irc is an IRC (Internet Relay Chat) server written in **C++98**. The goal of the project is to understand how a real network server works: TCP sockets, non-blocking I/O, multiplexing many clients, and implementing a text protocol defined by RFCs.

The server handles many clients at once on a single thread, using non-blocking sockets and one `poll()` loop. Users can register with a password, pick a nickname, send private messages, and create and join channels. Channel operators can kick and invite users, set the topic, and change channel modes (`i`, `t`, `k`, `o`, `l`). It works with real IRC clients (WeeChat, irssi, HexChat…) as well as plain `nc`.

## Instructions

### Build

```sh
make        # builds the ircserv binary
make clean  # removes build/
make fclean # removes build/ and ircserv
make re     # full rebuild
```

Requires a C++ compiler. The project is compiled with `c++ -Wall -Wextra -Werror -std=c++98`.

### Run

```sh
./ircserv <port> <password>
```

- `port`: listening port, between **1024 and 65535**
- `password`: the password every client must send with `PASS`

The server listens on all interfaces (`INADDR_ANY`) and stops on `SIGINT`, `SIGQUIT` or `SIGTERM`.

### Connect

With an IRC client, for example WeeChat (`ft_irc` is just a display name):

```
/server add ft_irc 127.0.0.1/<port> -password=<password>
/connect ft_irc
/join #42
```

Or by hand with `nc` (`-C` sends `\r\n` line endings):

```sh
nc -C 127.0.0.1 6667
PASS secret
NICK alice
USER alice 0 * :Alice
JOIN #42
PRIVMSG #42 :hello
```

Registration completes once `PASS`, `NICK` and `USER` have been received, and the server answers with the welcome replies `001` to `004`. `PASS` must come first; a wrong password closes the connection. Until registration is complete, any other command (except `QUIT` and `CAP`) gets `451 ERR_NOTREGISTERED`.

## Features

### Supported commands

| Command | Description |
|---|---|
| `PASS <password>` | Sends the server password |
| `NICK <nickname>` | Sets or changes the nickname (up to 9 characters, case-insensitive, must be unique) |
| `USER <user> <mode> <unused> :<realname>` | Sets the username and real name |
| `CAP` | Accepted and ignored, so clients that negotiate IRCv3 capabilities can still connect |
| `PING <token>` | Answers with `PONG` |
| `QUIT [:<reason>]` | Leaves the server; members of shared channels receive a `QUIT` |
| `PRIVMSG <target>[,<target>] :<text>` | Sends a message to users and/or channels (non-members can write to a channel, as there is no `+n` mode) |
| `JOIN <#chan>[,<#chan>] [<key>[,<key>]]` | Joins (or creates) one or more channels |
| `PART <#chan>[,<#chan>] [:<message>]` | Leaves one or more channels |
| `TOPIC <#chan> [:<topic>]` | Shows, sets or clears the channel topic |
| `INVITE <nick> <#chan>` | Invites a user to a channel (forwarded even if the channel does not exist yet) |
| `KICK <#chan> <nick> [:<reason>]` | Removes a user from a channel *(operator only)* |
| `MODE <#chan> [<modes> [<params>]]` | Shows or changes channel modes |

Commands are case-insensitive. Errors use the standard numeric replies (`401`, `403`, `404`, `431`–`433`, `441`, `442`, `461`, `462`, `464`, `471`–`475`, `482`…).

### Channels and operators

The user who creates a channel becomes its operator (shown with `@` in the `NAMES` list). A channel is deleted when its last member leaves. Channel names and nicknames are compared case-insensitively.

Channel modes, settable by operators with `MODE`:

| Mode | Effect |
|---|---|
| `+i` / `-i` | Invite-only channel (an invitation is valid for one join) |
| `+t` / `-t` | Only operators can change the topic |
| `+k <key>` / `-k <key>` | Sets or removes a channel key required by `JOIN` (up to 23 characters) |
| `+o <nick>` / `-o <nick>` | Grants or removes operator status |
| `+l <limit>` / `-l` | Sets or removes a member limit |

Modes can be combined in one command, e.g. `MODE #42 +itkl secretkey 5` or `MODE #42 -it+o bob`. Each change that takes effect is broadcast to the channel as its own `MODE` line, and at most three parameter changes are applied per command. `MODE #42` alone returns the current modes, with the key hidden as `*`.

## Technical choices

- **One `poll()` loop** watches the listening socket and every client socket. No call blocks.
- **Per-client input buffer**: incoming bytes are accumulated until a line ending (`\r\n` or `\n`), so commands split across several packets and several commands in one packet are both handled. Lines longer than 512 bytes get the client disconnected.
- **Per-client output buffer**: replies are queued, `POLLOUT` is enabled only while data is waiting, and partial `send()` calls are handled. A client that stops reading does not slow down the others.
- **Command table**: each command maps to a `Server` member function and says whether it requires a registered client.
- **Disconnection**: a client that quits or drops is removed from every channel and its pending invitations. Queued replies are flushed first when possible, and channel members receive a `QUIT`.

### Project structure

```
include/
  Server.hpp     poll loop, client management, command dispatch
  Client.hpp     per-client state: buffers, identity, registration
  Channel.hpp    members, operators, invitations, topic, modes
  Response.hpp   reply builder and IRC numeric codes
  Helper.hpp     nickname / key validation, IRC case mapping
source/
  main.cpp
  Server.cpp               setup and main poll loop
  ServerNetwork.cpp        accept, recv, send, POLLOUT handling
  ServerCommands.cpp       command table and dispatch
  ServerLookup.cpp         client / channel lookups, target parsing
  ServerNotifications.cpp  welcome and quit notifications
  Client.cpp, Channel.cpp, Response.cpp, Broadcast.cpp, Helper.cpp
  commands/
    connection/  PASS, NICK, USER, CAP, PING, QUIT
    channel/     JOIN, PART, TOPIC, INVITE, KICK, MODE
    messaging/   PRIVMSG
```

## Resources

### IRC protocol specifications

* [RFC 1459 — Internet Relay Chat Protocol](https://www.rfc-editor.org/rfc/rfc1459)
* [RFC 2812 — Internet Relay Chat: Client Protocol](https://www.rfc-editor.org/rfc/rfc2812)
* [RFC 2811 — Internet Relay Chat: Channel Management](https://www.rfc-editor.org/rfc/rfc2811)

### Additional resources

* [chirc — Example IRC Communications](http://chi.cs.uchicago.edu/chirc/irc_examples.html)
* [Multiple waiting in C (French)](https://yunes.informatique.univ-paris-diderot.fr/wp-content/uploads/cours/RESEAU/065-C-NIO.pdf)
* [IRC References](https://dd.ircdocs.horse/refs/)
* [Modern IRC Client Protocol](https://modern.ircdocs.horse)

### AI usage

AI was used for the following tasks:

* **Understanding the subject**: answering questions about the subject and the IRC protocol.
* **Testing**: generating Python scripts that connect to the server and check its behavior (registration, error replies, channel modes, fragmented input, disconnections, many simultaneous clients, runs under valgrind).
* **Documentation**: drafting and structuring this README from the source code.
