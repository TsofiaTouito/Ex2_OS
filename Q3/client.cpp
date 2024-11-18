#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>

// Sends a command to the server using TCP protocol
void sendTcpCommand(const std::string& command) {
    const char* SERVER_IP = "127.0.0.1"; // Server IP address (localhost)
    const int PORT = 8080;              // TCP server port

    // Create TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "TCP Socket creation failed\n";
        return;
    }

    // Set up the server address structure
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address\n";
        close(sock);
        return;
    }

    // Connect to the TCP server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "TCP Connection failed\n";
        close(sock);
        return;
    }

    // Send the command with a newline at the end
    std::string message = command + "\r\n";
    send(sock, message.c_str(), message.size(), 0);

    // Receive and print the server response
    char buffer[1024] = {0};
    read(sock, buffer, sizeof(buffer));
    std::cout << "Server response: " << buffer << std::endl;

    // Close the TCP socket
    close(sock);
}

// Sends a command to the server using UDP protocol
void sendUdpCommand(const std::string& command) {
    const char* SERVER_IP = "127.0.0.1"; // Server IP address (localhost)
    const int PORT = 8081;              // UDP server port

    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "UDP Socket creation failed\n";
        return;
    }

    // Set up the server address structure
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    // Send the command with a newline at the end
    std::string message = command + "\r\n";
    sendto(sock, message.c_str(), message.size(), 0, (const struct sockaddr*)&serv_addr, sizeof(serv_addr));

    // Receive and print the server response
    char buffer[1024] = {0};
    socklen_t len = sizeof(serv_addr);
    recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&serv_addr, &len);
    std::cout << "Server response: " << buffer << std::endl;

    // Close the UDP socket
    close(sock);
}

// Main function to interact with the user and send commands to the server
int main() {
    while (true) {
        std::string protocol, command;

        // Prompt user to choose a protocol
        std::cout << "Enter protocol (TCP/UDP): ";
        std::cin >> protocol;
        std::cin.ignore(); // Ignore trailing newline

        // Prompt user to enter the command
        std::cout << "Enter command: ";
        std::getline(std::cin, command);

        // Send command based on the chosen protocol
        if (protocol == "TCP") {
            sendTcpCommand(command);
        } else if (protocol == "UDP") {
            sendUdpCommand(command);
        } else {
            std::cout << "Invalid protocol\n";
        }
    }
    return 0;
}
