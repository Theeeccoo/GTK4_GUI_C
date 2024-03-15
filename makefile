# all: contrib src main


# contrib: array.c
# 	gcc -o $@ $< 

# main: main.c 
# 	gcc -o $@ $< `pkg-config --cflags --libs gtk4` -lm -lX11 -lXrandr


# Directories
SRCDIR = $(CURDIR)/src
CONTRIBDIR = $(CURDIR)/contrib
BINDIR = $(CURDIR)/bin

INCLUDEDIR = $(CURDIR)/include
MYLIBDIR = $(INCLUDEDIR)/mylib

# Variables
CC = gcc
# CFLAGS = -Wall -Wextra -Werror -I$(INCLUDEDIR) -I$(MYLIBDIR) `pkg-config --cflags --libs gtk4` -lm -lX11 -lXrandr
CFLAGS = -I$(INCLUDEDIR) -I$(MYLIBDIR) `pkg-config --cflags --libs gtk4` -lm -lX11 -lXrandr
LDFLAGS = `pkg-config --cflags --libs gtk4` -lm -lX11 -lXrandr

# Source Files
SRC = $(wildcard $(SRCDIR)/*.c) 
CONTRIB_SRC = $(wildcard $(CONTRIBDIR)/*.c)
OBJ_SRC = $(SRC:$(SRCDIR)/%.c=$(BINDIR)/%.o) 
OBJ_CONTRIB = $(CONTRIB_SRC:$(CONTRIBDIR)/%.c=$(BINDIR)/%.o)

# Main Target
TARGET = main



# Phony Targets
.PHONY: all clean

all: $(BINDIR)/$(TARGET)

# Linkage
$(BINDIR)/$(TARGET): $(OBJ_CONTRIB) $(OBJ_SRC)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compiling
$(BINDIR)/%.o: $(SRCDIR)/%.c | $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR)/%.o: $(CONTRIBDIR)/%.c | $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Creating Binary Directory
$(BINDIR): 
	mkdir -p $@

# Cleaning
clean:
	rm -rf $(BINDIR)
