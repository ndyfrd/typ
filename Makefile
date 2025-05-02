# A simple Makefile for compiling small projects

# set the compiler
CC := gcc

# set the compiler flags
CFLAGS := --std=c99 -Wall 

# add header files here
HDRS := 

# add source files here
SRCS := typ.c

# generate names of object files
OBJS := $(SRCS:.c=.o)

# name of executable
EXEC := ~/bin/typ

# default recipe
all: $(EXEC)

# recipe for building the final executable
$(EXEC): $(OBJS) $(HDRS) Makefile
	$(CC) -o $@ $(OBJS) $(CFLAGS)

# recipe for building object files
#$(OBJS): $(@:.o=.c) $(HDRS) Makefile
#	$(CC) -o $@ $(@:.o=.c) -c $(CFLAGS)

# recipe to clean the workspace
clean:
	rm -f $(EXEC) $(OBJS)

.PHONY: all clean
