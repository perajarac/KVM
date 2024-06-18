# Variables
CC = gcc
CXX = g++
CFLAGS = -Wall -g
CXXFLAGS = -Wall -g

# Targets
all: mini_hypervisor MyClassWrapper

mini_hypervisor: mini_hypervisor.c
	$(CC) $(CFLAGS) mini_hypervisor.c -o mini_hypervisor

MyClassWrapper: MyClassWrapper.o
	$(CXX) $(CXXFLAGS) MyClassWrapper.o -o MyClassWrapper

MyClassWrapper.o: MyClassWrapper.cpp
	$(CXX) $(CXXFLAGS) -c MyClassWrapper.cpp -o MyClassWrapper.o

clean:
	rm -f mini_hypervisor MyClassWrapper MyClassWrapper.o
