#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    const char* SERVER_IP = "127.0.0.1";  // Server IP address (localhost)
    const int PORT = 8080;  // Port to connect to the server
    int sock = 0;  // Socket file descriptor
    struct sockaddr_in serv_addr;  // Server address structure
    char buffer[1024] = {0};  // Buffer to receive server response

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error\n";
        return -1;  // Return on error
    }

    // Set up server address structure
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);  // Convert port number to network byte order

    // Convert IP address to binary format and store in the sockaddr_in structure
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported\n";
        return -1;  // Return on error
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed\n";
        return -1;  // Return on error
    }

    // Main loop to interact with the server
    while (true) {
        std::string command;
        std::cout << "Enter command: ";
        std::getline(std::cin, command);  // Read user input

        // Exit the loop if user types "exit"
        if (command == "exit") break;

        // Append CRLF to the command
        command += "\r\n";
        // Send the command to the server
        send(sock, command.c_str(), command.size(), 0);

        // Clear buffer and wait for server response
        memset(buffer, 0, sizeof(buffer));
        int valread = read(sock, buffer, sizeof(buffer));  // Read server response
        if (valread > 0) {
            // Print the server response
            std::cout << "Server response: " << buffer << std::endl;
        }
    }

    // Close the socket after exiting the loop
    close(sock);

    return 0;
}
