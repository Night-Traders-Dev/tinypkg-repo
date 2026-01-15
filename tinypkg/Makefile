# Makefile for tinypkg - Lightweight package manager for embedded systems
# 
# Targets:
#   make               Build tinypkg binary
#   make test          Run test suite
#   make install       Install tinypkg
#   make clean         Clean build artifacts
#   make valgrind      Run with valgrind memory check
#   make docs          Generate documentation

# ============================================================================
# Configuration
# ============================================================================

CC ?= gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -fPIC
LDFLAGS = -lm
INSTALL_PREFIX ?= /usr/local

# Debug/Release build
DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CFLAGS += -g -O0 -DDEBUG
    LDFLAGS += -g
else
    CFLAGS += -O2 -DNDEBUG
endif



# ============================================================================
# Source Files
# ============================================================================

SOURCES = \
    src/main.c \
    src/util.c \
    src/repo.c \
    src/build.c 

HEADERS = \
    src/util.h \
    src/repo.h \
    src/build.h \

OBJECTS = $(SOURCES:.c=.o)
TARGET = tinypkg

# ============================================================================
# Targets
# ============================================================================

.PHONY: all test clean install uninstall valgrind docs help

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "Linking $@..."
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

%.o: %.c $(HEADERS)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================================
# Installation
# ============================================================================

install: $(TARGET)
	@echo "Installing tinypkg to $(INSTALL_PREFIX)..."
	mkdir -p $(INSTALL_PREFIX)/bin
	install -m 0755 $(TARGET) $(INSTALL_PREFIX)/bin/$(TARGET)
	mkdir -p $(INSTALL_PREFIX)/etc/tinypkg
	@echo "Installation complete"
	@echo "Configuration template available at $(INSTALL_PREFIX)/etc/tinypkg/"

uninstall:
	@echo "Uninstalling tinypkg..."
	rm -f $(INSTALL_PREFIX)/bin/$(TARGET)
	@echo "Uninstallation complete"

# ============================================================================
# Documentation
# ============================================================================

docs:
	@echo "Generating documentation..."
	doxygen Doxyfile
	@echo "Documentation generated in docs/"

# ============================================================================
# Cleanup
# ============================================================================

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(OBJECTS) $(TARGET)
	rm -rf docs/
	@echo "Clean complete"

distclean: clean
	@echo "Removing all generated files..."
	rm -f *.o src/*.o
	@echo "Distribution clean complete"

# ============================================================================
# Code Quality
# ============================================================================

lint:
	@echo "Running code quality checks..."
	cppcheck --enable=all --suppress=missingIncludeSystem src/
	@echo "Lint complete"

format:
	@echo "Formatting code..."
	clang-format -i src/*.c src/*.h
	@echo "Format complete"

# ============================================================================
# Development Targets
# ============================================================================

rebuild: clean all

verbose:
	$(MAKE) DEBUG=1 CFLAGS="$(CFLAGS) -v"

static: $(TARGET)
	@echo "Building static library..."
	ar rcs libtinypkg.a $(OBJECTS)
	@echo "Static library: libtinypkg.a"

# ============================================================================
# Help
# ============================================================================

help:
	@echo "tinypkg - Lightweight package manager for embedded systems"
	@echo ""
	@echo "Available targets:"
	@echo "  make               Build tinypkg binary"
	@echo "  make test          Run test suite"
	@echo "  make install       Install to $(INSTALL_PREFIX)"
	@echo "  make uninstall     Uninstall from $(INSTALL_PREFIX)"
	@echo "  make clean         Remove build artifacts"
	@echo "  make distclean     Remove all generated files"
	@echo "  make valgrind      Run with memory checking"
	@echo "  make docs          Generate API documentation"
	@echo "  make lint          Run code quality checks"
	@echo "  make format        Format source code"
	@echo "  make rebuild       Clean and rebuild"
	@echo "  make static        Build static library"
	@echo "  make help          Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  CC               C compiler (default: gcc)"
	@echo "  CFLAGS           C compiler flags"
	@echo "  DEBUG            Build with debug symbols (default: 1)"
	@echo "  INSTALL_PREFIX   Installation prefix (default: /usr/local)"
	@echo ""
	@echo "Examples:"
	@echo "  make DEBUG=0                    Build release version"
	@echo "  make INSTALL_PREFIX=$$HOME      Install to home directory"
	@echo "  make install INSTALL_PREFIX=$$ make install INSTALL_PREFIX=$$HOME"

.PHONY: help rebuild verbose static lint format
