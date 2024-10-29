# Define default target
all: add2PB findPhone

# Compile add2PB
add2PB: add2PB.o
	g++ add2PB.o -o add2PB

# Compile findPhone
findPhone: findPhone.o
	g++ findPhone.o -o findPhone

# Compile add2PB.cpp to add2PB.o
add2PB.o: add2PB.cpp
	g++ -c add2PB.cpp -o add2PB.o

# Compile findPhone.cpp to findPhone.o
findPhone.o: findPhone.cpp
	g++ -c findPhone.cpp -o findPhone.o

# Clean up build artifacts
clean:
	rm -f add2PB findPhone add2PB.o findPhone.o

