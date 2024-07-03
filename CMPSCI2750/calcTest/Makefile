#Compiler and flags
GCC = gcc
CFLAGS = -Wall

# List of dependencies
DEPS = mathOperation.h mathOperation.c test.h test.c doMath.c

# List of object files
OBJ = mathOperation.o test.o doMath.o

# Rule for creating object files
%.o: %.c $(DEPS)
	$(GCC) $(CFLAGS) -c $< -o $@

# Rule for creating executable
doMath: $(OBJ)
	$(GCC) $(CFLAGS) $^ -o $@


