# start OR1
# Compiler and compiler flags

CC = g++
CFLAGS = -g -Wall -std=c++11

# Dependencies
DEPS = main.cpp token.h scanner.h scanner.cpp parser.h parser.cpp node.h node.cpp symbol.h symbolTable.h symbolTable.cpp

# Object files
OBJ = main.o scanner.o parser.o node.o symbolTable.o

# Create objects from source files
%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Create executable from objects
P3: $(OBJ)
	$(CC) $^ -o $@
# end OR1
