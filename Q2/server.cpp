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

const long long MAX_ATOMS = 1000000000000000000LL;
long long carbon_atoms = 0;
long long oxygen_atoms = 0;
long long hydrogen_atoms = 0;

std::mutex atom_lock;

struct Molecule {
    int carbon;
    int hydrogen;
    int oxygen;
};

const std::map<std::string, Molecule> molecules = {
    {"WATER", {0, 2, 1}},
    {"CARBON DIOXIDE", {1, 0, 2}},
    {"GLUCOSE", {6, 12, 6}},
    {"ALCOHOL", {2, 6, 1}}
};

// Parse atom addition commands
bool parseAtomCommand(const std::string& command, std::string& atom, long long& count) {
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

// Process atom addition
std::string processAtomCommand(const std::string& command) {
    std::string atom;
    long long count;

    if (!parseAtomCommand(command, atom, count)) {
        return "invalid command";
    }

    std::lock_guard<std::mutex> guard(atom_lock);
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
    return "OK\r\n";
}

// Process molecule delivery
std::string processMoleculeCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string deliver, molecule;
    unsigned int count;
    if (!(iss >> deliver >> molecule >> count) || deliver != "DELIVER") {
        return "ERROR\r\n";
    }

    auto it = molecules.find(molecule);
    if (it == molecules.end()) return "ERROR\r\n";

    const Molecule& mol = it->second;

    std::lock_guard<std::mutex> guard(atom_lock);
    long long required_c = mol.carbon * count;
    long long required_h = mol.hydrogen * count;
    long long required_o = mol.oxygen * count;

    if (carbon_atoms >= required_c && hydrogen_atoms >= required_h && oxygen_atoms >= required_o) {
        carbon_atoms -= required_c;
        hydrogen_atoms -= required_h;
        oxygen_atoms -= required_o;
        std::cout << "Delivered " << count << " " << molecule << std::endl;
        return "OK\r\n";
    }

    std::cout << "Failed to deliver " << count << " " << molecule << std::endl;
    return "ERROR\r\n";
}

// TCP Server
void tcpServer(int port) {
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

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        std::cout << "New client connected via TCP" << std::endl;

        std::thread([new_socket]() {
            char buffer[1024];
            while (true) {
                memset(buffer, 0, sizeof(buffer));
                int valread = read(new_socket, buffer, sizeof(buffer));
                if (valread <= 0) break;

                std::string command(buffer);
                command.erase(command.find_last_not_of("\r\n") + 1); // Remove trailing CRLF
                std::cout << "TCP command received: " << command << std::endl;

                std::string response = processAtomCommand(command);
                std::cout << "Response: " << response << std::endl; // Log the response
                send(new_socket, response.c_str(), response.size(), 0);
            }
            close(new_socket);
        }).detach();
    }
}

// UDP Server
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

int main() {
    std::thread tcp_thread(tcpServer, 8080);
    std::thread udp_thread(udpServer, 8081);

    tcp_thread.join();
    udp_thread.join();

    return 0;
}
