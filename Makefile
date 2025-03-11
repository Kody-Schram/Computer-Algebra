# Makefile for compiling all .c files

# Define the compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra

# Builds for normal execution
SRC = $(wildcard *.c) \
	  $(wildcard parsing/*.c) \
	  $(wildcard parsing/codegen/*.c)
OBJ = $(SRC:.c=.o)
EXEC = main

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

# Builds for testing
TEST_SRC = $(wildcard *.c) \
	  	   $(wildcard parsing/*.c) \
	       $(wildcard parsing/codegen/*.c)
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_EXEC = test

$(TEST_EXEC): $(TEST_OBJ)
	$(CC) $(TEST_OBJ) -o $(TEST_EXEC)

# Rule to compile .c files into .o object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the generated files
clean:
	rm -f $(OBJ) $(EXEC) $(TEST_OBJ) $(TEST_EXEC)

.PHONY: clean
