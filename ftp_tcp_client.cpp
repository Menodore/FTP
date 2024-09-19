#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fstream>

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Socket creation error." << endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "172.21.81.84", &serv_addr.sin_addr) <= 0) {
        cout << "Invalid address/ Address not supported." << endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Connection failed." << endl;
        return -1;
    }

    while (true) {
        string command;
        cout << "Enter FTP command: ";
        getline(cin, command);
        send(sock, command.c_str(), command.size(), 0);

        if (command.rfind("SAVE", 0) == 0) {
            string filename = command.substr(5);
            ifstream file(filename, ios::binary);
            file.read(buffer, BUFFER_SIZE);
            send(sock, buffer, file.gcount(), 0);
            file.close();
        }

        memset(buffer, 0, BUFFER_SIZE);
        int valread = recv(sock, buffer, BUFFER_SIZE, 0);
        cout << "Server: " << buffer << endl;

        if (command == "QUIT") {
            break;
        }
    }

    close(sock);
    return 0;
}

