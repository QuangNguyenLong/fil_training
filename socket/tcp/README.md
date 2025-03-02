# A sample TCP socket tutorial

---

This tutorial provides a simple example of TCP socket programming in C, demonstrating how to establish communication between a client and a server. The example includes a basic **TCP client** and **TCP server** that exchange messages over a network connection.

## I. Compilation & Execution

### 1. Compile the server and client

```sh
mkdir build
cd build
cmake ..
make
```

or just use `gcc`:

```sh
gcc client/main.c -o tcp_client
gcc server/main.c -o tcp_server
```

### 2. Run the server first

```sh
./tcp_server
```

The server listens and waits for a client to connect.

### 3. Run the client in another terminal

```sh
./tcp_client
```

The client connects to the server, sends a message, and receives a response.

---

## II. Expected Output

### 1. Server Terminal
```
[CLIENT]	recv 15 bytes: I love fil <3
[SERVER]	send 19 bytes: I love fil too <3
```

### 2. Client Terminal
```
[CLIENT]	send 15 bytes: I love fil <3
[SERVER]	recv 19 bytes: I love fil too <3
```

---

## III. Must Know
- The [client](client/main.c) and [server](server/main.c) source code, **read** it please.
- The sample code uses IPv4 (`AF_INET`) and TCP (`SOCK_STREAM`).
- The sample code uses `send()` and `recv()` functions to handle data transmission (there are many more ways to do this).

---

## IV. Homework
- Modify the sample code to send a file (e.g. [sample.mp4](sample.mp4)) from server to client after accepting the connection.

