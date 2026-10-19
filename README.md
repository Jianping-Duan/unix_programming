# UNIX Programming in C

A collection of UNIX system programming examples implemented in C, using FreeBSD as the primary development environment. The project covers file operations, process management, signals, threads, interprocess communication, network programming, I/O multiplexing, and virtual memory.

## Overview

This project provides practical examples of UNIX system programming, with an emphasis on system calls and library functions commonly used in C programs.

The source code is organized into modules based on related system programming topics and interfaces. Each C source file is designed to be compiled into a separate executable, making it easier to examine individual examples and understand how specific system calls and library functions are used.

The examples cover a broad range of UNIX programming topics, from file and process management to threads, interprocess communication, sockets, and advanced I/O. The corresponding function documentation can be consulted through the `man(2)` and `man(3)` manual sections, where applicable.

## Source Roadmap

| Directory | Description |
| --- | --- |
| `include` | Common header files and shared definitions |
| `file` | File operations, including file attributes, I/O, directories, and file locking |
| `signals` | Signal handling |
| `process` | Process creation, execution, termination, daemon processes, process groups, sessions, job control, monitoring, priorities, scheduling, and resource management |
| `threads` | POSIX threads, including basic operations, cancellation, thread safety, and synchronization |
| `pipefifos` | Pipes and FIFOs |
| `ipckey` | System V IPC key generation |
| `msgqueue` | System V and POSIX message queues |
| `semaphore` | System V and POSIX semaphores |
| `shmem` | System V and POSIX shared memory |
| `socket` | UNIX domain sockets, Internet domain sockets, and server programming examples |
| `advio` | Advanced I/O, including `select`, `poll`, and `kqueue`/`kevent` |
| `terminals` | Terminal-related functions and operations |
| `time` | Time-related functions |
| `tmrslp` | Timers and sleep functions |
| `vmmap` | Virtual memory and memory mapping |
| `bin` | Executable files generated during compilation |

## Build and Platforms

FreeBSD is the primary development and testing environment for this project. Most of the code examples have also been tested successfully on Linux.

The project uses Makefiles to organize compilation, with individual C source files generating corresponding executables.

Some examples use interfaces or behaviors specific to FreeBSD or other UNIX-like systems. Depending on the target platform, certain source files or build rules may require adjustments.

## License

This project is licensed under the BSD 3-Clause License. See [LICENSE](LICENSE) for details.
