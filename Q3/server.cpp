#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mutex>
#include <map>
#include <vector>

// Maximum allowable atom count
const long long MAX_ATOMS = 1000000000000000000LL;

// Global atom counts
long long carbon_atoms = 0;
long long oxygen_atoms = 0;
long long hydrogen_atoms = 0;

// Mutex to ensure thread-safe access to atom counts
std::mutex atom_lock;

// Molecule structure to define required atom counts for each type
struct Molecule {
    int carbon;
    int hydrogen;
    int oxygen;
};

// Predefined list of molecules and their atom requirements
const std::map<std::string, Molecule> molecules = {
    {"WATER", {0, 2, 1}},
    {"CARBON DIOXIDE", {1, 0, 2}},
    {"GLUCOSE", {6, 12, 6}},
    {"ALCOHOL", {2, 6, 1}},
    {"SOFT DRINK", {7, 14, 9}},
    {"VODKA", {8, 20, 8}},
    {"CHAMPAGNE", {3, 8, 4}}
};

// Function to handle keyboard input commands
// Commands are entered directly in the terminal and processed
bool processKeyboardCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string gen;

    // Validate that the command starts with "GEN"
    if (!(iss >> gen) || gen != "GEN") {
        std::cout << "ERROR: Invalid command!" << std::endl;
        return false;
    }

    // Parse molecule name and quantity
    std::string rest;
    std::getline(iss, rest);
    rest.erase(0, rest.find_first_not_of(" \t")); // Trim leading spaces
    rest.erase(rest.find_last_not_of(" \t") + 1); // Trim trailing spaces

    size_t last_space = rest.find_last_of(" ");
    std::string drink;
    long long quantity = 1; // Default quantity is 1

    if (last_space != std::string::npos && std::isdigit(rest[last_space + 1])) {
        drink = rest.substr(0, last_space); // Extract molecule name
        quantity = std::stoll(rest.substr(last_space + 1)); // Extract quantity
    } else {
        drink = rest;
    }

    // Verify that the molecule is valid
    auto it = molecules.find(drink);
    if (it == molecules.end()) {
        std::cout << "ERROR: Invalid drink!" << std::endl;
        return false;
    }

    const Molecule& molecule = it->second;

    // Lock for thread-safe operations
    std::lock_guard<std::mutex> guard(atom_lock);

    long long max_molecules = std::min(
        carbon_atoms / molecule.carbon,
        std::min(hydrogen_atoms / molecule.hydrogen,
                 oxygen_atoms / molecule.oxygen)
    );

    // Check if sufficient atoms are available
    if (carbon_atoms <= 0 || hydrogen_atoms <= 0 || oxygen_atoms <= 0 || max_molecules <= 0) {
        std::cout << "ERROR: Not enough atoms to generate the molecules!" << std::endl;
        return false;
    }

    // Deduct atoms for one molecule
    carbon_atoms -= molecule.carbon;
    hydrogen_atoms -= molecule.hydrogen;
    oxygen_atoms -= molecule.oxygen;

    // Log success
    std::cout << "Generated " << drink << std::endl;
    std::cout << "Remaining atoms: Carbon = " << carbon_atoms
              << ", Hydrogen = " << hydrogen_atoms
              << ", Oxygen = " << oxygen_atoms << std::endl;
    std::cout << "You can generate " << max_molecules - 1 << " more " << drink << std::endl;

    // Print how many of each molecule can still be generated
    for (const auto& mol : molecules) {
        const Molecule& m = mol.second;
        long long max_possible = std::min(
            carbon_atoms / m.carbon,
            std::min(hydrogen_atoms / m.hydrogen,
                     oxygen_atoms / m.oxygen)
        );
        std::cout << "You can generate " << max_possible << " more " << mol.first << std::endl;
    }

    return true;
}

// Function to process atom addition commands
std::string processAtomCommand(const std::string& command) {
    std::string atom;
    long long count;
    std::istringstream iss(command);
    std::string add;

    // Validate that the command starts with "ADD"
    if (!(iss >> add >> atom >> count) || add != "ADD") {
        return "invalid command";
    }

    // Validate atom type
    if (atom != "CARBON" && atom != "OXYGEN" && atom != "HYDROGEN") {
        return "invalid command";
    }

    // Lock for thread-safe addition
    std::lock_guard<std::mutex> guard(atom_lock);

    // Add atoms to the appropriate counter
    if (atom == "CARBON") {
        if (carbon_atoms + count > MAX_ATOMS) return "error: carbon atoms limit exceeded";
        carbon_atoms += count;
        std::cout << "Added " << count << " Carbon" << std::endl;
    } else if (atom == "OXYGEN") {
        if (oxygen_atoms + count > MAX_ATOMS) return "error: oxygen atoms limit exceeded";
        oxygen_atoms += count;
        std::cout << "Added " << count << " Oxygen" << std::endl;
    } else if (atom == "HYDROGEN") {
        if (hydrogen_atoms + count > MAX_ATOMS) return "error: hydrogen atoms limit exceeded";
        hydrogen_atoms += count;
        std::cout << "Added " << count << " Hydrogen" << std::endl;
    }

    // Log remaining atom counts
    std::cout << "Remaining atoms: Carbon = " << carbon_atoms
              << ", Hydrogen = " << hydrogen_atoms
              << ", Oxygen = " << oxygen_atoms << std::endl;

    return "OK\r\n";
}

// Function to process molecule delivery commands
std::string processMoleculeCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string deliver;

    // Validate that the command starts with "DELIVER"
    if (!(iss >> deliver) || deliver != "DELIVER") {
        return "ERROR\r\n";
    }

    // Parse molecule name and quantity
    std::string rest;
    std::getline(iss, rest);
    rest.erase(0, rest.find_first_not_of(" \t")); // Trim leading spaces
    rest.erase(rest.find_last_not_of(" \t") + 1); // Trim trailing spaces

    size_t last_space = rest.find_last_of(" ");
    std::string molecule;
    long long count = 1;

    if (last_space != std::string::npos && std::isdigit(rest[last_space + 1])) {
        molecule = rest.substr(0, last_space);
        count = std::stoll(rest.substr(last_space + 1));
    } else {
        molecule = rest;
    }

    auto it = molecules.find(molecule);
    if (it == molecules.end()) return "ERROR\r\n";

    const Molecule& mol = it->second;

    // Lock for thread-safe operations
    std::lock_guard<std::mutex> guard(atom_lock);
    long long required_c = mol.carbon * count;
    long long required_h = mol.hydrogen * count;
    long long required_o = mol.oxygen * count;

    if (carbon_atoms >= required_c && hydrogen_atoms >= required_h && oxygen_atoms >= required_o) {
        carbon_atoms -= required_c;
        hydrogen_atoms -= required_h;
        oxygen_atoms -= required_o;
        std::cout << "Delivered " << count << " " << molecule << std::endl;

        // Log remaining atom counts
        std::cout << "Remaining atoms: Carbon = " << carbon_atoms
                  << ", Hydrogen = " << hydrogen_atoms
                  << ", Oxygen = " << oxygen_atoms << std::endl;

        return "OK\r\n";
    }

    // Log failure
    std::cout << "Failed to deliver " << count << " " << molecule << std::endl;
    return "ERROR\r\n";
}

// TCP server to handle client connections
void tcpServer(int port) {
    // Server initialization and setup
    int server_fd, new_socket;
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
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "TCP server listening on port " << port << std::endl;

    // Accept incoming client connections
    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        std::cout << "New client connected via TCP" << std::endl;

        // Handle client in a separate thread
        std::thread([new_socket]() {
            char buffer[1024];
            while (true) {
                memset(buffer, 0, sizeof(buffer));
                int valread = read(new_socket, buffer, sizeof(buffer));
                if (valread <= 0) break;

                std::string command(buffer);
                command.erase(command.find_last_not_of("\r\n") + 1);
                std::cout << "TCP command received: " << command << std::endl;

                std::string response = processAtomCommand(command);
                std::cout << "Response: " << response << std::endl;
                send(new_socket, response.c_str(), response.size(), 0);
            }
            close(new_socket);
        }).detach();
    }
}

// UDP server to handle client requests
void udpServer(int port) {
    int sockfd;
    char buffer[1024];
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "UDP server listening on port " << port << std::endl;

    // Listen for client requests
    while (true) {
        socklen_t len = sizeof(cliaddr);
        memset(buffer, 0, sizeof(buffer));
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cliaddr, &len);
        if (n > 0) {
            std::string command(buffer);
            command.erase(command.find_last_not_of("\r\n") + 1); // Remove trailing CRLF
            std::cout << "UDP command received: " << command << std::endl;

            std::string response = processMoleculeCommand(command);
            std::cout << "Response: " << response << std::endl; // Log the response
            sendto(sockfd, response.c_str(), response.size(), 0, (const struct sockaddr*)&cliaddr, len);
        }
    }
}

// Main function
int main() {
    // Start TCP and UDP servers in separate threads
    std::thread udp_thread(udpServer, 8081);
    std::thread tcp_thread(tcpServer, 8080);

    // Process keyboard commands from the terminal
    while (true) {
        std::string command;
        std::getline(std::cin, command);
        processKeyboardCommand(command);
    }

    udp_thread.join();
    tcp_thread.join();
    return 0;
}
