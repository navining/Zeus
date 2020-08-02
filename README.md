# Zeus
A high performance, cross-platform Internet Communication Engine. Developed with native socket API. Aim at handling millions of concurrent connections.

## Features
### Cross Platform
- Both client and server can run on Windows, Linux or MacOS.
- Cross-platform communication between client and server.
- Support different IO-multiplexing Models
### Lightweight
- Developed with native socket API and C++ standard libraries, no dependency on any external libraries.
- Along with an one-click compile script to deploy.
### High Performance
- Current concurrency ability: 10K connections, 1Gbps I/O with single thread
- Target concurrency ability: 1M concurrent connections.
### Good Availability
- A complete and detailed log system
- Friendly configuration files
- Heartbeat detection and flow control is available
- Customizable thread pool, memory pool and object pool
### Multiple IO-Multiplexing Models
- select
- IOCP (TODO)
- epoll
### Multiple Transfer Protocols
- TCP
- UDP (TODO)
- FTP (TODO)
- HTTP (TODO)

# Examples

There are examples of a TCP client and TCP server under `examples/`. The server is an echo server, which sends back messages

## Build
### Linux & MacOS
CMake 3.0.0 or higher.

g++ 4.7 or higher.
```
git clone https://github.com/navining/Zeus.git
cd Zeus/examples
./build.sh
```

### Windows
Visual Studio 2015 or higher. Open `Zeus.sln` and build with VS.

## Run

Executable files are generated under `bin/`.

Sample script are provided as well, where programmers can custom the parameters.

### Linux & MacOS
`server.sh` and `client.sh` under `server/` and `client/`.

### Windows
`server.bat` and `client.bat` under `server/` and `client/`.
