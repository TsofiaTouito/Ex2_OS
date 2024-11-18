#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>

void sendTcpCommand(const std::string& command) {
    const char* SERVER_IP = "127.0.0.1";
    const int PORT = 8080;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "TCP Socket creation failed\n";
        return;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

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

    std::string message = command + "\r\n";
    send(sock, message.c_str(), message.size(), 0);
    char buffer[1024] = {0};
    read(sock, buffer, sizeof(buffer));
    std::cout << "Server response: " << buffer << std::endl;
    close(sock);
}

void sendUdpCommand(const std::string& command) {
    const char* SERVER_IP = "127.0.0.1";
    const int PORT = 8081;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "UDP Socket creation failed\n";
        return;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    std::string message = command + "\r\n";
    sendto(sock, message.c_str(), message.size(), 0, (const struct sockaddr*)&serv_addr, sizeof(serv_addr));

    char buffer[1024] = {0};
    socklen_t len = sizeof(serv_addr);
    recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&serv_addr, &len);
    std::cout << "Server response: " << buffer << std::endl;

    close(sock);
}

int main() {
    while (true) {
        std::string protocol, command;
        std::cout << "Enter protocol (TCP/UDP): ";
        std::cin >> protocol;
        std::cin.ignore();
        std::cout << "Enter command: ";
        std::getline(std::cin, command);

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
