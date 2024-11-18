#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <unistd.h>
#include <netinet/in.h>
#include <vector>
#include <sys/select.h>

const unsigned long long MAX_ATOMS = 1000000000000000000;  // Change to unsigned int

unsigned long long carbon_atoms = 0;
unsigned long long oxygen_atoms = 0;
unsigned long long hydrogen_atoms = 0;

bool parseCommand(const std::string& command, std::string& atom, unsigned int& count) {
    std::istringstream iss(command);
    std::string add;
    if (!(iss >> add >> atom >> count) || add != "ADD") {
        return false;
    }
    if (atom != "CARBON" && atom != "OXYGEN" && atom != "HYDROGEN") {
        return false;
    }
    return true;
}

std::string processCommand(const std::string& command) {
    std::string atom;
    unsigned int count;

    if (!parseCommand(command, atom, count)) {
        std::cout << "Invalid command received: " << command << std::endl;
        return "invalid command";
    }

    if (atom == "CARBON") {
        if (carbon_atoms + count > MAX_ATOMS) {
            std::cout << "Too many CARBON atoms. Current: " << carbon_atoms << ", Attempted to add: " << count << std::endl;
            return "carbon atoms limit exceeded";
        }
        carbon_atoms += count;
        std::cout << "Added " << count << " CARBON atoms. Total: " << carbon_atoms << std::endl;
    } else if (atom == "OXYGEN") {
        if (oxygen_atoms + count > MAX_ATOMS) {
            std::cout << "Too many OXYGEN atoms. Current: " << oxygen_atoms << ", Attempted to add: " << count << std::endl;
            return "oxygen atoms limit exceeded";
        }
        oxygen_atoms += count;
        std::cout << "Added " << count << " OXYGEN atoms. Total: " << oxygen_atoms << std::endl;
    } else if (atom == "HYDROGEN") {
        if (hydrogen_atoms + count > MAX_ATOMS) {
            std::cout << "Too many HYDROGEN atoms. Current: " << hydrogen_atoms << ", Attempted to add: " << count << std::endl;
            return "hydrogen atoms limit exceeded";
        }
        hydrogen_atoms += count;
        std::cout << "Added " << count << " HYDROGEN atoms. Total: " << hydrogen_atoms << std::endl;
    }

    return "added " + std::to_string(count) + " " + atom + " atoms";
}


int main() {
    const int PORT = 8080;
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
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

    std::cout << "Server is listening on port " << PORT << std::endl;

    fd_set read_fds;
    int max_fd = server_fd;
    std::vector<int> client_sockets;

    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);

        for (int client : client_sockets) {
            FD_SET(client, &read_fds);
            if (client > max_fd) {
                max_fd = client;
            }
        }

        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
        if (activity < 0) {
            perror("select error");
            break;
        }

        if (FD_ISSET(server_fd, &read_fds)) {
            int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
            if (new_socket < 0) {
                perror("accept");
                continue;
            }
            std::cout << "New connection accepted" << std::endl;
            client_sockets.push_back(new_socket);
        }

        for (auto it = client_sockets.begin(); it != client_sockets.end();) {
            int client_socket = *it;
            if (FD_ISSET(client_socket, &read_fds)) {
                char buffer[1024] = {0};
                int valread = read(client_socket, buffer, sizeof(buffer));
                if (valread <= 0) {
                    // Client disconnected
                    std::cout << "Client disconnected" << std::endl;
                    close(client_socket);
                    it = client_sockets.erase(it); // Remove from the list
                    continue;
                }

                std::string command(buffer);


                if (command.size() >= 2 && command.substr(command.size() - 2) == "\r\n") {
                    command = command.substr(0, command.size() - 2);
                }
                std::cout << "Received command: " << command << std::endl;

                std::string response = processCommand(command);
                send(client_socket, response.c_str(), response.size(), 0);
            }
            ++it;
        }
    }

    close(server_fd);
    return 0;
}
