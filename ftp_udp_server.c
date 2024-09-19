
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#define IP_PROTOCOL 0
#define PORT_NO 15050
#define NET_BUF_SIZE 1024
#define NOFILE "File Not Found!"

// Function to clear the buffer
void bufferClear(char *buf)
{
    int i;
    for (i = 0; i < NET_BUF_SIZE; i++)
        buf[i] = '\0';
}

// Function to save the file sent by the client
void saveFile(int sockfd, struct sockaddr_in *client_addr, socklen_t addr_len, char *filename)
{
    char buffer[NET_BUF_SIZE];
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error opening file for writing: %s\n", filename);
        return;
    }

    int nBytes;
    while (1)
    {
        bufferClear(buffer);
        nBytes = recvfrom(sockfd, buffer, NET_BUF_SIZE, 0, (struct sockaddr *)client_addr, &addr_len);
        if (strcmp(buffer, "EOF") == 0) // End of file transfer
            break;
        fwrite(buffer, sizeof(char), nBytes, fp);
    }
    fclose(fp);
    printf("File saved: %s\n", filename);
}

// Function to transmit a file to the client
void transmitFile(int sockfd, struct sockaddr_in *client_addr, socklen_t addr_len, char *filename)
{
    char buffer[NET_BUF_SIZE];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("File not found: %s\n", filename);
        sendto(sockfd, NOFILE, strlen(NOFILE), 0, (struct sockaddr *)client_addr, addr_len);
        return;
    }

    while (1)
    {
        int nBytes = fread(buffer, sizeof(char), NET_BUF_SIZE, fp);
        sendto(sockfd, buffer, nBytes, 0, (struct sockaddr *)client_addr, addr_len);
        if (nBytes < NET_BUF_SIZE)
        {
            sendto(sockfd, "EOF", 3, 0, (struct sockaddr *)client_addr, addr_len); // End of file signal
            break;
        }
    }
    fclose(fp);
    printf("File transmitted: %s\n", filename);
}

// Function to handle directory listing
void listDirectory(int sockfd, struct sockaddr_in *client_addr, socklen_t addr_len, char *dirname)
{
    struct dirent *de;
    DIR *dr = opendir(dirname);
    char buffer[NET_BUF_SIZE];

    if (dr == NULL)
    {
        printf("Could not open directory: %s\n", dirname);
        sendto(sockfd, "Directory Not Found", 19, 0, (struct sockaddr *)client_addr, addr_len);
        return;
    }

    while ((de = readdir(dr)) != NULL)
    {
        snprintf(buffer, NET_BUF_SIZE, "%s\n", de->d_name);
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)client_addr, addr_len);
    }
    sendto(sockfd, "EOF", 3, 0, (struct sockaddr *)client_addr, addr_len); // End of directory listing
    closedir(dr);
}

int main()
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char netbuf[NET_BUF_SIZE];

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL);
    if (sockfd < 0)
        printf("Failed to create socket!\n");
    else
        printf("Socket created successfully: %d\n", sockfd);

    // Bind to port
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NO);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0)
        printf("Binding successful!\n");
    else
        printf("Binding failed!\n");

    while (1)
    {
        bufferClear(netbuf);

        // Receive command from client
        recvfrom(sockfd, netbuf, NET_BUF_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        printf("Received command: %s\n", netbuf);

        // Handle MKDIR command
        if (strncmp(netbuf, "MKDIR", 5) == 0)
        {
            char dirname[NET_BUF_SIZE];
            sscanf(netbuf + 6, "%s", dirname);
            if (mkdir(dirname, 0777) == 0)
                printf("Directory created: %s\n", dirname);
            else
                printf("Failed to create directory: %s\n", dirname);
        }

        // Handle SAVE command
        else if (strncmp(netbuf, "SAVE", 4) == 0)
        {
            char filename[NET_BUF_SIZE];
            sscanf(netbuf + 5, "%s", filename);
            printf("Saving file: %s\n", filename);
            saveFile(sockfd, &client_addr, addr_len, filename);
        }

        // Handle RETR command
        else if (strncmp(netbuf, "RETR", 4) == 0)
        {
            char filename[NET_BUF_SIZE];
            sscanf(netbuf + 5, "%s", filename);
            printf("Retrieving file: %s\n", filename);
            transmitFile(sockfd, &client_addr, addr_len, filename);
        }

        // Handle LIST command
        else if (strncmp(netbuf, "LIST", 4) == 0)
        {
            char dirname[NET_BUF_SIZE];
            sscanf(netbuf + 5, "%s", dirname);
            printf("Listing directory: %s\n", dirname);
            listDirectory(sockfd, &client_addr, addr_len, dirname);
        }

        // Unknown command
        else
        {
            printf("Unknown command: %s\n", netbuf);
        }
    }

    close(sockfd);
    return 0;
}
