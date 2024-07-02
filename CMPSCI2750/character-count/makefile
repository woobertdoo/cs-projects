# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall

# Dependencies
DEPS = main.c countCharacters.c countCharacters.h caseConversion.c caseConversion.h

# Object files
OBJ = main.o countCharacters.o caseConversion.o

# Create objects from source files
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Create executable from objects
project4: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
