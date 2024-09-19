# FTP
Codes on FTP client server written in C/C++
Function Calls Used in TCP and UDP FTP Implementations

**TCP FTP Implementation**
1. socket()
	Purpose: Creates a new socket for communication.
	Usage:
	1. Server: socket(AF_INET, SOCK_STREAM, 0)
	2. Client: socket(AF_INET, SOCK_STREAM, 0)
	How It Works: Initializes a socket using IPv4 (AF_INET) and TCP (SOCK_STREAM). Returns a file descriptor if successful; otherwise, it returns -1.
	Limitations: Requires additional error checking to ensure the socket is successfully created.
2. setsockopt()
	Purpose: Sets socket options, such as allowing address reuse.
	Usage: setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))
	How It Works: Modifies socket behavior, such as reusing an address immediately after the program ends, which is useful for server restarts.
	Limitations: If not properly configured, can lead to security risks or unintended behavior.
3. bind()
	Purpose: Binds the server socket to a specific IP address and port.
	Usage: bind(server_fd, (struct sockaddr*)&address, sizeof(address))
	How It Works: Associates the socket with the IP address and port number specified in the sockaddr_in structure.
	Limitations: If the address or port is already in use, binding will fail.
4. listen()
	Purpose: Prepares the socket to listen for incoming connections.
	Usage: listen(server_fd, 3)
	How It Works: Sets the socket to listen mode, with a backlog queue of pending connections.
	Limitations: The backlog size is limited, which can lead to refused connections during high traffic.
5. accept()
	Purpose: Accepts a new incoming connection from a client.
	Usage: new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)
	How It Works: Blocks until a connection request is received and then returns a new socket file descriptor for the connection.
	Limitations: Accepting connections is blocking unless set otherwise; concurrent handling needs threads.
6. connect()
	Purpose: Establishes a connection to the server from the client.
	Usage: connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))
	How It Works: Initiates a connection to the server's address and port.
	Limitations: Blocking by default and can hang if the server is not responding.
7. recv() and send()
	Purpose: Receives data from and sends data to the connected socket.
	Usage:
	1. Server: recv(client_socket, buffer, BUFFER_SIZE, 0)
	2. Client: send(sockfd, command.c_str(), command.size(), 0)
	How It Works: recv() reads data into the buffer, while send() transmits data from the buffer to the client/server.
	Limitations: Blocking by default; potential for buffer overflows or incomplete transfers without proper handling.
8. close()
	Purpose: Closes the socket connection.
	Usage: close(socket_fd)
	How It Works: Terminates the connection and releases resources associated with the socket.
	Limitations: Data in transit may be lost if closed abruptly without proper shutdown.

**UDP FTP Implementation**
1. socket()
	Purpose: Creates a new socket for communication.
	Usage:
	1. Server: socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL)
	2. Client: socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL)
	How It Works: Initializes a socket using IPv4 (AF_INET) and UDP (SOCK_DGRAM). Returns a file descriptor if successful; otherwise, it returns -1.
	Limitations: Lacks the connection-oriented features of TCP, which means no direct connection management.
2. bind()
	Purpose: Binds the server socket to a specific IP address and port.
	Usage: bind(sockfd, (struct sockaddr*)&con_addr, sizeof(con_addr))
	How It Works: Associates the UDP socket with the specified IP and port.
	Limitations: Same as TCP; binding will fail if the address is in use.
3. recvfrom() and sendto()
	Purpose: Receives data from and sends data to a specific address using the socket.
	Usage:
	1. Server: recvfrom(sockfd, netbuf, NET_BUF_SIZE, sendrecvflag, (struct sockaddr*)&con_addr, &addrlen)
	2. Client: sendto(sockfd, netbuf, NET_BUF_SIZE, sendrecvflag, (struct sockaddr*)&con_addr, addrlen)
	How It Works: recvfrom() reads data into the buffer from a specific client, while sendto() transmits data to the specified client.
	Limitations: Since UDP is connectionless, it does not guarantee message delivery, ordering, or error checking. Messages can be lost or received out of order.
4. fopen(), fread(), fwrite(), and fclose()
	Purpose: Standard C file handling functions used for reading from and writing to files during file transfers.
	Usage:
	1. fopen() opens a file for reading or writing.
	2. fread() reads data from a file into a buffer.
	3. fwrite() writes data from a buffer to a file.
	4. fclose() closes an opened file.
	How It Works: These functions are used within the SAVE and RETR commands to handle file I/O on the server side.
	Limitations: File handling errors like permission issues, missing files, or corrupted writes can occur and need careful error checking.

**Summary of Limitations in Both Implementations**
1. Blocking Calls: Most of the socket functions (accept(), recv(), send()) are blocking by default, which can cause performance bottlenecks or hang the program if not handled properly.
2. Error Handling: Both TCP and UDP implementations have minimal error handling. Failures such as network issues, file errors, or unexpected disconnections are not robustly managed.
3. Concurrency: The TCP implementation uses threads to handle multiple clients, but excessive threads can lead to resource exhaustion. 4. The UDP implementation lacks stateful connection handling, making client management more complex.
5. Security: No encryption or authentication mechanisms are implemented, exposing the server to unauthorized access and data tampering.

**Functions in the handle_client Section**
1. MKDIR <directory_name>
	Description: Creates a directory with the specified name.
	Limitations: The server creates the directory in its current working directory. It does not handle permissions errors or existing directories gracefully.
2. SAVE <filename>
	Description: Saves a file received from the client to the server’s filesystem.
	Limitations: The server saves the file with the exact bytes received, which can include junk or incorrect data if the transfer is interrupted. There is no validation of file contents.
3. RETR <filename>
	Description: Retrieves a file from the server and sends it to the client.
	Limitations: If the file is too large, the server may struggle to handle the transfer efficiently due to buffer size limits. Errors are not well-handled if the file is missing or 	permissions restrict access.
4. LIST <directory_name>
	Description: Lists the contents of the specified directory, sending filenames back to the client.
	Limitations: The command assumes the directory exists and is accessible. It does not check if the directory is empty or handle errors cleanly.
5. RMDIR <directory_name>
	Description: Removes the specified directory if it is empty.
	Limitations: The command only works if the directory is empty. It does not handle non-empty directories and does not provide the option to forcefully remove contents.
6. EXIT
	Description: Closes the client connection.
	Limitations: The server closes the connection immediately without confirming if there are pending transfers or unprocessed commands.

**General Limitations**
	Error Handling: The current implementation has minimal error handling, which can result in unexpected behavior or crashes if commands are improperly formatted or if the server encounters filesystem issues.
	Concurrency: While the TCP server handles multiple clients using threads, the UDP server's handling of multiple clients is limited due to the nature of UDP, which lacks built-in connection management.
	Security: The server does not implement authentication or encryption, making it vulnerable to unauthorized access or data tampering.
	Buffer Size: The BUFFER_SIZE is fixed, which limits the amount of data transferred in one go. Large files may require multiple reads/writes, which aren't handled optimally.

**Future Improvements**
+
1. Implement detailed error handling and logging.
2. Add authentication to restrict access to authorized clients only.
3. Support for file integrity checks during transfers.
4. Implement robust directory management, allowing for recursive removal of directories.
