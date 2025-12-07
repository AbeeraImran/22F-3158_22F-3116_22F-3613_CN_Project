# 22F-3158_22F-3116_22F-3613_CN_Project

# NU-Information Exchange System (CN Semester Project)

## Project Overview
This project is a multi-campus communication system designed to model the network infrastructure of FAST-NUCES. It utilizes a **Hybrid Client-Server Architecture** built with C++ socket programming on Linux. 

The system allows for reliable inter-campus messaging (TCP) while simultaneously monitoring the "Online" status of campuses using a connectionless heartbeat mechanism (UDP).

## Key Features
* **Hybrid Protocol:** Simultaneous use of TCP (Reliable) and UDP (Fast).
* **Multi-threading:** Server handles multiple clients concurrently without blocking.
* **Real-time Monitoring:** "Heartbeat" mechanism tracks active users.
* **Admin Broadcasting:** System-wide alerts sent from the central server.
* **Custom Protocol:** Text-based parsing for Authentication, Routing, and Status updates.

## How We Handled Concurrency
One of the primary challenges in this project was managing multiple campus clients simultaneously. A standard single-threaded server blocks I/O operations, meaning if one client is sending a message, others have to wait.

To solve this, we implemented **Multi-threading** using the C++ `std::thread` library:
1.  **Detached Threads:** Upon every new `accept()` call, the server spawns a dedicated thread to handle that specific client's communication loop. This allows the main server loop to remain free to accept new incoming connections.
2.  **Thread Safety (Mutex Locks):** Since multiple threads attempt to read and write to the global `clients` map simultaneously, we faced race conditions. We resolved this by implementing `std::mutex`. We lock the shared resources before writing (e.g., when a client logs in) and unlock them immediately after, ensuring data integrity.

## Installation & Usage

### Prerequisites
* Linux / Ubuntu Environment
* GCC Compiler (`g++`)

### Compilation
We use the `-pthread` flag to link the POSIX threads library.

```bash
# Compile Server
g++ server.cpp -o server -pthread

# Compile Client
g++ client.cpp -o client -pthread
