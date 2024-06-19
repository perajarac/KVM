# Variables
CC = gcc
CXX = g++
CFLAGS = -Wall -g
CXXFLAGS = -Wall -g

# Targets
all: mini_hypervisor

mini_hypervisor: mini_hypervisor.o CLIParser.o
	$(CXX) $(CXXFLAGS) mini_hypervisor.o CLIParser.o -o mini_hypervisor -lstdc++

mini_hypervisor.o: mini_hypervisor.cpp
	$(CXX) $(CXXFLAGS) -c mini_hypervisor.cpp -o mini_hypervisor.o

CLIParser.o: CLIParser.cpp
	$(CXX) $(CXXFLAGS) -c CLIParser.cpp -o CLIParser.o

clean:
	rm -f mini_hypervisor mini_hypervisor.o CLIParser.o
