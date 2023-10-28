# Peer-to-Peer File Sharing

This project implements a peer-to-peer file-sharing system consisting of two primary components: the `Server` and the `Client`. The system facilitates direct file sharing between clients connected to the server and allows them to communicate and exchange files seamlessly.

## Server (Server.cpp)

### Overview
The `Server.cpp` code embodies the server-side application responsible for managing client connections, maintaining a list of connected clients, and orchestrating file-sharing interactions among the connected peers.

- **Key Features**:
  - Maintains a client list with essential details such as IP, port, and unique serial number.
  - Manages client connection and disconnection, file sharing, and notifications to other clients about changes in the network.

### Code Structure
- **Client Struct**: Defines the structure for a connected client, encompassing IP, port, serial number, and file descriptor.
- **Input Function**: Manages client inputs, including file registration, disconnection, and communication among connected clients.
- **Main Function**: Establishes the server's setup, creates and manages socket connections, and handles client interactions.

## Client (Client.cpp)

### Overview
The `Client.cpp` code represents the client-side application, enabling users to connect to the server, communicate with other clients, share files, and engage in chat sessions.

- **Key Features**:
  - Connects to the server to join the file-sharing network and shares available files.
  - Communicates with the server for initiating chat sessions and file transfers among connected clients.

### Code Structure
- **File Transfer Functions**: Functions managing file transfers between clients in the network.
- **Connection Establishment**: Contains functions to connect to the server and manage chat sessions with other clients.
- **Main Function**: Establishes a connection with the server, shares available files, initiates chat sessions, and handles direct file transfers among clients.

## Functionality Overview
The server application governs the connection of clients, updates the list of available files, and handles client disconnection notifications. Meanwhile, the client application connects to the server, shares available files, interacts with the server for chat sessions, and performs direct file transfers with other connected clients.

The client-side application manages interactions with the server, initiates file transfers, maintains chat sessions, and directly exchanges files with other connected clients.

## Final Notes
This project offers a fundamental but functional peer-to-peer file-sharing system, enabling connected clients to communicate and exchange files directly. The system encourages decentralized data sharing and facilitates seamless communication between peers in the network.

---
*Documented by ChatGPT*
