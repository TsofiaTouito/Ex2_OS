#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <unistd.h>
#include <netinet/in.h>
#include <vector>
#include <sys/select.h>

// Constants and Global Variables
const unsigned long long MAX_ATOMS = 1000000000000000000;  // Maximum allowed atoms for each element (change to unsigned int)
unsigned long long carbon_atoms = 0;  // Counter for CARBON atoms
unsigned long long oxygen_atoms = 0;  // Counter for OXYGEN atoms
unsigned long long hydrogen_atoms = 0;  // Counter for HYDROGEN atoms

/**
 * Parse the command string to extract atom type and count.
 * @param command The input command string.
 * @param atom The type of atom to be added (e.g., "CARBON", "OXYGEN", "HYDROGEN").
 * @param count The number of atoms to add.
 * @return True if the command is valid, false otherwise.
 */
bool parseCommand(const std::string& command, std::string& atom, unsigned int& count) {
    std::istringstream iss(command);
    std::string add;
    if (!(iss >> add >> atom >> count) || add != "ADD") {
        return false;  // Invalid command or wrong format
    }
    if (atom != "CARBON" && atom != "OXYGEN" && atom != "HYDROGEN") {
        return false;  // Invalid atom type
    }
    return true;
}

/**
 * Process the command to add atoms and check if the total does not exceed the limit.
 * @param command The input command string.
 * @return A response string indicating the result of the command.
 */
std::string processCommand(const std::string& command) {
    std::string atom;
    unsigned int count;

    // Parse the command and check its validity
    if (!parseCommand(command, atom, count)) {
        std::cout << "Invalid command received: " << command << std::endl;
        return "invalid command";  // Invalid command format
    }

    // Handle atom addition and check for overflow
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

    // Return success message
    return "added " + std::to_string(count) + " " + atom + " atoms";
}

int main() {
    // Network initialization
    const int PORT = 8080;  // Port to listen on
    int server_fd;  // Server socket file descriptor
    struct sockaddr_in address;  // Socket address structure
    int opt = 1;  // Option to reuse address
    int addrlen = sizeof(address);  // Length of address structure

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind socket to address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port " << PORT << std::endl;

    fd_set read_fds;  // File descriptor set for select()
    int max_fd = server_fd;  // Maximum file descriptor
    std::vector<int> client_sockets;  // List of client sockets

    while (true) {
        FD_ZERO(&read_fds);  // Clear file descriptor set
        FD_SET(server_fd, &read_fds);  // Add server socket to the set

        // Add all client sockets to the set
        for (int client : client_sockets) {
            FD_SET(client, &read_fds);
            if (client > max_fd) {
                max_fd = client;
            }
        }

        // Wait for activity on sockets
        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
        if (activity < 0) {
            perror("select error");
            break;
        }

        // Check if there's an incoming connection request
        if (FD_ISSET(server_fd, &read_fds)) {
            int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
            if (new_socket < 0) {
                perror("accept");
                continue;
            }
            std::cout << "New connection accepted" << std::endl;
            client_sockets.push_back(new_socket);  // Add new client to the list
        }

        // Check for incoming data from clients
        for (auto it = client_sockets.begin(); it != client_sockets.end();) {
            int client_socket = *it;
            if (FD_ISSET(client_socket, &read_fds)) {
                char buffer[1024] = {0};
                int valread = read(client_socket, buffer, sizeof(buffer));
                if (valread <= 0) {
                    // Client disconnected
                    std::cout << "Client disconnected" << std::endl;
                    close(client_socket);
                    it = client_sockets.erase(it);  // Remove client from the list
                    continue;
                }

                std::string command(buffer);

                // Remove possible newline characters from the command
                if (command.size() >= 2 && command.substr(command.size() - 2) == "\r\n") {
                    command = command.substr(0, command.size() - 2);
                }
                std::cout << "Received command: " << command << std::endl;

                // Process the command and send the response back to the client
                std::string response = processCommand(command);
                send(client_socket, response.c_str(), response.size(), 0);
            }
            ++it;
        }
    }

    close(server_fd);  // Close the server socket
    return 0;
}
