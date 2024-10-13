
# **Peer-to-Peer Chat Application**

## **Overview**
This is a peer-to-peer (P2P) chat application built using C++ sockets. It allows multiple clients to communicate over a network through **broadcasts** and **direct messages (DMs)**. The application uses **multithreading** to handle simultaneous tasks and supports both client-to-server and client-to-client communication.

---

## **Features**
- **Broadcast Messages:** Send messages to all connected peers.
- **Direct Messages (DMs):** Privately communicate with specific peers.
- **Multi-threaded Communication:** Handles incoming and outgoing messages concurrently.
- **Dynamic Peer Discovery:** Clients receive updated peer lists from the server.
- **Simple Command Interface:** Use `/broadcast` and `/DM` commands to communicate.

---

## **How It Works**
- **Server**: Listens for incoming connections from clients and provides them with information about other connected peers.
- **Client**: Connects to the server, sends and receives messages from other peers via broadcast or DM.
- **Peer Communication**: Each client runs a mini-server to accept messages from other peers.

---

## Project Structure
The project consists of the following key files:

- **server.cpp**: Contains the implementation of the server that handles client connections, message broadcasting, and direct messaging.
- **client.cpp**: Contains the implementation of the client that connects to the server and sends/receives messages.
- **utils.h**: Provides utility functions and definitions used by both the server and client.
- **colors.h**: Defines color codes for terminal output using ANSI escape sequences for better visibility and user experience.

---

## **Commands**
1. **`/broadcast`**: Send a message to all connected clients.  
   Example:  
   ```
   /broadcast Hello everyone!
   ```

2. **`/DM`**: Send a private message to a specific client by selecting the client’s number from the list.  
   Example:
   ```
   /DM  
   The list of the clients connected are:  
   1. Name: lakshit, Port: 42944, IP: 127.0.0.1  
   Enter the number of the client you want to DM: 1  
   Enter the DM Message: Hello, Lakshit!
   ```

3. **`/exit`**: Exit the chat application.

---

## **Setup and Installation**

1. **Prerequisites**:
   - C++ compiler (e.g., `g++`)
   - Linux or Unix-like environment for socket programming

2. **Compilation**:
   Use the following commands to compile the program:
   ```bash
   g++ server.cpp -lpthread -o client
   ```
   ```bash
   g++ client.cpp -lpthread -o client
   ```

3. **Running the Server**:
   Start the server with the following command:
   ```bash
   ./server <PORT>
   ```

4. **Running the Client**:
   Start the client with:
   ```bash
   ./client <SERVER_IP> <PORT>
   ```

---

## **Example Workflow**

1. **Start the Server**:
   ```
   ./server 8080
   ```

2. **Connect Clients**:
   Each client connects to the server:
   ```
   ./client 127.0.0.1 8080
   ```

3. **Broadcast a Message**:
   ```
   /broadcast Hello everyone!
   ```

4. **Send a DM**:
   ```
   /DM  
   Enter the number of the client you want to DM: 1  
   Enter the DM Message: Hi there!
   ```

5. **Exit**:
   ```
   /exit
   ```
---
