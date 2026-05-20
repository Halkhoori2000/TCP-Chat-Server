# TCP-Chat-Server — C Multi-Client TCP Chat Server

A chat server that lets multiple people message each other in real time from a terminal. Anyone can connect, pick a username, and send messages that are instantly broadcast to everyone else. The server handles up to 20 simultaneous users without spawning extra processes or threads.

Written in C using POSIX TCP sockets. A single-threaded server uses `select()` to monitor all connected client sockets and stdin simultaneously, replacing the naive fork-per-client model. Enforces username uniqueness on connection, broadcasts messages with the sender prefix to all other file descriptors, and cleans up the FD set cleanly on disconnect.

**[Live Demo →](https://halkhoori2000.github.io/TCP-Chat-Server/)**

## Use Cases
- Terminal-based team communication for development environments or server rooms with no GUI
- Backend messaging layer for multiplayer games and applications where reliability matters more than low latency
- IoT device command and telemetry channels serving a small number of known clients on a local network
- Teaching platform for network programming — demonstrates core socket, multiplexing, and protocol concepts without the complexity of threading

## Challenges
- **Partial reads**: TCP is a stream protocol, not a message protocol — a single send() from a client may arrive as multiple recv() calls; without message framing, a broadcast can be split mid-transmission, requiring buffer management to reassemble complete messages reliably
- **Broken connection detection**: a client that crashes without closing its socket does not trigger a clean disconnect — recv() returning 0 or -1 must be detected and the FD removed from the master set immediately, or select() will continue signalling on the dead socket in an infinite loop
- **FD_SET bookkeeping**: select() modifies the fd_set in place on every call, requiring the master set to be copied to a working set before each select(); failing to remove a disconnected client causes repeated false-positive signals and prevents new clients from connecting once the FD limit is reached

---

## Features

- **Multi-client server** — handles up to 20 simultaneous connections on a single thread using `select()`
- **select() multiplexing** — replaces the naive fork-per-client model; monitors all sockets and stdin in one loop with no busy-wait
- **Username registration** — server prompts each client on connect; enforces unique usernames across all active sessions
- **Broadcast messaging** — every message is forwarded to all other connected clients with the sender's username prepended
- **Clean disconnect** — typing `exit` removes the client from the FD set, frees its slot, and notifies the server terminal

---

## Tech Stack

| Layer | Technology |
|---|---|
| Language | C (C99) |
| Networking | POSIX TCP sockets (`sys/socket.h`, `netinet/in.h`) |
| I/O Model | `select()` — file descriptor set multiplexing |
| Build | `gcc` (no external dependencies) |
| Platform | Linux / Ubuntu |

---

## Architecture

```
┌──────────────────────────────────────────────────┐
│                    SERVER                        │
│  select() loop monitors:                         │
│    • serverSocket  ← new connections             │
│    • fileDesc[0..N] ← messages from clients      │
│                                                  │
│  On new connection:                              │
│    accept() → prompt username → validate unique  │
│                                                  │
│  On message received:                            │
│    broadcast to all other fileDesc entries       │
└────────────┬─────────────────────────────────────┘
             │ TCP (port 1026)
    ┌────────┴────────┐
    │                 │
┌───▼───┐         ┌───▼───┐
│Client │  . . .  │Client │   (up to 20)
│select │         │select │
│stdin + │         │stdin + │
│socket  │         │socket  │
└────────┘         └────────┘
```

The client also uses `select()` to monitor both stdin (user input) and the socket (incoming messages) simultaneously — keeping the terminal responsive with no blocking reads.

---

## Build & Run

**Requirements:** GCC, Linux/Ubuntu

```bash
# Clone
git clone https://github.com/Halkhoori2000/TCP-Chat-Server.git
cd TCP-Chat-Server/src

# Compile
gcc -o server server.c
gcc -o client client.c

# Start server (terminal 1)
./server
# Output: Listening...

# Connect clients (terminal 2, 3, ...)
./client
# Enter username: alice

# In another terminal
./client
# Enter username: bob

# Chat — alice types a message, bob receives:
# alice: hello bob
```

> **Note:** The hardcoded IP `10.0.2.15` is the VirtualBox NAT default. To run on your machine, update `inet_aton("10.0.2.15", ...)` in both `client.c` and `server.c` to `127.0.0.1`.

---

## Project Structure

```
TCP-Chat-Server/
├── src/
│   ├── server.c    ← TCP server with select() loop, username mgmt, broadcast
│   └── client.c    ← TCP client with select() loop for stdin + socket
└── index.html      ← Interactive demo (GitHub Pages)
```

---

## Course

CMPSC 311 — Introduction to Systems Programming  
The Pennsylvania State University · Summer 2022
