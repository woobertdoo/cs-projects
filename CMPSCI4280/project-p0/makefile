# start OR1
# Compiler and compiler flags

CC = g++
CFLAGS = -g -Wall -std=c++11

# Dependencies
DEPS = main.cpp node.h node.cpp buildTree.h buildTree.cpp traversal.h traversal.cpp

# Object files
OBJ = main.o node.o buildTree.o traversal.o

# Create objects from source files
%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Create executable from objects
P0: $(OBJ)
	$(CC) $^ -o $@
# end OR1
