#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

#define IP_PROTOCOL 0
#define PORT_NO 15050
#define NET_BUF_SIZE 1024
#define IP_ADDRESS "127.0.0.1"

// Function to clear the buffer
void bufferClear(char *buf)
{
    int i;
    for (i = 0; i < NET_BUF_SIZE; i++)
        buf[i] = '\0';
}

// Function to send a file to the server
void sendFile(int sockfd, struct sockaddr_in *server_addr, socklen_t addr_len, char *filename)
{
    char buffer[NET_BUF_SIZE];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("File not found: %s\n", filename);
        return;
    }

    while (1)
    {
        int nBytes = fread(buffer, sizeof(char), NET_BUF_SIZE, fp);
        sendto(sockfd, buffer, nBytes, 0, (struct sockaddr *)server_addr, addr_len);
        if (nBytes < NET_BUF_SIZE)
        {
            sendto(sockfd, "EOF", 3, 0, (struct sockaddr *)server_addr, addr_len); // End of file signal
            break;
        }
    }
    fclose(fp);
    printf("File sent: %s\n", filename);
}

// Function to receive a file from the server
void receiveFile(int sockfd, struct sockaddr_in *server_addr, socklen_t addr_len, char *filename)
{
    char buffer[NET_BUF_SIZE];
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error opening file for writing: %s\n", filename);
        return;
    }

    while (1)
    {
        bufferClear(buffer);
        recvfrom(sockfd, buffer, NET_BUF_SIZE, 0, (struct sockaddr *)server_addr, &addr_len);
        if (strcmp(buffer, "EOF") == 0) // End of file transfer
            break;
        fwrite(buffer, sizeof(char), NET_BUF_SIZE, fp);
    }
    fclose(fp);
    printf("File received: %s\n", filename);
}

// Function to receive directory listing from the server
void receiveDirectoryList(int sockfd, struct sockaddr_in *server_addr, socklen_t addr_len)
{
    char buffer[NET_BUF_SIZE];
    while (1)
    {
        bufferClear(buffer);
        recvfrom(sockfd, buffer, NET_BUF_SIZE, 0, (struct sockaddr *)server_addr, &addr_len);
        if (strcmp(buffer, "EOF") == 0) // End of directory listing
            break;
        printf("%s", buffer);
    }
}

// Driver code
int main()
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    char command[NET_BUF_SIZE];
    char filename[NET_BUF_SIZE];

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL);
    if (sockfd < 0)
    {
        printf("Failed to create socket!\n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NO);
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    while (1)
    {
        printf("Enter FTP command (MKDIR, SAVE, RETR, LIST): ");
        fgets(command, NET_BUF_SIZE, stdin);
        command[strlen(command) - 1] = '\0'; // Remove newline

        // MKDIR command
        if (strncmp(command, "MKDIR", 5) == 0)
        {
            sendto(sockfd, command, strlen(command), 0, (struct sockaddr *)&server_addr, addr_len);
        }

        // SAVE command
        else if (strncmp(command, "SAVE", 4) == 0)
        {
            sscanf(command + 5, "%s", filename);
            sendto(sockfd, command, strlen(command), 0, (struct sockaddr *)&server_addr, addr_len);
            sendFile(sockfd, &server_addr, addr_len, filename);
        }

        // RETR command
        else if (strncmp(command, "RETR", 4) == 0)
        {
            sscanf(command + 5, "%s", filename);
            sendto(sockfd, command, strlen(command), 0, (struct sockaddr *)&server_addr, addr_len);
            receiveFile(sockfd, &server_addr, addr_len, filename);
        }

        // LIST command
        else if (strncmp(command, "LIST", 4) == 0)
        {
            sendto(sockfd, command, strlen(command), 0, (struct sockaddr *)&server_addr, addr_len);
            receiveDirectoryList(sockfd, &server_addr, addr_len);
        }

        // Invalid command
        else
        {
            printf("Unknown command!\n");
        }
    }

    close(sockfd);
    return 0;
}
