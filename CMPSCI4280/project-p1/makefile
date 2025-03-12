# start OR1
# Compiler and compiler flags

CC = g++
CFLAGS = -g -Wall -std=c++11

# Dependencies
DEPS = main.cpp token.h scanner.h scanner.cpp testScanner.h testScanner.cpp

# Object files
OBJ = main.o scanner.o testScanner.o

# Create objects from source files
%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Create executable from objects
P1: $(OBJ)
	$(CC) $^ -o $@
# end OR1
