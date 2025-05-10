# start OR1
# Compiler and compiler flags

CC = g++
CFLAGS = -g -Wall -std=c++11

# Dependencies
DEPS = main.cpp token.h scanner.h scanner.cpp parser.h parser.cpp node.h node.cpp symbol.h symbolTable.h symbolTable.cpp codeGenerator.h codeGenerator.cpp

# Object files
OBJ = main.o scanner.o parser.o node.o symbolTable.o codeGenerator.o

# Create objects from source files
%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Create executable from objects
P4: $(OBJ)
	$(CC) $^ -o $@
# end OR1
