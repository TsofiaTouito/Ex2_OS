# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall

# Output executables
SERVER = server
CLIENT = client

# Source files
SERVER_SRC = server.cpp
CLIENT_SRC = client.cpp

# Object files
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)

# Default target
all: $(SERVER) $(CLIENT)

# Build the server executable
$(SERVER): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $(SERVER) $(SERVER_OBJ)

# Build the client executable
$(CLIENT): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) -o $(CLIENT) $(CLIENT_OBJ)

# Compile server source to object file
$(SERVER_OBJ): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -c $(SERVER_SRC)

# Compile client source to object file
$(CLIENT_OBJ): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -c $(CLIENT_SRC)

# Clean up compiled files
clean:
	rm -f $(SERVER) $(CLIENT) $(SERVER_OBJ) $(CLIENT_OBJ)

# Phony targets
.PHONY: all clean
