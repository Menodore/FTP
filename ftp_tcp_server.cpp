#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <thread>
#include <filesystem>
#include <fstream>

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

void handle_client(int client_socket, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            cout << "Client disconnected.\n";
            close(client_socket);
            break;
        }

        string command(buffer);
        cout << "Client " << inet_ntoa(client_addr.sin_addr) << ":"
             << ntohs(client_addr.sin_port) << " - Command: " << command << endl;

         if (command.rfind("MKDIR", 0) == 0) {
            string dirname = command.substr(6);
            filesystem::create_directory(dirname);
            string response = "Directory " + dirname + " created.";
            send(client_socket, response.c_str(), response.size(), 0);
        }
        else if (command.rfind("SAVE", 0) == 0) {
            string filename = command.substr(5);
            ofstream file(filename, ios::binary);
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            file.write(buffer, bytes_received);
            file.close();
            string response = "File " + filename + " saved.";
            send(client_socket, response.c_str(), response.size(), 0);
        }
        else if (command.rfind("RETR", 0) == 0) {
            string filename = command.substr(5);
            ifstream file(filename, ios::binary);
            if (file.is_open()) {
                while (!file.eof()) {
                    file.read(buffer, BUFFER_SIZE);
                    send(client_socket, buffer, file.gcount(), 0);
                }
                file.close();
            } else {
                string response = "File not found.";
                send(client_socket, response.c_str(), response.size(), 0);
            }
        }
/*        else if (command.rfind("LIST", 0) == 0) {
            string dirname = command.substr(5);
            string file_list;
            for (const auto& entry : filesystem::directory_iterator(dirname)) {
                file_list += entry.path().filename().string() + "\n";
            }
            send(client_socket, file_list.c_str(), file_list.size(), 0);
        }*/
        else if (command.rfind("RMDIR", 0) == 0) {
    // Check if a directory name is provided
    if (command.size() > 6) {
        string dirname = command.substr(6); // Extract the directory name
        try {
            // Attempt to remove the directory
            if (filesystem::exists(dirname) && filesystem::is_directory(dirname)) {
                if (filesystem::is_empty(dirname)) {
                    filesystem::remove(dirname);
                    string response = "Directory " + dirname + " removed successfully.";
                    send(client_socket, response.c_str(), response.size(), 0);
                } else {
                    string response = "Directory not empty, cannot be removed.";
                    send(client_socket, response.c_str(), response.size(), 0);
                }
            } else {
                string response = "Directory not found or invalid.";
                send(client_socket, response.c_str(), response.size(), 0);
            }
        } catch (const filesystem::filesystem_error &e) {
            string response = "Error removing directory: " + string(e.what());
            send(client_socket, response.c_str(), response.size(), 0);
        }
    } else {
        string response = "Directory name required for RMDIR.";
        send(client_socket, response.c_str(), response.size(), 0);
    }
}

        else if (command.rfind("LIST", 0) == 0) {
    // Check if a directory name is provided
    string dirname;
    if (command.size() > 5) {
        dirname = command.substr(5); // Extract directory name if provided
    } else {
        dirname = "."; // Default to current directory if no directory is provided
    }

    // Check if the directory exists
    if (filesystem::exists(dirname) && filesystem::is_directory(dirname)) {
        string file_list;
        for (const auto& entry : filesystem::directory_iterator(dirname)) {
            file_list += entry.path().filename().string() + "\n";
        }
        send(client_socket, file_list.c_str(), file_list.size(), 0);
    } else {
        string response = "Directory not found or invalid.";
        send(client_socket, response.c_str(), response.size(), 0);
    }
}

        else {
            string response = "Unknown command.";
            send(client_socket, response.c_str(), response.size(), 0);
        }
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    cout << "FTP server is ready to serve clients...\n";

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        cout << "Client connected: " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << endl;
        thread(handle_client, new_socket, address).detach();
    }

    return 0;
}
