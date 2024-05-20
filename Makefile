# Make mcc

# Compiler and compiler flags
CC := gcc
CFLAGS := -Ofast -nostdlib -m64 -std=c2x -Wall
INC := ./src/dep/inc

# Source directories
SRCDIR := ./src
OBJDIR := ./bin/obj
BIN := ./bin/bin
MANDIR := ./man

# Files
LICENSE := ./LICENSE
README := ./README.md

# Package directories
PKGDIR := ./bin/pkg
PKGLIBDIR := ./lib

# Out directories
OUT := ./bin/

# Source files
SRCS := $(shell find $(SRCDIR) -name '*.c')
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

# Executable
EXE := ./bin/bin/mcc

# Default target
all: clean dirs $(EXE) package

# Create directories
dirs:
	@mkdir -p $(OBJDIR) $(PKGDIR) $(OUT) $(BIN)

# Compile rule
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)  # Create the directory if it doesn't exist
	@echo "Compiling $< to $@"
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

# Link rule
$(EXE): $(OBJS)
	@echo "Linking objects to $@"
	$(CC) -o $@ $^ -lc -lgcc

# Package rule
package:
	@echo "Making package in $(PKGDIR)"
	@cp $(EXE) $(PKGDIR)
	@echo "Adding libs..."
	@cp -r $(PKGLIBDIR) $(PKGDIR)
	@echo "Adding man files..."
	@cp -r $(MANDIR) $(PKGDIR)
	@cp $(LICENSE) $(PKGDIR)
	@cp $(README) $(PKGDIR)
	@echo "Package created successfully."

# Clean rule
clean:
	@echo "Cleaning in $(OUT)"
	rm -rf $(OUT)

.PHONY: all clean package dirs
