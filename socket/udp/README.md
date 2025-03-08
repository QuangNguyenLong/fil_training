# A sample UDP socket tutorial

## I. Compilation & Execution
 
### 1. Compile the sender and receiver
 
```sh
mkdir build
cd build
cmake ..
make
```
 
or just use `gcc`:
 
```sh
gcc sender/main.c -o udp_sender
gcc receiver/main.c -o udp_receiver
```

### 2. Run the receiver first
 
```sh
./udp_receiver
```

The receiver waits for a message from sender.

### 3. Run the sender in another terminal

 ```sh
 ./udp_sender
 ```

The sender sends a message.

---

## II. Expected Output

### 1. Sender Terminal
```
[SENDER]	sent 15 bytes: I love fil <3
```

### 2. Receiver Terminal
```
[RECEIVER]	received 15 bytes: I love fil <3
```

---

## III. Must Know
- The [sender](sender/main.c) and [receiver](receiver/main.c) source code, **read** it please.
- The sample code uses IPv4 (`AF_INET`) and UDP (`SOCK_DGRAM`).

---

## IV. Homework
 - Modify the sample code to send message to `receiver` over a wireless network (e.g. Wifi) and explain the behaviour.



