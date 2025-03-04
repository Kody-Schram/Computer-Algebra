# Makefile for compiling all .c files

# Define the compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra

# Automatically find all .c files in the current directory
SRC = $(wildcard *.c) $(wildcard parsing/*.c)

# Define the object files (generated from source files)
OBJ = $(SRC:.c=.o)

# Define the final output executable
EXEC = main

# Default rule to compile the executable
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

# Rule to compile .c files into .o object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the generated files
clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: clean
