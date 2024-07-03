# Compiler flags
GCC = gcc
CFLAGS = -Wall -lm 

# Target Dependencies
mathWork: mathWork.o 
	$(GCC) $(CFLAGS) mathWork.o -o mathWork

mathWork.o: mathWork.c
	$(GCC) $(CFLAGS) -c mathWork.c
