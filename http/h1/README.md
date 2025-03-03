# A sample HTTP/1.1 GET tutorial

---

This tutorial provides a simple example of HTTP/1.1 GET Request using TCP socket programming in C, demonstrating how a HTTP client request a resource to a HTTP server. The example includes a basic **HTTP client** with GET Request.

---

## I. Compilation & Execution

### 1. Compile the HTTP client

```sh
mkdir build
cd build
cmake ..
make
```

### 2. Run the HTTP client

```sh
./http_client
```

The client will create a GET Request to a HTTP server specified in [main.c](client/main.c)

---

## II. Expected Output

A HTTP Respond from the HTTP server.

---

## III. Must Know
- The [client](client/main.c) source code, **read** it please.
- The sample code uses TCP Socket to perform HTTP Request.
- The Sample code only implemented `GET` Request (there are many more).

---

## IV. Homework
- Modify the sample code to perform POST Request instead.
