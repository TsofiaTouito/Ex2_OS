#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>

// Sends a TCP command to a server
void sendTcpCommand(const std::string& command) {
    const char* SERVER_IP = "127.0.0.1";  // Server address
    const int PORT = 8080;  // Port for TCP connection

    // Create TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "TCP Socket creation failed\n";
        return;
    }

    // Set up server address
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address and connect to server
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address\n";
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "TCP Connection failed\n";
        close(sock);
        return;
    }

    // Send command to server
    std::string message = command + "\r\n";
    send(sock, message.c_str(), message.size(), 0);

    // Receive server response
    char buffer[1024] = {0};
    read(sock, buffer, sizeof(buffer));
    std::cout << "Server response: " << buffer << std::endl;

    // Close the socket
    close(sock);
}

// Sends a UDP command to a server
void sendUdpCommand(const std::string& command) {
    const char* SERVER_IP = "127.0.0.1";  // Server address
    const int PORT = 8081;  // Port for UDP connection

    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "UDP Socket creation failed\n";
        return;
    }

    // Set up server address
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    // Send command to server
    std::string message = command + "\r\n";
    sendto(sock, message.c_str(), message.size(), 0, (const struct sockaddr*)&serv_addr, sizeof(serv_addr));

    // Receive server response
    char buffer[1024] = {0};
    socklen_t len = sizeof(serv_addr);
    recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&serv_addr, &len);
    std::cout << "Server response: " << buffer << std::endl;

    // Close the socket
    close(sock);
}

// Main function to handle user input and send commands
int main() {
    while (true) {
        std::string protocol, command;

        // Prompt user for protocol (TCP or UDP)
        std::cout << "Enter protocol (TCP/UDP): ";
        std::cin >> protocol;
        std::cin.ignore();  // Ignore remaining newline from previous input

        // Prompt user for command
        std::cout << "Enter command: ";
        std::getline(std::cin, command);

        // Send command based on selected protocol
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
