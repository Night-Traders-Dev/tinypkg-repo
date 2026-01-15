CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c99 -D_POSIX_C_SOURCE=200809L -fPIE -fPIC
CFLAGS += -D_FORTIFY_SOURCE=2 -fstack-protector-strong -Wformat-security
CFLAGS += -Iinclude

LDFLAGS := -lm -lyaml

# Source files
SOURCES := src/main.c src/common.c src/repo.c src/build.c src/util.c

# Object files (compiled to build directory)
OBJECTS := $(SOURCES:src/%.c=build/%.o)

# Header files (for dependency tracking)
HEADERS := include/common.h include/repo.h include/build.h include/util.h include/config.h

TARGET := tinypkg
PREFIX := $(HOME)/.local
BUILD_DIR := build

all: $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link objects into target binary
$(TARGET): $(BUILD_DIR) $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

# Compile C source files to object files
build/%.o: src/%.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

distclean: clean
	rm -rf $(BUILD_DIR)
	rm -rf $(HOME)/.cache/tinypkg
	rm -rf $(HOME)/.local/bin/tinypkg

install: $(TARGET)
	mkdir -p $(PREFIX)/bin
	cp $(TARGET) $(PREFIX)/bin/
	chmod 755 $(PREFIX)/bin/$(TARGET)
	@echo "Installed to $(PREFIX)/bin/$(TARGET)"
	@echo "Add '$(PREFIX)/bin' to your PATH if needed"

uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)

help:
	@echo "TinyPkg Build Targets:"
	@echo "  make           - Build tinypkg binary"
	@echo "  make clean     - Remove object files and binary"
	@echo "  make distclean - Remove build artifacts, cache, and installation"
	@echo "  make install   - Install to ~/.local/bin/"
	@echo "  make uninstall - Remove installation"
	@echo ""
	@echo "Build directory: $(BUILD_DIR)/"
	@echo "Target binary:   $(TARGET)"

.PHONY: all clean distclean install uninstall help
